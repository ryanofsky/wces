<?

require_once("wbes/server.inc");
require_once("legacy/widgets/widgets.inc");

function legacy_array_point($key,&$thearray) // puts the internal array pointer to the specified key, if there is an equivalent php function somewhere I couldn't find it
{
	reset($thearray);
	while (list($k) = each($thearray))
	if ($k == $key)
	{
		$k = key($thearray);
		if (isset($k)) prev($thearray); else end($thearray);
		return true;
	}
	reset($thearray);
	return false;
}

class legacy_Wizard extends legacy_FormWidget
{
  var $steps;     // ordered array of strings, contains names of wizard steps
  var $cancelurl; // string, target url when wizard is cancelled
  var $visited;   // 0 based array of booleans, the step has been previously displayed;
  var $showstep;  // current step
  var $finished;  // boolean, user clicked finish button
  var $sated;     // enough data has been entered, finished button is clickable
  
  var $hidesteps = array();

  function legacy_Wizard($steps,$cancelurl,$prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->steps = $steps;
    $this->cancelurl = $cancelurl;
    $this->finished = false;
    $this->sated = false;
    $this->visited = Array();
    $this->showstep = 0;
    $this->dumpscript();
  }
  
  function drawstep($stepno,$hidden) // abstract virtual public
  {
    assert(0);
  }
  
  function isfinished()
  {
    return $this->finished;
  }
  
  function crawl($skipsteps = array())
  {
    if ($this->loadattribute("back"))
      $move = -1;
    else if ($this->loadattribute("next"))
      $move = 1;
    else 
      $move = 0;
    
    legacy_array_point($this->showstep,$this->steps);
    $this->showstep = key($this->steps);
    if ($move)
    do
    {
      if ($move < 0) prev($this->steps); else next($this->steps);
      $oldkey = $this->showstep;
      $this->showstep = key($this->steps);
      if(!isset($this->showstep)) {$this->showstep = $oldkey; break; }
    }	
    while(isset($skipsteps[$this->showstep]) && $skipsteps[$this->showstep]);
  }
  
  function loadvalues($crawl = true)
  {
    $this->showstep = $this->loadattribute("showstep");
    $this->finished = $this->loadattribute("finish") ? true : false;
    
    /* Safer finish only works from the last page, otherwise jumps to the last page...
    if ($this->finished)
    { 
      end($this->steps);
      $last = key($this->steps);
      if ($this->showstep != $last)
      {
        $this->finished = false;
        $this->showstep = $last;
      };  
    }  
    */
    
    $this->visited = unserialize((string)$this->loadattribute("visited"));
    if (!is_array($this->visited)) $this->visited = array();
    $this->visited = array_pad($this->visited, count($this->steps),false);
    $this->visited[$this->showstep] = true;  
    
    if ($crawl) $this->crawl();
  }

  function display($hidden)
  {
    global $wces_path;
   
    $visited = $this->visited; //extract
    $showstep = $this->showstep;

    $this->printattribute("visited",serialize($visited));
    $this->printattribute("showstep",$showstep);
    if ($hidden) $this->printattribute("finish",$this->isfinished() ? 1 : 0);
    
if (!$hidden)
{
?>

<table align=center border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td width=3 height=3><img src="<?=$wces_path?>media/wizard/b1.gif" width=3 height=3></td>
    <td height=3 background="<?=$wces_path?>media/wizard/b2.gif"><img src="<?=$wces_path?>media/wizard/b2.gif" width=1 height=3></td>
    <td width=3 height=3><img src="<?=$wces_path?>media/wizard/b3.gif" width=3 height=3></td>
  </tr>
  <tr>
    <td width=3 background="<?=$wces_path?>media/wizard/b4.gif"><img src="<?=$wces_path?>media/wizard/b4.gif" width=3 height=1></td>
    <td>
      <table border=0 cellspacing=0 cellpadding=0>
        <tr>
          <td height=18 colspan=4 bgcolor="#08286B" background="<?=$wces_path?>media/wizard/title.jpg">
            <a onmousedown="button_push('<?=$this->prefix?>_wizardx')" onmouseup="button_unpush('<?=$this->prefix?>_wizardx')"   onmouseout="button_unpush('<?=$this->prefix?>_wizardx')" href="<?=$this->cancelurl?>"><img name="<?=$this->prefix?>_wizardx" src="<?=$wces_path?>media/wizard/close.gif" border=0 height=18 width=20 align=right></a><img src="<?=$wces_path?>media/nada.gif" width=2 height=1><font face="System" color="#FFFFFF">Reporting Wizard</font>
            <script>AttachPushed('<?=$this->prefix?>_wizardx','wizardclosep.gif')</script>
          </td>
        </tr>
        <tr><td colspan=4 bgcolor="#D6D3CE" width=506 height=1><img src="<?=$wces_path?>media/nada.gif" width="1" height="1"></td></tr>
        <tr><td bgcolor="#000000" background="<?=$wces_path?>media/0x000000.gif" width=160 height=8><img src="<?=$wces_path?>media/nada.gif" width=160 height=8></td><td colspan=3 bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" width=340 height=8><img src="<?=$wces_path?>media/nada.gif" width=340 height=8></td></tr>
        <tr>
          <td bgcolor="#000000" background="<?=$wces_path?>media/0x000000.gif" width=160 height=296>
<table border=0 cellpadding=0 cellspacing=0>
<?
reset($this->steps);
$first = key($this->steps);
end($this->steps);
$last = key($this->steps);

foreach($this->steps as $key => $value)
{
  if (isset($this->hidesteps[$key]) && $this->hidesteps[$key]) continue;
  $href = "javascript:document.forms['" . $this->form . "']['" . $this->prefix . "_showstep'].value = $key; void(document.forms['" . $this->form . "'].submit())";
  $suffix = ($key == $showstep ? "a" : ($visited[$key] ? "v" : ""));
  if ($key == $first)
  {?><tr><td><a href="<?=$href?>"><img src="<?=$wces_path?>media/wizard/node_top<?=$suffix?>.gif" width=40 height=16 border=0></a></td><td><a href="<?=$href?>"><font face=verdana size="-1" color="#FFFFFF"><?=$value?></font></a></td></tr><tr><td><img src="<?=$wces_path?>media/wizard/node_connect.gif" width=40 height=16></td></tr><?="\n"?><?}
  else if ($key == $last)
  {?><tr><td><a href="<?=$href?>"><img src="<?=$wces_path?>media/wizard/node_bot<?=$suffix?>.gif" width=40 height=16 border=0></a></td><td><a href="<?=$href?>"><font face=verdana size="-1" color="#FFFFFF"><?=$value?></font></a></td></tr><?="\n"?><?}
  else
  {?><tr><td background="<?=$wces_path?>media/wizard/node_connect.gif"><a href="<?=$href?>"><img src="<?=$wces_path?>media/wizard/node_mid<?=$suffix?>.gif" width=40 height=16 border=0></a></td><td><a href="<?=$href?>"><img src="<?=$wces_path?>media/nada.gif" width=8 height=16 align=middle border=0><font face=verdana size="-1" color="#FFFFFF"><?=$value?></font></a></td></tr> <tr><td background="<?=$wces_path?>media/wizard/node_connect.gif"><img src="<?=$wces_path?>media/wizard/node_connect.gif" width=40 height=16></td><td>&nbsp;</td></tr><?="\n"?><?}
};
?>
</table>
          </td>
          <td bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" width=8 valign=top><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
          <td bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" width=324 height=296 valign=top>
<?
}
foreach($this->steps as $key => $value)
{
  $this->drawstep($key,$hidden || ($key != $showstep));
}  
if (!$hidden)
{
?>
          </td>
          <td bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" width=8 valign=top><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
        </tr>
        <tr>
          <td colspan=4 bgcolor="#000000" background="<?=$wces_path?>media/0x000000.gif" width=506 height=5><img src="<?=$wces_path?>media/nada.gif" width="1" height="5"></td>
        </tr>
        <tr>
          <td colspan=4 bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" height=40>
             <table align=right border=0 cellspacing=0 cellpadding=0>
               <tr>
                 <td><input type=button name="<?=$this->prefix?>_cancel" onclick="window.location='<?=$this->cancelurl?>'"style="font-family: Verdana; width:64 px" value="Cancel"></td>
                 <td><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
                 <td><input type=submit name="<?=$this->prefix?>_back"style="font-family: Verdana; width:64 px" value="&lt; Back"<?=$showstep == 0 ? " disabled" : ""?>></td>
                 <td><input type=submit name="<?=$this->prefix?>_next" style="font-family: Verdana; width:64 px" value="Next &gt;"<?=$showstep == $last ? " disabled" : ""?>></td>
                 <td><img src="<?=$wces_path?>media/nada.gif" width=16 height=1></td>
                 <td><input type=submit name="<?=$this->prefix?>_finish" style="font-family: Verdana; width:64 px" value="Finish"></td>
                 <td><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
               <tr>
             </table>
          </td>
        </tr>
      </table>
    </td>
    <td width=3 background="<?=$wces_path?>media/wizard/b6.gif"><img src="<?=$wces_path?>media/wizard/b6.gif" width=3 height=1></td>
  </tr>
  <tr>
    <td width=3 height=3><img src="<?=$wces_path?>media/wizard/b7.gif" width=3 height=3></td>
    <td height=3 background="<?=$wces_path?>media/wizard/b8.gif"><img src="<?=$wces_path?>media/wizard/b8.gif" width=1 height=3></td>
    <td width=3 height=3><img src="<?=$wces_path?>media/wizard/b9.gif" width=3 height=3></td>
  </tr>
</table>    
<?  
}
  }

  function dumpscript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;
    legacy_ImageButton::dumpscript();
    $str = <<<EOS
<script>
<!--

function get(id)
{
  if (document.getElementById)
    return document.getElementById(id);
  else if (document.all)
    return document.all.item(id);
  else return null;    
};

function show(id)
{
  var o = get(id);
  if (o && o.style) o.style.display = "block";
};

function hide(id)
{
  var o = get(id);
  if (o && o.style) o.style.display = "none";
};

//-->
</script>
EOS;
    headPrint($str);  
  }
}
?>
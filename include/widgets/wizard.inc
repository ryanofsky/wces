<?

require_once("wbes/server.inc");
require_once("widgets/widgets.inc");

function array_point($key,&$thearray) // puts the internal array pointer to the specified key, if there is an equivalent php function somewhere I couldn't find it
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

class Wizard extends ParentWidget
{
  var $steps;     // ordered array of strings, contains names of wizard steps
  var $cancelUrl; // string, target url when wizard is cancelled
  var $visited;   // 0 based array of booleans, the step has been previously displayed;
  var $showStep;  // current step
  var $done;      // boolean, user clicked finish button
  var $sated;     // enough data has been entered, done button is clickable

  var $hidesteps = array();

  function Wizard($steps, $cancelUrl, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->steps = $steps;
    $this->cancelUrl = $cancelUrl;
    $this->done = false;
    $this->sated = false;
    $this->visited = array();
    $this->showStep = 0;
    $this->dumpscript();
  }

  function printStep($stepno, $hidden, $hiddenChildren)
  {
    // printStep is called once for each step in the wizard in the display()
    // and displayHidden() methods.
    //
    //   $stepno contains the step number to display
    //
    //   $visible is a boolean. When it is true, step $stepno's controls and 
    //     text should be displayed. When it is false, hidden fields needed
    //     to preserve the state of the step should be printed. Even when visible
    //     is false, the displayHidden() methods on the step's children widgets
    //     should not be called unless $hiddenChildren is true.
    //    
    //   $hiddenChildren is a boolean that is only true when $visible is false. 
    //     When it is true the printStep implementation should call the 
    //     displayHidden() methods on any children widgets associated with the
    //     step $stepno.
  }

  function crawl($skipsteps = array())
  {
    if ($this->readValue("back"))
      $move = -1;
    else if ($this->readValue("next"))
      $move = 1;
    else
      $move = 0;

    array_point($this->showStep,$this->steps);
    $this->showStep = key($this->steps);
    if ($move)
    do
    {
      if ($move < 0) prev($this->steps); else next($this->steps);
      $oldkey = $this->showStep;
      $this->showStep = key($this->steps);
      if(!isset($this->showStep)) {$this->showStep = $oldkey; break; }
    }
    while(isset($skipsteps[$this->showStep]) && $skipsteps[$this->showStep]);
  }

  function loadState($crawl = true)
  {
    ParentWidget::loadState();
    $this->showStep = $this->readValue("showStep");
    $this->done = (bool)$this->readValue("done");

    /* Safer finish only works from the last page, otherwise jumps to the last page...
    if ($this->done)
    {
      end($this->steps);
      $last = key($this->steps);
      if ($this->showStep != $last)
      {
        $this->done = false;
        $this->showStep = $last;
      };
    }
    */

    $this->visited = unserialize((string)$this->readValue("visited"));
    if (!is_array($this->visited)) $this->visited = array();
    $this->visited = array_pad($this->visited, count($this->steps),false);
    $this->visited[$this->showStep] = true;

    if ($crawl) $this->crawl();
  }

  function printState()
  {
    $this->printValue("visited", serialize($this->visited));
    $this->printValue("showStep", $this->showStep);
  }
  
  function printHidden()
  {
    $this->printValue("done", (int)$this->done);
    foreach($this->steps as $key => $value)
      $this->printStep($key, false, false);
  }

  function printVisible()
  {
    global $wces_path;

    $visited = $this->visited; //extract
    $showStep = $this->showStep;

    $wizardx = $this->fullName("wizardx");
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
        <a onmousedown="button_push('<?=$wizardx?>')" onmouseup="button_unpush('<?=$wizardx?>')" onmouseout="button_unpush('<?=$wizardx?>')" href="<?=$this->cancelUrl?>"><img name="<?=$wizardx?>" src="<?=$wces_path?>media/wizard/close.gif" border=0 height=18 width=20 align=right></a><img src="<?=$wces_path?>media/nada.gif" width=2 height=1><font face="System" color="#FFFFFF">Reporting Wizard</font>
        <script>AttachPushed('<?=$wizardx?>','wizardclosep.gif')</script>
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
      $showStepName = $this->fullName('showStep');
      if (isset($this->hidesteps[$key]) && $this->hidesteps[$key]) continue;
      $href = "javascript:document.forms['{$this->form->formName}']['$showStepName'].value = $key; void(document.forms['" . $this->form->formName . "'].submit())";
      $suffix = ($key == $showStep ? "a" : (isset($visited[$key]) && $visited[$key] ? "v" : ""));
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
    foreach($this->steps as $key => $value)
    {
      $hidden = $key != $showStep;
      $this->printStep($key, !$hidden, $hidden);
    }

    $cancelName = $this->fullName('cancel');
    $backName = $this->fullName('back');
    $nextName = $this->fullName('next');
    $doneName = $this->fullName('done');
?>
      </td>
      <td bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" width=8 valign=top><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
    </tr>
    <tr><td colspan=4 bgcolor="#000000" background="<?=$wces_path?>media/0x000000.gif" width=506 height=5><img src="<?=$wces_path?>media/nada.gif" width="1" height="5"></td></tr>
    <tr>
      <td colspan=4 bgcolor="#D6D3CE" background="<?=$wces_path?>media/0xD6D3CE.gif" height=40>
        <table align=right border=0 cellspacing=0 cellpadding=0>
        <tr>
          <td><input type=button name="<?=$cancelName?>" onclick="window.location='<?=$this->cancelUrl?>'"style="font-family: Verdana; width:64 px" value="Cancel"></td>
          <td><img src="<?=$wces_path?>media/nada.gif" width=8 height=1></td>
          <td><input type=submit name="<?=$backName?>"style="font-family: Verdana; width:64 px" value="&lt; Back"<?=$showStep == 0 ? " disabled" : ""?>></td>
          <td><input type=submit name="<?=$nextName?>" style="font-family: Verdana; width:64 px" value="Next &gt;"<?=$showStep == $last ? " disabled" : ""?>></td>
          <td><img src="<?=$wces_path?>media/nada.gif" width=16 height=1></td>
          <td><input type=submit name="<?=$doneName?>" style="font-family: Verdana; width:64 px" value="Finish"></td>
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

  function DumpScript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;
    ImageButton::dumpscript();
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
    html_head_append($str);
  }
}
?>
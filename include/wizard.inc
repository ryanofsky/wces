<%
require_once("server.inc");
require_once("widgets.inc");

class Wizard extends Widget
{
  var $steps;     // 0 based array of strings, contains names of wizard steps
  var $cancelurl; // string, target url when wizard is cancelled
  var $visited;   // 0 based array of booleans, the step has been previously displayed;
  var $showstep;  // current step
  var $finished;  // boolean, user clicked finish button
  var $sated;     // enough data has been entered, finished button is clickable
  var $skip;      // 0 based array of booleans, true if the step is to be skipped
  
  function Wizard($steps,$cancelurl,$prefix,$form,$formmethod)
  {
    $this->Widget($prefix,$form,$formmethod);
    $this->steps = $steps;
    $this->cancelurl = $cancelurl;
    $this->finished = false;
    $this->sated = false;
    $this->skip = array_pad(array(),count($steps),false);
    $this->visited = Array();
    $this->showstep = 0;
  }
  
  function drawstep($stepno,$hidden) // abstract virtual public
  {
    assert(0);
  }
  
  function isfinished()
  {
    return $this->finished;
  }
  
  function loadvalues()
  {
    $this->showstep = $this->loadattribute("showstep");
    $this->finished = $this->loadattribute("finish") ? true : false;
    
    $move = 0;
    if ($this->finished)
    { 
      $last = count($this->steps) -1;
      if ($this->showstep != $last)
      {
        $this->finished = false;
        $this->showstep = $last;
      };  
    }  
    if ($this->loadattribute("back"))
      $move = -1;
    else if ($this->loadattribute("next"))
      $move = 1;
    
    $laststep = 0;
    reset ($this->skip);
    if ($move)
    while (list ($key, $val) = each ($this->skip))
    {
      if ($key == $this->showstep)
      {
        if ($move < 0)
          { $this->showstep = $laststep; break; }
        else if ($move > 0)
        {
          while(list($key, $val) = each($this->skip))
            if (!$val) { $this->showstep = $key; break; }
          break;  
        };      
      }
      if (!$val) $laststep = $key;
    };
    
    $this->showstep = min($this->showstep, count($this->steps) - 1);
    $this->showstep = max($this->showstep,0);
    $this->visited = unserialize($this->loadattribute("visited"));
    if (!is_array($this->visited)) $this->visited = array();
    $this->visited = array_pad($this->visited, count($this->steps),false);
    $this->visited[$this->showstep] = true;  
  }

  function display($hidden)
  {
    global $server_wcespath;
    $this->dumpscript();
    
    $visited = $this->visited; //extract
    $showstep = $this->showstep;

    $this->printattribute("visited",serialize($visited));
    $this->printattribute("showstep",$showstep);
    if ($hidden) $this->printattribute("finish",$this->isfinished() ? 1 : 0);
    
if (!$hidden)
{
%>

<table align=center border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td width=3 height=3><img src="<%=$server_wcespath%>media/wizardb1.gif" width=3 height=3></td>
    <td height=3 background="<%=$server_wcespath%>media/wizardb2.gif"><img src="<%=$server_wcespath%>media/wizardb2.gif" width=1 height=3></td>
    <td width=3 height=3><img src="<%=$server_wcespath%>media/wizardb3.gif" width=3 height=3></td>
  </tr>
  <tr>
    <td width=3 background="<%=$server_wcespath%>media/wizardb4.gif"><img src="<%=$server_wcespath%>media/wizardb4.gif" width=3 height=1></td>
    <td>
      <table border=0 cellspacing=0 cellpadding=0>
        <tr>
          <td height=18 colspan=4 bgcolor="#08286B" background="<%=$server_wcespath%>media/wizardtitle.jpg">
            <a onmousedown="button_push('<%=$this->prefix%>_wizardx')" onmouseup="button_unpush('<%=$this->prefix%>_wizardx')"   onmouseout="button_unpush('<%=$this->prefix%>_wizardx')" href="<%=$this->cancelurl%>"><img name="<%=$this->prefix%>_wizardx" src="<%=$server_wcespath%>media/wizardclose.gif" border=0 height=18 width=20 align=right></a><img src="<%=$server_wcespath%>media/nada.gif" width=2 height=1><font face="System" color="#FFFFFF">Reporting Wizard</font>
            <script>AttachPushed('<%=$this->prefix%>_wizardx','wizardclosep.gif')</script>
          </td>
        </tr>
        <tr><td colspan=4 bgcolor="#D6D3CE" width=506 height=1><img src="<%=$server_wcespath%>media/nada.gif" width="1" height="1"></td></tr>
        <tr><td bgcolor="#000000" background="<%=$server_wcespath%>media/0x000000.gif" width=160 height=8><img src="<%=$server_wcespath%>media/nada.gif" width=160 height=8></td><td colspan=3 bgcolor="#D6D3CE" background="<%=$server_wcespath%>media/0xD6D3CE.gif" width=340 height=8><img src="<%=$server_wcespath%>media/nada.gif" width=340 height=8></td></tr>
        <tr>
          <td bgcolor="#000000" background="<%=$server_wcespath%>media/0x000000.gif" width=160 height=296>
<table border=0 cellpadding=0 cellspacing=0>
<%
foreach($this->steps as $key => $value)
{
  $href = "javascript:document.forms['" . $this->form . "']['" . $this->prefix . "_showstep'].value = $key; void(document.forms['" . $this->form . "'].submit())";
  $suffix = ($key == $showstep ? "a" : ($visited[$key] ? "v" : ""));
  if ($key == 0)
  {%><tr><td><a href="<%=$href%>"><img src="<%=$server_wcespath%>media/wizardnode_top<%=$suffix%>.gif" width=40 height=16 border=0></a></td><td><a href="<%=$href%>"><font face=verdana size="-1" color="#FFFFFF"><%=$value%></font></a></td></tr><tr><td><img src="<%=$server_wcespath%>media/wizardnode_connect.gif" width=40 height=16></td></tr><%="\n"%><%}
  else if ($key == count($this->steps) - 1)
  {%><tr><td><a href="<%=$href%>"><img src="<%=$server_wcespath%>media/wizardnode_bot<%=$suffix%>.gif" width=40 height=16 border=0></a></td><td><a href="<%=$href%>"><font face=verdana size="-1" color="#FFFFFF"><%=$value%></font></a></td></tr><%="\n"%><%}
  else
  {%><tr><td background="<%=$server_wcespath%>media/wizardnode_connect.gif"><a href="<%=$href%>"><img src="<%=$server_wcespath%>media/wizardnode_mid<%=$suffix%>.gif" width=40 height=16 border=0></a></td><td><a href="<%=$href%>"><img src="<%=$server_wcespath%>media/nada.gif" width=8 height=16 align=middle border=0><font face=verdana size="-1" color="#FFFFFF"><%=$value%></font></a></td></tr> <tr><td background="<%=$server_wcespath%>media/wizardnode_connect.gif"><img src="<%=$server_wcespath%>media/wizardnode_connect.gif" width=40 height=16></td><td>&nbsp;</td></tr><%="\n"%><%}
};
%>
</table>
          </td>
          <td bgcolor="#D6D3CE" background="<%=$server_wcespath%>media/0xD6D3CE.gif" width=8 valign=top><img src="<%=$server_wcespath%>media/nada.gif" width=8 height=1></td>
          <td bgcolor="#D6D3CE" background="<%=$server_wcespath%>media/0xD6D3CE.gif" width=324 height=296 valign=top>
<%
}
foreach($this->steps as $key => $value)
{
  $this->drawstep($key,$hidden || ($key != $showstep));
}  
if (!$hidden)
{
%>
          </td>
          <td bgcolor="#D6D3CE" background="<%=$server_wcespath%>media/0xD6D3CE.gif" width=8 valign=top><img src="<%=$server_wcespath%>media/nada.gif" width=8 height=1></td>
        </tr>
        <tr>
          <td colspan=4 bgcolor="#000000" background="<%=$server_wcespath%>media/0x000000.gif" width=506 height=5><img src="<%=$server_wcespath%>media/nada.gif" width="1" height="5"></td>
        </tr>
        <tr>
          <td colspan=4 bgcolor="#D6D3CE" background="<%=$server_wcespath%>media/0xD6D3CE.gif" height=40>
             <table align=right border=0 cellspacing=0 cellpadding=0>
               <tr>
                 <td><input type=button name="<%=$this->prefix%>_cancel" onclick="window.location='<%=$this->cancelurl%>'"style="font-family: Verdana; width:64 px" value="Cancel"></td>
                 <td><img src="<%=$server_wcespath%>media/nada.gif" width=8 height=1></td>
                 <td><input type=submit name="<%=$this->prefix%>_back"style="font-family: Verdana; width:64 px" value="&lt; Back"<%=$showstep == 0 ? " disabled" : ""%>></td>
                 <td><input type=submit name="<%=$this->prefix%>_next" style="font-family: Verdana; width:64 px" value="Next &gt;"<%=$showstep >= count($this->steps) - 1 ? " disabled" : ""%>></td>
                 <td><img src="<%=$server_wcespath%>media/nada.gif" width=16 height=1></td>
                 <td><input type=submit name="<%=$this->prefix%>_finish" style="font-family: Verdana; width:64 px" value="Finish"></td>
                 <td><img src="<%=$server_wcespath%>media/nada.gif" width=8 height=1></td>
               <tr>
             </table>
          </td>
        </tr>
      </table>
    </td>
    <td width=3 background="<%=$server_wcespath%>media/wizardb6.gif"><img src="<%=$server_wcespath%>media/wizardb6.gif" width=3 height=1></td>
  </tr>
  <tr>
    <td width=3 height=3><img src="<%=$server_wcespath%>media/wizardb7.gif" width=3 height=3></td>
    <td height=3 background="<%=$server_wcespath%>media/wizardb8.gif"><img src="<%=$server_wcespath%>media/wizardb8.gif" width=1 height=3></td>
    <td width=3 height=3><img src="<%=$server_wcespath%>media/wizardb9.gif" width=3 height=3></td>
  </tr>
</table>    
<%  
}
  }
  
  
  function dumpscript()
  {
    global $Wizard_dumped;
    if (!$Wizard_dumped)
    {
      $Wizard_dumped = true;
      ImageButton::dumpscript();
%>
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
<%      
    }
  }



}



%>

<%

function taskwindow_start($title)
{
  global $taskwindow_inuse;
  $taskwindow_inuse = true;
%>
<script>
  function _p(str) { document.write(str); _t.write(str) };
  
  var _t = window.open("", "task", "width=300,height=400,channelmode=0,dependent=0,directories=0,fullscreen=0,location=0,menubar=0,resizable=1,scrollbars=1,status=1,toolbar=0").document;
  _t.open(); //("text/html", "replace");
  _t.write('<title><%=$title%></title>');
  _t.write('<center><form><input type=button value="Close This Window" onclick="window.close()"></form></center>');
</script>
<%
}

function taskwindow_end()
{
  global $taskwindow_inuse;
  taskwindow_flush();
  if($taskwindow_inuse)
  {
    %><script>_t.close();</script><%  
  };  
}

$taskwindow_cache = "";
$taskwindow_inuse = false;

function taskwindow_print($str)
{
  global $taskwindow_inuse, $taskwindow_cache;
  if ($taskwindow_inuse)
  {
    if ($taskwindow_cache) taskwindow_flush();
    %><script>_p("<%=addcslashes($str,"\\\"\n\r")%>");</script><%  
  }
  else
    print($str);
}

function taskwindow_cprint($str)
{
  global $taskwindow_inuse, $taskwindow_cache;
  if ($taskwindow_inuse)
    $taskwindow_cache .= $str;
  else
  {
    taskwindow_flush();
    print($str);  
  }; 
};

function taskwindow_flush()
{
  global $taskwindow_inuse, $taskwindow_cache;
  if ($taskwindow_cache != "")
  {  
    if ($taskwindow_inuse)
    {
      print ("<script>\n");
      $l = strlen($taskwindow_cache);
      for ($i = 0; $i < $l; $i += 240)
      {
        echo "_p(\"" , addcslashes(substr($taskwindow_cache,$i,240),"\\\"\n\r") , "\");\n";
      }
      echo "_p(\"" , addcslashes(substr($taskwindow_cache,$i),"\\\"\n\r") , "\");\n";
      print("</script>\n");
    }
    else
      print($taskwindow_cache);
    $taskwindow_cache = "";
  };  
};
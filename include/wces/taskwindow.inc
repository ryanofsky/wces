<%

function taskwindow_start($title, $localtoo = true)
{
  global $taskwindow_inuse, $taskwindow_localtoo;
  $taskwindow_inuse = true;
  $taskwindow_localtoo = $localtoo ? ", true" : "";
%>
<script>
  function pr(str, localtoo) { if(localtoo) document.write(str); if (window.task) window.taskd.write(str) };
  window.task = window.open("", "task", "width=400,height=400,channelmode=0,dependent=0,directories=0,fullscreen=0,location=0,menubar=0,resizable=1,scrollbars=1,status=1,toolbar=0");
  window.taskd = window.task.document
  window.taskd.open(); //("text/html", "replace");
  window.task.isok = false;
  if (!window.task.opener) window.task.opener = window;
  window.taskd.write('<html><head><title><%=$title%></title></head>');
  window.taskd.write('<body><center><form><input type=button value="Close This Window" onclick="if (window.isok) window.close(); else alert(\'Please wait for the task to complete\');"></form></center>');
</script>
<%
}

function taskwindow_end()
{
  global $taskwindow_inuse;
  taskwindow_flush();
  if($taskwindow_inuse)
  {
    %><script>window.taskd.write('</body></html>'); window.taskd.close(); window.task.isok = true;</script><%  
  };  
}

$taskwindow_cache = "";
$taskwindow_inuse = false;

function taskwindow_print($str)
{
  global $taskwindow_inuse, $taskwindow_cache, $taskwindow_localtoo;
  if ($taskwindow_inuse)
  {
    if ($taskwindow_cache) taskwindow_flush();
    %><script>pr("<%=addcslashes($str,"\\\"\n\r")%>"<%=$taskwindow_localtoo%>);</script><%  
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
  global $taskwindow_inuse, $taskwindow_cache, $taskwindow_localtoo;
  if ($taskwindow_cache != "")
  {  
    if ($taskwindow_inuse)
    {
      print ("<script>\n");
      $l = strlen($taskwindow_cache);
      for ($i = 0; $i < $l; $i += 240)
      {
        echo "pr(\"" , addcslashes(substr($taskwindow_cache,$i,240),"\\\"\n\r") , "\"${taskwindow_localtoo});\n";
      }
      echo "pr(\"" , addcslashes(substr($taskwindow_cache,$i),"\\\"\n\r") , "\"${taskwindow_localtoo});\n";
      print("</script>\n");
    }
    else
      print($taskwindow_cache);
    $taskwindow_cache = "";
  }; 
  flush(); 
};
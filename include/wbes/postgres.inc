<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/taskwindow.inc");

function pg_query($sql, $db, $file = __FILE__, $line = __LINE__)
{
  global $db_debug, $server_isproduction;
  
  if ($db_debug) $t = microtime();
  
  $result = pg_exec($db,$sql);
  if ($db_debug) $time = gettime(microtime()) - gettime($t);
  
  if ($db_debug || !$result)
  {
    taskwindow_cprint("<p>SQL Query in <b>$file</b>, line <b>$line</b><br>\n");
    taskwindow_cprint('<pre style="background-color:#DDDDDD; width:100%">' . $sql . "\n</pre><br>\n");
    taskwindow_cprint($result ? "<font color=blue>Success</font>" : "<font color=red>Failure</font>");
    if ($result)
    {
      $rows = pg_numrows($result);
      taskwindow_cprint("<br>Time: $time seconds<br>Affected Rows: $rows");
    }
    else
      taskwindow_cprint("<br>Error: " . pg_errormessage($db));
    taskwindow_cprint("</p>\n");
    if ($server_isproduction && !$result)
      sendwarning("Query failed.\nError message: " . pg_errormessage() . "\nQuery: $sql",$file,$line);
  }
  return $result;
}

?>
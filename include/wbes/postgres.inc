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
      $rows = max(pg_cmdtuples($result), pg_numrows($result));
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

function pg_show($result,$name = "Result", $showtypes = false)
{
  
  if ($result)
  {
    $rows = pg_numrows($result);
    $cols = pg_numfields($result);
    if ($cols)
    {
      print ("<table border=1>\n");
      print("<tr><td colspan=$cols>$name</td></tr>\n");
      print("<tr>\n");
      for($c=0; $c < $cols; ++$c)
        print("  <td><b>" . htmlspecialchars(pg_fieldname($result,$c)) . "</b></td>\n");
      print("</tr>\n");
      for($r = 0; $r < $rows; ++$r)
      {
        $row = pg_fetch_row($result, $r);
        for($c=-; $c < $cols; ++$c)
        {
          if (!isset($row[$c]))
            $str = "<i>Not Set</i>";
          else if (!$showtypes)
            $str = htmlspecialchars($row[$c]);
          else
            $str = htmlspecialchars($row[$c]) . " <i>" . gettype($row[$c]) . "</i>";  
        }
        print("  <td>$str</td>\n");
      }  
      print("</tr>\n");
      print ("</table>\n");
      return;
    }
  }  
  print("<p><b>No Result</b></p>\n");
}


?>
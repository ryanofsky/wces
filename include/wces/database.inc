<?

require_once("wbes/general.inc");
require_once("wbes/taskwindow.inc");
require_once("wces/database_selectmonster.inc");

$db_debug = false;

function db_set($items)
{
  $list = "(";
  $first = true;
  foreach($items as $item)
  {
    if ($first) $first = false; else $list .= ", ";
    $list .= "'" . addslashes($item) . "'";
  }
  $list .= ")";
  return $list;
}

function db_exec($sql, $db, $file = __FILE__, $line = __LINE__)
{
  global $db_debug, $server_isproduction;
  
  if ($db_debug) $t = microtime();
  $result = mysql_query($sql,$db);
  if ($db_debug) $time = gettime(microtime()) - gettime($t);
  
  if ($db_debug || (!$result && !$server_isproduction))
  {
    taskwindow_cprint("<p>SQL Query in <b>$file</b>, line <b>$line</b><br>\n");
    taskwindow_cprint('<pre style="background-color:#DDDDDD; width:100%">' . $sql . "\n</pre><br>\n");
    taskwindow_cprint($result ? "<font color=blue>Success</font>" : "<font color=red>Failure</font>");
    if ($result)
    {
      $rows = strcasecmp(substr(ltrim($result), 0, 6),"SELECT") == 0 ? mysql_num_rows($result) : mysql_affected_rows($db);
      taskwindow_cprint("<br>Time: $time seconds<br>Affected Rows: $rows");
    }
    else
      taskwindow_cprint("<br>Error " . mysql_errno() . ": " . mysql_error());
    taskwindow_cprint("</p>\n");
  }  
  else if (!$result)
    sendwarning("Query failed.\nError Code: " . mysql_errno() . "\nError message: " . mysql_error() . "\nQuery: $sql",$file,$line);
  return $result;
}

//-----------------------------------------------------------------------------
// General databasing functions. Slow and not very flexible, but used heavily
//  by import.inc to keep that code short and simple.


function db_addrow($db,$table,$values)
{
  global $db_debug;
  
  $query = "INSERT INTO $table (";
  $vals = " VALUES (";
  $first = true;
  foreach($values as $key => $value)
  {
    if ($first) $first = false; else { $query .= ", "; $vals .= ", "; }
    $query .= "$key";
    if ($value === false)
      $vals .= "NULL";
    else if (is_array($value))
      $vals .= db_set($value);
    else
      $vals .= "'" . addslashes($value) . "'";
  };
  $query = $query . ")" . $vals . ")";
  
  if (!db_exec($query,$db, __FILE__, __LINE__)) return false;
  $insertid = mysql_insert_id($db);
  return $insertid ? $insertid : true;
};

function db_whereclause($keys) //used internally
{
  if (is_array($keys) && count($keys) > 0)
  {
    $query = " WHERE ";
    $first = true;
    foreach($keys as $key => $value)
    {
      if ($first) $first = false; else $query .= " && ";
      if ($value === false)
        $query .= "$key IS NULL";
      else if ($value === true)
        $query .= "$key IS NOT NULL";
      else   
        $query .= "$key = '" . addslashes($value) . "'";
    }
    return $query;
  }
  else
    return "";  
}

function db_updatevalues($db,$table,$keys,$values)
{
  global $db_debug;

  if (is_array($values) && count($values) > 0)
  {
    $query = "UPDATE $table SET ";
    $first = true;
    foreach($values as $key => $value)
    {
      if ($first) $first = false; else $query .= ", ";
      $query .= "$key = ";
      if ($value === false)
        $query .= "NULL";
      else if (is_array($value))
        $query .= db_set($value);
      else
        $query .= "'" . addslashes($value) . "'";
    }
    $query .= db_whereclause($keys);
  
    return db_exec($query,$db,__FILE__,__LINE__);
  }
  else
  {
    if ($db_debug) taskwindow_cprint("<b>Error: Nothing to Update</b><br>");
    return false;
  }
    
};

function db_select($db,$table,$keys,$columns,$suffix = "") //used internally
{
  global $db_debug;

  if (!$columns) $columns = Array("*");

  $query = "SELECT ";
  $first = true;
  foreach($columns as $value)
  {
    if ($first) $first = false; else $query .= ", ";
    $query .= $value;
  }
  $query .= " FROM $table";
  $query .= db_whereclause($keys);
  $query .= " $suffix";
  
  return db_exec($query,$db,__FILE__,__LINE__);
};

function db_getrow($db,$table,$keys,$columns)
{
  global $db_debug;
  $result = db_select($db,$table,$keys,$columns);
  $rows = mysql_num_rows($result);
  if ($rows == 1)
    $r = mysql_fetch_assoc($result);
  else if ($rows > 1)
  {
    if ($db_debug)
    { 
      taskwindow_cprint ("<b>ERROR: DUPLICATE ROWS FOUND IN '$table'</b><br>");
      while ($j = mysql_fetch_assoc($result))
        db_printrow($j);
      flush();  
    }  
    $r = false;
  }
  else 
    $r = 0;
  mysql_free_result($result);
  return $r;
};

function db_getvalue($db,$table,$keys,$column)
{
  $row = db_getrow($db,$table,$keys,Array($column));
  if (is_array($row)) return $row[$column]; else return $row;
}

function db_getrows($db,$table,$keys,$columns)
{
  $result = db_select($db,$table,$keys,$columns);
  $rows = Array();
  while($row = mysql_fetch_assoc($result))
    array_push($rows,$row);
  mysql_free_result($result);
  return count($rows) == 0 ? 0 : $rows;
};

function db_getcolumn($db,$table,$keys,$column)
{
  $result = db_select($db,$table,$keys,Array($column));
  $col = Array();
  while($value = mysql_fetch_assoc($result))
    array_push($col,$value[$column]);
  mysql_free_result($result);
  return $col;
};

function db_replace($db, $table, $keys, $update, $idcol)
{
  $result = db_getvalue($db,$table,$keys,$idcol);
  if ($result)
  {
    if (is_array($update)) db_updatevalues($db, $table, Array($idcol => $result), $update);
    return $result;
  }
  else
  {
    if (is_array($update)) $keys = array_merge($keys,$update);
    return db_addrow($db,$table,$keys);
  }       
};

function db_replacenoid($db, $table, $keys, $update)
{
  $result = db_getrow($db,$table,$keys,0);
  if ($result)
  {
    if (is_array($update)) db_updatevalues($db, $table, $keys, $update);
    return $result;
  }
  else
  {
    if (is_array($update)) $keys = array_merge($keys,$update);
    return db_addrow($db,$table,$keys);
  }       
};

function db_delete($db,$table,$keys)
{
  global $db_debug;
  $query = "DELETE FROM $table" . db_whereclause($keys);
  return db_exec($query, $db, __FILE__, __LINE__);
};

function db_printrow($row)
{
  taskwindow_cprint("<table border=1>");
  foreach($row as $key => $value)
  {
    if (!($key === $key + 0))
      taskwindow_cprint("<tr><td>$key</td><td>$value</td></tr>");
  }
  taskwindow_cprint("</table>\n");
};

function db_printresult($result)
{
  if ($result)
  {
    taskwindow_cprint("<table border=1>\n<tr>\n");
    $n = mysql_num_fields($result);
    for($i=0; $i<$n; ++$i)
      taskwindow_cprint("  <td>" . mysql_field_name($result,$i) . "</td>\n");
    print("</tr>\n");   
 
    while ($row = mysql_fetch_row($result))
    {
      taskwindow_cprint("<tr>");
      for($i=0; $i<$n; ++$i)
        taskwindow_cprint("<td>" . $row[$i] . "</td>");
      taskwindow_cprint("</tr>\n"); 
    };
  }
  else  
  {
    taskwindow_cprint("<tr><td><b>Bad Result</b></td></tr>");
  }
  taskwindow_cprint("</table>");  
}

?>
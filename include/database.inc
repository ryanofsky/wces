<%

require_once("taskwindow.inc");

$db_debug = false;

function db_addrow($db,$table,$values)
{
  global $db_debug;
  
  if (is_array($values) && count($values) > 0)
  {
    $query = "INSERT INTO $table (";
    $vals = " VALUES (";
    $first = true;
    foreach($values as $key => $value)
    {
      if ($first) $first = false; else { $query .= ", "; $vals .= ", "; }
      $query .= "$key";
      $vals .= $value === false ? "NULL" : "'" . addslashes($value) . "'";
    };
    $query = $query . ")" . $vals . ")";
    
    if ($db_debug) taskwindow_cprint("$query<br>");
  
    if (!mysql_query($query,$db))
    {
      if ($db_debug) taskwindow_cprint("<b>Query Syntax Error</b><br>");
      return false;
    }
    return mysql_insert_id($db);
  }
  else
  {
    if ($db_debug) taskwindow_cprint("<b>Error: Nothing to Insert!</b><br>");
    return false;  
  };  
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
      $query .= "$key = " . ($value === false ? "NULL" : "'" . addslashes($value) . "'");
    }
    $query .= db_whereclause($keys);
  
    if ($db_debug) taskwindow_cprint("$query<br>");
    $result = mysql_query($query,$db);
    if (!$result && $db_debug) taskwindow_cprint("<b>SQL Syntax Error</b><br>");
    return $result;
  }
  else
  {
    if ($db_debug) taskwindow_cprint("<b>Error: Nothing to Update</b><br>");
    return false;
  }
    
};

function db_select($db,$table,$keys,$columns) //used internally
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
  
  if ($db_debug) taskwindow_cprint("$query<br>");
  
  $result = mysql_query($query,$db);
  if (!$result && $db_debug) taskwindow_cprint("<b>SQL Syntax Error</b><br>");
  return $result;
};

function db_getrow($db,$table,$keys,$columns)
{
  global $db_debug;
  $result = db_select($db,$table,$keys,$columns);
  $rows = mysql_num_rows($result);
  if ($rows == 1)
    $r = mysql_fetch_array($result);
  else if ($rows > 1)
  {
    if ($db_debug)
    { 
      print ("<b>ERROR: DUPLICATE ROWS FOUND IN '$table'</b><br>");
      while ($j = mysql_fetch_array($result))
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
  while($row = mysql_fetch_array($result))
    array_push($rows,$row);
  mysql_free_result($result);
  return count($rows) == 0 ? 0 : $rows;
};

function db_getcolumn($db,$table,$keys,$column)
{
  $result = db_select($db,$table,$keys,Array($column));
  $col = Array();
  while($value = mysql_fetch_array($result))
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
  if ($db_debug) taskwindow_cprint("$query<br>");
  $result = mysql_query($query,$db);
  if (!$result && $db_debug) taskwindow_cprint("<b>SQL Syntax Error</b><br>");
  return $result;
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
  print ("<table border=1>\n");
  if ($result)
  {
    
    while ($row = mysql_fetch_array($result))
    {
      print("<tr>");
      foreach($row as $key => $value)
        if (!($key === $key + 0)) print("<td>$key = $value</td>");
      print("</tr>\n"); 
    };
  }
  else  
  {
    print("<tr><td><b>Bad Result</b></td></tr>");
  }
  print("</table>");  
}

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
%>
<%

require_once("wces/taskwindow.inc");

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

function db_exec($sql,$db,$line = __LINE__,$file = __FILE__)
{
  global $db_debug;
  $result = mysql_query($sql,$db);
  if ($db_debug || !$result)
  {
    taskwindow_cprint("<p>SQL Query in <b>$file<b>, line <b>$line</b><br>\n");
    taskwindow_cprint('<pre style="background-color: #DDDDDD width:100%">' . $sql . '</pre><br>\n');
    taskwindow_cprint($result ? "<font color=blue>Success</font>" : "<font color=red>Failure</font>");
    taskwindow_cprint("</p>\n");
  }
  return $result;
}
//-----------------------------------------------------------------------------
// SelectMonster class.

/*

This class allows for the construction of certain types of sql SELECT queries.
It is used by the reporting code to avoid redundancy and to cut down on the 
excessive sql string manipulation that would otherwise be neccessary in 
constructing many complex and flexible queries. Specifically, the
SelectMonster class makes it easier to efficiently implement what I call
"Parent and Child" queries. 

... more documentation soon ...

These are the notes I used to keep things straight while I wrote the
method bodies:

  Parent vs. Child

  - different columns
  - same joins, conditions, where clauses
  - child grouping is superset of parent grouping
  - grouped columns are neccessarily ordered columns for the parent
  - the child's order by list is nearly identical to the parent's order by list,
    the only exception is that the child list can have some ordered columns
     of it's own FOLLOWING the parent's order by list.
  
  requirements:
  - children can also be parents
  - parents can have multiple children
  - children have only one parent

*/

define("db_SelectMonster_left",1);
define("db_SelectMonster_inner",2);
define("db_SelectMonster_prefix","SelectMonsterChild");

class db_SelectMonster
{
  var $columns, $where, $having, $roottable; // string literals
  var $tables, $jointypes, $leftcols, $rightcols; // arrays
  var $groupby, $orderby, $orderbydesc; // arrays
  
  var $result; // result of query
  var $row; // current row from result
  var $size; // number of rows in result

  var $childid;
  var $children;

  function db_SelectMonster()
  {
    $this->columns = "";
    $this->where = "";
    $this->having = "";
    
    $this->roottable = "";
    $this->tables = array();
    $this->jointypes = array();
    $this->leftcols = array();
    $this->rightcols = array();
    $this->groupby = array();
    $this->orderby = array();
    $this->orderbydesc = array();
    
    $this->result = 0;
    $this->row = array();
    $this->size = 0;
    
    $this->children = 0;
    $this->childid = 0;
  }

  function addjoin($table,$jointype, $leftcol,$rightcol)
  {
    $found = array_keys($this->tables,$table);
    if (count($found) > 0)
      foreach($found as $key)
        if ($this->leftcols[$key] == $leftcol && $this->rightcols[$key] == $rightcol) 
        {
          $this->jointypes[$foundkey] = max($this->jointypes[$foundkey],$jointype);
          return;
        }  
    
    $this->tables[] = $table;
    $this->jointypes[] = $jointype;
    $this->leftcols[] = $leftcol;
    $this->rightcols[] = $rightcol;
  }

  function addcolumn($column,$dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->columns,$column) === false))
    {
      if ($this->columns) $this->columns .= ", ";
      $this->columns .= $column;
    }  
  }

  function addwhere($condition, $dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->where,$condition) === false))
    {
      if ($this->where) $this->where .= " AND ";
      $this->where .= $condition;
    }  
  }
  
  function addhaving($condition, $dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->having,$condition) === false))
    {
      if ($this->having) $this->having .= " AND ";
      $this->having .= $condition;
    }  
  }

  function addgroup($column)
  {
    if (!in_array($column,$this->groupby))
      $this->groupby[] = $column;
  }
  
  function addordering($column,$descending=false,$position=0)
  {
    if ($position === true) $position = -1;
    array_splice($this->orderby, $position, 0, $column);
    array_splice($this->orderbydesc, $position, 0, $descending);
    
    $found = array_keys($this->orderby,$column);
    array_shift($found);
    $lost = 0;
    while(count($found))
    {
      $key = array_shift($found) - $lost;
      array_splice($this->orderby,$key,1);
      array_splice($this->orderbydesc,$key,1);
      ++$lost;
    };
  }

  function settable($table)
  {
    $this->roottable = $table;
  }

  function bond(&$child, &$parent) // PRIVATE STATIC
  {
    ++$parent->children;
    $child->childid = $parent->children;
    
    if ($parent->columns) $parent->columns .= ", "; else $parent->columns = "";
    $parent->columns .= "COUNT(";

    if (count($child->groupby) > 0)
    {
      $parent->columns .= "DISTINCT ";
      $first = true;
      foreach($child->groupby as $group)
      {
        if ($first) $first=false; else $parent->columns .= ", ";
        $parent->columns .= $group;
      }
    }
    else
      $parent->columns .= "*";
    
    $parent->columns .= ") AS " . db_SelectMonster_prefix . $child->childid;
      
    foreach($parent->groupby as $grouping)
    {
      if (in_array($grouping,$child->groupby)) 
        $child->groupby[] = $grouping;
     
      if (!in_array($grouping,$parent->orderby))
      {
        $this->orderby[] = $grouping;
        $this->orderbydesc[] = false;
      }  
    }
    
    array_splice($child->orderby,0,0,$parent->orderby);
    array_splice($child->orderbydesc,0,0,$parent->orderbydesc); 
  }

  function makeparent(&$parent)
  {
    $parent->where = $this->where;
    $parent->having = $this->having;
    $parent->roottable = $this->roottable;
    $parent->tables = $this->tables;
    $parent->jointypes = $this->jointypes;
    $parent->leftcols = $this->leftcols;
    $parent->rightcols = $this->rightcols;

    db_SelectMonster::bond($this,$parent);
  }

  function makechild(&$child)  
  {
    $child->where = $this->where;
    $child->having = $this->having;
    $child->roottable = $this->roottable;
    $child->tables = $this->tables;
    $child->jointypes = $this->jointypes;
    $child->leftcols = $this->leftcols;
    $child->rightcols = $this->rightcols;

    db_SelectMonster::bond($child,$this);
  }
  
  function childsize(&$child)
  {
    return (int)($this->row[db_SelectMonster_prefix . $child->childid]);
  }

  function advance()
  {
    if ($this->row = mysql_fetch_array($this->result))
      return true;
    else
      return false;
  }

  function execute($db)
  {
    global $db_debug;    

    $sql_columns = $this->columns ? $this->columns : "*";
    $sql_from = $this->roottable;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    $sql_joins = "";
    foreach($this->tables as $tableno => $table)
    {
      if ($this->jointypes[$tableno] == db_SelectMonster_inner)
        $sql_joins .= " INNER JOIN $table ON " . ($this->leftcols[$tableno]) . " = " . ($this->rightcols[$tableno]);
      else if ($this->jointypes[$tableno] == db_SelectMonster_left)
        $sql_joins .= " LEFT JOIN $table ON " . ($this->leftcols[$tableno]) . " = " . ($this->rightcols[$tableno]);
      else if ($db_debug)
        taskwindow_cprint("<b>Error from: db_SelectMonster->execute, invalid join type</b><br>");
    };

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    $sql_groupby = "";
    foreach($this->groupby as $group)
    {
      if ($sql_groupby) $sql_groupby .= ", "; else $sql_groupby = "GROUP BY ";
      $sql_groupby .= $group;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    $sql_orderby = "";
    foreach($this->orderby as $key => $order)
    {
      if ($sql_orderby) $sql_orderby .= ", "; else $sql_orderby = "ORDER BY ";
      $sql_orderby .= $order . ($this->orderbydesc[$key] ? " DESC" : "");
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    $sql_where = $this->where ? "WHERE " . $this->where : "";
    $sql_having = $this->having ? "HAVING " . $this->having : "";

    $sql = "SELECT $sql_columns FROM $sql_from $sql_joins $sql_where $sql_groupby $sql_orderby $sql_having";
    $this->result = mysql_query($sql,$db);
    if (!$this->result) print("<textarea rows=20 cols=80 id=textarea1 name=textarea1>SQL ERROR\n---------\n\n$sql</textarea><br>");
    $this->size = mysql_num_rows($this->result);
  }
};


//-----------------------------------------------------------------------------
// General databasing functions. Slow and not very flexible, but used heavily
//  by import.inc to keep that code short and simple.


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

%>
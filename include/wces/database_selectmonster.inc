<?


  /**
  * SelectMonster class
  * 
  * <p>This class allows for the construction of certain types of sql SELECT queries.
  * It is used by the reporting code to avoid redundancy and to cut down on the 
  * excessive sql string manipulation that would otherwise be necessary in 
  * constructing many complex and customizable queries. Specifically, the
  * SelectMonster class makes it easier to efficiently implement what I call
  * "Parent and Child" queries.</p>
  * 
  * <p>... more documentation soon ...</p>
  * 
  * <p>These are the notes I used to keep things straight while I wrote the
  * method bodies:</p>
  * 
  * <p>Parent vs. Child</p>
  * 
  * <ul>
  *   <li> different columns </li>
  *   <li> same joins, conditions, where clauses </li>
  *   <li> child grouping is superset of parent grouping </li>
  *   <li> parent's grouped columns are also ordered columns </li>
  *   <li> the child's order by list is nearly identical to the parent's order by list,
  *        the only exception is that the child list can have some ordered columns
  *        of it's own FOLLOWING the parent's order by list. </li>
  * </ul>  
  * 
  * <p>Requirements:</p>
  * 
  * <ul>
  *   <li> children can also be parents </li>
  *   <li> parents can have multiple children </li>
  *   <li> children have only one parent </li>
  * </ul>
  */

class db_SelectMonster
{
  /**
  * Comma separated list of fields to retrieve. Part of the SQL select statement.
  * @var string
  * @access private
  */
  var $columns;
  
  /**
  * SQL expression that evaluates to a boolean. Put in the SQL WHERE clause. Can be empty.
  * @var string
  * @access private
  */
  var $where;
  
  /**
  * SQL expression that evaluates to a boolean. Put in the SQL HAVING clause. Can be empty.
  * @var string
  * @access private
  */
  var $having;
  
  /**
  * SQL table to select from.
  * @var string
  * @access private
  */
  var $roottable; 
  
  /**
  * Zero based array containing table names, used to construct JOIN statements.
  * @var array
  * @access private
  */
  var $tables;
  
  /**
  * Zero based array containing join types, used to construct JOIN statements.
  * Possible values are db_SelectMonster_left and db_SelectMonster_inner
  * @var array
  * @access private
  */
  var $jointypes;
  
  /**
  * Zero based array containing the name of the first column to join by, used to construct JOIN statements.
  * @var array
  * @access private
  */
  var $leftcols;
  
  /**
  * Zero based array containing the name of the second column to join by, used to construct JOIN statements.
  * @var array
  * @access private
  */
  var $rightcols; 
  
  /**
  * Zero based array containing the names of columns to group by, used to construct the GROUP BY clause.
  * @var array
  * @access private
  */
  var $groupby;
  
  /**
  * Zero based array containing the names of columns to order by, used to construct the ORDER BY clause.
  * @var array
  * @access private
  */
  var $orderby;
  
  /**
  * Zero based array of booleans used to construct the ORDER BY clause. True of column should be in descending order.
  * @var array
  * @access private
  */
  var $orderbydesc; // arrays
  
  /**
  * mySQL result identifier from the last call to {@link execute execute()}
  * @var identifier
  * @access public
  */
  var $result;
  
  /**
  * mySQL row from the last call to {@link advance advance()}
  * @var array
  * @access public
  */
  var $row;
  
  /**
  * number of rows in the {@link $result $result} result set
  * @var integer
  * @access public
  */
  var $size;
  
  /**
  * child id
  * @var integer
  * @access public
  */
  var $childid;
  
  /**
  * array of references to children
  * @var array
  * @access public
  */
  var $children;

  /**
  * Constructor.
  * Instantiates an empty db_SelectMonster object.
  * @access public  
  */	
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
    
    $this->children = array();
    $this->childid = 0;
  }

  /**
  * Add a table join to the query. The resulting SQL fragment looks like:
  * <pre>... {LEFT | INNER} JOIN $table ON $leftcol = $rightcol ...</pre>
  * @param string  Name of table to join with.  
  * @param integer Join type, possible values are db_SelectMonster_left and db_SelectMonster_inner
  * @param string  Column name from the table being joined
  * @param string  Column name from an existing table
  * @access public
  */	
  function addjoin($table,$jointype, $leftcol,$rightcol)
  {
    $found = array_keys($this->tables,$table);
    if (count($found) > 0)
      foreach($found as $key)
        if ($this->leftcols[$key] == $leftcol && $this->rightcols[$key] == $rightcol) 
        {
          $this->jointypes[$key] = max($this->jointypes[$key],$jointype);
          return;
        }  
    
    $this->tables[] = $table;
    $this->jointypes[] = $jointype;
    $this->leftcols[] = $leftcol;
    $this->rightcols[] = $rightcol;
  }

  /**
  * Add another column to the SELECT statement. Resulting SQL fragment looks like:
  * <pre>SELECT [existing_columns, ] $column FROM ...
  * where existing_columns are the columns that have already been added.
  * @param string  Name of column to add.
  * @param boolean  Optional, causes column to be added only if it hasn't been added before.
  * @access public
  */	
  function addcolumn($column,$dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->columns,$column) === false))
    {
      if ($this->columns) $this->columns .= ", ";
      $this->columns .= $column;
    }  
  }

  /**
  * Adds a condition to the WHERE clause. Resulting SQL fragment looks like:
  * <pre>... WHERE [existing_conditions AND ] $condition ...</pre>
  * where existing_conditions are the conditions that have already been added.
  * @param string   Name of condition to add.
  * @param boolean  Optional, causes condition to be added only if it hasn't
  *                 been added before.
  * @access public
  */	
  function addwhere($condition, $dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->where,$condition) === false))
    {
      if ($this->where) $this->where .= " AND ";
      $this->where .= $condition;
    }  
  }
  
  /**
  * Adds a condition to the HAVING clause. Resulting SQL fragment looks like:
  * <pre>... HAVING [existing_conditions AND ] $condition ...</pre>
  * where existing_conditions are the conditions that have already been added.
  *
  * @param string   Name of condition to add.
  * @param boolean  Optional, causes condition to be added only if it hasn't
  *                 been added before.
  * @access public
  */
  function addhaving($condition, $dupecheck = false)
  {
    if(!$dupecheck || !(strpos($this->having,$condition) === false))
    {
      if ($this->having) $this->having .= " AND ";
      $this->having .= $condition;
    }  
  }

  /**
  * Adds another column to the GROUP BY clause. Resulting SQL fragment looks like:
  * <pre>... GROUP BY [existing_columns, ] $column ...
  * where existing_columns are the columns that have already been added.
  *
  * @param string  Name of column to add.
  * @param boolean  Optional, causes column to be added only if it hasn't been added before.
  * @access public
  */
  function addgroup($column)
  {
    if (!in_array($column,$this->groupby))
      $this->groupby[] = $column;
  }

  /**
  * Adds another column to the GROUP BY clause. Resulting SQL fragment looks like:
  * <pre>... ORDER BY [existing_columns, ] $column [DESC] [, more_existing_columns]...
  * where existing_columns and more_existing)columns are the columns that have
  * already been added.
  *
  * @param string   Name of column to add.
  * @param boolean  Optional, TRUE to sort in descending order, FALSE to sort
  *                 in ascending order. Default is ascending.
  * @param integer  Optional, Position to place new ORDER BY column relative
  *                 to existing ones. Can be negative. Examples: 0 is first,
  *                 1 is second, Boolean true for last. Default is first.
  * @access public
  */
  function addordering($column,$descending=false,$position=0)
  {
    if ($position === true) $position = count($this->orderby);
    array_splice($this->orderby, $position, 0, $column);
    array_splice($this->orderbydesc, $position, 0, $descending);
/*    
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
*/    
  }

  /**
  * Specify which table to SELECT from. Resulting SQL fragment looks like:
  * <pre>SELECT ... FROM $table ...</pre>
  *
  * @param string  Name of table to select from.
  * @access public
  */
  function settable($table)
  {
    $this->roottable = $table;
  }

  /**
  * Internal static method. Ensures that all of parent's GROUP BY columns are
  * present in the child's GROUP BY section and the parent's ORDER BY section. 
  * It then copies the parent's entire ORDER BY section to the beginning of the
  * child's ORDER BY section.
  *
  * @param object Reference to a child object
  * @param object Reference to a parent object
  * @access private
  */
  function fixchild(&$child, &$parent)
  {
    foreach($parent->groupby as $grouping)
    {
      if (!in_array($grouping,$child->groupby))
        $child->groupby[] = $grouping;
     
      if (!in_array($grouping,$parent->orderby))
      {
        $parent->orderby[] = $grouping;
        $parent->orderbydesc[] = false;
      }  
    }
    
    array_splice($child->orderby,0,0,$parent->orderby);
    array_splice($child->orderbydesc,0,0,$parent->orderbydesc); 
  }

  /**
  * Internal static method. Initializes parent's and child's $children and
  * $childid members, adds a column to each parent row containing the number
  * of children. Enforces grouping and ordering rules. See {@link fixchild
  * fixchild()}
  *
  * @param object Reference to a child object
  * @param object Reference to a parent object
  * @access private
  */
  function bond(&$child, &$parent) // PRIVATE STATIC
  {
    $parent->children[] = &$child;
    $child->childid = count($parent->children);
    
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
      
    db_SelectMonster::fixchild($child,$parent);
    
    foreach($child->children as $key => $value)
      if ($value)
        db_SelectMonster::fixchild($child->children[$key],$parent);
  }

  /**
  * Turns the argument into a parent of this query. The parent's fields, joins,
  * and where conditions are overwritten, and rules concerning order by and
  * group by clauses are enforced.
  *
  * @param object Reference to a query that will become the parent of this one.
  * @access public
  */
  function makeparent(&$parent) // turn argument into a fully fledged parent
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

  /**
  * Turns the argument into a child of this query. The child's fields, joins,
  * and where conditions are overwritten, and rules concerning order by and
  * group by clauses are enforced.
  *
  * @param object Reference to a query that will become a child of this one.
  * @access public
  */
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

  /**
  * Retrieves the number of children rows in the current row.
  *
  * @param object Reference to a child query.
  * @return integer Retrieves the number of children rows in the current row.
  * @access public
  */  
  function childsize(&$child)
  {
    return (int)($this->row[db_SelectMonster_prefix . $child->childid]);
  }

  /**
  * Fills the {@link $row $row} variable with the next row in the result set.
  * @access public
  */  
  function advance()
  {
    if ($this->row = mysql_fetch_assoc($this->result))
      return true;
    else
      return false;
  }

  /**
  * Executes the SQL query using the conditions specified since construction.
  *
  * @param identifier Handle to mysql connection.
  * @access public
  */  
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

    $this->result = db_exec("SELECT $sql_columns FROM $sql_from $sql_joins $sql_where $sql_groupby $sql_orderby $sql_having",$db,__FILE__,__LINE__);
    $this->size = mysql_num_rows($this->result);
  }
};

define("db_SelectMonster_left",1);
define("db_SelectMonster_inner",2);
define("db_SelectMonster_prefix","SelectMonsterChild");

?>
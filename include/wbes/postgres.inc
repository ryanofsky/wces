<?

require_once("wbes/server.inc");
require_once("wbes/general.inc");
require_once("wbes/taskwindow.inc");

function pg_go($sql, $db, $file = __FILE__, $line = __LINE__)
{
  global $db_debug, $WBES_DEBUG;

  if ($db_debug) $t = microtime();
  $result = @pg_exec($db,$sql);
  if ($db_debug) $time = gettime(microtime()) - gettime($t);
  
  if ($db_debug || (!$result && $WBES_DEBUG))
  {
    print("<p>SQL Query in <b>$file</b>, line <b>$line</b><br>\n");
    print('<pre style="background-color:#DDDDDD; width:100%">' . $sql . "\n</pre><br>\n");
    print($result ? "<font color=blue>Success</font>" : "<font color=red>Failure</font>");
    if ($result)
    {
      $rows = max(pg_cmdtuples($result), pg_numrows($result));
      print("<br>Time: $time seconds<br>Affected Rows: $rows");
    }
    else
      print("<br>Error: " . pg_errormessage($db));
    print("</p>\n");
    @ob_flush(); flush();
    if (!$result)
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
        print("<tr>\n"); 
        $row = pg_fetch_row($result, $r);
        for($c = 0; $c < $cols; ++$c)
        {
          if (!isset($row[$c]))
            $str = "<i><small><small>null</small></small></i>";
          else if (!$showtypes)
            $str = strlen($row[$c]) ? htmlspecialchars($row[$c]) : '&nbsp;';
          else
            $str = htmlspecialchars($row[$c]) . " <small><small><i>" . gettype($row[$c]) . "</i></small></small>";  
          print("  <td>$str</td>\n");
        }
        print("</tr>\n");  
      }  
      print ("</table>\n");
      return;
    }
  }  
  print("<p><b>No Result</b></p>\n");
}

class pg_wrapper
{
  var $row = false;
  var $rkey = 0;
  var $rows = 0;
  var $result = NULL;
  
  function pg_wrapper($result)
  {
    $this->rows = pg_numrows($result);
    $this->result = $result;
    $this->advance();
  }
  
  function advance()
  {
    if ($this->rkey < $this->rows)
      $this->row = pg_fetch_array($this->result,$this->rkey,PGSQL_ASSOC);
    else
      $this->row = false;
    ++$this->rkey;
    return $this->row;
  }
}

function array_make($num, $val)
{
  return array_pad(array(), $num, $val); 
}

class pg_segmented_wrapper
{
  var $result;
  var $splitcol;
  var $row;
  var $rkey = 0;
  var $rows = 0;
  var $split = false;
  var $last;
  
  function pg_segmented_wrapper($result, $splitcol)
  {
    $this->result = $result;
    $this->splitcol = $splitcol;
    $this->rkey = 0;
    $this->rows = pg_numrows($result);
    
    $this->last = NULL;
    $this->row = $this->rkey < $this->rows ? pg_fetch_array($this->result,$this->rkey,PGSQL_ASSOC) : NULL;
    
    if (is_array($this->splitcol))
      $this->split = array_pad(array(), count($this->splitcol), true);
    else
      $this->split = true;
  } 
  
  function advance($skip = false)
  {
    if (!$skip) $this->last = $this->row;
    ++$this->rkey;
    if ($this->rkey < $this->rows)
    {
      $this->row = pg_fetch_array($this->result,$this->rkey,PGSQL_ASSOC);
      
      if (is_array($this->splitcol))
      {
        $changed = false; $this->split = array();
        foreach($this->splitcol as $s)
          $this->split[] = $changed = $changed || $this->last[$s] !== $this->row[$s];
      }
      else
        $this->split = $this->last[$this->splitcol] !== $this->row[$this->splitcol];
    }
    else
    {
      $this->row = NULL;
      if (is_array($this->splitcol))
        $this->split = array_pad(array(), count($this->splitcol), true);
      else
        $this->split = true;
    }
  }
  
  function skipAdvance()
  {
    $this->advance(true);
  }

  function peek($split_num = 0)
  {
    if ($this->rkey+1 >= $this->rows)
      return true;
    $key = is_array($this->splitcol) ? $this->splitcol[$split_num] : $this->splitcol;
    return (bool)($this->row[$key] != pg_result($this->result, $this->rkey+1, $key));
    
  }
}

function pg_explode($sa)
{
  $ret = array(); $start = 1; $end = $quoted = $escaped = false; 
  
  $l = strlen($sa) - 1; // ignore braces at beginning and end {}
  $started = 1;
  for($i = 1; $i <= $l; ++$i)
  {
    if ($i == $l)
      $end = $i;
    else
    {
      $c = $sa[$i];
      $is_comma = $c == ',';
      $is_quote = $c == '"';   //"  fix for bad editiors which don't properly handle quote escaping. :-)
      $is_escape = $c == '\\';
    
      if ($is_comma && !$quoted)
        $end = $i;
      else if ($is_quote)  
      {
        if ($quoted && !$escaped)
          $end = $i;
        else if (!$quoted)
        {
          $started = $i + 1;
          $quoted = true;
        }
      }
      
      if ($escaped)
        $escaped = false;
      else if ($is_escape)
        $escaped = true;
    }
    
    if ($end)
    {
      if ($end >= $started) $ret[] = stripslashes(substr($sa, $started, $end - $started));
      if ($quoted) { $quoted = false; $started = $i + 2; } else $started = $i + 1;
      $end = false;
    }
  }
  return $ret;
}

function pg_implode($a)
{
  $r = "";
  foreach($a as $v)
  {
    if($r) $r .= ",";
    $r .= '"' . addslashes($v) . '"';
  }
  return "'\{" . addslashes($r) . "}'";
}

// takes timestamp as seconds past epoch, returns representation
// as postgres string. Call with no arguments to return current 
// time. 
function pg_date($time = false)
{
  if (isset($time))
  {
    $format = 'Y-m-d H:i:s-00';
    if ($time !== false)
      $date = gmdate($format, $time);
    else
      $date = gmdate($format);
    return "'$date'";   
  }
  return "NULL";
}


// escape literal a literal string for use in a postgres posix regular expression
function pg_reg_escape($text)
{
  return preg_replace('([\\^\\.\\[\\$\\(\\)\\|\\*\\+\\?\\{\\\\])', '\\\\${0}', $text);
}

function pg_select_array($sql, $db, $file, $line)
{
  $r = pg_go($sql, $db, $file, $line);
  $n = pg_numrows($r);
  $a = array();
  for ($i = 0; $i < $n; ++$i)
  {
    list($k, $v) = pg_fetch_array($r, $i, PGSQL_NUM);
    $a[$k] = $v;
  }
  return $a;
}

?>

<?

function legacy_report_meter($percent)
{
  global $wces_path;
  $out = "<img src=\"${wces_path}/media/meter/left.gif\" width=6 height=13>";
  for($i = 0; $i < 5; ++$i)
  {
    $num = $percent - 20 * $i;
    if ($num < 0) $num = 0;
    if ($num < 10) $num = "0" . $num;
    if ($num > 20) $num = 20;
    $out .= "<img src=\"${wces_path}media/meter/mid${num}.gif\" width=21 height=13>";
  };
  $out .= "<img src=\"${wces_path}media/meter/right.gif\" width=5 height=13>";
  return $out;
};

function legacy_report_mode($distribution)
{
  $max = $maxv = false;
  foreach($distribution as $score => $people)
  if ($max === false || $people >= $maxv) 
  {
    $max = $score;
    $maxv = $people;
  }
  return $max;
}

function legacy_report_avg($distribution)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $score * $people;
  }
  return $sum == 0 ? 0 : $wsum / $sum;
}

function legacy_report_sd($distribution, $avg)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $people * pow(($score - $avg),2);
  }
  return $sum == 0 ? 0 : sqrt($wsum/$sum);
}

class legacy_SegmentedQuery
{
  var $result;
  var $column;
  var $last;
  
  function legacy_SegmentedQuery($result, $column)
  {
    $this->result = $result;
    $this->column = $column;
    $this->last = false;
  }
  
  function advance()
  {
    $this->row = mysql_fetch_assoc($this->result);
    if ($this->row && $this->last == $this->row[$this->column])
    {
      $this->last = $this->row[$this->column];
      return true;
    }
    else
    {
      $this->last = $this->row[$this->column];
      return false;    
    }
  }
}

function legacy_addwhere(&$where, $clause)
{
  $where .= $where ? " AND " : "WHERE ";
  $where .= $clause;
}

function legacy_addorder(&$order, $clause)
{
  $order .= $order ? ", " : "ORDER BY ";
  $order .= $clause;
}

function legacy_addgroup(&$group, $clause)
{
  $group .= $group ? ", " : "GROUP BY ";
  $group .= $clause;
}

function legacy_addcolumn(&$columns, $clause)
{
  $columns .= $columns ? ", " : "";
  $columns .= $clause;
}

?>
<?

function report_meter($percent)
{
  global $server_media;
  $out = "<img src=\"${server_media}/meter/left.gif\" width=6 height=13>";
  for($i = 0; $i < 5; ++$i)
  {
    $num = $percent - 20 * $i;
    if ($num < 0) $num = 0;
    if ($num < 10) $num = "0" . $num;
    if ($num > 20) $num = 20;
    $out .= "<img src=\"${server_media}/meter/mid${num}.gif\" width=21 height=13>";
  };
  $out .= "<img src=\"${server_media}/meter/right.gif\" width=5 height=13>";
  return $out;
};

function report_mode($distribution)
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

function report_avg($distribution)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $score * $people;
  }
  return $sum == 0 ? 0 : $wsum / $sum;
}

function report_sd($distribution, $avg)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $people * pow(($score - $avg),2);
  }
  return $sum == 0 ? 0 : sqrt($wsum/$sum);
}

class SegmentedQuery
{
  var $result;
  var $column;
  var $last;
  
  function SegmentedQuery($result, $column)
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

function addwhere(&$where, $clause)
{
  $where .= $where ? " AND " : "WHERE ";
  $where .= $clause;
}

function addorder(&$order, $clause)
{
  $order .= $order ? ", " : "ORDER BY ";
  $order .= $clause;
}

function addgroup(&$group, $clause)
{
  $group .= $group ? ", " : "GROUP BY ";
  $group .= $clause;
}

function addcolumn(&$columns, $clause)
{
  $columns .= $columns ? ", " : "";
  $columns .= $clause;
}

?>
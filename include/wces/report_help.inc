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
  return $sum <= 1 ? 0 : sqrt($wsum/($sum - 1));
}

function report_median($distribution)
{
  $sum = 0;
  foreach($distribution as $score => $people)
    $sum += $people;

  // $i1 and $i2 are one-based indices into sorted lists of results
  if ($sum % 2 == 1)
    $i1 = $i2 = ($sum + 1) / 2;
  else
  {
    $i1 = $sum / 2;
    $i2 = $i1 + 1; 
  }

  $v1 = $v2 = NULL;
  $psum = 0;
  foreach($distribution as $score => $people)
  {
    $psum += $people;
    if (!isset($v1) && $psum >= $i1)
      $v1 = $score;
    if (!isset($v2) && $psum >= $i2)
      $v2 = $score;
      
    if (isset($v1) && isset($v2)) break;
  }
  return ($v1 + $v2) / 2.0;
}

?>
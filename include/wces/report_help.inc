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

function GraphSize(&$params, $datax = NULL)
{
  $params['angle'] = 50;
  $params['size'] = 12;
  $params['family'] = FF_VERDANA;
  $params['style'] = FS_NORMAL;
  $params['graphWidth'] = 300;
  $params['graphHeight'] = 200;
  $params['graphMargin'] = 30;
  
  if (isset($params['width']) && isset($params['height']))
  {
    $params['bottom'] = $params['height'] - $params['graphHeight'] - 2 * $params['graphMargin'];
    $params['left'] = $params['width'] - $params['graphWidth'] - 2 * $params['graphMargin'];
  }
  else
  {
    $ttf = new TTF();
    $file = $ttf->File($params['family'], $params['style']);
  
    $lowest = -1000; $highest = 1000;
    $xmargin = 0;
    $p = 0;
    $segmentWidth = (double)$params['graphWidth'] / (double)count($datax);
    foreach($datax as $txt)
    {
      $leftest = 1000; $rightest = -1000;
      $bbox = ImageTTFBBox($params['size'],$params['angle'],$file,$txt);
      for($i = 0; $i < 4; ++$i)
      {
        $x = $bbox[2 * $i];
        $y = $bbox[2 * $i + 1 ];
        if ($y > $lowest) $lowest = $y;
        if ($y < $highest) $highest = $y;
        if ($x < $leftest) $leftest = $x;
        if ($x > $rightest) $rightest = $x;
      };
      
      if ($rightest - $leftest)
      {
        $nx = $rightest - $leftest - ($p + 0.5 )* $segmentWidth;
        if ($nx > $xmargin) $xmargin = $nx;
      }
      ++$p;
    };
    
    $params['left'] = $xmargin;
    $params['bottom'] = $lowest - $highest;
    $params['width'] = (int)($params['graphWidth'] + 2 * $params['graphMargin'] + $params['left']);
    $params['height'] = (int)($params['graphHeight'] + 2 * $params['graphMargin'] + $params['bottom']);
  }
};

function MakeGraph($datax, $datay)
{
  global $server_media;

  $params = array();
  GraphSize($params, $datax);
  
  return "<img src=\"$server_media/graphs/bars.php"
    . "?datax=" . htmlspecialchars(serialize($datax)) 
    . "&datay=" . htmlspecialchars(serialize($datay))
    . "&width=$params[width]&height=$params[height]"
    . "\""
    . " width=$params[width] height=$params[height]"
    . ">";
}


?>
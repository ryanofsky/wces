<?
//---------------------------------------------------------------------------------------------
// Text Table Maker

function leftalign($text,$width)
{
  $l = strlen($text);
  if ($l < $width)
    return $text . str_repeat(" ",$width - $l);
  else
    return $text;
};

function centeralign($text,$width)
{
  $l = strlen($text);
  $x = floor(($width-$l)/2.0);
  if ($l < $width)
    return str_repeat(" ",$x) . $text . str_repeat(" ",$width - $x - $l);
  else
    return $text;
};

function texttable($data,$formats)
{
  $widths = array();
  $heights = array();
  foreach($data as $rowname => $row)
  {
    $heights[$rowname] = 1;
    foreach($row as $colname => $col)
    if ($formats[$colname] === $formats[$colname] + 0)
    {
      $widths[$colname] = $formats[$colname];
      $data[$rowname][$colname] = explode("\n", wordwrap(str_replace("\r\n","\n",$data[$rowname][$colname]),$formats[$colname],"\n"));
      $count = sizeof($data[$rowname][$colname]);
      if ($heights[$rowname] < $count) $heights[$rowname] = $count;
    }  
    else  
    {
      $l = strlen($col);
      if (!isset($widths[$colname]) || ($l > $widths[$colname]))
        $widths[$colname] = $l;
    }
  }
  $line = "+";
  foreach($widths as $width)
  {
    $line .= str_repeat("-", 2 + $width) . "+";
  };
  $line .= "\n";
  
  $table = $line;
  
  foreach($data as $rowname => $row)
  {
    for($i = 0; $i < $heights[$rowname]; ++$i)
    {
      $table .= "|";
      foreach($widths as $colname => $width)
      {   
        if ($formats[$colname] === $formats[$colname] + 0)
          $table .= leftalign(" " . $row[$colname][$i],$width + 2);   
        else if ($i == floor($heights[$rowname] / 2))
        {
          if ($formats[$colname] == "center")
            $table .= centeralign(" " . $row[$colname],$width + 2);   
          else // ($formats[$colname] == "left")
            $table .= leftalign($row[$colname],$width + 2);   
        }
        else
          $table .= str_repeat(" ",$width + 2);
        $table .= "|";
      };
      $table .= "\n";
    }
    $table .= "$line";
  };
  return $table; 
};
?>
<?
//---------------------------------------------------------------------------------------------
// Text Table Maker

define('ALIGN_LEFT', 1);
define('ALIGN_CENTER', 2);
define('ALIGN_RIGHT', 3);
define('ALIGN_VCENTER', 8);
define('ALIGN_HMASK', 3);
define('ALIGN_VMASK', 8);

function Align($text, $width, $align)
{
  $l = strlen($text);
  if ($l >= $width) return $text;
  switch ($align)
  {
    case ALIGN_CENTER:
    $x = (int)(($width-$l)/2);
    return str_repeat(' ', $x) . $text . str_repeat(' ', $width - $x - $l);

    case ALIGN_RIGHT:
    return str_repeat(' ', $width - $l) . $text;

    case ALIGN_LEFT:
    default:
    return $text . str_repeat(' ', $width - $l);
  } 
}

function BreakText($text, &$exploded, &$height, &$width)
{
  if (is_int($width)) $text = wordwrap($text, $width, "\n");
  
  $exploded = explode("\n", $text);
  $height = count($exploded);

  if (!is_int($width))
  {
    $width = 0;
    foreach(array_keys($exploded) as $k)
    {
      $l = strlen($exploded[$k]);
      if ($l > $width) $width = $l; 
    }
  }
}

function ReBreakText(&$lines, $width, &$height)
{
  $nlines = array();
  foreach($lines as $line)
    array_splice
    ( $nlines, count($nlines), 0, explode
      ( "\n", wordwrap($line, $width, "\n")
      )
    );
  $lines = $nlines;
  $height = count($nlines);
}

function SetMax(&$a, $b)
{
  $av = isset($a) ? (int)$a : 0;
  $bv = isset($b) ? (int)$b : 0;
  if ($bv > $av) $a = $bv;
};

function UnixText($text)
{
  // convert PC line endings to unix endings.
  $text = str_replace("\r\n", "\n", $text);
  // convert mac line endings to unix endings.
  $text = str_replace("\r", "\n", $text);
  return $text;
}

function TextTable($data, $columnFormats, $maxWidth, $columnWidths = array(), $headFormats = null)
{
  $widths = $columnWidths;
  $heights = array();
  
  $newdata = array();
  $variableColumns = 0;
  foreach (array_keys($data) as $row)
  {
    $vc = 0;
    $newdata[$row] = array();
    foreach (array_keys($data[$row]) as $column)
    {
      $width = !isset($columnWidths[$column]) ? null : $columnWidths[$column];
      $text = UnixText($data[$row][$column]);
      BreakText($text, $newdata[$row][$column], $height, $width);
      SetMax($widths[$column], $width);
      SetMax($heights[$row], $height);
    }
    SetMax($variableColums, $vc);
  }
  
  // get counts
  $fixedWidth = count($widths) + 1;
  $varWidths = array();
  foreach($widths as $i => $width)
  {
    if (isset($columnWidths[$i]))
      $fixedWidth += $columnWidths[$i];
    else
      $varWidths[$i] = 0;
  }

  // allocate space among variable columns

  for(;;)
  {    
    $space = (double)($maxWidth - $fixedWidth);
    $last = 0;
    $i = 0;
    $n = count($varWidths);
    foreach (array_keys($varWidths) as $column)
    {
      ++$i;
      $endCol = (int)($space * $i / $n);
      $width = $endCol - $last;

      if ($width < 1) $width = 1;
      // if assigned width is too big, just use default width, and start over
      if ($width >= $widths[$column])
      {
        unset($varWidths[$column]);
        $fixedWidth += $widths[$column];
        continue 2;
      }
      $varWidths[$column] = $width;
      $last += $width;
    }

//  needed to emulate goto
    break;
  }
  
  foreach ($varWidths as $column => $width)
  {  
    $widths[$column] = $width;
    foreach (array_keys($data) as $row)
    {
      ReBreakText($newdata[$row][$column], $width, $height);
      SetMax($heights[$row], $height);
    }
  }
  
  $bar = "+";
  foreach($widths as $width)
    $bar .= str_repeat("-", $width) . "+";
  $bar .= "\n";

  $table = $bar;
  $firstRow = true;
  foreach(array_keys($data) as $row)
  {
    for($i = 0; $i < ($height = $heights[$row]); ++$i)
    {
      $table .= "|";
      foreach($widths as $column => $width)
      {
        $lines =& $newdata[$row][$column];
        $format = $firstRow && isset($headFormats[$column]) 
          ? $headFormats[$column] : $columnFormats[$column];
        
        if ($format & ALIGN_VCENTER)
          $j = $i + (int)(($height - count($lines)) / 2);
        else
          $j = $i;
          
        $text = isset($lines[$j]) ? $lines[$j] : '';
        $table .= Align($text, $widths[$column], $format);
        $table .= "|";
      };
      $table .= "\n";
    }
    $table .= "$bar";
    $firstRow = false;
  };
  return $table;
};

?>
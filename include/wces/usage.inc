<?

$usage_question_periods = "
  SELECT q.question_period_id, q.displayname
  FROM semester_question_periods AS q
  WHERE EXISTS (SELECT * FROM wces_topics AS t WHERE t.question_period_id = q.question_period_id)
  ORDER BY begindate
";

function usage_menu($choices, $fieldName, $currentvalues)
{
  global $PHP_SELF;
  $first = true;
  foreach ($choices as $value => $text)
  {
    if ($first) $first = false; else print(' | ');
    if ($value == $currentvalues[$fieldName])
      print($text);
    else
    {
      $params = '';
      foreach ($currentvalues as $k => $v)
      {
        if ($params) $params .= '&';
        if ($k == $fieldName)
          $params .= "$k=$value";
        else
          $params .= "$k=$v";
        $first = false;
      }
      print("<a href=\"$PHP_SELF?$params\">$text</a>");
    }
  }
}

function usage_sql_array($sql, $db, $file, $line)
{
  $r = pg_go($sql, $db, $file, $line);
  $n = pg_numrows($r);
  $a = array();
  for ($i = 0; $i < $r; ++$i)
  {
    list($k, $v) = pg_fetch_array($r, $i, PGSQL_NUM);
    $a[$k] = $v;
  }
  return $a;
}


?>
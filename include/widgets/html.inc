<?

$html_head = '';
$html_head = '';

function html_head_append($str)
{
  global $html_head;
  $html_head .= $str;
}

function html_body_append($str)
{
  global $html_body;
  $html_body .= $str;
}

function html_head()
{
  global $html_head;
  return $html_head;
  
}

function html_body()
{
  global $html_body;
  return $html_body;
}

?>
<?

$wbes = false;

function wbes_connect()
{
  global $wbes;
  if (!$wbes) $wbes = server_pginit("wces");
}

?>
<?

$wbes = false;

function wbes_connect()
{
  global $wbes, $wbes_database;
  if (!$wbes) $wbes = server_pginit($wbes_database);
}

?>
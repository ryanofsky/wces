<?

$wbes = false;

function wbes_connect()
{
  global $wbes, $WBES_DATABASE_NAME;
  if (!$wbes) $wbes = server_pginit($WBES_DATABASE_NAME);
}

?>

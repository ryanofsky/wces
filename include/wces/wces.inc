<?

require_once("wbes/server.inc");
require_once("wbes/wbes.inc");

$wces = NULL;
$wces_verbose = false;

function wces_connect()
{
  global $wces, $wbes;
  wbes_connect();
  $wces = $wbes;
}

function wces_oldconnect()
{
  $db = server_mysqlinit();
  mysql_select_db("wces",$db);
  return $db;
}

?>
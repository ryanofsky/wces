<?

require_once("wbes/server.inc");
require_once("wbes/postgres.inc");

$db_debug = true;
$db = server_pginit("testing");

$result = pg_go("SELECT * FROM test", $db, __FILE__, __LINE__);

for($i=0; $row = @pg_fetch_array($result,$i); ++$i) 
{ 
  print("let = " . $row["let"] . "\n");
} 


?>
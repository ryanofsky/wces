<?

require_once("wbes/wbes.inc");
require_once("wbes/postgres.inc");
//exit();

function delete_response($response_id, $depth = 0)
{
  global $wbes;

  print("$response_id ");

  pg_go("DELETE FROM responses WHERE response_id = $response_id", $wbes, __FILE__, __LINE__);
  $r = pg_go("SELECT DISTINCT response_id FROM responses WHERE parent = $response_id", $wbes, __FILE__, __LINE__);
  
  $n = pg_numrows($r);
  
  for ($i = 0; $i < $n; ++$i)
  {
    $child = (int)pg_result($r, $i, 0);
    delete_response($child, $depth + 1); 
  }
}

$db_debug = true;
wbes_connect();
//delete_response(217850);
//delete_response(4859987);
//delete_response(228383);
//delete_response(228405);
//delete_response(452469);
delete_response(474989);
delete_response(475001);
delete_response(475013);
delete_response(475025);
delete_response(475037);
delete_response(475088);
delete_response(475101);
delete_response(475114);
delete_response(475127);

?>
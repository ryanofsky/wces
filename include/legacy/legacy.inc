<?

require_once("wces/database.inc");

function legacy_info()
{
  static $professor_id = null;
  static $db = null; 

  if (!isset($db)) $db = wces_oldconnect();

  if (!isset($professor_id))
  {
    wces_connect(); global $wces;
    $user_id = (int)login_getuserid();
    $result = pg_go("SELECT oldid FROM temp_prof WHERE newid = $user_id", $wces, __FILE__, __LINE__);
    $professor_id = pg_numrows($result) == 1 ? (int) pg_result($result,0,0) : 0;
  }
  return array($db, $professor_id);
}

// from wces/oldquestions.inc
$legacy_TAQUESTIONS = array
(
  "overall" => "Overall Quality",
  "knowledgeability" => "Knowledgeability",
  "approachability" => "Approachability",
  "availability" => "Availability",
  "communication" => "Communication"
);

?>
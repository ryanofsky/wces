<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/usage.inc");

LoginProtect(LOGIN_ADMIN);

$f =& new Form(null, $_SERVER['PHP_SELF']);
$u =& new StudentUsage('usage', $f);
$f->loadState();

$displayname = "";
if ($u->options->question_period_id)
{
  // get information about selected question period
  $result = pg_go("
    SELECT displayname
    FROM question_periods
    WHERE question_period_id = {$u->options->question_period_id}
  ", $wces, __FILE__, __LINE__);

  $displayname = pg_fetch_result($result,0,0);
}

page_top("Student Usage Data" . ($displayname ? " for $displayname" : ""));

$u->display();

page_bottom();
?>

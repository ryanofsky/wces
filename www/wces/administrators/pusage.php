<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/usage.inc");

LoginProtect(LOGIN_ADMIN);

$f =& new Form(null, $_SERVER['PHP_SELF']);
$u =& new ProfessorUsage('usage', $f);
$f->loadState();

// get information about selected question period
$result = pg_go("
  SELECT displayname
  FROM semester_question_periods
  WHERE question_period_id = {$u->options->question_period_id}
", $wces, __FILE__, __LINE__);

extract(pg_fetch_array($result,0,PGSQL_ASSOC));

page_top("Professor Usage Data for $displayname");

$u->display();


page_bottom();
?>

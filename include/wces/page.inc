<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("widgets/html.inc");
require_once($WCES_CONFIG_PAGE_INCLUDE);

define('WCES_PAGE_NORMAL', 1);
define('WCES_PAGE_PRINTABLE', 2);
define('WCES_PAGE_BARE', 3);

class WcesPage
{
  var $mode = WCES_PAGE_NORMAL;
  
  // virtual methods:
  //
  // function printTop($title);
  // function printBottom();
  // function printHelp($topic);
}

function & WcesPageInstance()
{
  if (!isset($GLOBALS['WcesPageInstance']))
    $GLOBALS['WcesPageInstance'] =& new $GLOBALS['WCES_CONFIG_PAGE'];
  return $GLOBALS['WcesPageInstance'];
}

// for backward compatibility
function page_top($title, $hidden = false)
{
  $page =& WcesPageInstance();
  if ($hidden) $page->mode = PAGE_PRINTABLE;
  $page->printTop($title);
}

function page_bottom()
{
  $page =& WcesPageInstance();
  $page->printBottom();
}

$wces_path = $WCES_CONFIG_PATH;


function get_surveys()
{
  global $wces;
  static $survey_listing;
  
  if (!isset($survey_listing))
  {
    wces_connect();
    $login = LoginInstance();
    $user_id = $login->get('user_id');
    $survey_listing = pg_go("
      SELECT q.question_period_id, t.topic_id, t.item_id, t.specialization_id, t.class_id, get_class(t.class_id) AS name,
        CASE WHEN sr.user_id IS NULL THEN 0 ELSE 1 END AS surveyed
      FROM question_periods AS q
      INNER JOIN wces_topics AS t USING (question_period_id)
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = cl.class_id AND e.status = 1
      LEFT JOIN survey_responses AS sr ON sr.topic_id = t.topic_id AND sr.user_id = $user_id
      WHERE q.begindate < (SELECT NOW()) AND q.enddate > (SELECT NOW()) AND NOT t.cancelled
      GROUP BY q.question_period_id, t.topic_id, t.item_id, t.specialization_id, t.class_id, sr.user_id
      ORDER BY surveyed, name
    ", $wces, __FILE__, __LINE__);      
    
    if (pg_numrows($survey_listing) == 0)
    {
      $result = pg_go("
        SELECT EXISTS(SELECT * FROM semester_question_periods 
          WHERE begindate < (SELECT NOW()) AND enddate > (SELECT NOW()))
      ", $wces, __FILE__, __LINE__);
      if (pg_result($result,0,0) == 'f') $survey_listing = false;
    }
  }
  return $survey_listing;
}


?>

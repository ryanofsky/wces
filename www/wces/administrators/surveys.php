<?

require_once("wbes/component_text.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_pagebreak.inc");
require_once("wces/component_abet.inc");

require_once("wbes/server.inc");
require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/wces.inc");

login_protect(login_administrator);

define("TopicEditor_choose", 1);

$survey_debug = false;
$db_debug = 0;

class TopicEditor extends StatefullWidget
{
  // output
  var $barePage = false;

  // widgets
  var $editor;

  function TopicEditor($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->event->shortName(TopicEditor_choose, "topic_id");
  }

  function & handleEvent($event, $param, $new)
  {
    global $base_branch_id, $user_id, $factories;
    if ($event == TopicEditor_choose)
    {
      SurveyEditor::DumpScript();
      $this->editor =& new SurveyEditor($param, get_base($param), $user_id, $factories, "editor", $this);
      $this->loadChild($this->editor, $new);
      $this->barePage = $this->editor->barePage;
      return $this->editor;
    }
  }

  function printTopics($result, $class = false)
  {
    global $pagename, $ASID, $survey_debug;
    $n = pg_numrows($result);
    print("<ul>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      $style = $modified == 't' ? 'style="list-style-type: circle"' 
        : 'style="list-style-type: disc"'; 
      if ($class) $name = format_class($name);
      $no = $survey_debug ? (" ($topic_id" . ($parent ? ", $parent" : '') . ")") : '';
      print("  <li $style><a href=\""
        . $this->event->getUrl(TopicEditor_choose, $topic_id)
        . "$ASID\">$name</a>$no</li>\n");
    } 
    print("</ul>\n");         
  }

  function printVisible()
  {
    global $wces;
    wces_connect();

    if (isset($this->editor) && $this->editor->message)
      print($this->editor->message);

    print("<p>Choose a survey to edit.</p>");

    $result = pg_go("
      --SELECT topic_id, 'Base Questions' AS name
      --FROM wces_topics
      --WHERE category_id IS NULL AND class_id IS NULL
      SELECT * FROM
      (
        SELECT 1 AS topic_id, 'Base Questions' AS name, 1 AS ordinal, 't'::BOOLEAN AS modified, NULL AS parent
        UNION
        SELECT 4396, 'Base SEAS Questions', 2, 't', NULL
        UNION
        SELECT 3736, 'Base Engineering Questions', 3, 't', NULL
        UNION
        SELECT 3733, 'Base Bioengineering Questions', 4, 't', NULL
        UNION
        SELECT 3490, 'Gateway Lab Questions', 5, 't', NULL        
      ) AS t ORDER BY ordinal
    ", $wces, __FILE__, __LINE__);


    $this->printTopics($result);

    print("<h5>Individual Classes</h5>\n");

    $result = pg_go("
      SELECT t.topic_id, get_class(t.class_id) AS name, topic_modified(t.topic_id) AS modified, t.parent
      FROM question_periods AS q
      INNER JOIN question_periods_topics AS qt USING (question_period_id)
      INNER JOIN wces_topics AS t USING (topic_id)
      INNER JOIN classes AS cl USING (class_id)
      WHERE q.enddate > (select now()) AND t.category_id IS NOT NULL AND category_id <> 103
      GROUP BY t.topic_id, t.class_id, t.parent
      ORDER BY name
    ", $wces, __FILE__, __LINE__);

    $this->printTopics($result, true);
  }
};

function SimpleTop($title)
{
  global $server_media;
?>
<head>
<title><?=$title?></title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
</head>
<?
}

function SimpleBottom()
{}

login_protect(login_administrator);

$user_id = login_getuserid();
$factories = array
(
  new ChoiceFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory(),
  new AbetFactory(),
  new NewAbetFactory(),
  new BioAbetFactory()
);

$f = & new Form("f");
$t = & new TopicEditor("topics", $f);
$f->loadState();

if ($t->barePage)
  list($pageTop, $pageBottom) = array("SimpleTop", "SimpleBottom");
else
  list($pageTop, $pageBottom) = array("page_top", "page_bottom");

$pageTop("Survey Builder");
print("<form name=$f->formName method=post action=\"$f->pageName\">");
print($ISID);
$f->display();
$t->display();
print("</form>");
$pageBottom();

?>
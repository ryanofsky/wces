<?

require_once("wbes/surveyeditor.inc");
require_once("wces/login.inc");

LoginProtect(LOGIN_ADMIN);

define("TopicEditor_choose", 1);

class TopicEditor extends StatefullWidget
{
  // output
  var $barePage = false;

  // widgets
  var $editor;
  var $question_period;
  
  var $error = '';

  function TopicEditor($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->shortName('item_id'); 
    $this->shortName('specialization_id');
  }

  function & handleEvent($event, $param, $new)
  {
    if ($event == TopicEditor_choose)
    {
      $item_id = (int)$param[0];
      $specialization_id = (int)$param[1];
      SurveyEditor::DumpScript();
      $this->editor =& new SurveyEditor($item_id, $specialization_id, LoginValue('user_id'), 'editor', $this);
      $this->loadChild($this->editor, $new);
      $this->barePage = $this->editor->barePage;
      return $this->editor;
    }
  }

  function & loadState($new)
  {
    StatefullWidget::loadState($new);
    
    if ($new) return;
    
    $item_id = (int)$this->readValue('item_id');
    $specialization_id = (int)$this->readValue('specialization_id');
    if ($item_id && $specialization_id)
      $this->eventLoop(TopicEditor_choose, array($item_id, $specialization_id), true);
  }

  function printTopics($result)
  {
    global $pagename, $ASID, $survey_debug;
    $n = pg_numrows($result);
    print("<ul>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      $style = $modified == 't' ? 'style="list-style-type: circle"'
        : 'style="list-style-type: disc"';

      $no = $survey_debug ? (" ($specialization_id" . ($parent ? ", $parent" : '') . ")") : '';
      print("  <li $style><a href=\""
        . $this->getUrl(array('item_id' => $item_id, 'specialization_id' => $specialization_id))
        . "$ASID\">$name</a>$no</li>\n");
    }
    print("</ul>\n");
    return $n > 0;
  }

  function printVisible()
  {
    global $wces, $year, $semester;
    wces_connect();

    if (isset($this->editor) && $this->editor->message)
      print($this->editor->message);

    if ($this->error) print($this->error);

    print("<h4>Base Surveys</h4>\n");
    print("<p><i>Changes made to the base surveys will be automatically merged into the department surveys.</i></p>");

    $result = pg_go("
      SELECT 'Pre-presentation survey' AS name, 3566 AS specialization_id, 943 AS item_id, 't'::bool AS modified
      UNION
      SELECT 'Post-presentation survey', 3567, 944, 't'::bool
      ORDER BY specialization_id
    ", $wces, __FILE__, __LINE__);

    $this->printTopics($result);
    
    print("<h4>Department Surveys</h4>\n");
  
    $n = pg_numrows($result);
    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($result, $i, PGSQL_ASSOC));
      wces_connect();
  
      $r = pg_go("
        SELECT t.item_id, t.specialization_id, d.department_name AS name,
          specialization_modified(t.item_id, t.specialization_id) AS modified
        FROM gateway_topics AS t 
        NATURAL JOIN gateway_departments AS d
        INNER JOIN specializations AS s ON s.specialization_id = t.specialization_id
        WHERE s.parent = $specialization_id AND t.year = $year AND t.semester = $semester
        ORDER BY d.ordinal
      ", $wces, __FILE__, __LINE__);
    
      print("<h5>$name</h5>\n");
      $this->printTopics($r);
    }
  }
};

////////////////////////////
//////////////////////
///////

////////////////////////////////////////////////////////
//////////////////////////////////

///////////////////////////////// ///////////////////// ///////////////////////
//////////////////////////////////////////////

//////////////////////////

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

$f =& new Form("f");
$t =& new TopicEditor("topics", $f);
$f->loadState();

if ($t->barePage)
  list($pageTop, $pageBottom) = array("SimpleTop", "SimpleBottom");
else
  list($pageTop, $pageBottom) = array("page_top", "page_bottom");

$pageTop("Survey Builder");
print("<form name=$f->formName method=post action=\"$f->pageName\">");
print($AISID);
$f->display();
$t->display();
print("</form>");
$pageBottom();

?>

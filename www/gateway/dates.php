<?

require_once("wces/login.inc");

LoginProtect(LOGIN_ADMIN);

function format_date($timestamp, $time = true)
{
  return $timestamp ? date("m/d/Y" . ($time ? " g:i a" : ""), $timestamp) : "";
}

function parse_date($str)
{
  if (strlen($str) == 0)
     return false;
  else if (($timestamp = strtotime(str_replace("-","/",$str))) === -1)
    return false;
  else
    return $timestamp;
}

define("QuestionPeriodEditor_save", 1);
define("QuestionPeriodEditor_cancel", 2);

$dartSemesters = array("Spring", "Summer", "Fall");

class QuestionPeriodEditor extends StatefullWidget
{
  var $done = false;
  var $message = "";
  var $errors = array();
  
  function QuestionPeriodEditor($topic_id, $name, &$parent)
  {
    global $dartSemesters;
    $this->topic_id = (int)$topic_id;
    $this->StatefullWidget($name, $parent);
    $this->topic_id = (int)$topic_id;
    $this->beginDate =& new TextBox(0, 30, "", "beginDate", $this);
    $this->endDate =& new TextBox(0, 30, "", "endDate", $this);
    $this->year =& new TextBox(0, 4, "", "year", $this);
    $this->semester =& new DropBox($dartSemesters, "semester", $this);
  }

  function loadState($new)
  {
    assert(isset($new));
    StatefullWidget::loadState($new);
    
    if (!$new) return;
    
    global $wces, $semester, $year;

    if ($this->topic_id > 0)
    {
      wces_connect();     
      $r = pg_go("
        SELECT EXTRACT(EPOCH FROM begin_date) AS begindate, 
          EXTRACT(EPOCH FROM end_date) AS enddate, year, semester
        FROM gateway_topics
        WHERE topic_id = $this->topic_id
        ORDER BY begindate
      ", $wces, __FILE__, __LINE__);
      assert(pg_numrows($r) == 1);
      extract(pg_fetch_row($r, 0, PGSQL_ASSOC));
      $this->beginDate->text = format_date($begindate);
      $this->endDate->text = format_date($enddate);
      $this->year->text = $year;
      $this->semester->selected = $semester;
    }
    else
    {
      $y = date("Y");
      $this->beginDate->text = "1/1/$y 12:00 am";
      $this->endDate->text = "1/31/$y 12:00 am";
      $this->year->text = $y;
    }
  }
  
  function save()
  {
    global $wces;
    wces_connect();

    $bd = parse_date($this->beginDate->text);
    $ed = parse_date($this->endDate->text);

    if (!is_numeric($this->year->text)) $this->errors[] = "Invalid year";
    if ($bd === false) $this->errors[] = "Unable to parse begin date";
    if ($ed === false) $this->errors[] = "Unable to parse end date";
    
    if (count($this->errors)) return false;

    $bd = nulldate($bd);
    $ed = nulldate($ed);
    
    $yr = (int)($this->year->text);
    $sm = (int)($this->semester->selected);
    if ($this->topic_id <= 0)
    {
      $this->errors[] = "Invalid topic";
      return false;
    }
    else
    {
      return (bool)pg_go("
        UPDATE gateway_topics SET
          begin_date = $bd, end_date = $ed, semester = $sm, year = $yr
        WHERE topic_id = $this->topic_id
      ", $wces, __FILE__, __LINE__);  
    }
  }
  
  function & handleEvent($event, $param, $isNew)
  {
    switch ($event)
    {
      case QuestionPeriodEditor_save:
        if ($this->save())
        {
          $this->done = true;
          $this->message = "<p><font color=blue>Changes saved successfully</font></p>";
        }     
      break;

      case QuestionPeriodEditor_cancel:
        $this->done = true;
        $this->message = "<p><font color=red>No changes were saved.</font></p>";
      break;
    };
  }
  
  function display()
  {
    global $dart_semesters;
    if (count($this->errors) > 0)
    {
      print("<p>Please correct the following errors:</p>\n<ul>\n");
      foreach($this->errors as $e)
        print("  <li>$e</li>\n");
      print("</ul>\n");
    }
?>
<table>
<tr><td>Term:</td><td><? $this->semester->display(); $this->year->display(); ?></td></tr>
<tr><td>Begin Time:</td><td><? $this->beginDate->display(); ?></td></tr>
<tr><td>End Time:</td><td><? $this->endDate->display(); ?></td></tr>
<tr><td>&nbsp;</td><td><? $this->event->displayButton("Save", QuestionPeriodEditor_save); $this->event->displayButton("Cancel", QuestionPeriodEditor_cancel); ?></td></tr>
</table>
<?
  }  
};

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
    $this->shortName('topic_id'); 
  }

  function & handleEvent($event, $param, $new)
  {
    if ($event == TopicEditor_choose)
    {
      $this->editor =& new QuestionPeriodEditor((int)$param, 'qp', $this);
      $this->loadChild($this->editor, $new);
      return $this->editor;
    }
  }

  function & loadState($new)
  {
    StatefullWidget::loadState($new);
    
    if ($new) return;
    
    $topic_id = (int)$this->readValue('topic_id');
    if ($topic_id)
      $this->eventLoop(TopicEditor_choose, $topic_id, true);
  }

  function printTopics($result)
  {
    global $pagename, $ASID, $survey_debug;
    $n = pg_numrows($result);
    print("<ul>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));

      $no = $survey_debug ? (" ($specialization_id" . ($parent ? ", $parent" : '') . ")") : '';
      print("  <li><a href=\""
        . $this->getUrl(array('topic_id' => $topic_id))
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

    $result = pg_go("
      SELECT 'Pre-presentation survey' AS name, 3566 AS specialization_id, 943 AS item_id
      UNION
      SELECT 'Post-presentation survey', 3567, 944
      ORDER BY specialization_id
    ", $wces, __FILE__, __LINE__);

    $n = pg_numrows($result);
    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($result, $i, PGSQL_ASSOC));
      wces_connect();
  
      $r = pg_go("
        SELECT t.topic_id, d.department_name AS name
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

page_top("Question Periods");

$f =& new Form('form');
$t =& new TopicEditor("topics", $f);
$f->loadState();

print("<form name=$f->formName method=post action=\"$f->pageName\">");
print($AISID);
$f->display();
$t->display();
print("</form>");
page_bottom();


?>

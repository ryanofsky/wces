<?

define("TOPICS_LITERAL", 1);
define("TOPICS_CLASS", 2);
define("TOPICS_COURSE", 3);

require_once('widgets/basic.inc');
require_once('wbes/surveyeditor.inc');
require_once('config/topics.inc');


define("TopicEditor_choose", 1);

$survey_debug = false;
$db_debug = 0;

class TopicEditor extends StatefullWidget
{
  // output
  var $barePage = false;

  // widgets
  var $editor;
  var $question_period;
  
  function TopicEditor($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->event->shortName(TopicEditor_choose, array('specialization_id', 'item_id'));
    $this->question_period =& new QuestionPeriodSelector("Display surveys for", "qps", $this);
  }

  function & handleEvent($event, $param, $new)
  {
    global $base_branch_id, $user_id, $factories;
    if ($event == TopicEditor_choose)
    {
      SurveyEditor::DumpScript();
      $this->editor =& new SurveyEditor($param[1], $param[0], $user_id, 'editor', $this);
      $this->loadChild($this->editor, $new);
      $this->barePage = $this->editor->barePage;
      return $this->editor;
    }
  }

  function printTopics($result, $nameFormat)
  {
    global $pagename, $ASID, $survey_debug;
    $n = pg_numrows($result);
    print("<ul>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      $style = $modified == 't' ? 'style="list-style-type: circle"' 
        : 'style="list-style-type: disc"'; 
        
      if ($nameFormat == TOPICS_CLASS)
        $name = format_class($name);
      else if ($nameFormat == TOPICS_COURSE)
        $name = format_course($name);

      $no = $survey_debug ? (" ($specialization_id" . ($parent ? ", $parent" : '') . ")") : '';
      print("  <li $style><a href=\""
        . $this->event->getUrl(TopicEditor_choose, array($specialization_id, $item_id))
        . "$ASID\">$name</a>$no</li>\n");
    } 
    print("</ul>\n");         
  }

  function printVisible()
  {
    global $wces;
    wces_connect();

    $this->question_period->display();

    if (isset($this->editor) && $this->editor->message)
      print($this->editor->message);

    print("<p>Choose a survey to edit.</p>\n");

    $list = topics_list(get_question_period());
    
    foreach ($list as $row)
    {
      list($heading, $sql, $mode) = $row; 
      print($heading);
      $result = pg_go($sql, $wces, __FILE__, __LINE__);
      $this->printTopics($result, $mode);
    }
  }
};

class QuestionPeriodSelector extends ParentWidget
{
  var $box;
  var $event;
  var $text;
  
  function QuestionPeriodSelector($text, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->text = $text;
    $sql = "
      SELECT question_period_id AS id, displayname AS name
      FROM semester_question_periods
      ORDER BY begindate DESC
     ";
    $this->box =& new SqlBox($sql, false, 'box', $this);
    $this->event =& new EventWidget('event', $this);
  }
  
  function printVisible()
  {
    print($this->text);
    print("&nbsp;");
    $this->box->display();
    print("&nbsp;");
    $this->event->displayButton("Update", 1);
    print("<br>\n");
  }

  function loadInitialState()
  {
    ParentWidget::loadInitialState();
    $this->box->selected = get_question_period();
  }
  
  function loadState()
  {
    ParentWidget::loadState();
    if ($this->event->event)
      set_question_period($this->box->selected);
    else
      $this->box->selected = get_question_period();
  }
}

?>
<?

define("TOPICS_LITERAL", 1);
define("TOPICS_CLASS", 2);
define("TOPICS_COURSE", 3);

require_once('widgets/basic.inc');
require_once('widgets/SqlBox.inc');
require_once('wbes/surveyeditor.inc');
require_once('config/topics.inc');

define("TopicEditor_choose", 1);

$survey_debug = false;

class TopicEditor extends StatefullWidget
{
  // output
  var $barePage = false;

  // widgets
  var $editor;
  var $question_period;
  
  var $error = '';

  function TopicEditor($professor_mode, $name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    if (!$professor_mode)
      $this->question_period =& new QuestionPeriodSelector("Display surveys for", "qps", $this);
    $this->shortName('item_id'); 
    $this->shortName('specialization_id');
    $this->professor_mode = (bool)$professor_mode;
  }

  function & handleEvent($event, $param, $new)
  {
    if ($event == TopicEditor_choose)
    {
      $item_id = (int)$param[0];
      $specialization_id = (int)$param[1];
      SurveyEditor::DumpScript();
      if ($this->professor_mode)
      {
        $user_id = (int)LoginValue('user_id');
        global $wces;
        wces_connect();
        $r = pg_result(pg_go("
          SELECT EXISTS 
          ( SELECT * FROM enrollments_p AS e
            INNER JOIN wces_topics AS t USING (class_id)
            WHERE e.user_id = $user_id
               AND t.item_id = $item_id
               AND t.specialization_id = $specialization_id
          )
        ", $wces, __FILE__, __LINE__), 0, 0);
        if ($r != 't')
        {
          $this->error = "<p><font color=red>Invalid Survey</font></p>"; 
          return null;
        } 
      }
      $login =& LoginInstance();
      $this->editor =& new SurveyEditor($item_id, $specialization_id, $login->get('user_id'), 'editor', $this);
      $this->loadChild($this->editor, $new);
      $this->barePage = $this->editor->barePage;
      return $this->editor;
    }
  }

  function loadState($new)
  {
    StatefullWidget::loadState($new);
    if ($new) return;
    
    $item_id = $this->readValue('item_id');
    $specialization_id = $this->readValue('specialization_id');
    if ($item_id && $specialization_id)
      $this->eventLoop(TopicEditor_choose, array($item_id, $specialization_id), true);
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
        . $this->getUrl(array('item_id' => $item_id, 'specialization_id' => $specialization_id))
        . "$ASID\">$name</a>$no</li>\n");
    }
    print("</ul>\n");
    return $n > 0;
  }

  function printVisible()
  {
    global $wces;
    wces_connect();

    if ($this->question_period) $this->question_period->display();

    if (isset($this->editor) && $this->editor->message)
      print($this->editor->message);

    if ($this->error) print($this->error);

    print("<p>Choose a survey to edit.</p>\n");

    if (!$this->professor_mode)
      $list = topics_list(get_question_period());
    else
    {
      $user_id = (int)LoginValue('user_id');
      $list = array
      ( array
        ( "", "
            SELECT t.item_id, t.specialization_id, get_class(t.class_id) AS name, specialization_modified(t.item_id, t.specialization_id) AS modified
            FROM enrollments_p AS e
            INNER JOIN wces_topics AS t USING (class_id)
            INNER JOIN question_periods AS s USING (question_period_id)
            WHERE e.user_id = $user_id AND s.enddate > (SELECT NOW())
            ORDER BY name
          ", TOPICS_CLASS
        )
      );
    }

    $printed = false;
    foreach ($list as $row)
    {
      list($heading, $sql, $mode) = $row;
      print($heading);
      $result = pg_go($sql, $wces, __FILE__, __LINE__);
      if ($this->printTopics($result, $mode)) $printed = true;
    }
    if (!$printed) print("<p><i>No surveys found</i></p>\n");
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
      FROM question_periods
      ORDER BY begindate DESC
     ";
    $this->box =& new SqlBox($sql, false, 'box', $this);
    $this->event =& new EventWidget('event', $this);
  }

  function printVisible()
  {
    if ($this->text)
    {
      print($this->text);
      print("&nbsp;");
    }
    $this->box->selected = get_question_period();
    $this->box->display();
    print("&nbsp;");
    $this->event->displayButton("Refresh", 1);
    print("<br>\n");
  }

  function loadState($new)
  {
    ParentWidget::loadState($new);
    
    if ($this->event->event)
      set_question_period($this->box->selected);
    else
      $this->box->selected = get_question_period();
  }  
}

?>

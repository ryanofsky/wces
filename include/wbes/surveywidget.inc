<?

require_once("widgets/basic.inc");

define("SurveyWidget_save",1);
define("SurveyWidget_cancel",2);
define("SurveyWidget_next",3);
define("SurveyWidget_previous",4);

class SurveyWidget extends ParentWidget
{
  // parameters
  var $topic_id, $base_branch_id, $user_id, $question_period_id, $factories;
  var $backup = true;

  // output 
  var $done = false;
  var $failure = false; // failure on save
  var $message = "";

  // persistent 
  var $page = 0;
  var $survey = null;
  var $showErrors = false;
  var $sawLastPage = false;

  // widgets  
  var $ta = null;
  var $event;
  var $questionWidgets;
  
  // private
  var $isStale = true;

  // to add a ta section to the survey, assign an instance of TASurvey to this member:
  // $abc = & new SurveyWidget(...);
  // $abc->ta = & new TASurvey(...);

  function SurveyWidget($topic_id, $base_branch_id, $user_id, $question_period_id, &$factories, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->topic_id           = (int)$topic_id;
    $this->base_branch_id     = (int)$base_branch_id;
    $this->user_id            = (int)$user_id;
    $this->question_period_id = (int)$question_period_id;
    $this->factories =& $factories;
    $this->event =& new EventWidget("event", $this);
  }

  function loadInitialState()
  {
    ParentWidget::loadInitialState();
    $this->isStale = false;
    if (!$this->survey) $this->loadSurvey();
    $this->makeWidgets(true);
  } 

  function loadState()
  {
    ParentWidget::loadState();
    $this->survey = unserialize((string)$this->readValue("survey"));
    $this->showErrors= (bool)$this->readValue("showErrors");
    $this->page = (int)$this->readValue("page");
    $this->sawLastPage = (bool)$this->readValue("sawLastPage");
    $this->isStale = true;
    $this->makeWidgets(false);

    if ($this->event->event == SurveyWidget_save)
      $this->saveResponses();
    else if ($this->event->event == SurveyWidget_cancel)
    {
      $this->message = "<p><font color=red>Responses Not Saved</font></p>";
      $this->done = true;
    }
    else if ($this->event->event == SurveyWidget_next)
      ++$this->page;
    else if ($this->event->event == SurveyWidget_previous)
      --$this->page;
  }

  function loadSurvey()
  {
    global $wbes;
    wbes_connect();
    $this->survey =& new Survey();
    $this->survey->load($this->topic_id, $this->base_branch_id, $this->factories);
  }

  function makeWidgets($newLoad)
  {
    assert(get_class($this->survey) == "survey");
    
    $page = 0;
    foreach(array_keys($this->survey->components) as $k)
    {
      if ("pagebreak" == get_class($this->survey->components[$k])) ++$page;
      $c = & $this->survey->components[$k]->getwidget("c$k", $this);
      $c->page = $page;
      $this->loadChild($c, $newLoad);
      $this->questionWidgets[] = & $c;
    }

    if ($this->ta)
    {
      $this->ta->page = $page;
      $this->loadChild($c, $newLoad);
      $this->questionWidgets[] = &$this->ta;
    }

    $this->pages = $page;
  }

  function saveResponses()
  {
    global $wbes, $HTTP_POST_VARS, $server_feedback;

    $anonymous = true; // todo: decide how to make this this configurable
    global $wbes;

    $this->showErrors = true;
    foreach(array_keys($this->questionWidgets) as $key)
      if (count($this->questionWidgets[$key]->errors) > 0) return;

    wbes_connect();
    $sql = "BEGIN;\n";
    if ($anonymous)
    {
      $sql .= "INSERT INTO survey_responses(revision_id, question_period_id, response_id, topic_id, user_id) VALUES ({$this->survey->orig_id}, $this->question_period_id, NULL, $this->topic_id, $this->user_id);\n";
      $sql .= "INSERT INTO survey_responses(revision_id, question_period_id, date, topic_id) VALUES ({$this->survey->orig_id}, $this->question_period_id, NULL, $this->topic_id);\n";
    }
    else
      $sql .= "INSERT INTO survey_responses(revision_id, question_period_id, topic_id, user_id) VALUES ({$this->survey->orig_id}, $question_period_id, $this->topic_id, $this->user_id);\n";

    $sql .= "SELECT currval('response_ids')";
    $result = pg_go($sql, $wbes, __FILE__, __LINE__);

    $response_id = (int)pg_result($result, 0, 0);

    $failure = false;

    foreach(array_keys($this->questionWidgets) as $key)
      $failure = $failure || !$this->questionWidgets[$key]->save($response_id);

    $fields = "'" . addslashes(serialize($HTTP_POST_VARS)) . "'";

    // temporary, ensure that even if results not saved correctly, they will still
    // be recoverable
    //
    // CREATE TABLE safe_backup (
    //   user_id INTEGER,
    //   topic_id INTEGER,
    //   question_period_id INTEGER,
    //   date DATETIME DEFAULT 'now',
    //   form_vals TEXT
    // );
    //
    if ($this->backup)
    {
      $failure = $failure || !pg_go("
        INSERT INTO safe_backup (user_id, topic_id, question_period_id, form_vals)
        VALUES ($this->user_id, $this->topic_id, $this->question_period_id, $fields)
      ", $wbes, __FILE__, __LINE__);
    }

    if ($failure)
      pg_go("ROLLBACK", $wbes, __FILE__, __LINE__);
    else
      $failure = $failure || !pg_go("COMMIT", $wbes, __FILE__, __LINE__);

    if ($failure)
    {
      $this->failure = true;
      print('<p><strong><font size="+2"><font color=red>Warning:</font> Errors were encountered during the save process. Please report this to <a href="mailto:' . $server_feedback . '">' . $server_feedback . '</a>. Include your user name and text of any error messages that preceded this one.</font></strong></p>' . "\n");
    }
    else
      $this->message = "<p><font color=blue>Responses Saved</font></p>";

    $this->done = true;
  }

  function printState()
  {
    $this->printValue("survey", serialize($this->survey));
    $this->printValue("page", $this->page);
    $this->printValue("showErrors", (int)$this->showErrors);    
    $this->printValue("sawLastPage", (int)$this->sawLastPage);
  }

  function printVisible()
  {
    global $wces_path;

    if ($this->isStale) $this->printNavigation();

    if (count($this->questionWidgets) == 0)
      print("<p><i>This survey contains no questions.</i></p>");

    if ($this->showErrors)
    {
      $errortext = "";
      foreach(array_keys($this->questionWidgets) as $i)
      {
        $q = &$this->questionWidgets[$i];
        $q->showErrors = true;
        foreach($q->errors as $error)
          $errortext .= "  <li>$error</li>\n";
      }
      if ($errortext) print("<p><strong>The form is not complete. The missing fields are highlighted in <font color=red>red</font>.</strong></p>\n<ul>\n$errortext</ul>\n");
    }

    foreach(array_keys($this->questionWidgets) as $i)
    {
      $q = &$this->questionWidgets[$i];
      if ($q->page == $this->page)
      {
        print("<div>");
        $q->display();
        print("</div>\n<br>\n");
      }
      else
        $q->displayHidden();
    }

    print("<p>&nbsp;</p>\n");
    $this->printNavigation();
  }

  function printNavigation()
  {
    global $wces_path;

    $prev = $this->page > 0;
    $next = $this->page < $this->pages && $this->pages > 0;
    if (!$next) $this->sawLastPage = true;
    $save = ((bool)$this->user_id) && $this->sawLastPage;

    if ($save || $prev || $next)
    {
?>
<table width="100%" cellpadding=5 cellspacing=0>
<tr>
  <td align=center bgcolor="#B5CFE7">
    <? if ($prev) $this->event->displayButton("Previous Page", SurveyWidget_previous, false, ""); ?>
    <? if ($next) $this->event->displayButton("Next Page", SurveyWidget_next, false, ""); ?>
    <? if ($save) $this->event->displayButton("Save responses", SurveyWidget_save, false, ""); ?>
    <? if ($save) $this->event->displayButton("Return without saving", SurveyWidget_cancel, false, ""); ?>
  </td>
</tr>
</table>
<?
    }
  }
}

?>

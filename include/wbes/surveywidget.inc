<?

require_once("widgets/basic.inc");

define("SurveyWidget_save",1);
define("SurveyWidget_cancel",2);
define("SurveyWidget_next",3);
define("SurveyWidget_previous",4);

class SurveyWidget extends ParentWidget
{
  // parameters
  var $specialization_id, $base_branch_id, $user_id, $question_period_id;
  var $backup = true;
  var $anonymous = false;
  var $replacements = false;
  var $save_id = false;

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
  var $questionWidgets = array();

  // private
  var $isStale = true;
  var $pages = 0;

  // to add a ta section to the survey, assign an instance of TASurvey to $this->ta:
  // $abc =& new SurveyWidget(...);
  // $abc->ta =& new TASurvey(...);

  function SurveyWidget($item_id, $specialization_id, $user_id, $topic_id, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->item_id            = (int)$item_id;
    $this->specialization_id  = (int)$specialization_id;
    $this->user_id            = (int)$user_id;
    $this->topic_id = (int)$topic_id;
    $this->event =& new EventWidget("event", $this);
  }

  function loadInitialState()
  {
    ParentWidget::loadInitialState();
    $this->isStale = false;
    global $survey_factories;
    if (!$this->survey) $this->survey =& LoadSurvey($this->item_id, $this->specialization_id, $survey_factories);
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

  function makeWidgets($newLoad)
  {
    assert(get_class($this->survey) == "survey");
    $page = 0;
    foreach(array_keys($this->survey->components) as $k)
    {
      if ("pagebreak" == get_class($this->survey->components[$k])) ++$page;
      $c =& $this->survey->components[$k]->getwidget("c$k", $this);
      $c->page = $page;
      $this->loadChild($c, $newLoad);
      $this->questionWidgets[] = & $c;
    }

    if ($this->ta)
    {
      $this->ta->page = $page;
      $this->questionWidgets[] = &$this->ta;
    }

    $this->pages = $page;

    if (count($this->questionWidgets) == 0)
    {
      $this->message = "<p><font color=red>Survey is empty.</font></p>";
      $this->done = true;
      return;
    }
  }

  function saveResponses()
  {
    global $wbes, $_POST, $server_feedback;

    global $wbes;

    $this->showErrors = true;
    foreach(array_keys($this->questionWidgets) as $key)
      if (count($this->questionWidgets[$key]->errors) > 0) return;

    $s =& $this->survey;

    wbes_connect();
    $sql = "BEGIN;\n";
    if ($this->anonymous)
    {
      $sql .= "INSERT INTO survey_responses(revision_id, item_id, topic_id, response_id, specialization_id, user_id) VALUES ($s->revision_id, $s->item_id, $this->topic_id, NULL, $this->specialization_id, $this->user_id);\n";
      $sql .= "INSERT INTO survey_responses(revision_id, item_id, topic_id, date, specialization_id) VALUES ($s->revision_id, $s->item_id, $this->topic_id, NULL, $this->specialization_id);\n";
    }
    else
      $sql .= "INSERT INTO survey_responses(revision_id, item_id, topic_id, specialization_id, user_id) VALUES ($s->revision_id, $s->item_id, $this->topic_id, $this->specialization_id, $this->user_id);\n";

    $sql .= "SELECT currval('response_ids')";
    $result = pg_go($sql, $wbes, __FILE__, __LINE__);

    $response_id = (int)pg_result($result, 0, 0);

    $failure = false;

    foreach(array_keys($this->questionWidgets) as $key)
      $failure = $failure || !$this->questionWidgets[$key]->save($response_id);

    $fields = "'" . addslashes(serialize($_POST)) . "'";

    // temporary, ensure that even if results not saved correctly, they will still
    // be recoverable
    //
    // CREATE TABLE safe_backup (
    //   user_id INTEGER,
    //   topic_id INTEGER,
    //   item_id INTEGER,
    //   specialization_id INTEGER,
    //   date DATETIME DEFAULT 'now',
    //   form_vals TEXT
    // );
    //
    if ($this->backup)
    {
      $failure = $failure || !pg_go("
        INSERT INTO safe_backup (user_id, topic_id, item_id, specialization_id, form_vals)
        VALUES ($this->user_id, $this->topic_id, $this->item_id, $this->specialization_id, $fields)
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

    print("<p>Instructions: Fill in the form below and press the \"Submit Evaluation\" button when you are done. Note that you will not be able to go back and edit any part of the evaluation once it is submitted.</p>");

    if ($this->showErrors)
    {
      $errortext = "";
      foreach(array_keys($this->questionWidgets) as $i)
      {
        $q = &$this->questionWidgets[$i];
        $q->showErrors = true;
        if (isset($q->errors))
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
        if ($this->replacements) ob_start();
        $q->display();
        if ($this->replacements)
        {
          $contents = ob_get_contents();
          ob_end_clean();
          print(str_replace(array_keys($this->replacements), array_values($this->replacements), $contents));
        }
        
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
  <td align=center bgcolor="#68ACFF">
    <? if ($prev) $this->event->displayButton("Previous Page", SurveyWidget_previous, false, ""); ?>
    <? if ($next) $this->event->displayButton("Next Page", SurveyWidget_next, false, ""); ?>
    <? if ($save) $this->event->displayButton("Submit Evaluation", SurveyWidget_save, false, ""); ?>
    <? if ($save) $this->event->displayButton("Cancel", SurveyWidget_cancel, false, " onclick=\"return confirm('You survey responses will be not be saved if you continue.\\n\\nPress OK to continue, or Cancel to keep filling out the survey.');\""); ?>
  </td>
</tr>
</table>
<?
    }
  }
}

?>

<?

require_once("widgets/basic.inc");

define("SurveyWidget_save",1);
define("SurveyWidget_cancel",2);
define("SurveyWidget_next",3);
define("SurveyWidget_previous",4);

class SurveyWidget extends FormWidget
{
//public:

  var $finished = false;
  var $failure = false; // failure on save

//private:

  var $topic_id;
  var $base_branch_id;
  var $user_id;
  var $question_period_id;
  var $factories;

  var $action; // ActionButton widget
  var $form; // Form widget
  var $showerrors; // boolean
  var $survey = NULL;
  var $questionwidgets;
  var $page = 0;
  
  function SurveyWidget($topic_id, $base_branch_id, $user_id, $question_period_id, $factories, $prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->topic_id           = (int)$topic_id;
    $this->base_branch_id     = (int)$base_branch_id;
    $this->user_id            = (int)$user_id;
    $this->question_period_id = (int)$question_period_id;
    $this->factories          = $factories;

    $this->action = new ActionButton("${prefix}_action",$form,$formmethod);
    $this->form = new Form("${prefix}_form",$form,$formmethod);
    $this->showerrors = false;
  }

  function loadvalues()
  {
    $this->page = (int)$this->loadattribute("page");
    if (!$this->survey)
    {
      $this->survey = @unserialize((string)$this->loadattribute("survey"));
      if (get_class($this->survey) != "survey")
        $this->load();
    }

    $this->questionwidgets = array();
    $page = 0;
    foreach(array_keys($this->survey->components) as $k)
    {
      if ("pagebreak" == get_class($this->survey->components[$k])) ++$page;
      $c = $this->survey->components[$k]->getwidget("{$this->prefix}_$k",$this->form, $this->formmethod);
      $c->page = $page;
      $this->questionwidgets[] = $c;
    }
    $this->pages = $page;

    $this->form->loadvalues();
    if($this->form->isstale)
    {
      foreach(array_keys($this->questionwidgets) as $key)
        $this->questionwidgets[$key]->loadvalues();

      $this->action->loadvalues();
      $this->handleactions();
    }
  }

  function handleactions()
  {
    if ($this->action->action == SurveyWidget_save)
      $this->save();
    else if ($this->action->action == SurveyWidget_cancel)
      $this->finished = true;
    else if ($this->action->action == SurveyWidget_next)
      ++$this->page;
    else if ($this->action->action == SurveyWidget_previous)
      --$this->page;

    $this->action->action = "";
    $this->action->object = "";
  }

  function load()
  {
    global $wbes;
    wbes_connect();
    $this->survey = new Survey();
    $this->survey->load($this->topic_id,$this->base_branch_id,$this->factories);
  }

  function save()
  {
    global $wbes, $HTTP_POST_VARS;
    
    $anonymous = true; // todo: decide how to make this this configurable
    global $wbes;

    $this->showerrors = true;
    foreach(array_keys($this->questionwidgets) as $key)
      if (count($this->questionwidgets[$key]->errors) > 0) return;

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
    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
    
    $response_id = (int)pg_result($result, 0, 0);

    $failure = false;
    
    foreach(array_keys($this->questionwidgets) as $key)
      $failure = $failure || !$this->questionwidgets[$key]->save($response_id);
    
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
    
    pg_query("
      INSERT INTO safe_backup (user_id, topic_id, question_period_id, form_vals)
      VALUES ($this->user_id, $this->topic_id, $this->question_period_id, $fields)
    ", $wbes, __FILE__, __LINE__);
    
    if ($failure)
      pg_query("ROLLBACK", $wbes, __FILE__, __LINE__);
    else
      $failure = $failure || !pg_query("COMMIT", $wbes, __FILE__, __LINE__);    
      
    if ($failure)  
    {
      $this->failure = true;
      print('<p><strong><font size="+2"><font color=red>Warning:</font> Errors were encountered during the save process. Please report this to <a href="mailto:wces@columbia.edu">wces@columbia.edu</a>. Include your user name and text of any error messages that preceded this one.</font></strong></p>' . "\n");
    }
    $this->finished = true;
  }

  function display()
  {
    global $wces_path;

    $this->printattribute("page", $this->page);

    if ($this->failure) return;

    if (count($this->questionwidgets) == 0)
    {
      print("<p><strong>No surveys can be found.</strong></p>");
      return;
    }

    $this->form->display();
    $this->action->display();

    $this->printattribute("survey",serialize($this->survey));

    if ($this->form->isstale) $this->optionbar();
    if ($this->showerrors)
    {
      $errortext = "";
      foreach($this->questionwidgets as $questionwidget)
        if (isset($questionwidget->errors))
        foreach($questionwidget->errors as $error)
          $errortext .= "  <li>$error</li>\n";
      if ($errortext) print("<p><strong>The form is not complete. The missing fields are highlighted in <font color=red>red</font>.</strong></p>\n<ul>\n$errortext</ul>\n");
    }
    if ($this->action->action) $this->action->display();

    foreach(array_keys($this->questionwidgets) as $i)
    {
      $q = &$this->questionwidgets[$i];
      if ($q->page == $this->page)
      {
        print("<div>");
        $q->display(false, $this->showerrors);
        print("</div>\n<br>\n");
      }
      else
        $q->display(true);
    }
    print("<p>&nbsp;</p>\n");
    $this->optionbar();
  }

  function optionbar()
  {
    global $wces_path;
    
    $prev = $this->page > 0;
    $next = $this->page < $this->pages && $this->pages > 0;
    $save = (bool)$this->user_id;
    
    if ($save || $prev || $next)
    {
?>
<table width="100%" cellpadding=5 cellspacing=0>
<tr>
  <td align=center bgcolor="#B5CFE7">
    <? if ($prev) $this->action->display("Previous Page", SurveyWidget_previous, false, ""); ?>
    <? if ($next) $this->action->display("Next Page", SurveyWidget_next, false, ""); ?>
    <? if ($save) $this->action->display("Save responses", SurveyWidget_save, false, ""); ?>
    <? if ($save) $this->action->display("Return without saving", SurveyWidget_cancel, false, ""); ?>
  </td>
</tr>
</table>
<?
    }
  }
}

?>
<?

require_once("widgets/basic.inc");

define("SurveyWidget_save",1);
define("SurveyWidget_cancel",2);

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
  var $survey;
  var $questionwidgets;

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
    $this->survey = unserialize((string)$this->loadattribute("survey"));
    if (get_class($this->survey) != "survey")
      $this->load();

    $this->questionwidgets = array();
    foreach(array_keys($this->survey->components) as $k)
      $this->questionwidgets[] = $this->survey->components[$k]->getwidget("{$this->prefix}_$k",$this->form, $this->formmethod);

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
    {
      $this->save();
    }
    else if ($this->action->action == SurveyWidget_cancel)
    {
      $this->finished = true;
    }

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
    global $wbes;
    
    $anonymous = true; // todo: decide how to make this this configurable
    global $wbes;

    $this->showerrors = true;
    foreach(array_keys($this->questionwidgets) as $key)
      if (count($this->questionwidgets[$key]->errors) > 0) return;

    $GLOBALS['db_debug'] = true;
    
    wbes_connect();
    $sql = "BEGIN;\n";
    if ($anonymous)
    {
      $sql .= "INSERT INTO completions(completion_id, topic_id, user_id) VALUES (NULL, $this->topic_id, $this->user_id);\n";
      $sql .= "INSERT INTO completions(date, topic_id) VALUES (NULL, $this->topic_id);\n";
    }
    else
      $sql .= "INSERT INTO completions(topic_id, user_id) VALUES ($this->topic_id, $this->user_id);\n";
    
    $sql .= "SELECT currval('completion_ids')";
    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
    
    $completion_id = pg_result($result, 0, 0);
    
    foreach(array_keys($this->questionwidgets) as $key)
      $this->questionwidgets[$key]->save($completion_id);
    
    $result = pg_query("
      COMMIT;
      INSERT INTO completions(topic_id, user_id) VALUES ($this->topic_id, $this->user_id);
      SELECT currval('completion_ids');
    ", $wbes, __FILE__, __LINE__);    
    
    $failure = false;

    // todo: write save code here

    $this->finished = true;
    if ($failure)
    {
      $this->failure = true;
      print('<p><strong><font size="+2"><font color=red>Warning:</font> Errors were encountered during the save process. Please report this to <a href="mailto:wces@columbia.edu">wces@columbia.edu</a>. Include your user name and text of any error messages that preceded this one.</font></strong></p>' . "\n");
    }
  }

  function display()
  {
    global $wces_path;

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
        foreach($questionwidget->errors as $error)
          $errortext .= "  <li>$error</li>\n";
      if ($errortext) print("<p><strong>The form is not complete. The missing fields are highlighted in <font color=red>red</font>.</strong></p>\n<ul>\n$errortext</ul>\n");
    }
    if ($this->action->action) $this->action->display();

    foreach(array_keys($this->questionwidgets) as $i)
    {
      print("<div>");
      $this->questionwidgets[$i]->display(false, $this->showerrors);
      print("</div>\n<br>\n");
    }

    print("<p>&nbsp;</p>\n");
    $this->optionbar();
  }

  function optionbar()
  {
    global $wces_path;
    if ($this->user_id)
    {
?>
<table width="100%" cellpadding=5 cellspacing=0>
<tr>
  <td align=center bgcolor=black background="<?=$wces_path?>media/0x000000.gif">
    <? $this->action->display("Save responses", SurveyWidget_save, false, ""); ?>
    <? $this->action->display("Return without saving", SurveyWidget_cancel, false, ""); ?>
  </td>
</tr>
</table>
<?
    }
  }
}

?>
<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");

$server_isproduction = false;

function format_date($timestamp, $time = true)
{
  return $timestamp ? date("m/d/Y" . ($time ? " g:i a" : ""), $timestamp) : "";
}

function parse_date($str)
{
  if (($timestamp = strtotime(str_replace("-","/",$str))) === -1)
    return false;
  else
    return $timestamp;
}

login_protect(login_administrator);

define("QuestionPeriodEditor_save", 1);
define("QuestionPeriodEditor_cancel", 2);

define("QuestionPeriodList_edit", 1);
define("QuestionPeriodList_delete", 2);

$dartSemesters = array("Spring", "Summer","Fall", "Winter");

class QuestionPeriodEditor extends FormWidget
{
  var $done = false;
  var $message = "";
  var $errors = array();
  
  function QuestionPeriodEditor($question_period_id, $prefix, $form, $formmethod)
  {
    global $dartSemesters;
    $this->question_period_id = (int)$question_period_id;
    $this->FormWidget($prefix, $form, $formmethod);
    $this->question_period_id = (int)$question_period_id;
    $this->displayName = new TextBox(0, 30, "", "{$prefix}_displayName", $form, $formmethod);
    $this->beginDate = new TextBox(0, 30, "", "{$prefix}_beginDate", $form, $formmethod);
    $this->endDate = new TextBox(0, 30, "", "{$prefix}_endDate", $form, $formmethod);
    $this->year = new TextBox(0, 4, "", "{$prefix}_year", $form, $formmethod);
    $this->semester = new DropBox($dartSemesters, "{$prefix}_semester", $form, $formmethod);
    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
    $this->profDate = new TextBox(0, 30, "", "{$prefix}_profDate", $form, $formmethod);
  }

  function loadDefaults()
  {
    global $wces;
    if ($this->question_period_id > 0)
    {
      wces_connect();     
      $r = pg_go("SELECT displayname, EXTRACT(EPOCH FROM begindate) AS begindate, EXTRACT(EPOCH FROM enddate) AS enddate, year, semester, EXTRACT(EPOCH FROM profdate) AS profdate FROM semester_question_periods WHERE question_period_id = $this->question_period_id", $wces, __FILE__, __LINE__);
      assert(pg_numrows($r) == 1);
      extract(pg_fetch_row($r, 0, PGSQL_ASSOC));
      $this->displayName->text = $displayname;
      $this->beginDate->text = format_date($begindate);
      $this->endDate->text = format_date($enddate);
      $this->year->text = $year;
      $this->semester->selected = $semester;
      $this->profDate->text = format_date($profdate);
    }
    else
    {
      $y = date("Y");
      $this->beginDate->text = "1/1/$y 12:00 am";
      $this->endDate->text = "1/31/$y 12:00 am";
      $this->profDate->text = "2/7/$y 12:00 am";
      $this->year->text = $y;
    }
  }
  
  function save()
  {
    global $wces;
    wces_connect();

    $bd = parse_date($this->beginDate->text);
    $ed = parse_date($this->endDate->text);
    $rd = parse_date($this->profDate->text);
    if ($bd === false) $this->errors[] = "Unable to parse begin date";
    if ($bd === false) $this->errors[] = "Unable to parse end date";
    if ($rd === false) $this->errors[] = "Unable to parse professor date";
    
    if (count($this->errors)) return false;

    $bd = nulldate($bd);
    $ed = nulldate($ed);
    $rd = nulldate($rd);
    
    $dn = quot($this->displayName->text);
    $yr = (int)($this->year->text);
    $sm = (int)($this->semester->selected);
    if ($this->question_period_id <= 0)
    {
      $r = pg_go("
        INSERT INTO semester_question_periods(displayname, begindate, enddate,
        semester, year, profdate) VALUES ($dn, $bd, $ed, $sm, $yr, $rd);
        SELECT currval('question_period_ids');
      ", $wces, __FILE__, __LINE__);
      if (!$r) return false;
      $this->question_period_ids = (int)pg_result($r, 0, 0);
      return true;
    }
    else
    {
      return (bool)pg_go("
        UPDATE semester_question_periods SET
          displayname = $dn, begindate = $bd, enddate = $ed,
          semester = $sm, year = $yr, profdate = $rd
        WHERE question_period_id = $this->question_period_id
      ", $wces, __FILE__, __LINE__);  
    }
  }
  
  function loadValues()
  {
    $this->action->loadValues();
    $this->displayName->loadValues();;
    $this->beginDate->loadValues();
    $this->endDate->loadValues();
    $this->year->loadValues();
    $this->semester->loadValues();
    $this->profDate->loadValues();
    if ($this->action->action == QuestionPeriodEditor_save)
    {
      if ($this->save())
      {
        $this->done = true;
        $this->message = "<p><font color=blue>Question Period Editor: Changes saved successfully</font></p>";
      }
    }
    else if ($this->action->action == QuestionPeriodEditor_cancel)
    {
      $this->done = true;
      $this->message = "<p><font color=red>Question Period Editor: No changes were saved.</font></p>";
    }
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
<tr><td>Display Name:</td><td><? $this->displayName->display(); ?></td></tr>
<tr><td>Begin Time:</td><td><? $this->beginDate->display(); ?></td></tr>
<tr><td>End Time:</td><td><? $this->endDate->display(); ?></td></tr>
<tr><td>Professor Results Date:</td><td><? $this->profDate->display(); ?></td></tr>
<tr><td>&nbsp;</td><td><? $this->action->display("Save", QuestionPeriodEditor_save); $this->action->display("Cancel", QuestionPeriodEditor_cancel); ?></td></tr>
</table>
<?
  }
  
};

class QuestionPeriodList extends FormWidget
{
  var $question_period_id = 0;
  var $editor = NULL;
  var $modalChild = NULL;
  var $message = "";

  function QuestionPeriodList($prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
  }
  
  function loadValues()
  {
    $this->action->loadValues();
    $a = (int)$this->loadAttribute("question_period_id");
    $editing = $a != 0;
    for(;;)
    if ($editing)
    {
      if ($a != 0) $this->question_period_id = $a;
      $this->editor = new QuestionPeriodEditor($this->question_period_id, "{$this->prefix}_editor", $this->form, $this->formmethod);
      if ($a != 0) $this->editor->loadValues(); else $this->editor->loadDefaults(); 
      $this->message .= $this->editor->message;
      if ($this->editor->done)
        $this->question_period_id = 0;
      else
      {
        $this->modalChild = &$this->editor;
        $this->editor->modal = true;
      }
      return;
    }   
    else
    {   
      switch ($this->action->action)
      {
        case QuestionPeriodList_edit:
          $editing = true;
          $this->question_period_id = $this->action->object == "new" ? -1 : (int)$this->action->object;
        break;
        case QuestionPeriodList_delete:
          $this->deleteq((int)$this->action->object);
          return;  
        default:
          return;
      }
    }
  }
  
  function deleteq($question_period_id)
  {
    global $wces;
    wces_connect();
    $ref = (int)pg_result(pg_go("SELECT references_question_period($question_period_id)", $wces, __FILE__, __LINE__),0,0);
    if ($ref != 0)
      $this->message = "<p><font color=red>Unable to delete question period $question_period_id because there are survey responses associated with it.</font></p>";
    else
      pg_go("DELETE FROM semester_question_periods WHERE question_period_id = $question_period_id", $wces, __FILE__, __LINE__);
  }
  
  function display()
  {
    global $wces, $dartSemesters;
    print($this->message);
    $this->printAttribute("question_period_id", $this->question_period_id);
    if ($this->modalChild)
    {
      $this->modalChild->display();
      return;
    }

    wces_connect();
    $r = pg_go("SELECT question_period_id, displayname, EXTRACT(EPOCH FROM begindate) AS begindate, EXTRACT(EPOCH FROM enddate) AS enddate, semester, year FROM semester_question_periods ORDER BY begindate", $wces, __FILE__, __LINE__);
    $n = pg_numrows($r);

    print("<table border=1>\n");
    print("<tr><td><b>ID</b></td><td><b>Term</b></td><td><b>Name</b></td><td><b>Begin Time</b></td><td><b>End Time</b></td><td>&nbsp;</td></tr>\n");

    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
      $bd = format_date($begindate);
      $ed = format_date($enddate);
      print("<tr><td>$question_period_id</td><td>$dartSemesters[$semester] $year</td><td>$displayname</td><td>$bd</td><td>$ed<td>");
      $this->action->display("Edit...", QuestionPeriodList_edit, $question_period_id);
      $this->action->display("Delete", QuestionPeriodList_delete, $question_period_id);
      print("</td></tr>\n");
    }
    print("</table>\n");
    print("<p>");
    $this->action->display("New Question Period...", QuestionPeriodList_edit, "new");
    print("</p>");
  }
}

page_top("Question Periods");
print("<form method=post>\n");
print($ISID);
$q = new QuestionPeriodList("qpl", "f", WIDGET_POST);
$q->loadValues();
$q->display();
print("</form>\n");
page_bottom();

?>

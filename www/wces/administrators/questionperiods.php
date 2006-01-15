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
  if (strlen($str) == 0)
     return false;
  else if (($timestamp = strtotime(str_replace("-","/",$str))) === -1)
    return false;
  else
    return $timestamp;
}

LoginProtect(LOGIN_ADMIN);

define("QuestionPeriodEditor_save", 1);
define("QuestionPeriodEditor_cancel", 2);

define("QuestionPeriodList_edit", 1);
define("QuestionPeriodList_delete", 2);

$dartSemesters = array("Spring", "Summer","Fall", "Winter");

class QuestionPeriodEditor extends StatefullWidget
{
  var $done = false;
  var $message = "";
  var $errors = array();
  
  function QuestionPeriodEditor($question_period_id, $name, &$parent)
  {
    global $dartSemesters;
    $this->question_period_id = (int)$question_period_id;
    $this->StatefullWidget($name, $parent);
    $this->question_period_id = (int)$question_period_id;
    $this->displayName =& new TextBox(0, 30, "", "displayName", $this);
    $this->beginDate =& new TextBox(0, 30, "", "beginDate", $this);
    $this->endDate =& new TextBox(0, 30, "", "endDate", $this);
    $this->year =& new TextBox(0, 4, "", "year", $this);
    $this->semester =& new DropBox($dartSemesters, "semester", $this);
    $this->profDate =& new TextBox(0, 30, "", "profDate", $this);
    $this->oracleDate =& new TextBox(0, 30, "", "oracleDate", $this);
  }

  function loadState($new)
  {
    assert(isset($new));
    StatefullWidget::loadState($new);
    
    if (!$new) return;
    
    global $wces;
    if ($this->question_period_id > 0)
    {
      wces_connect();     
      $r = pg_go("SELECT displayname, EXTRACT(EPOCH FROM begindate) AS begindate, EXTRACT(EPOCH FROM enddate) AS enddate, year, semester, EXTRACT(EPOCH FROM profdate) AS profdate, EXTRACT(EPOCH FROM oracledate) AS oracledate FROM question_periods WHERE question_period_id = $this->question_period_id ORDER BY begindate", $wces, __FILE__, __LINE__);
      assert(pg_numrows($r) == 1);
      extract(pg_fetch_row($r, 0, PGSQL_ASSOC));
      $this->displayName->text = $displayname;
      $this->beginDate->text = format_date($begindate);
      $this->endDate->text = format_date($enddate);
      $this->year->text = $year;
      $this->semester->selected = $semester;
      $this->profDate->text = format_date($profdate);
      $this->oracleDate->text = format_date($oracledate);
    }
    else
    {
      $y = date("Y");
      $this->beginDate->text = "1/1/$y 12:00 am";
      $this->endDate->text = "1/31/$y 12:00 am";
      $this->profDate->text = "2/7/$y 12:00 am";
      $this->oracleDate->text = "2/7/$y 12:00 am";
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
    $od = parse_date($this->oracleDate->text);

    if ($bd === false) $this->errors[] = "Unable to parse begin date";
    if ($bd === false) $this->errors[] = "Unable to parse end date";
    if ($rd === false) $this->errors[] = "Unable to parse professor date";
    if ($od === false) $this->errors[] = "Unable to parse oracle date";
    
    if (count($this->errors)) return false;

    $bd = nulldate($bd);
    $ed = nulldate($ed);
    $rd = nulldate($rd);
    $od = nulldate($od);
    
    $dn = quot($this->displayName->text);
    $yr = (int)($this->year->text);
    $sm = (int)($this->semester->selected);
    if ($this->question_period_id <= 0)
    {
      $r = pg_go("
        INSERT INTO question_periods(displayname, begindate, enddate,
        semester, year, profdate, oracledate) VALUES ($dn, $bd, $ed, $sm, $yr, $rd, $od);
        SELECT currval('question_period_ids');
      ", $wces, __FILE__, __LINE__);
      if (!$r) return false;
      $this->question_period_id = (int)pg_result($r, 0, 0);
      return true;
    }
    else
    {
      return (bool)pg_go("
        UPDATE question_periods SET
          displayname = $dn, begindate = $bd, enddate = $ed,
          semester = $sm, year = $yr, profdate = $rd, oracledate = $od
        WHERE question_period_id = $this->question_period_id
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
          $this->message = "<p><font color=blue>Question Period Editor: Changes to question period $this->question_period_id saved successfully</font></p>";
        }     
      break;

      case QuestionPeriodEditor_cancel:
        $this->done = true;
        $this->message = "<p><font color=red>Question Period Editor: Changes to question period $this->question_period_id were not saved.</font></p>";
      break;
    };
    return $null;
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
<tr><td>Oracle Results Date:</td><td><? $this->oracleDate->display(); ?></td></tr>
<tr><td>&nbsp;</td><td><? $this->event->displayButton("Save", QuestionPeriodEditor_save); $this->event->displayButton("Cancel", QuestionPeriodEditor_cancel); ?></td></tr>
</table>
<?
  }  
};

class QuestionPeriodList extends StatefullWidget
{
  var $question_period_id = 0;
  var $editor = null;
  var $modalChild = null;
  var $message = "";

  function QuestionPeriodList($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
  }
  
  function & handleEvent($event, $param, $isNew)
  {
    switch ($event)
    {
      case QuestionPeriodList_edit:
        if ($param == "new") $param = -1;
        $this->editor =& new QuestionPeriodEditor($param, "editor", $this);
        $this->loadChild($this->editor, $isNew);
      return $this->editor;

      case QuestionPeriodList_delete:
        $this->deleteq((int)$param);
      break;
    };
    return $null;
  }
  
  function deleteq($question_period_id)
  {
    global $wces;
    wces_connect();
    $ref = (int)pg_result(pg_go("SELECT references_question_period($question_period_id)", $wces, __FILE__, __LINE__),0,0);
    if ($ref == 0)
      pg_go("DELETE FROM question_periods WHERE question_period_id = $question_period_id", $wces, __FILE__, __LINE__);
    else
      $this->message = "<p><font color=red>Unable to delete question period $question_period_id because there are survey responses associated with it.</font></p>";
  }
  
  function printVisible()
  {
    global $wces, $dartSemesters;
    
    if (isset($this->editor->message))
      print($this->editor->message);
    
    if (isset($this->message))
      print($this->message);

    wces_connect();
    $r = pg_go("SELECT question_period_id, displayname, EXTRACT(EPOCH FROM begindate) AS begindate, EXTRACT(EPOCH FROM enddate) AS enddate, semester, year FROM question_periods WHERE year IS NOT NULL ORDER BY begindate, enddate", $wces, __FILE__, __LINE__);
    $n = pg_numrows($r);

    print("<table border=1>\n");
    print("<tr><td><b>ID</b></td><td><b>Term</b></td><td><b>Name</b></td><td><b>Begin Time</b></td><td><b>End Time</b></td><td>&nbsp;</td></tr>\n");

    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
      $bd = format_date($begindate);
      $ed = format_date($enddate);
      print("<tr><td>$question_period_id</td><td>$dartSemesters[$semester] $year</td><td>$displayname</td><td>$bd</td><td>$ed<td>");
      $this->event->displayButton("Edit...", QuestionPeriodList_edit, $question_period_id);
      $this->event->displayButton("Delete", QuestionPeriodList_delete, $question_period_id);
      print("</td></tr>\n");
    }
    print("</table>\n");
    print("<p>");
    $this->event->displayButton("New Question Period...", QuestionPeriodList_edit, "new");
    print("</p>");
  }
}

page_top("Question Periods");

$f =& new Form('form');
$q =& new QuestionPeriodList("qpl", $f);
$f->loadState();

print("<form method=post>\n");
print($ISID);
$f->display();
$q->display();
print("</form>\n");

page_bottom();

?>

<?

require_once("widgets/basic.inc");
require_once("widgets/wizard.inc");
require_once("widgets/dynamiclist.inc");
require_once("wces/database.inc");

//---------------------------------------------------------------------------------------------
// Report Wizard

define("ReportWizard_start",0);
define("ReportWizard_output",1);
define("ReportWizard_qperiods",2);
define("ReportWizard_depts",3);
define("ReportWizard_profs",4);
define("ReportWizard_courses",5);
define("ReportWizard_aggreg",6);
define("ReportWizard_order",7);
define("ReportWizard_finish",8);
define("ReportWizard_output_www",0);
define("ReportWizard_output_email",1);
define("ReportWizard_output_pemail",2);
define("ReportWizard_output_raw",3);

class ReportOutput extends ParentWidget
{
  function ReportOutput($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->output          =& new RadioButton(true, "output", $this);
    $this->includeText     =& new CheckBox("includeText", $this);
    $this->includeTa       =& new CheckBox("includeTa", $this);
    // for html output only
    $this->printable       =& new CheckBox("printable", $this);
    $this->includePies     =& new CheckBox("includePies", $this);
    $this->bigHead         =& new CheckBox("bigHead", $this);    
  }

  function loadState($new)
  {
    assert(isset($new));
    ParentWidget::loadState($new);
    if ($new)
    {
      $this->output->checked = ReportWizard_output_www;
      $this->includeText->checked = true;
      $this->includeTa->checked = true;
  
      $this->printable->checked = true;
      $this->includePies->checked = false;
      $this->bigHead->checked = true;              
    }
  }
  
  function printVisible()
  {
    $wwwo = $this->name('wwwoptions');
?>
<div><b>Choose a report format:</b></div>
<table>
<tr>
  <td valign=top><? $this->output->display(ReportWizard_output_www, "onclick=\"show('$wwwo')\""); ?></td>
  <td valign=top><? $this->output->displayLabel(ReportWizard_output_www, "Display summarized results"); ?></td>
</tr>
<tr>
  <td valign=top><? $this->output->display(ReportWizard_output_email, "onclick=\"hide('$wwwo')\""); ?></td>
  <td valign=top><? $this->output->displayLabel(ReportWizard_output_email, "Email summarized results"); ?></td>
</tr>
<? /*
<tr>
  <td valign=top><? $this->output->display(ReportWizard_output_raw, "onclick=\"hide('$wwwo')\""); ?></td>
  <td valign=top><? $this->output->displayLabel(ReportWizard_output_raw, "Download raw results"); ?></td>
</tr>
*/ ?>
<tr>
  <td valign=top><? $this->output->display(ReportWizard_output_pemail, "onclick=\"hide('$wwwo')\""); ?></td>
  <td valign=top><? $this->output->displayLabel(ReportWizard_output_pemail, "Send professors email containing their results"); ?></td>
</tr>
</table>

<div><b>Include these sections:</b></div>
<table>
<tr>
  <td><? $this->includeText->display(); ?></td>
  <td><? $this->includeText->displayLabel("Comments"); ?></td>
</tr>
<tr>
  <td><? $this->includeTa->display(); ?></td>
  <td><? $this->includeTa->displayLabel("TA Ratings"); ?></td>
</tr>
</table>

<div id="<?=$wwwo?>"<?=$this->output->checked == ReportWizard_output_email ? ' style="display:none"': ''?>>
<p><b>Options for web reports only:</b><br>
<? $this->printable->display(); ?> <? $this->printable->displayLabel("Produce a printable report"); ?><br>
<? $this->includePies->display(); ?> <? $this->includePies->displayLabel("Show Pie Graphs in Usage Statistics"); ?><br>
<? $this->bigHead->display(); ?> <? $this->bigHead->displayLabel("Exhaustive Class and Professor listings"); ?><br>
</p>
</div>
<?
  }
};

class ReportWizard extends Wizard
{
  var $groupClasses;
  var $groupCourses;
  var $groupProfessors;
  var $groupDepartments;
  var $groupQuestionPeriods;

  var $depts = NULL;

  function ReportWizard($name, &$parent)
  {
    global $wces_path;
    $steps = array("Start","Output","Question Periods","Departments","Professors","Courses","Aggregation","Order","Finish"); 
    $this->Wizard($steps, $wces_path, $name, $parent);

    if (!(LoginValue('status') & LOGIN_ADMIN))
      $this->depts = LoginValue('depts');

    $this->output               =& new ReportOutput("output", $this);
    $this->groupClasses         =& new CheckBox("groupClasses", $this);
    $this->groupCourses         =& new CheckBox("groupCourses", $this);
    $this->groupProfessors      =& new CheckBox("groupProfessors", $this);
    $this->groupDepartments     =& new CheckBox("groupDepartments", $this);
    $this->groupQuestionPeriods =& new CheckBox("groupQuestionPeriods", $this);
    
    $labels = array(QUESTION_PERIODS => "Question Period", DEPARTMENTS => "Department", PROFESSORS => "Professor", COURSES => "Course");
    $this->order =& new DynamicList(25, 4, $labels, "order", $this);    

    /////////////////////////////// ORDERING SETUP ///////////////////////////////////////

    if ($this->depts)
      $dwhere = "\nWHERE department_id in (" . implode($this->depts, ",") . ")";
    else 
      $dwhere = "";

    $this->queries = array(
      CATEGORIES => "SELECT category_id AS id, name FROM categories ORDER BY category_id",
      QUESTION_PERIODS => "SELECT question_period_id AS id, displayname AS name FROM question_periods WHERE year IS NOT NULL ORDER BY question_period_id DESC",
      DEPARTMENTS => "
        SELECT t.department_id AS id, d.code::text || ' - ' || d.name::text AS name
        FROM rwtopics AS t
        INNER JOIN departments AS d USING (department_id)$dwhere
        GROUP BY t.department_id, d.code, d.name
        ORDER BY d.code",
      COURSES => "
        SELECT t.course_id AS id, s.code || ' ' || c.divisioncode || c.code || ' ' || COALESCE(c.name,'') AS name
        FROM rwtopics AS t
        INNER JOIN courses AS c USING (course_id)
        INNER JOIN subjects AS s USING (subject_id)
        GROUP BY s.code, c.divisioncode, c.code, t.course_id, c.name
        ORDER BY s.code, c.divisioncode, c.code",
      PROFESSORS => "
        SELECT t.user_id AS id, u.lastname || ', ' || u.firstname AS name
        FROM rwtopics AS t
        INNER JOIN users AS u USING (user_id)
        GROUP BY u.lastname, u.firstname, t.user_id
        ORDER BY u.lastname, u.firstname
      "
    );
  }
 
  function loadState($new)
  {
    assert(isset($new));
    if ($new)
    {
      Wizard::loadState($new);
      $this->groupClasses->checked = true;
      $this->groupQuestionPeriods->checked = true;
      $this->groupDepartments->checked = true;
      $this->makeCriteria(false, false, $this->depts, false, false);      
      return;
    }
    
    Wizard::loadState($new, false);

    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////

    $this->crawl
    ( array
      ( ReportWizard_aggreg => $this->output->output->checked == ReportWizard_output_email,
        ReportWizard_order =>  $this->output->output->checked == ReportWizard_output_email
      )
    );

    //////////////////////////////// CRITERIA SETUP //////////////////////////////////////

    $t = $this->readValue("categories");
    $q = $this->readValue("questionperiods");
    $d = $this->depts ? $this->depts : $this->readValue("departments");
    $p = $this->readValue("professors");
    $c = $this->readValue("courses");
    $this->makeCriteria($t, $q, $d, $p, $c);
  }
  
  function makeCriteria($t, $q, $d, $p, $c)
  {
    global $wces;

    if (!is_array($t) || !(count($t) > 0))
    {
      wces_connect();
      $t = false;
      if (!$this->depts)
      {
        $result = pg_go("SELECT category_id FROM categories ORDER BY category_id LIMIT 1", $wces, __FILE__, __LINE__);
        if (pg_numrows($result))
          $t = array((int) pg_result($result, 0, 0));
      }
    }
    else if (in_array("all",$t)) $t = false;

    if (!is_array($q) || !(count($q) > 0))
    {
      wces_connect();
      $question_period_id = get_question_period();
      $q = $question_period_id ? array($question_period_id) : false;
    }
    else if (in_array("all",$q)) $q = false;

    if (!is_array($p) || !(count($p) > 0) || in_array("all",$p)) $p = false;
    if (!is_array($d) || !(count($d) > 0) || in_array("all",$d)) $d = false;
    if (!is_array($c) || !(count($c) > 0) || in_array("all",$c)) $c = false;

    $this->criteria = array(QUESTION_PERIODS => $q, DEPARTMENTS => $d, PROFESSORS => $p, COURSES => $c, CATEGORIES => $t, CLASSES => false);
  }

  function selectQuery($which, $sql, $needstemp = false, $block = true, $needprof = false)
  {
    global $wces;
    wces_connect();

    if ($needstemp)
    {
      $c = $this->criteria;
      if ($block) $c[$which] = false;
      report_findtopics("rwtopics", $c, $needprof);
    }

    return pg_go($sql, $wces, __FILE__, __LINE__);
  }

  function printSelect($which, $result, $name, $all)
  {
    $criteria = $this->criteria[$which];
    if (!$criteria) $criteria = array("all");
    $hash = array_bflip($criteria);

    $full = $this->name($name) . '[]';

    print("<select name=\"$full\" size=5 multiple>\n");
    print("<option value=all" . (isset($hash["all"]) ? " selected" : "") . ">$all</option>\n");

    $n = pg_numrows($result);
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array ($result, $i, PGSQL_ASSOC));
      $selected = isset($hash[$id]) && $hash[$id] ? " selected" : "";
      print("<option value=\"$id\"$selected>$name</option>");
    }
    print("</select>\n");
  }

  function printCriteria($sql, &$report, $name)
  {
    global $wces;

    $result = pg_go($sql, $wces, __FILE__, __LINE__);
    $n = pg_numrows($result);
    for($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_array ($result, $i, PGSQL_ASSOC);
      $report .= "$name: {$row['name']} ({$row['id']})\n";
    }
  }

  function printStep($stepno, $visible, $hiddenChildren)
  {
    assert(!$visible || !$hiddenChildren);
    
    switch($stepno)
    {
      case ReportWizard_start:
        if ($visible)
        {
          $result = $this->selectQuery(CATEGORIES, $this->queries[CATEGORIES]);
          print("<p>Welcome to the reporting wizard. This wizard will allow you to display results or send out professor emails for one or more of the survey groups listed below.</p>\n");
          $this->printSelect(CATEGORIES, $result, 'categories', '--- All Survey Categories ---');
        }
        else
          $this->printArrayValue('categories', $this->criteria[CATEGORIES]);
      break;
      case ReportWizard_output:
        if ($visible)
          $this->output->display();
        else
          $this->output->displayHidden();
      break;
      case ReportWizard_qperiods:
        if ($visible)
        {
          $result = $this->selectQuery(QUESTION_PERIODS, $this->queries[QUESTION_PERIODS]);
          print("<p>Surveys included in the report must come from the following question period(s):</p>\n");
          $this->printSelect(QUESTION_PERIODS, $result, 'questionperiods', '--- Any Question Period ---');
        }
        else
          $this->printArrayValue('questionperiods', $this->criteria[QUESTION_PERIODS]);
  
      break;
      case ReportWizard_depts:
        if ($visible)
        {
          $result = $this->selectQuery(DEPARTMENTS, $this->queries[DEPARTMENTS], true);
          print("<p>Surveys included in the report must come from classes in these departments:</p>\n");
          $this->printSelect(DEPARTMENTS, $result, 'departments', '--- Any Department ---');
        }
        else
          $this->printArrayValue('departments', $this->criteria[DEPARTMENTS]);
      break;
      case ReportWizard_courses:
        if ($visible)
        {
          $result = $this->selectQuery(COURSES, $this->queries[COURSES], true);
          print("<p>Surveys included in the report must come from these courses:</p>\n");
          $this->printSelect(COURSES, $result, 'courses', '--- Any Course ---');
        }
        else
          $this->printArrayValue('courses', $this->criteria[COURSES]);
      break;
      case ReportWizard_profs:
        if ($visible)
        {
          $result = $this->selectQuery(PROFESSORS, $this->queries[PROFESSORS], true, true, true);
          print("<p>Surveys included in the report must come from classes with these professors:</p>\n");
          $this->printSelect(PROFESSORS, $result, 'professors', '--- Any Professor ---');
        }
        else
          $this->printArrayValue('professors', $this->criteria[PROFESSORS]);
      break;
      case ReportWizard_aggreg:
        if ($visible)
        {
          if ($this->output->output->checked == ReportWizard_output_email)
            print("<p>Aggregation is not available in email reports. Choose a web based report to use this feature.</p>");
          else
          {
?>
<p><font size="-1">Check the survey properties below to determine how results are to be aggregated. Surveys that have the checked properties in common will have their scores added up and averaged together. For example, checking 'Departments' and 'Question Periods' will aggregate all surveys the come from the same department AND question period. To prevent any aggregation, select 'Classes' and 'Question Periods.' To aggregate all surveys together, leave all properties unchecked.</font></p>
<p>
<? $this->groupClasses->display(); ?> <? $this->groupClasses->displayLabel("Classes"); ?><br>
<? $this->groupCourses->display(); ?> <? $this->groupCourses->displayLabel("Courses"); ?><br>
<? $this->groupProfessors->display(); ?> <? $this->groupProfessors->displayLabel("Professors"); ?><br>
<? $this->groupDepartments->display(); ?> <? $this->groupDepartments->displayLabel("Departments"); ?><br>
<? $this->groupQuestionPeriods->display(); ?> <? $this->groupQuestionPeriods->displayLabel("Question Periods"); ?><br>
</p>
<?
          };
        }
        
        if ($hiddenChildren || ($visible && $this->output->output->checked == ReportWizard_output_email))
        {
          $this->groupClasses->displayHidden();
          $this->groupCourses->displayHidden();
          $this->groupProfessors->displayHidden();
          $this->groupDepartments->displayHidden();
          $this->groupQuestionPeriods->displayHidden();          
        }
        
      break;
      case ReportWizard_order:
        if ($visible)
        {
          if($this->output->output->checked == ReportWizard_output_email)
          {
            print("<p>Ordering options are not available in email reports. Choose a web based report to use this feature.</p>");
          }
          else
          {
            print("<p>Report pages will be ordered by these properties (highest priority first):</p>");
            $this->order->display();
          }
        }
        if ($hiddenChildren || ($visible && $this->output->output->checked == ReportWizard_output_email))
          $this->order->displayHidden();
      break;
      case ReportWizard_finish:
        if ($visible)
        {
          print("Click finish to generate the report<br><textarea rows=15 cols=40 wrap=off contenteditable=false>" . $this->summarize() . "</textarea>");
        };
      break;
    };
  }

  function summarize($initialized = false)
  {
    global $wces;

    if (!$initialized)
    {
      wces_connect();
      report_findtopics("rwtopics", $this->criteria, $this->criteria[PROFESSORS]);
    }

    $report = "------------------------\nREPORT OPTIONS\n\n";

    $report .= "Output: " . ($this->output->output->checked == ReportWizard_output_email ? "Professor Email" : "Web Report") . "\n";
    $report .= "Comments: " . ($this->output->includeText->checked ? "Yes" : "No") . "\n";
    $report .= "TA Ratings: " . ($this->output->includeTa->checked ? "Yes" : "No") . "\n";

    if ($this->output->output->checked == ReportWizard_output_www)
    {
      $report .= "Pie Graphs: " . ($this->output->includePies->checked ? "Yes" : "No") . "\n";
      $report .= "Printable: " . ($this->output->printable->checked ? "Yes" : "No") . "\n";
      $report .= "Exhaustive Listings: " . ($this->output->bigHead->checked ? "Yes" : "No") . "\n";

      $first = true;
      $report .= "Group by: ";
      if ($this->groupClasses->checked) { if ($first) $first = false; else $report.=", "; $report.="Classes"; }
      if ($this->groupCourses->checked)  { if ($first) $first = false; else $report.=", "; $report.="Courses"; }
      if ($this->groupProfessors->checked)  { if ($first) $first = false; else $report.=", "; $report.="Professors"; }
      if ($this->groupDepartments->checked)  { if ($first) $first = false; else $report.=", "; $report.="Departments"; }
      if ($this->groupQuestionPeriods->checked) { if ($first) $first = false; else $report.=", "; $report.="Question Periods"; }
      if ($first) $report .= "None";
      $report .= "\n";

      $report .= "Order by: ";
      $first = true;
      foreach($this->order->items as $item)
      {
        if ($first) $first = false; else $report .= ", ";
        $report .= $item;
      }
      $report .= "\n";
    }

    $report .= "\n------------------------\nREPORT CRITERIA\n\n";

    $list = $this->criteria[CATEGORIES];
    if ($list)
    {
      $group = db_set($list);
      $this->printCriteria("SELECT category_id AS id, name FROM categories WHERE category_id IN $group ORDER BY category_id", $report, "Category");
    }
    else
      $report .= "Category: ANY\n";

    $list = $this->criteria[QUESTION_PERIODS];
    if ($list)
    {
      $group = db_set($list);
      $this->printCriteria("SELECT question_period_id AS id, displayname AS name FROM question_periods WHERE question_period_id IN $group ORDER BY question_period_id DESC", $report, "Question Period");
    }
    else
      $report .= "Question Period: ANY\n";

    if ($this->criteria[DEPARTMENTS])
      $this->printCriteria($this->queries[DEPARTMENTS], $report, "Department");
    else
      $report .= "Department: ANY\n";

    if ($this->criteria[PROFESSORS])
      $this->printCriteria($this->queries[PROFESSORS], $report, "Professor");
    else
      $report .= "Professor: ANY\n";

    if ($this->criteria[COURSES])
      $this->printCriteria($this->queries[COURSES], $report, "Course");
    else
      $report .= "Course: ANY\n";

    return $report;
  }
}

?>

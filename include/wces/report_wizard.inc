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
define("ReportWizard_output_pemail",1);

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
  
  function loadInitialState()
  {
    $this->output->checked = ReportWizard_output_www;
    $this->includeText->checked = true;
    $this->includeTa->checked = true;

    $this->printable->checked = true;
    $this->includePies->checked = false;
    $this->bigHead->checked = true;        
  }
  
  function printVisible()
  {
    $wwwo = $this->fullName('wwwoptions');
?>
<p><b>Choose a report format:</b><br>
<? $this->output->display(ReportWizard_output_www, "onclick=\"show('$wwwo')\""); ?> <? $this->output->displayLabel(ReportWizard_output_www, "Display Report on Web Page"); ?><br>
<? $this->output->display(ReportWizard_output_email, "onclick=\"hide('$wwwo')\""); ?> <? $this->output->displayLabel(ReportWizard_output_email, "Email Report to one or people"); ?><br>
<? $this->output->display(ReportWizard_output_pemail, "onclick=\"hide('$wwwo')\""); ?> <? $this->output->displayLabel(ReportWizard_output_pemail, "Email professor reports"); ?>
</p>

<p><b>Include these sections:</b><br>
<? $this->includeText->display(); ?> <? $this->includeText->displayLabel("Comments"); ?><br>
<? $this->includeTa->display(); ?> <? $this->includeTa->displayLabel("TA Ratings"); ?><br>
</p>

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

    if (!(login_getstatus() & login_administrator))
      $this->depts = login_getdepts();

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
      CATEGORIES => "SELECT survey_category_id AS id, name FROM survey_categories ORDER BY survey_category_id",
      QUESTION_PERIODS => "SELECT question_period_id AS id, displayname AS name FROM question_periods ORDER BY question_period_id DESC",
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
        GROUP BY t.course_id, s.code, c.divisioncode, c.code, c.name
        ORDER BY s.code, c.code",
      PROFESSORS => "
        SELECT t.user_id AS id, u.lastname || ', ' || u.firstname AS name
        FROM rwtopics AS t
        INNER JOIN users AS u USING (user_id)
        GROUP BY t.user_id, u.lastname, u.firstname
        ORDER BY u.lastname, u.firstname
      "
    );
  }

  function loadInitialState()
  {
    Wizard::loadInitialState();
    $this->groupClasses->checked = true;
    $this->groupQuestionPeriods->checked = true;
    $this->groupDepartments->checked = true;
    $this->makeCriteria(false, false, $this->depts, false, false);
  }
  
  function loadState()
  {
    Wizard::loadState(false);

    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////

    $this->crawl
    ( array
      ( ReportWizard_aggreg => $this->output->output->checked == ReportWizard_output_email,
        ReportWizard_order =>  $this->output->output->checked == ReportWizard_output_email,
        ReportWizard_output => true
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
      if ($this->depts)
        $t = false;
      else
        $t = array((int)pg_result(pg_go("SELECT survey_category_id FROM survey_categories ORDER BY survey_category_id LIMIT 1", $wces, __FILE__, __LINE__),0,0));
    }
    else if (in_array("all",$t)) $t = false;

    if (!is_array($q) || !(count($q) > 0))
    {
      wces_connect();
      //todo: restore this default $q = array((int)pg_result(pg_go("SELECT MAX(question_period_id) FROM question_periods", $wces, __FILE__, __LINE__),0,0));
      $q = false;
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

    $full = $this->fullName($name) . '[]';

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

  function printStep($stepno, $hidden, $hiddenChildren)
  {
    switch($stepno)
    {
      case ReportWizard_start:
        if ($hidden)
          $this->printArrayValue('categories', $this->criteria[CATEGORIES]);
        else
        {
          $result = $this->selectQuery(CATEGORIES, $this->queries[CATEGORIES]);
          print("<p>Welcome to the reporting wizard. This wizard will allow you to display results or send out professor emails for one or more of the survey groups listed below.</p>\n");
          $this->printSelect(CATEGORIES, $result, 'categories', '--- All Survey Categories ---');
        };
      break;
      case ReportWizard_output:
        if (!$hidden)
          $this->output->display();
        else if ($hiddenChildren)
          $this->output->displayHidden();
      break;
      case ReportWizard_qperiods:
        if ($hidden)
          $this->printArrayValue('questionperiods', $this->criteria[QUESTION_PERIODS]);
        else
        {
          $result = $this->selectQuery(QUESTION_PERIODS, $this->queries[QUESTION_PERIODS]);
          print("<p>Surveys included in the report must come from the following question period(s):</p>\n");
          $this->printSelect(QUESTION_PERIODS, $result, 'questionperiods', '--- Any Question Period ---');
        };
      break;
      case ReportWizard_depts:
        if ($hidden)
          $this->printArrayValue('departments', $this->criteria[DEPARTMENTS]);
        else
        {
          $result = $this->selectQuery(DEPARTMENTS, $this->queries[DEPARTMENTS], true);
          print("<p>Surveys included in the report must come from classes in these departments:</p>\n");
          $this->printSelect(DEPARTMENTS, $result, 'departments', '--- Any Department ---');
        }
      break;
      case ReportWizard_courses:
        if ($hidden)
          $this->printArrayValue('courses', $this->criteria[COURSES]);
        else
        {
          $result = $this->selectQuery(COURSES, $this->queries[COURSES], true);
          print("<p>Surveys included in the report must come from these courses:</p>\n");
          $this->printSelect(COURSES, $result, 'courses', '--- Any Course ---');
        }
      break;
      case ReportWizard_profs:
        if ($hidden)
          $this->printArrayValue('professors', $this->criteria[PROFESSORS]);
        else
        {
          $result = $this->selectQuery(PROFESSORS, $this->queries[PROFESSORS], true, true, true);
          print("<p>Surveys included in the report must come from classes with these professors:</p>\n");
          $this->printSelect(PROFESSORS, $result, 'professors', '--- Any Professor ---');
        }
      break;
      case ReportWizard_aggreg:
        if ($hidden) break;
        
        if ($this->output->output->checked == ReportWizard_output_email)
        {
          $this->groupClasses->displayHidden();
          $this->groupCourses->displayHidden();
          $this->groupProfessors->displayHidden();
          $this->groupDepartments->displayHidden();
          $this->groupQuestionPeriods->displayHidden();
        }
        
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
      break;
      case ReportWizard_order:
        if ($hidden || $this->output->output->checked == ReportWizard_output_email)
          $this->order->displayHidden();

        if (!$hidden)
        {
          if($this->output->output->checked == ReportWizard_output_email)
            print("<p>Ordering options are not available in email reports. Choose a web based report to use this feature.</p>");
          else
          {
            print("<p>Report pages will be ordered by these properties (highest priority first):</p>");
            $this->order->display();
          }
        }
      break;
      case ReportWizard_finish:
        if (!$hidden)
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
      report_findtopics("rwtopics", $this->criteria, true);
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
      $this->printCriteria("SELECT survey_category_id AS id, name FROM survey_categories WHERE survey_category_id IN $group ORDER BY survey_category_id", $report, "Category");
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
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

class ReportWizard extends Wizard
{
  var $output;

  var $includestandard;
  var $includecustom;
  var $includeabet;
  var $includetext;
  var $includeta;
  var $includepies;
  var $printable;
  var $bighead;

  var $groupclasses;
  var $groupcourses;
  var $groupprofessors;
  var $groupdepartments;
  var $groupquestionperiods;

  var $depts;

  function ReportWizard($prefix,$form,$formmethod)
  {
    global $wces_path;

    $this->Wizard
    (
      array("Start","Output","Question Periods","Departments","Professors","Courses","Aggregation","Order","Finish"),
      $wces_path,
      $prefix,$form,$formmethod
    );

    $this->depts = login_getdepts();

    $this->output = new RadioButton(true, $this->prefix . "_output",$this->form,$this->formmethod);
    $this->includestandard = new CheckBox("{$prefix}_includestandard", $this->form, $this->formmethod);
    $this->includecustom = new CheckBox("{$prefix}_includecustom", $this->form, $this->formmethod);
    $this->includeabet = new CheckBox("{$prefix}_includeabet", $this->form, $this->formmethod);
    $this->includetext = new CheckBox("{$prefix}_includetext", $this->form, $this->formmethod);
    $this->includeta = new CheckBox("{$prefix}_includeta", $this->form, $this->formmethod);
    $this->printable = new CheckBox("{$prefix}_printable",$this->form,$this->formmethod);
    $this->includepies = new CheckBox("{$prefix}_includepies", $this->form, $this->formmethod);
    $this->bighead = new CheckBox("{$prefix}_bighead",$this->form,$this->formmethod);

    $this->groupclasses = new CheckBox("{$prefix}_groupclasses", $this->form, $this->formmethod);
    $this->groupcourses = new CheckBox("{$prefix}_groupcourses", $this->form, $this->formmethod);
    $this->groupprofessors = new CheckBox("{$prefix}_groupprofessors", $this->form, $this->formmethod);
    $this->groupdepartments = new CheckBox("{$prefix}_groupdepartments", $this->form, $this->formmethod);
    $this->groupquestionperiods = new CheckBox("{$prefix}_groupquestionperiods", $this->form, $this->formmethod);
  }

  function loadvalues()
  {
    global $wces;
    Wizard::loadvalues(false);

    //////////////////////////////// WIDGET SETUP ////////////////////////////////////////

    if ($this->visited[ReportWizard_output])
    {
      $this->output->loadvalues();
      $this->includestandard->loadvalues();
      $this->includecustom->loadvalues();
      $this->includeabet->loadvalues();
      $this->includetext->loadvalues();
      $this->includeta->loadvalues();
      $this->printable->loadvalues();
      $this->includepies->loadvalues();
      $this->bighead->loadvalues();
    }
    else
    {
      $this->output->checked = ReportWizard_output_www;
      $this->includestandard->checked = true;
      $this->includecustom->checked = true;
      $this->includeabet->checked = true;
      $this->includetext->checked = true;
      $this->includeta->checked = true;
      $this->printable->checked = true;
      $this->includepies->checked = false;
      $this->bighead->checked = true;
    }

    if ($this->visited[ReportWizard_aggreg])
    {
      $this->groupclasses->loadvalues();
      $this->groupcourses->loadvalues();
      $this->groupprofessors->loadvalues();
      $this->groupdepartments->loadvalues();
      $this->groupquestionperiods->loadvalues();
    }
    else
    {
      $this->groupclasses->checked = true;
      $this->groupquestionperiods->checked = true;
      $this->groupdepartments->checked = true;
    }

    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////

    $this->crawl
    (
      array
      (
        ReportWizard_aggreg => $this->output->checked == ReportWizard_output_email,
        ReportWizard_order =>  $this->output->checked == ReportWizard_output_email,
        ReportWizard_output =>  true
      )
    );

    //////////////////////////////// CRITERIA SETUP //////////////////////////////////////

    $t = $this->loadattribute("categories");
    $q = $this->loadattribute("questionperiods");
    $d = $this->depts ? $this->depts : $this->loadattribute("departments");
    $p = $this->loadattribute("professors");
    $c = $this->loadattribute("courses");

    if (!is_array($t) || !(count($t) > 0))
    {
      wces_connect();
      $t = array((int)pg_result(pg_query("SELECT survey_category_id FROM survey_categories ORDER BY survey_category_id LIMIT 1", $wces, __FILE__, __LINE__),0));
    }
    else if (in_array("all",$t)) $t = false;

    if (!is_array($q) || !(count($q) > 0))
    {
      wces_connect();
      //todo: restore this default $q = array((int)pg_result(pg_query("SELECT MAX(question_period_id) FROM question_periods", $wces, __FILE__, __LINE__),0,0));
      $q = false;
    }
    else if (in_array("all",$q)) $q = false;

    if (!is_array($p) || !(count($p) > 0) || in_array("all",$p)) $p = false;
    if (!is_array($d) || !(count($d) > 0) || in_array("all",$d)) $d = false;
    if (!is_array($c) || !(count($c) > 0) || in_array("all",$c)) $c = false;

    $this->criteria = array(QUESTION_PERIODS => $q, DEPARTMENTS => $d, PROFESSORS => $p, COURSES => $c, CATEGORIES => $t);
    unset($q,$d,$p,$c,$t);

    /////////////////////////////// ORDERING SETUP ///////////////////////////////////////

    $labels = array(QUESTION_PERIODS => "Question Period", DEPARTMENTS => "Department", PROFESSORS => "Professor", COURSES => "Course");
    $this->order = new DynamicList(25,4,$labels, $this->prefix . "_order", $this->form, $this->formmethod);
    $this->order->loadvalues();

    $this->queries = array(
      CATEGORIES => "SELECT survey_category_id AS id, name FROM survey_categories ORDER BY survey_category_id",
      QUESTION_PERIODS => "SELECT question_period_id AS id, displayname AS name FROM question_periods ORDER BY question_period_id DESC",
      DEPARTMENTS => "
        SELECT t.department_id AS id, d.code::text || ' - ' || d.name::text AS name
        FROM rwtopics AS t
        INNER JOIN departments AS d USING (department_id)
        GROUP BY t.department_id, d.code, d.name
        ORDER BY d.code",
      COURSES => "
        SELECT t.course_id AS id, s.code || ' ' || c.code || ' ' || COALESCE(c.name,'') AS name
        FROM rwtopics AS t
        INNER JOIN courses AS c USING (course_id)
        INNER JOIN subjects AS s USING (subject_id)
        GROUP BY t.course_id, s.code, c.code, c.name
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

  function selectquery($which, $sql, $needstemp = false, $block = true, $needprof = false)
  {
    global $wces;
    wces_connect();

    if ($needstemp)
    {
      $c = $this->criteria;
      if ($block) $c[$which] = false;
      report_findtopics("rwtopics", $c, $needprof);
    }

    return pg_query($sql, $wces, __FILE__, __LINE__);
  }

  function selectshow($which, $result, $name, $all)
  {
    $criteria = $this->criteria[$which];
    if (!$criteria) $criteria = array("all");
    $hash = array_bflip($criteria);

    print("<select name=\"{$this->prefix}_{$name}[]\" size=5 multiple>\n");
    print("<option value=all" . (isset($hash["all"]) ? " selected" : "") . ">$all</option>\n");

    $n = pg_numrows($result);
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array ($result, $i, PGSQL_ASSOC));
      $selected = $hash[$id] ? " selected" : "";
      print("<option value=\"$id\"$selected>$name</option>");
    }
    print("</select>\n");
  }

  function critshow($sql, &$report, $name)
  {
    global $wces;

    $result = pg_query($sql, $wces, __FILE__, __LINE__);
    $n = pg_numrows($result);
    for($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_array ($result, $i, PGSQL_ASSOC);
      $report .= "$name: {$row['name']} ({$row['id']})\n";
    }
  }

  function drawstep($stepno,$hidden)
  {
    switch($stepno)
    {
      case ReportWizard_start:
        if ($hidden)
          $this->preservearrayattribute("categories");
        else
        {
          $result = $this->selectquery(CATEGORIES, $this->queries[CATEGORIES]);
          print("<p>Welcome to the reporting wizard. This wizard will allow you to display results or send out professor emails for one or more of the survey groups listed below.</p>\n");
          $this->selectshow(CATEGORIES, $result, "categories", "--- All Survey Categories ---");
        };
      break;
      case ReportWizard_output:
        if ($hidden)
        {
          $this->output->display(true);
          $this->includestandard->display(true);
          $this->includecustom->display(true);
          $this->includeabet->display(true);
          $this->includetext->display(true);
          $this->includeta->display(true);
          $this->includepies->display(true);
          $this->printable->display(true);
          $this->bighead->display(true);
        }
        else
        {
?>
<p><font color=red>These options aren't working right now.</font></p>
<p><b>Choose a report format:</b><br>
<? $this->output->display(ReportWizard_output_www,   "onclick=\"show('{$this->prefix}_wwwoptions')\""); ?> <? $this->output->displaylabel(ReportWizard_output_www, "Display Report on Web Page"); ?><br>
<? $this->output->display(ReportWizard_output_email, "onclick=\"hide('{$this->prefix}_wwwoptions')\""); ?> <? $this->output->displaylabel(ReportWizard_output_email, "Email Report to one or people"); ?><br>
<? $this->output->display(ReportWizard_output_pemail, "onclick=\"hide('{$this->prefix}_wwwoptions')\""); ?> <? $this->output->displaylabel(ReportWizard_output_pemail, "Email professor reports"); ?>
</p>

<p><b>Include these sections:</b><br>
<? $this->includestandard->display(); ?> <? $this->includestandard->displaylabel("Standard Questions"); ?><br>
<? $this->includecustom->display(); ?> <? $this->includecustom->displaylabel("Custom Questions"); ?><br>
<? $this->includeabet->display(); ?> <? $this->includeabet->displaylabel("ABET Questions"); ?><br>
<? $this->includetext->display(); ?> <? $this->includetext->displaylabel("Comments"); ?><br>
<? $this->includeta->display(); ?> <? $this->includeta->displaylabel("TA Ratings"); ?><br>
</p>

<div id="<?=$this->prefix?>_wwwoptions"<?=$this->output->checked == ReportWizard_output_email ? ' style="display:none"': ''?>>
<p><b>Options for web reports only:</b><br>
<? $this->printable->display(); ?> <? $this->printable->displaylabel("Produce a printable report"); ?><br>
<? $this->includepies->display(); ?> <? $this->includepies->displaylabel("Show Pie Graphs in Usage Statistics"); ?><br>
<? $this->bighead->display(); ?> <? $this->bighead->displaylabel("Exhaustive Class and Professor listings"); ?><br>
</p>
</div>
<?
        };
      break;
     case ReportWizard_qperiods:
        if ($hidden)
          $this->preservearrayattribute("questionperiods");
        else
        {
          $result = $this->selectquery(QUESTION_PERIODS, $this->queries[QUESTION_PERIODS]);
          print("<p>Surveys included in the report must come from the following question period(s):</p>\n");
          $this->selectshow(QUESTION_PERIODS, $result, "questionperiods", "--- Any Question Period ---");
        };
      break;
      case ReportWizard_depts:
        if ($hidden)
          $this->preservearrayattribute("departments");
        else
        {
          $result = $this->selectquery(DEPARTMENTS, $this->queries[DEPARTMENTS], true);
          print("<p>Surveys included in the report must come from classes in these departments:</p>\n");
          $this->selectshow(DEPARTMENTS, $result, "departments", "--- Any Department ---");
        }
      break;
      case ReportWizard_courses:
        if ($hidden)
          $this->preservearrayattribute("courses");
        else
        {
          $result = $this->selectquery(COURSES, $this->queries[COURSES], true);
          print("<p>Surveys included in the report must come from these courses:</p>\n");
          $this->selectshow(COURSES, $result, "courses", "--- Any Course ---");
        }
      break;
      case ReportWizard_profs:
        if ($hidden)
          $this->preservearrayattribute("professors");
        else
        {
          $result = $this->selectquery(PROFESSORS, $this->queries[PROFESSORS], true, true, true);
          print("<p>Surveys included in the report must come from classes with these professors:</p>\n");
          $this->selectshow(PROFESSORS, $result, "professors", "--- Any Professor ---");
        }
      break;
      case ReportWizard_aggreg:
        if ($hidden || $this->output->checked == ReportWizard_output_email)
        {
          $this->groupclasses->display(true);
          $this->groupcourses->display(true);
          $this->groupprofessors->display(true);
          $this->groupdepartments->display(true);
          $this->groupquestionperiods->display(true);
        }
        if (!$hidden)
        {
          if ($this->output->checked == ReportWizard_output_email)
            print("<p>Aggregation is not available in email reports. Choose a web based report to use this feature.</p>");
          else
          {
?>
<p><font size="-1">Check the survey properties below to determine how results are to be aggregated. Surveys that have the checked properties in common will have their scores added up and averaged together. For example, checking 'Departments' and 'Question Periods' will aggregate all surveys the come from the same department AND question period. To prevent any aggregation, select 'Classes' and 'Question Periods.' To aggregate all surveys together, leave all properties unchecked.</font></p>
<p>
<? $this->groupclasses->display(); ?> <? $this->groupclasses->displaylabel("Classes"); ?><br>
<? $this->groupcourses->display(); ?> <? $this->groupcourses->displaylabel("Courses"); ?><br>
<? $this->groupprofessors->display(); ?> <? $this->groupprofessors->displaylabel("Professors"); ?><br>
<? $this->groupdepartments->display(); ?> <? $this->groupdepartments->displaylabel("Departments"); ?><br>
<? $this->groupquestionperiods->display(); ?> <? $this->groupquestionperiods->displaylabel("Question Periods"); ?><br>
</p>
<?
          };
        };
      break;
      case ReportWizard_order:
        if ($hidden || $this->output->checked == ReportWizard_output_email)
          $this->order->display(true);

        if (!$hidden)
        {
          if($this->output->checked == ReportWizard_output_email)
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

    $report .= "Output: " . ($this->output->checked == ReportWizard_output_email ? "Professor Email" : "Web Report") . "\n";
    $report .= "Standard Questions: " . ($this->includestandard->checked ? "Yes" : "No") . "\n";
    $report .= "Custom Questions: " . ($this->includecustom->checked ? "Yes" : "No") . "\n";
    $report .= "ABET Questions: " . ($this->includeabet->checked ? "Yes" : "No") . "\n";
    $report .= "Comments: " . ($this->includetext->checked ? "Yes" : "No") . "\n";
    $report .= "TA Ratings: " . ($this->includeta->checked ? "Yes" : "No") . "\n";

    if ($this->output->checked == ReportWizard_output_www)
    {
      $report .= "Pie Graphs: " . ($this->includepies->checked ? "Yes" : "No") . "\n";
      $report .= "Printable: " . ($this->printable->checked ? "Yes" : "No") . "\n";
      $report .= "Exhaustive Listings: " . ($this->bighead->checked ? "Yes" : "No") . "\n";

      $first = true;
      $report .= "Group by: ";
      if ($this->groupclasses->checked) { if ($first) $first = false; else $report.=", "; $report.="Classes"; }
      if ($this->groupcourses->checked)  { if ($first) $first = false; else $report.=", "; $report.="Courses"; }
      if ($this->groupprofessors->checked)  { if ($first) $first = false; else $report.=", "; $report.="Professors"; }
      if ($this->groupdepartments->checked)  { if ($first) $first = false; else $report.=", "; $report.="Departments"; }
      if ($this->groupquestionperiods->checked) { if ($first) $first = false; else $report.=", "; $report.="Question Periods"; }
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
      $this->critshow("SELECT survey_category_id AS id, name FROM survey_categories WHERE survey_category_id IN $group ORDER BY survey_category_id", $report, "Category");
    }
    else
      $report .= "Category: ANY\n";

    $list = $this->criteria[QUESTION_PERIODS];
    if ($list)
    {
      $group = db_set($list);
      $this->critshow("SELECT question_period_id AS id, displayname AS name FROM question_periods WHERE question_period_id IN $group ORDER BY question_period_id DESC", $report, "Question Period");
    }
    else
      $report .= "Question Period: ANY\n";

    if ($this->criteria[DEPARTMENTS])
      $this->critshow($this->queries[DEPARTMENTS], $report, "Department");
    else
      $report .= "Department: ANY\n";

    if ($this->criteria[PROFESSORS])
      $this->critshow($this->queries[PROFESSORS], $report, "Professor");
    else
      $report .= "Professor: ANY\n";

    if ($this->criteria[COURSES])
      $this->critshow($this->queries[COURSES], $report, "Course");
    else
      $report .= "Course: ANY\n";

    return $report;
  }
}

?>
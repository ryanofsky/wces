<?

require_once("legacy/widgets/basic.inc");
require_once("legacy/widgets/wizard.inc");
require_once("legacy/widgets/dynamiclist.inc");
require_once("legacy/wces/database_selectmonster.inc");
require_once("wces/database.inc");

//---------------------------------------------------------------------------------------------
// legacy_Report legacy_Wizard

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

class legacy_ReportWizard extends legacy_Wizard
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

  var $depts = false;

  function legacy_ReportWizard($prefix,$form,$formmethod)
  {
    global $wces_path, $wces;
    
    $this->legacy_Wizard
    ( 
      array("Start","Output","Question Periods","Departments","Professors","Courses","Aggregation","Order","Finish"),
      $wces_path,
      $prefix,$form,$formmethod
    );
    
    if (!(LoginValue('status') & LOGIN_ADMIN))
    {
      $this->depts = array();
      $d = LoginValue('depts');    
      if (is_array($d) && count($d) > 0)
      {
        wces_connect();
        $r = pg_go("SELECT oldid FROM temp_dept WHERE newid IN (" . implode($d, ",") . ")", $wces, __FILE__, __LINE__);
        $n = pg_numrows($r);
        for($i = 0; $i < $n; ++$i)
          $this->depts[] = pg_result($r, $i, 0);      
      }
    }
  }

  function loadvalues()
  {
    global $db;
    
    //////////////////////////////// WIZARD SETUP ////////////////////////////////////////    
    
    legacy_Wizard::loadvalues(false);

    //////////////////////////////// WIDGET SETUP ////////////////////////////////////////    
    
    $this->output = new legacy_RadioButton(true, $this->prefix . "_output",$this->form,$this->formmethod);
    $this->includestandard = new legacy_CheckBox($this->prefix . "_includestandard", $this->form, $this->formmethod);
    $this->includecustom = new legacy_CheckBox($this->prefix . "_includecustom", $this->form, $this->formmethod);
    $this->includeabet = new legacy_CheckBox($this->prefix . "_includeabet", $this->form, $this->formmethod);
    $this->includetext = new legacy_CheckBox($this->prefix . "_includetext", $this->form, $this->formmethod);
    $this->includeta = new legacy_CheckBox($this->prefix. "_includeta", $this->form, $this->formmethod);
    $this->printable = new legacy_CheckBox($this->prefix . "_printable",$this->form,$this->formmethod);
    $this->includepies = new legacy_CheckBox($this->prefix . "_includepies", $this->form, $this->formmethod);
    $this->bighead = new legacy_CheckBox($this->prefix . "_bighead",$this->form,$this->formmethod);
    

    $this->groupclasses = new legacy_CheckBox($this->prefix . "_groupclasses", $this->form, $this->formmethod);
    $this->groupcourses = new legacy_CheckBox($this->prefix . "_groupcourses", $this->form, $this->formmethod);
    $this->groupprofessors = new legacy_CheckBox($this->prefix . "_groupprofessors", $this->form, $this->formmethod);
    $this->groupdepartments = new legacy_CheckBox($this->prefix . "_groupdepartments", $this->form, $this->formmethod);
    $this->groupquestionperiods = new legacy_CheckBox($this->prefix . "_groupquestionperiods", $this->form, $this->formmethod);
    
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
    
    $this->crawl(array(ReportWizard_aggreg => ($this->output->checked == ReportWizard_output_email) ? true : false, ReportWizard_order => ($this->output->checked == ReportWizard_output_email) ? true : false));

    //////////////////////////////// CRITERIA SETUP //////////////////////////////////////    

    $t = $this->loadattribute("topics");
    $q = $this->loadattribute("questionperiods");
    $d = $this->depts ? $this->depts : $this->loadattribute("departments");
    $p = $this->loadattribute("professors");
    $c = $this->loadattribute("courses");

    if (!is_array($t) || !(count($t) > 0))
    {
      if (!$db) $db = wces_connect();
      $t = array(mysql_result(db_exec("SELECT topicid FROM topics ORDER BY topicid LIMIT 1", $db, __FILE__, __LINE__),0));
    }
    else if (in_array("all",$t)) $t = false;
    
    if (!is_array($q) || !(count($q) > 0)) 
    {
      if (!$db) $db = wces_connect();
      $q = array(mysql_result(mysql_query("SELECT questionperiodid FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC LIMIT 1", $db),0));
    }
    else if (in_array("all",$q)) $q = false;
    
    if (!is_array($p) || !(count($p) > 0) || in_array("all",$p)) $p = false;
    if (!is_array($d) || !(count($d) > 0) || in_array("all",$d)) $d = false;
    if (!is_array($c) || !(count($c) > 0) || in_array("all",$c)) $c = false;
    
    $this->criteria = array("questionperiods" => $q, "departments" => $d, "professors" => $p, "courses" => $c, "topics" => $t);
    unset($q,$d,$p,$c,$t);

    /////////////////////////////// ORDERING SETUP ///////////////////////////////////////

    $labels = array("Question Period", "Department", "Professor", "Course", "Class Section");
    $this->ordernames = array("questionperiods", "departments", "professors", "courses", "classes");
    
    $this->order = new legacy_DynamicList(25,5,$labels, $this->prefix . "_order", $this->form, $this->formmethod);
    $this->order->loadvalues();

    //////////////////////////// SELECTMONSTER SETUP /////////////////////////////////////
    
    $this->query = new legacy_db_SelectMonster();
    $this->query->settable("answersets AS a");
    $this->query->addjoin("classes AS cl",db_SelectMonster_inner,"cl.classid","a.classid");
    $this->purequery = $this->query;
    $c = $this->criteria;
    if ($this->showstep == ReportWizard_start) $c["topics"] = false;
    if ($this->showstep == ReportWizard_qperiods) $c["questionperiods"] = false;
    if ($this->showstep == ReportWizard_depts && !$this->depts) $c["departments"] = false;
    if ($this->showstep == ReportWizard_profs) $c["professors"] = false;
    if ($this->showstep == ReportWizard_courses) $c["courses"] = false;
    $c["classes"] = false;
    $this->makecriteria($this->query,$c);
    $this->makecriteria($this->purequery,$this->criteria);
    unset($c);
  }

  function drawstep($stepno,$hidden)
  {  
    global $db;
    
    switch($stepno)
    {
      case ReportWizard_start:
        if ($hidden)
          $this->preservearrayattribute("topics");
        else
        {
          if (!$db) $db = wces_connect();
          $result = db_exec("SELECT topicid, name FROM topics ORDER BY topicid",$db,__FILE__,__LINE__);
          $criteria = $this->criteria["topics"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;

          print("<p>Welcome to the reporting wizard. This wizard will allow you to display results or send out professor emails for one or more of the survey groups listed below.</p>\n");
          print('<select name="' . $this->prefix . '_topics[]" size=5 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . '>--- All Survey Groups ---</option>' . "\n");
          while($row = mysql_fetch_array($result))
          {
            $topicid = $name = "";
            extract($row);
            $selected = $hash[$topicid] ? " selected" : "";
            print("<option value=\"$topicid\"$selected>$name</option>");
          }
          print("</select>\n");
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
<p><b>Choose a report format:</b><br>
<? $this->output->display(ReportWizard_output_www,   "onclick=\"show('" . $this->prefix . "_wwwoptions" . "')\""); ?> <? $this->output->displaylabel(ReportWizard_output_www, "Display Report on Web Page"); ?><br>
<? $this->output->display(ReportWizard_output_email, "onclick=\"hide('" . $this->prefix . "_wwwoptions" . "')\""); ?> <? $this->output->displaylabel(ReportWizard_output_email, "Email reports to professors"); ?>
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
          if (!$db) $db = wces_connect();
          $result = mysql_query("SELECT questionperiodid, year, semester, description FROM questionperiods ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
          $criteria = $this->criteria["questionperiods"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;

          print("<p>Surveys included in the report must come from the following question period(s):</p>\n");
          print('<select name="' . $this->prefix . '_questionperiods[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . '>--- Any Question Period ---</option>' . "\n");
          while($row = mysql_fetch_array($result))
          {
            $questionperiodid = ""; $year = ""; $semester = ""; $description = "";
            extract($row);
            $selected = $hash[$questionperiodid] ? " selected" : "";
            print("<option value=\"$questionperiodid\"$selected>" . ucwords($semester) . " $year $description</option>");
          }
          print("</select>\n");
        };
      break;
      case ReportWizard_depts:
        if ($hidden)
          $this->preservearrayattribute("departments");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("d.departmentid, d.name, d.code");
          $this->query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
          $this->query->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");
          $this->query->addgroup("d.departmentid");
          $this->query->addordering("d.code");
          $this->query->execute($db);
          $criteria = $this->criteria["departments"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;
 
          print("<p>Surveys included in the report must come from classes in these departments:</p>\n");
          print('<select name="' . $this->prefix . '_departments[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . '>--- Any Department ---</option>' . "\n");

          while($this->query->advance())
          {
            extract($this->query->row);
            $selected = $hash[$departmentid] ? " selected" : "";
            print("<option value=\"$departmentid\"$selected>$code - $name</option>\n");
          }
          print("</select>\n");
        }
      break;  
      case ReportWizard_courses:
        if ($hidden)
          $this->preservearrayattribute("courses");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename");
          $this->query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
          $this->query->addjoin("subjects AS s",db_SelectMonster_inner,"s.subjectid","c.subjectid");
          $this->query->addgroup("c.courseid");
          $this->query->addordering("coursename");
          $this->query->execute($db);
          $criteria = $this->criteria["courses"];
          if (!$criteria) $criteria = array("all");
          $hash = array();
          foreach($criteria as $value)
            $hash[$value] = true;

          print("<p>Surveys included in the report must come from these courses:</p>\n");
          print('<select name="' . $this->prefix . '_courses[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . ">--- Any Course ---</option>\n");
        
          while($this->query->advance())
          {
            extract($this->query->row);
            $selected = $hash[$courseid] ? " selected" : "";
            print("<option value=\"$courseid\"$selected>$coursename</option>");
          }          
          print("</select>\n");
        }
      break; 
      case ReportWizard_profs:
        if ($hidden)
          $this->preservearrayattribute("professors");
        else
        {
          if (!$db) $db = wces_connect();
          $this->query->addcolumn("p.professorid, p.name");
          $this->query->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
          $this->query->addgroup("p.professorid");
          $this->query->execute($db);
          $criteria = $this->criteria["professors"];
          if (!$criteria) $criteria = array("all");
          $hash= array();
          foreach($criteria as $value)
            $hash[$value] = true;
          print("<p>Surveys included in the report must come from classes with these professors:</p>\n");
          print('<select name="' . $this->prefix . '_professors[]" size=7 multiple>' . "\n");
          print('<option value="all"' . ($hash["all"] ? " selected" : "") . ">--- Any Professor ---</option>\n");
          $plist = array();
          while($this->query->advance())
          if ($name = $this->query->row["name"]) 
          {
            $pos = strrpos($name," ");
            array_push($plist,array("first" => substr($name,0,$pos), "last" => substr($name,$pos), "professorid" => $this->query->row["professorid"]));
          }
          usort($plist,"legacy_pcmp");
          foreach($plist as $p)
          {
            extract($p);
            $selected = $hash[$professorid] ? " selected" : "";
            print("<option value=\"$professorid\"$selected>$last, $first</option>");
          }  
          print("</select>\n");  
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
  
  function summarize()
  {
    global $db;
    if (!$db) $db = wces_connect();
     
    $report = "------------------------\nREPORT OPTIONS\n\n";
     
    $report .= "Output: " . ($this->output->checked == ReportWizard_output_email ? "Professor Email" : "Web legacy_Report") . "\n";
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
        $report .= $this->order->orderonlyitems[$item];  
      }
      $report .= "\n";
    }

    $report .= "\n------------------------\nREPORT CRITERIA\n\n";

    $list = $this->criteria["topics"];
    if ($list)
    {
      $group = db_set($list);
      $y = db_exec("SELECT topicid, name FROM topics WHERE topicid IN $group ORDER BY topicid",$db,__FILE__,__LINE__);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Topic: $name ($topicid)\n";
      };
    }
    else
      $report .= "Topic: ANY\n";

    $list = $this->criteria["questionperiods"];
    if ($list)
    {
      $group = db_set($list);
      $y = mysql_query("SELECT questionperiodid, year, semester, description FROM questionperiods WHERE questionperiodid IN $group ORDER BY year DESC, semester DESC, questionperiodid DESC",$db);
      while($row = mysql_fetch_array($y))
      {
        extract($row);
        $report .= "Question Period: " . ucwords($semester) . " $year $description ($questionperiodid)\n";
      };
    }
    else
      $report .= "Question Period: ANY\n";
        
    $q = $this->purequery;
    if ($this->criteria["departments"])
    {
      $q->addcolumn("d.departmentid, d.name, d.code");
      $q->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
      $q->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");
      $q->addgroup("d.departmentid");
      $q->addordering("d.code");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Department: $code $name ($departmentid)\n";
      };
    }
    else
      $report .= "Department: ANY\n";
    
    $q = $this->purequery;
    if ($this->criteria["professors"])
    {
      $q->addcolumn("p.professorid, p.name");
      $q->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
      $q->addgroup("p.professorid");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Professor: $name ($professorid)\n";
      };  
    }
    else
      $report .= "Professor: ANY\n";
       
    $q = $this->purequery;
    if ($this->criteria["courses"])
    {
      $q->addcolumn("c.courseid, concat(s.code, c.code, ' ', IFNULL(c.name,'')) AS coursename");
      $q->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
      $q->addjoin("subjects AS s",db_SelectMonster_inner,"s.subjectid","c.subjectid");
      $q->addgroup("c.courseid");
      $q->addordering("coursename");
      $q->execute($db);
      while($q->advance())
      {
        extract($q->row);
        $report .= "Course: $coursename ($courseid)\n";
      };
    }
    else
      $report .= "Course: ANY\n";
    
    return $report;     
  }
  
  function makecriteria(&$query,&$criteria)
  {
    if ($criteria["topics"])
    {
      $query->addwhere("a.topicid IN " . db_set($criteria["topics"]));
    };
    
    if ($criteria["questionperiods"])
    {
      $query->addwhere("qp.questionperiodid IN " . db_set($criteria["questionperiods"]));
      $query->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
    }; 
    
    if ($criteria["departments"])
    {
      $query->addwhere("d.departmentid IN " . db_set($criteria["departments"]));
      $query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
      $query->addjoin("departments AS d",db_SelectMonster_inner,"d.departmentid","c.departmentid");    
    };
    
    if ($criteria["professors"])
    {
      $query->addwhere("p.professorid IN " . db_set($criteria["professors"]));
      $query->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
    };
    
    if ($criteria["courses"])
    {
      $query->addwhere("c.courseid IN " . db_set($criteria["courses"]));
      $query->addjoin("courses AS c",db_SelectMonster_inner,"c.courseid","cl.courseid");
    }; 
    
    if (isset($criteria["classes"]) && $criteria["classes"])
    {
      $query->addwhere("cl.classid IN " . db_set($criteria["classes"]));
    }
  }
}

function legacy_pcmp($a, $b)
{
  return strcmp($a["last"],$b["last"]);
} 
?>
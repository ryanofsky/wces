<?

require_once("widgets/basic.inc");
require_once("wces/report_page.inc");
require_once("wces/report_generate.inc");

require_once("legacy/legacy.inc");
require_once("legacy/wces/cheesy.inc");
require_once("legacy/wces/report_page.inc");
require_once("legacy/wces/report_generate.inc");

define("ERA_LEGACY", 1);
define("ERA_CHEESY", 2);
define("ERA_POSTGRES", 3);

define('ProfessorResults_choose', 1);

class ProfessorResults extends ParentWidget
{
  var $csv = false;
  var $printable;
  var $results;
  
  function ProfessorResults($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->urlShortName('class_id');
    $this->urlShortName('question_period_id');
    $this->urlShortName('era');
    $this->urlShortName('printable');
  }

  function & loadState()
  {
    $mode = WIDGET_FORM | WIDGET_URL;
    
    $this->question_period_id = (int)$this->readValue('question_period_id', $mode);

    $cid = (int)$this->readValue('class_id', $mode);
    $this->class_id = $cid ? $cid : false;
    
    $era = $this->readValue('era', $mode);
    $this->era = isset($era) ? (int)$era : ERA_POSTGRES;
    
    $this->printable = (bool)$this->readValue('printable', $mode);
    
    if ($this->question_period_id)
    {
      $this->results =& new ResultPage($this->class_id, $this->question_period_id, $this->era, 'results', $this);
      $this->results->modal = true;
      $this->loadChild($this->results, true);
    }
  }
  
  function printVisible()
  {
    global $wces;
  
    $user_id = login_getuserid();
    wces_connect();
  
    print("<h3>" . login_getname() . " - Survey Responses</h3>\n");
  
    $result = pg_go("
      SELECT t.topic_id, t.class_id, get_class(t.class_id) AS class_info, q.question_period_id, q.displayname,
        (SELECT COUNT(DISTINCT user_id) FROM survey_responses AS sr WHERE sr.topic_id = t.topic_id) AS responses   
      FROM enrollments AS e
      INNER JOIN wces_topics AS t USING (class_id)
      INNER JOIN semester_question_periods AS q USING (question_period_id)
      WHERE e.user_id = $user_id AND e.status = 3 AND q.profdate < (SELECT NOW())
      ORDER BY q.enddate DESC, q.question_period_id DESC, class_info
    ", $wces, __FILE__, __LINE__);
  
    $q = new pg_segmented_wrapper($result, 'question_period_id');
    while($q->row)
    {
      extract($q->row);
      if ($q->split) print("<h4>$displayname</h4>\n<ul>\n");
      if ($responses == 0)
        print("  <li>" . format_class($class_info) . " (No Responses Found)</li>");
      else
        print("  <li><a href=\"" . $this->getUrl(array('class_id' => $class_id,
          'question_period_id' => $question_period_id, 'era' => ERA_POSTGRES))
          . "\">" . format_class($class_info) . "</a></li>");
      $q->advance();
      if ($q->split) print("</ul>\n");
    }

    // display links to fall 2001 data (stored in cheesyresponses mysql tables)
    
    list($db, $professor_id) = legacy_info();

    $questionperiods = db_exec("
      SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
      cl.classid, cl.divisioncode, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT cr.userid, cr.classid) AS hasanswers
      FROM classes AS cl
      INNER JOIN groupings AS g ON cl.classid = g.linkid AND g.linktype = 'classes'
      INNER JOIN questionperiods AS qp ON cl.year = qp.year AND cl.semester = qp.semester
      INNER JOIN courses AS c ON c.courseid = cl.courseid
      INNER JOIN subjects AS s USING (subjectid)
      LEFT JOIN cheesyresponses AS cr ON (cr.classid = cl.classid AND cr.questionperiodid = qp.questionperiodid)
      WHERE 8 <= qp.questionperiodid AND qp.questionperiodid <= 9 AND cl.professorid = $professor_id
      GROUP BY qp.questionperiodid, cl.classid
      ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
    ", $db, __FILE__, __LINE__);
  
    $sg = new legacy_SegmentedQuery($questionperiods, 'questionperiodid');
    $sg->advance();
    
    while ($sg->row)
    {
      $questionperiodid = $divisioncode = $semester = $year = $description = "";
      extract($sg->row);

      print("<h4>" . ucfirst($semester) . " $year $description</h4>\n");
      print("<ul>\n");
      
      do
      {
        $classid = $divisioncode = $scode = $code = $section = $name = $clname = $hasanswers = "";
        extract($sg->row);
        
        if ($clname) $name .= " - $clname";
        if ($hasanswers)
          print("  <li><a href=\"" . $this->getUrl(array('class_id' => $classid,
           'question_period_id' => $questionperiodid, 'era' => ERA_CHEESY))
            . "\">$scode $divisioncode$code <i>$name</i> Section $section</a></li>");
        else
          print("  <li>$scode $divisioncode$code <i>$name</i> Section $section (No Responses Found)</li>");
      }
      while ($sg->advance());

      print("</ul>");      
    }
  
    // display links to pre-fall 2001 data (stored in original mysql tables)
    $questionperiods = db_exec("
      SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
      cl.classid, s.code as scode, c.code, cl.divisioncode, cl.section, c.name, cl.name as clname, COUNT(DISTINCT a.answersetid) AS hasanswers
      FROM questionperiods AS qp
      INNER JOIN classes AS cl ON cl.year = qp.year AND cl.semester = qp.semester
      INNER JOIN courses AS c USING (courseid)
      INNER JOIN subjects AS s USING (subjectid)
      LEFT JOIN answersets AS a ON (a.classid = cl.classid AND a.questionperiodid = qp.questionperiodid)
      WHERE qp.questionperiodid <= 7 AND cl.professorid = $professor_id
      GROUP BY qp.questionperiodid, cl.classid
      ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
    ", $db, __FILE__, __LINE__);
  
    $sg = new legacy_SegmentedQuery($questionperiods, 'questionperiodid');
    $sg->advance();
    
    while ($sg->row)
    {
      $questionperiodid = $divisioncode = $semester = $year = $description = "";
      extract($sg->row);

      print("<h4>" . ucfirst($semester) . " $year $description</h4>\n");
      print("<ul>\n");
      
      $count = 0;
      
      do
      {
        $classid = $divisioncode = $scode = $code = $section = $name = $clname = $hasanswers = "";
        extract($sg->row);
        
        if ($clname) $name .= " - $clname";
        if ($hasanswers)
        {
          ++$count;
          print("  <li><a href=\"" . $this->getUrl(array('class_id' => $classid,
           'question_period_id' => $questionperiodid, 'era' => ERA_LEGACY))
            . "\">$scode $divisioncode$code <i>$name</i> Section $section</a></li>");
        }
        else
          print("  <li>$scode $divisioncode$code <i>$name</i> Section $section (No Responses Found)</li>");
      }
      while ($sg->advance());

     if ($count > 1)
       print("  <li><a href=\"" . $this->getUrl(array('question_period_id' =>
         $questionperiodid, 'era' => ERA_LEGACY)) 
         . "\">All Classes Combined</a></li>\n</ul>");

      print("</ul>");      
    }
  }
  
  function printModalChildren()
  {
    $this->printValue('question_period_id', $this->question_period_id, WIDGET_URL);
    $this->printValue('class_id', $this->class_id, WIDGET_URL);
    $this->printValue('era', $this->era, WIDGET_URL);
    print("<h3><a href=\"" . $this->getUrl(array('question_period_id' => false,
      'class_id' => false, 'era' => false, 'printable' => false)) 
      . "\">Back</a>");

    if (!$this->printable)
    {
      print(" | <a href=\"" . $this->getUrl(array('printable' => 1)) . "\">Printable Page</a>");
    }
    print("</h3>\n<hr>\n");
    ParentWidget::printModalChildren();
  }
}

class ResultPage extends ParentWidget
{
  var $done = false;
  
  function ResultPage($class_id, $question_period_id, $era, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->class_id = $class_id;
    $this->question_period_id = $question_period_id;
    $this->era = $era;
  }
  
  function printVisible()
  {
    $user_id = login_getuserid();

    if ($this->era == ERA_LEGACY)
    {
      list($db, $professor_id) = legacy_info();      
    
      $sqloptions = array ("standard" => true, "custom" => true);
      $groups = Array("classes" => $this->class_id ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
      $header = $ratings = $listclasses = $listprofessors = $abet = $responses = $tas = true;
      $sort = array("classes","questionperiods","professors","courses","departments");
      $criteria = array("professors" => array($professor_id), "classes" => $this->class_id ? array($this->class_id) : false, "topics" => false, "questionperiods" => array($this->question_period_id), "departments" => false, "courses" => false);
    
      legacy_report_makequeries($db, $sqloptions, $criteria, $groups,
        $sort, $header, $listclasses, $listprofessors, $ratings, $abet,
        $responses, $tas);
    
      $displayoptions = array("pies" => true);
    
      $outhtml = "<br>"; $text = false;
    
      legacy_report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
        $listprofessors, $ratings, $abet, $responses, $tas);
      
      print($outhtml);

    }
    else if ($this->era == ERA_CHEESY)
    {
      list($db, $professor_id) = legacy_info();
      $showcsv = false;
      cheesy_showresults($db, $this->question_period_id, $this->class_id, $professor_id, $showcsv);
    }
    else if ($this->era == ERA_POSTGRES)
    {
      $sqloptions = array ("standard" => true, "custom" => true);
    
      $criteria = array
      (
        PROFESSORS => array($user_id),
        CLASSES => array($this->class_id),
        COURSES => false,
        DEPARTMENTS => false,
        QUESTION_PERIODS => array($this->question_period_id),
        CATEGORIES => false
      );
    
      $sort = array(QUESTION_PERIODS, COURSES, DEPARTMENTS, PROFESSORS, CATEGORIES);
      $groups = array(QUESTION_PERIODS => 1, CLASSES => 1, COURSES => 1, DEPARTMENTS => 1, PROFESSORS => 1, CATEGORIES => 0);
    
      wces_connect();
      report_findtopics("rwtopics", $criteria);
      report_findgroups("rwtopics", $groups, $sort);
    
      $html = $text = false;
    
      $options = array('skipLines' => true);
      makeall(true, $html, $text, $groups, $options);      
 
      /*
      global $wces_path;
      print("<h4>Results Download</h4>\n");
      print("<p>Download the raw survey responses as a spreadsheet.</p>\n");
      print("<blockquote><a href=\"url\"><img src=\"{$wces_path}media/report/download.gif\" width=16 height=16 border=0 alt=\"results.csv\" align=absmiddle></a> <a href=\"url\">results.csv</a></blockquote>\n");
      */
    }
    else
      print("Unknown era value $this->era at " . __FILE__ . ':' . __LINE__ . "<br>\n");
  }
}

?>
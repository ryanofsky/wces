<%

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wces/report_page.inc");
  
  login_protect(login_professor);
  $profid = login_getprofid();

function listclasses()
{
  global $profid,$profname,$db;

  print("<h3>$profname - Survey Responses</h3>\n");

  $questionperiods = db_exec("
    SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
    cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT a.answersetid) AS hasanswers
    FROM questionperiods AS qp
    INNER JOIN classes AS cl ON cl.year = qp.year AND cl.semester = qp.semester
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN answersets AS a ON (a.classid = cl.classid AND a.questionperiodid = qp.questionperiodid)
    WHERE cl.professorid = '$profid'
    GROUP BY qp.questionperiodid, cl.classid
    ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
  ", $db, __FILE__, __LINE__);
  
  $count = $lqp = 0; $first = true;
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
    extract($questionperiod);

    if ($questionperiodid != $lqp)
    {
      if ($first)
        $first = false;
      else
      {
        if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");      
        print("</ul>");
      } 
      $count = 0;
      $lqp = $questionperiodid;
      print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
      print("<ul>\n");
    }

    if ($clname) $name .= " - $clname";
    if ($hasanswers)
    {
      ++$count;
      print("  <li><a href=\"seeresults.php?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
    }
    else
      print("  <li>$scode$code$section <i>$name</i> (No Responses Available)</li>");
  }
  
  if (!$first)
  {
    if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");      
    print("</ul>");
  } 
  
};  
  
function showresults($db,$questionperiodid,$classid)
{
  global $profid;
  
  print('<h3><a href="seeresults.php">Back</a></h3><hr>');
  
  $sqloptions = array ("standard" => true, "custom" => true);
  $groups = Array("classes" => $classid ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $header = $ratings = $listclasses = $listprofessors = $abet = $responses = $tas = true;
  $sort = array("classes","questionperiods","professors","courses","departments");
  $criteria = array("professors" => array($profid), "classes" => $classid ? array($classid) : false, "topics" => false, "questionperiods" => array($questionperiodid), "departments" => false, "courses" => false);
  
  report_makequeries($db, $sqloptions, $criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet, 
    $responses, $tas);
  
  $displayoptions = array("pies" => true);

  $outhtml = "<br>"; $text = false;

  report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
    $listprofessors, $ratings, $abet, $responses, $tas);
    
  print($outhtml);
}

page_top("Survey Results","0010");
$db = wces_connect();
$profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");

param($questionperiodid);
param($classid);

if ($questionperiodid)
  showresults($db,$questionperiodid,$classid);
else
  listclasses();

page_bottom();

%>









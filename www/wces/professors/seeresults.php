<%

  require_once("page.inc");
  require_once("database.inc");
  require_once("reporting.inc");
  
  login_protect(login_professor);
  $profid = login_getprofid();

  page_top("Survey Results","0010");
  $db = wces_connect();
  $profname = db_getvalue($db,"Professors",Array("professorid" => $profid),"name");

function listclasses()
{
  global $profid,$profname,$db;

  $sql = 
  "SELECT a.answersetid, a.questionperiodid, qp.year, qp.semester, qp.description, cl.classid, s.code as scode, c.code as code, cl.section, c.name
  FROM Classes as cl 
  INNER JOIN AnswerSets AS a USING (classid)
  LEFT JOIN QuestionPeriods as qp USING (questionperiodid)
  LEFT JOIN Courses AS c ON (cl.courseid = c.courseid)
  LEFT JOIN Subjects AS s USING (subjectid)
  WHERE cl.professorid = $profid
  ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, cl.classid";
  
  print("<h3>$profname - Survey Responses</h3>\n");
  
  $y = mysql_query($sql,$db);
  $oldquestionperiodid = 0;
  $oldclassid = 0;
  while($result = mysql_fetch_array($y))
  {
    extract($result);
    if ($oldquestionperiodid != $questionperiodid)
    {
      if ($oldquestionperiodid != 0) print("  <li><a href=\"?questionperiodid=$oldquestionperiodid\">All Classes Combined</a></li>\n</ul>\n");
      $semester = ucwords($semester);
      print("<h4>$semester $year - $description</h4>\n<ul>");    
    }
    if ($classid != $oldclassid || $oldquestionperiodid != $questionperiodid)
    {
      print("  <li><a href=\"?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
    };
    $oldquestionperiodid = $questionperiodid;
    $oldclassid = $classid;
  };
  if ($first)
    print("<p><b>No Responses Found</b></p>");
  else  
    print("  <li><a href=\"?questionperiodid=$oldquestionperiodid\">All Classes Combined</a></li>\n</ul>");
      
};  
  
function printresults($questionperiodid,$classid)
{
  global $profid,$profname,$db;

  $questionperiodid = addslashes($questionperiodid);
  $classid = addslashes($classid);

  $classcond = $classid ? " AND cl.classid = '$classid'" : "";
  
  $sql = "SELECT a.answersetid, a.questionsetid
  FROM AnswerSets AS a
  INNER JOIN Classes as cl USING (classid)
  LEFT JOIN Courses as c USING (courseid)
  LEFT JOIN Subjects as s USING (subjectid)
  WHERE a.questionperiodid = '$questionperiodid' AND cl.professorid = '$profid'$classcond
  ORDER BY a.questionsetid";

  $y = mysql_query($sql,$db);

  if (mysql_num_rows($y) == 0) { listclasses(); return; };
  
  print("<h3>$profname - Survey Responses (<a href=\"seeresults.php\">Back</a>)</h3>\n");
  
  $oldquestionsetid = 0;
  while($result = mysql_fetch_array($y))
  {
    extract($result);
    if ($questionsetid != $oldquestionsetid)
    {
      if ($ansids)
      {
        report_display($db,$ansids,true,true,true);
      };  
      $ansids = Array($answersetid);  
    }
    else
    {
      array_push($ansids,$answersetid);
    }
    $oldquestionsetid = $questionsetid;
  };
  report_display($db,$ansids,true,true,true);
  
  print("<p><b><a href=\"seeresults.php\">Back</a></b></p>");
  
};


if ($questionperiodid)
  printresults($questionperiodid,$classid);
else
  listclasses();

page_bottom();

%>
<?

require_once("wbes/surveyeditor.inc");
require_once("wces/page.inc");
require_once("wces/page.inc");
require_once("wces/database.inc");
require_once("wces/oldquestions.inc");
login_protect(login_professor);
$profid = login_getprofid();

param($editclass);
param($auto);

$db = wces_connect();
$editclass = db_getvalue($db,"classes",array("professorid" => $profid, "classid" => $editclass),"classid");
wces_GetCurrentQuestionPeriod($db, $questionperiodid, $description, $year, $semester);

$list = true;
$tiny = false;
$message = "";
if ($editclass)
{
  $q = new SurveyEditor("prefix","f",WIDGET_POST);
  $q->editclass = $editclass;
  $q->loadvalues();
  $list = $q->state == SurveyEditor_done;
  $tiny = $q->barepage;
  $message = $q->message;
}  
  
if ($tiny)
{
?>
<head>
<title>Preview Window</title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
</head>
<body class=preview>
<?
}
else
  page_top("Survey Builder");
  
if ($auto && $SurveyEditor_ns4)
{
?>
<p><font color=red><b>
We've detected that you are running Netscape 4.x and have directed you to a Netscape 4.x
compatible page. If, in fact, you are using a standards compliant browser follow
<a href="<?=$server_url->toString(false,false,true)?>">this link</a>.</b></font></p>
<?
} 

$thispage = $server_url->toString(false,true);

if ($list)
{
  print($message);
  $profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");
  global $db, $profid,$profname;
  print("<p>Choose a class from the list below to edit or preview its custom questions.</p>");
  print("<h3>List of Classes for $profname</h3>\n<UL>\n");
  
  $classes = db_exec("
  SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, cl.name AS clname, s.code as scode, COUNT(cr.userid) AS filled
  FROM classes AS cl
  INNER JOIN groupings AS g ON g.linktype = 'classes' AND g.linkid = cl.classid
  INNER JOIN courses AS c ON c.courseid = cl.courseid
  INNER JOIN subjects AS s USING (subjectid)
  LEFT JOIN cheesyresponses AS cr ON cr.classid = cl.classid AND cr.questionperiodid = $questionperiodid
  WHERE cl.professorid = '$profid'
  GROUP BY cl.classid
  ORDER BY s.code, c.code, cl.section
  ",$db,__FILE__,__LINE__);
  
  $first = true;    
  while ($class = mysql_fetch_array($classes))
  {
    $filled = $first = $classid = $section = $year = $semester = $code = $name = $scode = $clname = "";
    extract($class);
    if ($clname) $clname = "- $clname";
    $preview = "<A HREF=\"{$wces_path}administrators/info.php?surveyid=$classid&back={$wces_path}/professors/editsurveys.php\">Preview</A>\n";
    $edit = $filled ? "<i>no longer editable</i>" : "<A HREF=\"$thispage?editclass=$classid\">Edit</A>";
    print ("  <LI>$scode$code <i>$name$clname</i> (section $section) - $edit - $preview</LI>\n");
  }
  if ($first)
    print("None of your classes are being surveyed this semester.");
  print("</UL>\n");
}
else // !$list
{
  $q->dumpscript();
  print("<form name=f method=post action=\"$thispage?editclass=$editclass\">");
  $q->display();
  print("</form>\n<p>&nbsp;</p><p>&nbsp;</p>");
}
      
if ($tiny) print("</body>"); else page_bottom();

?>
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
  print("<p>Choose a class from the list below to edit its custom questions.</p>");
  print("<h3>List of Classes for $profname</h3>\n<UL>\n");
  
  wces_Findclasses($db,"currentclasses");
  $classes = db_exec("
  SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, cl.name AS clname, s.code as scode
  FROM currentclasses as cc
  INNER JOIN classes AS cl USING (classid)
  INNER JOIN courses AS c USING (courseid)
  INNER JOIN subjects AS s USING (subjectid)
  WHERE cl.professorid = '$profid'",$db,__FILE__,__LINE__);
  
  $first = true;    
  while ($class = mysql_fetch_array($classes))
  {
    $first = $classid = $section = $year = $semester = $code = $name = $scode = $clname = "";
    extract($class);
    if ($clname) $clname = "- $clname";
    print ("  <LI><A HREF=\"$thispage?editclass=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name$clname</i></A> (section $section)</LI>\n");
  }
  if ($first)
    print("None of your classes are being surveyed this semester.");
  print("</UL>\n");
}
else // !$list
{
  print("<form name=f method=post action=\"$thispage?editclass=$editclass\">");
  $q->dumpscript();
  $q->display();
  print("</form>\n<p>&nbsp;</p><p>&nbsp;</p>");
}
      
if ($tiny) print("</body>"); else page_bottom();

?>
<%

/*

Jackie,

The following people have been "unenrolled" from any classes that they did not survey. As long as they have filled out at least one survey they will appear on the list of people who completed all of their surveys.

acc40
bg171
cmt25
djh31
drt14
gb203
hdr25
jes141
kk357
hdr25
lkj10
nt102
pgs54
rj220
sc1204
sy192
wl318
yf91
yk478

One of these people, hdr25, said that the system was missing these two courses, Cell Biology (BIOL W4041) and Nucleic acids and Protein
synthesis BCHMG4026. These courses are not being evaluated through WCES.

I did not realize the problem was as widespread of this. Normally I wouldn't like to make changes to the login code during the middle of evaluations because it is so easy to break and so hard to debug. But I added an extra step in the login process that will clear out any old enrollments as the person logs in and keep only the newest data which is obtained directly from AcIS. Hopefully this will put an end to most of these login problems.


*/

require_once("wces/general.inc");
require_once("wces/wces.inc");



$db = wces_connect();
db_exec("lkdjlk",$db,__FILE__, __LINE__);

$questionperiodid = wces_Findquestionsetsta($db,"qsets");

$names = array("acc40", "bg171", "cmt25", "djh31", "drt14", "gb203", "hdr25", "jes141", "kk357", "lkj10", "nt102", "pgs54", "rj220", "sc1204", "sy192", "wl318", "yf91", "yk478");
foreach($names as $name)
{
  $db_debug = true;
  
  $userid = db_getvalue($db, "users", array("cunix" => $name), "userid");

  // delete enrollments in current semester, for classes with surveys that are not completed
  $classes = db_exec(
  "SELECT cl.classid FROM classes AS cl
  INNER JOIN qsets AS qs ON (qs.classid = cl.classid)
  INNER JOIN enrollments AS e ON e.userid = '$userid' AND e.classid = qs.classid
  LEFT JOIN answersets AS a ON a.questionsetid = qs.questionsetid AND a.classid = qs.classid AND a.questionperiodid = '$questionperiodid'
  LEFT JOIN completesurveys AS cs ON cs.answersetid = a.answersetid AND cs.userid = e.userid
  WHERE cs.userid IS NULL
  GROUP BY cl.classid", $db, __FILE__, __LINE__);
  
  $classset = "";
  while($row = mysql_fetch_assoc($classes))
  {
    if ($classset) $classset .= ", ";
    $classset .= $row["classid"];
  }
  print("$name - $classset<br>");
  if ($classset) db_exec("DELETE FROM enrollments WHERE classid IN ($classset) AND userid = '$userid'", $db, __FILE__, __LINE__);
  print("<hr>");
}

%>

<?

require_once("wbes/server.inc");
require_once("team/login.inc");

$server_isproduction = false;
//$db_debug = true;

function team_connect()
{
  $db = server_mysqlinit();
  mysql_select_db("team",$db);
  return $db;
};


$wces_path = "/wces/";
$team_path = "/team/";
$wces_this = $server_url->toString(false,true,true);
$wces_ns4 = substr($HTTP_SERVER_VARS["HTTP_USER_AGENT"],0,9) == "Mozilla/4" && strpos($HTTP_SERVER_VARS["HTTP_USER_AGENT"], "compatible;") === false;

function page_top($title, $minimal = false)
{
  global $wces_this, $team_path, $wces_path, $LOGIN_PROMPTING, $wces_ns4, $db, $student_open;
  $bgcolor = $minimal ? "FFFFFF" : "0065AD";
  
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd"> 
<html>
<head>
<title><?=$title?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<LINK REL="stylesheet" type="text/css" href="<?=$wces_path?>media/style.css">
<script>
<!--

function showhelp(topic)
{
  var remote = open("<?=$wces_path?>help.php?help=" + topic,"help","height=435,width=435,alwaysLowered=0,alwaysRaised=0,channelmode=0,dependent=0,directories=0,fullscreen=0,hotkeys=1,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=1,toolbar=0,z-lock=0");
  remote.focus();
}

// -->
</script>
</head>
<body class=main>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr> 
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
<tr> 
  <td colspan=2 bgcolor="#3387DA"><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt=spacer></td>
</tr>
<tr> 
  <td colspan=2> 
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr bgcolor="#FFFDEC"> 
      <td><a href="<?=$team_path?>index.php"><img src="<?=$team_path?>Left_Logo.jpg" width=96 height=72 border=0 alt=Home></a></td>
      <td><img src="<?=$team_path?>Middle_Header.gif" width=396 height=72 alt="Columbia University Web Based Course Evaluation System"></td>
      <td bgcolor="#FFFDEC" align=right><img src="<?=$wces_path?>media/page/Right_Logo.gif" width=113 height=72 alt=Crown></td>
    </tr>
    </table>
  </td>
</tr>
</table><table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr> 
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
<? if ($minimal) { ?>
</table>
<? } else { ?>
<tr> 
  <td valign=top bgcolor="#DEDEDE" class=sidelinks>
<?
  $uni = login_getuni();
  if ($uni == "rey4") $student_open = true;
  $status = login_getstatus();
  if ($uni)
  {}
  //  page_link("Log off $uni","login/logout.php",true);
  //else
  //  page_link("Log In","index.php", !$LOGIN_PROMPTING);

//  if ($uni)
//    page_link("Home", "index.php", "${team_path}index.php" == $wces_this || $team_path == $wces_this ? false : true);
  
  $nospace = true;
 
  if ($status & login_administrator)
  {
//    if ($nospace) $nospace = false; else page_spacer();
//    page_link("Student Usage",           "administrators/susage.php");
//    page_link("Professor Usage",         "administrators/pusage.php");
//    page_link("Mass Mailing",            "administrators/massmail.php");
//    page_link("Reporting Wizard",        "administrators/report.php");
//    page_link("Fake Login",              "administrators/fakelogin.php");
//    page_link("Enrollment Viewer",       "administrators/enrollment.php");
//    page_link("Professor Listing",       "administrators/pstats.php");
//    page_link("Semester Initialization", "administrators/semester.php");
//    page_link("Data Upload",             "administrators/import.php");
  }

  if (($status & login_user))
  {
//    if (!isset($team)) $team = wces_connect();
//    
//    $userid = login_getuserid();
//    
//    $classes = db_exec("
//      SELECT cl.classid, c.name
//      FROM enrollments AS e
//      INNER JOIN groupings AS g ON e.classid = g.linkid AND g.linktype = 'classes'
//      INNER JOIN classes AS cl ON g.linkid = cl.classid AND year = '$year' AND semester = '$semester'
//      INNER JOIN courses AS c USING (courseid)
//      LEFT JOIN cheesyresponses AS cr ON cr.userid = e.userid AND cr.classid = e.classid AND cr.questionperiodid = '$questionperiodid'
//      WHERE e.userid = '$userid' AND cr.userid IS NULL
//      ", $db, __FILE__, __LINE__);
//    
//    if (mysql_num_rows($classes) > 0) { if ($nospace) $nospace = false; else page_spacer(); }
//    
//    while($row = mysql_fetch_assoc($classes))
//    {
//      page_link($row["name"],"students/survey.php?classid=" . $row["classid"],"","");
//    }
  }
  
?>
  </td>
  <td valign="top" width="99%" background="<?=$wces_path?>media/page/Background_Logo.gif"> 
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr>
      <td valign="top"> 
<? } // if minimal ?>
<div class=pagecontents>
<!-- BEGIN PAGE CONTENT -->
<?
}

function page_bottom($minimal = false)
{
  global $wces_path, $oracle_root;
?>
<!-- END PAGE CONTENT -->
</div>
<? if ($minimal) { ?>
<table width="100%" border=0 cellpadding=0 cellspacing=0>
<? } else { ?>
      </td>
      <td background="<?=$wces_path?>media/page/Right_LengthBanner.gif"><img src="<?=$wces_path?>media/nada.gif" width="44" height="8"></td>
    </tr>
    </table>
  </td>
</tr>
<? } // if minimal ?>
  <tr>
    <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width="1" height="3"></td>
  </tr>
  <tr> 
    <td bgcolor="">&nbsp;</td>
    <td bgcolor="" align=right class=extlinks>
      Team Developer is copyrighted by Jack McGourty, Ph.D. and Kenneth P. DeMeuse, Ph.D. 2001
   </td>
  </tr>  
  
</table>
</body>
</html>
<?
}

function page_link($caption,$link,$extlink = "",$params='style="white-space: nowrap;"')
{
  global $team_path, $wces_this;
  if (((boolean)$extlink) === $extlink)
  {
    $islink = $extlink;
    $extlink = "";
  }  
  else
    $islink = strpos($wces_this, "$team_path$link") !== 0;

  if ($islink)  
    print("    <p><a href=\"$team_path$link$extlink\" class=navlinks $params>$caption</a></p>\n");
  else
    print("    <p class=navlinks $params>$caption</p>\n");
}

function page_spacer()
{
  global $wces_path;
  print("    <p><img src=\"$wces_path/media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");
}

function page_showhelp($topic)
{
  global $wces_path;
  print("<a href=\"javascript:showhelp('$topic')\"><img src=\"$wces_path/media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
}

?>
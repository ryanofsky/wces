<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/oracle.inc");

$wces_path = "/wces/";
$wces_this = $server_url->path;

function page_top($title, $minimal = false)
{
  global $wces_this, $wces_path, $LOGIN_PROMPTING;
  $bgcolor = $minimal ? "FFFFFF" : "0065AD";
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd"> 
<html>
<head>
<title><?=$title?></title>
<LINK REL="stylesheet" type="text/css" href="<?=$wces_path?>media/style.css">
<script>
<!--

function showhelp(topic)
{
  var remote = open("<?=$wces_path?>help.php?help=" + topic,"help","height=400,width=400,alwaysLowered=0,alwaysRaised=0,channelmode=0,dependent=0,directories=0,fullscreen=0,hotkeys=1,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=1,toolbar=0,z-lock=0");
  remote.focus();
}

// -->
</script>
</head>
<body background="<?=$wces_path?>media/0x<?=$bgcolor?>.gif" topmargin="0" leftmargin="0" rightmargin="0" marginwidth="0" marginheight="0" link="black" alink="#000000" vlink="#003256">
<table width="100%"<? if (!$minimal) { ?> height="100%"<? } ?> border=0 cellpadding=0 cellspacing=0>
<tr><td colspan=2 height=15 bgcolor="#0065AD"><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
<tr>
  <td colspan=2 bgcolor="#FFE051" background="<?=$wces_path?>media/0xFFE051.gif" height=54>
    <a href="<?=$wces_path?>index.php"><img src="<?=$wces_path?>media/orangecrown.gif" width=50 height=54 border=0 align="absmiddle" alt="SEAS"></a>
    <font face="Trebuchet MS, Arial, Helvetica, sans-serif" size=6 why="netscape4sucks">
    <span class="title">Course Evaluations</span>
    </font>
  </td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$wces_path?>media/0xC0C0C0.gif"><img src="<?=$wces_path?>media/nada.gif" width=150 height=15 alt="[ spacer ]"></td>
  <td width="100%" height=15 bgcolor="#0065AD"><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<? if ($minimal) { ?>
</table>
<? } else { ?>
<tr>
  <td bgcolor="#C0C0C0" background="media/0xC0C0C0.gif" valign=top>
  <div class=spaced>
<?
  $uni = login_getuni();
  $status = login_getstatus();

  if ($uni)
    page_link("Log off $uni","login/logout.php",true);
  else
    page_link("Log In","index.php", !$LOGIN_PROMPTING);

  if ($uni)
    page_link("Home", "index.php", "${wces_path}index.php" == $wces_this || $wces_path == $wces_this);
  
  $nospace = true;
 
  if ($status & login_administrator)
  {
    if ($nospace) $nospace = false; else page_spacer();
    page_link("Student Usage",               "administrators/susage.php");
    page_link("Professor Usage",             "administrators/pusage.php");
    page_link("Mass Mailing",                "administrators/massmail.php");
    page_link("Reporting Wizard",            "administrators/report.php");
    page_link("Fake Login",                  "administrators/fakelogin.php");
    page_link("Enrollment Viewer",           "administrators/enrollment.php");
    page_link("Professor Listing",           "administrators/pstats.php");
    page_link("New Semester Initialization", "administrators/semester.php");
    page_link("Data Upload",                 "administrators/import.php");
  }

  if ($status & login_professor)
  {
    if ($nospace) $nospace = false; else page_spacer();
    if ($status & login_knownprofessor)
    {
      page_link("Upcoming Surveys", "professors/editsurveys.php");
      page_link("Survey Results",   "professors/seeresults.php");
      page_link("Survey Responses", "professors/respond/multiclasses.php");
      page_link("Past Averages",    "professors/respond/historygraph.php");
      page_link("Edit Profile",     "professors/infoedit.php");
    }
    page_link("Update Association", "login/profsearch.php");
  }
  
  if ($status & login_student)
  {
    if ($nospace) $nospace = false; else page_spacer();
    page_link("Survey #1",      "students/survey.php?surveyid=1");
    page_link("Survey #2",      "students/survey.php?surveyid=2");
    page_link("Email Settings", "students/optout.php");
  }
  
  if ($nospace) $nospace = false; else page_spacer();
  page_link("System Overview", "about/overview.php");
  page_link("Feedback", "about/feedback.php");
  page_link("Credits", "about/credits.php");

  
?>
  </div>
  </td>
  <td width="100%" height="100%" bgcolor="#FFFFFF" background="<?=$wces_path?>media/0xFFFFFF.gif" valign=top rowspan=2><?
} // if minimal ?>
  <div class=spaced>

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
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$wces_path?>media/0xC0C0C0.gif" align=center valign=bottom height=70><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt="[ spacer ]"><br><a href="<?= $oracle_root ?>"><img src="<?=$wces_path?>media/oraclebutton.jpg" width=130 height=40 border=0 alt="Oracle"></a><br><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<? } // if minimal ?>
<tr><td colspan=2 height=15 bgcolor="#0065AD"><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
</table>

</body>
</html>
<?
}

function page_link($caption,$link,$islink = 0)
{
  global $wces_path, $wces_this;
  if (((boolean)$islink) !== $islink)
    $islink = "$wces_path$link" != $wces_this;
  if ($islink)  
    print("    <p><a href=\"$wces_path$link\" class=navlinks>$caption</a></p>\n");
  else
    print("    <p class=navlinks>$caption</p>\n");
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
<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/oracle.inc");

$wces_path = "/";
$wces_this = $server_url->toString(false,true,true);
$wces_ns4 = substr($HTTP_SERVER_VARS["HTTP_USER_AGENT"],0,9) == "Mozilla/4" && strpos($HTTP_SERVER_VARS["HTTP_USER_AGENT"], "compatible;") === false;

$wces_closed = true;

function page_top($title, $minimal = false)
{
  global $wces_this, $wces_path, $LOGIN_PROMPTING, $wces_ns4, $QSID, $ASID, $survey_listing, $wces, $wces_closed;
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
<?=$headPrintStr?>
</head>
<body class=main><?=$absolutePrintStr?>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr>
  <td colspan=2 bgcolor="#007700" class=sidelinks><img src="<?=$wces_path?>media/nada.gif" width=1 height=10 alt=spacer></td>
</tr>
<tr>
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
<tr>
  <td colspan=2>
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr bgcolor="#FFFDEC">
      <td><img src="<?=$wces_path?>media/nada.gif" width=3 height=83 alt=spacer><a href="<?=$wces_path?>index.php<?=$QSID?>"><img src="<?=$wces_path?>media/dart/thayer-logo-sm.gif" width=154 height=78 border=0 alt=Home></a><br><img src="<?=$wces_path?>media/nada.gif" width=1 height=5 alt=spacer></td>
      <td><img src="<?=$wces_path?>media/dart/wces.jpg" width=390 height=23 alt="Columbia University Web Based Course Evaluation System"></td>
    </tr>
    </table>
  </td>
</tr>
<tr>
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
</table><table width="100%" border=0 cellspacing=0 cellpadding=0>
<? if ($minimal) { ?>
</table>
<? } else { ?>
<tr>
  <td valign=top bgcolor="#DEDEDE" class=sidelinks>
<?
  $user_id = login_getuserid();
  $status = login_getstatus();

  if ($user_id)
  {
    page_link("Log off","login/logout.php$QSID",true);
    if ($fake = login_getfake())
      page_link("Log off fake", "administrators/info.php?nofake=1$ASID", true);
  }
  else
    page_link("Log In","index.php$QSID", !$LOGIN_PROMPTING);

  if ($user_id)
    page_link("Home", "index.php$QSID", "${wces_path}index.php" == $wces_this || $wces_path == $wces_this ? false : true);

  if ($status & login_administrator)
  {
    page_spacer();
    page_link("Edit Surveys",     "administrators/surveys.php",  $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
    page_link("Student Usage",    "administrators/susage.php", $QSID);
    page_link("Professor Usage",  "administrators/pusage.php", $QSID);
    page_link("Course Database",  "administrators/info.php", $QSID);
    page_link("Reporting Wizard", "administrators/report.php", $QSID);
    page_link("Mass Mailing",     "administrators/massmail.php", $QSID);
    page_link("Data Import",      "administrators/import.php", $QSID);
    page_link("Survey Remove",    "administrators/surveyslasher.php", $QSID);
  }

  if ($status & login_professor)
  {
    if ($status & login_knownprofessor)
    {
      page_spacer();
      page_link("Upcoming Surveys", "professors/surveys.php", $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
      page_link("Survey Results",   "professors/seeresults.php", $QSID);
      //page_link("Survey Responses", "professors/respond/multiclasses.php", $QSID);
      //page_link("Past Averages",    "professors/respond/historygraph.php", $QSID);
      //page_link("Edit Profile",     "professors/infoedit.php", $QSID);
    }
  }

  if (!$wces_closed && ($status & login_student))
  {
    wces_connect();
    
    $result = pg_query("
      SELECT question_period_id, displayname, year, semester
      FROM semester_question_periods
      WHERE question_period_id = (SELECT get_question_period())
    ", $wces, __FILE__, __LINE__);

    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
    
    $user_id = login_getuserid();
    wces_connect();
    
    $survey_listing = pg_query("
      SELECT t.class_id, get_class(t.class_id) AS name,
        CASE WHEN sr.user_id IS NULL THEN 0 ELSE 1 END AS surveyed
      FROM wces_topics AS t
      INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = t.class_id AND e.status = 1
      LEFT JOIN survey_responses AS sr ON sr.topic_id = t.topic_id AND sr.question_period_id = $question_period_id AND sr.user_id = $user_id
      INNER JOIN classes AS cl ON cl.class_id = e.class_id AND cl.year = $year AND cl.semester = $semester
      GROUP BY sr.user_id, t.class_id
      ORDER BY surveyed, name
    ", $wces, __FILE__, __LINE__);
    
    $n = pg_numrows($survey_listing);

    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($survey_listing,$i,PGSQL_ASSOC));
      if ($surveyed) break;
      if ($i == 0) page_spacer();
      page_link(format_class($name,0,0,1,0,0,0), "students/survey.php?class_id=$class_id", $QSID, "");
    }
  }

  page_spacer();
  page_link("System Overview", "about/overview.php", $QSID);
  page_link("Feedback", "about/feedback.php", $QSID);
  page_link("Credits", "about/credits.php", $QSID);
?>
  </td>
  <td valign="top" width="99%" background="<?=$wces_path?>media/dart/back.gif">
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
</table>
</body>
</html>
<?
  $headPrintStr = $absolutePrintStr = false;
}

function page_link($caption,$link,$extlink = "",$params='style="white-space: nowrap;"')
{
  global $wces_path, $wces_this;

  if (is_string($extlink))
  {
    $islink = strpos($wces_this, "$wces_path$link") !== 0;
  }
  else
  {
    $islink = $extlink;
    $extlink = "";
  }

  if ($islink)
    print("    <p><a href=\"$wces_path$link$extlink\" class=navlinks $params>$caption</a></p>\n");
  else
    print("    <p class=navlinks $params>$caption</p>\n");
}

function page_spacer()
{
  global $wces_path;
  static $first = true;
  if ($first)
    $first = false;
  else
    print("    <p><img src=\"{$wces_path}media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");
}

function page_showhelp($topic)
{
  global $wces_path;
  print("<a href=\"javascript:showhelp('$topic')\"><img src=\"$wces_path/media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
}

?>

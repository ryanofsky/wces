<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/oracle.inc");

$wces_this = $server_url->toString(false,true,true);
$wces_ns4 = isset($_SERVER["HTTP_USER_AGENT"]) && substr($_SERVER["HTTP_USER_AGENT"],0,9) == "Mozilla/4" && strpos($_SERVER["HTTP_USER_AGENT"], "compatible;") === false;

$absolutePrintStr = $headPrintStr = "";

$page_links = 'page_links';

function page_links()
{
  global $wces_this, $wces_path, $LOGIN_PROMPTING, $wces_ns4, $QSID, $ASID, $survey_listing, $wces, $wces_closed, $absolutePrintStr, $headPrintStr;      
  $uni = login_getuni();
  $user_id = login_getuserid();
  $status = login_getstatus();

  if ($user_id)
  {
    page_link("Log off $uni","login/logout.php$QSID",true);
    if ($fake = login_getfakeuni())
      page_link("Log on as $fake", "administrators/info.php?nofake=1$ASID", true);
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
    page_link("Enrollment Database",  "administrators/info.php", $QSID);
    page_link("Reporting Wizard", "administrators/choose.php", $QSID);
    page_link("Mass Mailing",     "administrators/massmail.php", $QSID);
    page_link("Data Import",      "administrators/import.php", $QSID);
    page_link("Question Periods", "administrators/questionperiods.php", $QSID);
    page_link("Survey Remove",    "administrators/surveyslasher.php", $QSID);
  }

  if ($status & login_deptadmin)
  {
    page_spacer();
    page_link("Reporting Wizard", "administrators/report.php", $QSID);
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

  if ($status & login_student)
  {
    $survey_listing = get_surveys();
    $wces_closed = !$survey_listing;

    if ($survey_listing)
    {
      $n = pg_numrows($survey_listing);
      for($i = 0; $i < $n; ++$i)
      {
        extract(pg_fetch_array($survey_listing,$i,PGSQL_ASSOC));
        if ($surveyed) break;
        if ($i == 0) page_spacer();
        $cn = format_class($name,'%n');
        if (!$cn) $cn = format_class($name, '%c');
        page_link($cn, "students/survey.php?class_id=$class_id&question_period_id=$question_period_id", $ASID, "");
      }
    }
  }

  page_spacer();
  page_link("System Overview", "about/overview.php", $QSID);
  page_link("Feedback", "about/feedback.php", $QSID);
  page_link("Credits", "about/credits.php", $QSID);
}

function get_surveys()
{
  global $wces;
  static $survey_listing;
  
  if (!isset($survey_listing))
  {
    wces_connect();
   
    $user_id = login_getuserid();
    $survey_listing = pg_go("
      SELECT q.question_period_id, t.specialization_id AS topic_id, t.class_id, get_class(t.class_id) AS name,
        CASE WHEN sr.user_id IS NULL THEN 0 ELSE 1 END AS surveyed
      FROM question_periods AS q
      INNER JOIN question_periods_topics AS qt USING (question_period_id)
      INNER JOIN wces_topics AS t ON t.specialization_id = qt.topic_id
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN enrollments AS e ON e.user_id = $user_id AND e.class_id = cl.class_id AND e.status = 1
      LEFT JOIN survey_responses AS sr ON sr.specialization_id = t.specialization_id AND sr.question_period_id = q.question_period_id AND sr.user_id = $user_id
      WHERE q.begindate < (SELECT NOW()) AND q.enddate > (SELECT NOW())
      GROUP BY q.question_period_id, t.specialization_id, t.class_id, sr.user_id
      ORDER BY surveyed, name
    ", $wces, __FILE__, __LINE__);      
    
    if (pg_numrows($survey_listing) == 0)
    {
      $result = pg_go("
        SELECT EXISTS(SELECT * FROM semester_question_periods 
          WHERE begindate < (SELECT NOW()) AND enddate > (SELECT NOW()))
      ", $wces, __FILE__, __LINE__);
      if (pg_result($result,0,0) == 'f') $survey_listing = false;
    }
  }
  return $survey_listing;
}

function page_top($title, $minimal = false)
{
  global $page_links, $wces_this, $wces_path, $LOGIN_PROMPTING, $wces_ns4, $QSID, $ASID, 
    $wces, $wces_closed, $absolutePrintStr, $headPrintStr;
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
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
<tr>
  <td colspan=2 bgcolor="#68ACFF"><img src="<?=$wces_path?>media/nada.gif" width=1 height=15 alt=spacer></td>
</tr>
<tr>
  <td colspan=2>
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr bgcolor="#FFFDEC">
      <td><a href="<?=$wces_path?>index.php<?=$QSID?>"><img src="<?=$wces_path?>media/page/Left_Logo.gif" width=164 height=72 border=0 alt=Home></a></td>
      <td><img src="<?=$wces_path?>media/page/Middle_Header.gif" width=396 height=72 alt="Columbia University Web Based Course Evaluation System"></td>
      <td bgcolor="#FFFDEC" align=right><img src="<?=$wces_path?>media/page/Right_Logo.gif" width=113 height=72 alt=Crown></td>
    </tr>
    </table>
  </td>
</tr>
</table><table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr>
  <td bgcolor="#68ACFF">&nbsp;</td>
  <td bgcolor="#68ACFF" align=right class=extlinks>
    <a href="http://www.columbia.edu/">Columbia University</a> |
    <a href="http://www.engineering.columbia.edu/">SEAS</a> |
    <a href="<?=$wces_path?>about/feedback.php<?=$QSID?>">Feedback</a>
  </td>
</tr>
<tr>
  <td colspan=2 bgcolor="#000000"><img src="<?=$wces_path?>media/nada.gif" width=1 height=3 alt=spacer></td>
</tr>
<? if ($minimal) { ?>
</table>
<? } else { ?>
<tr>
  <td valign=top bgcolor="#DEDEDE" class=sidelinks>
<? $page_links(); ?>
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

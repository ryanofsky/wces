<!-- 68ACFF to  #007700 #007700 -->

<?

require_once("wbes/server.inc");
require_once("wces/login.inc");
require_once("wces/oracle.inc");

$wces_path = "/";
$wces_this = $server_url->toString(false,true,true);
$wces_ns4 = substr($HTTP_SERVER_VARS["HTTP_USER_AGENT"],0,9) == "Mozilla/4" && strpos($HTTP_SERVER_VARS["HTTP_USER_AGENT"], "compatible;") === false;

function page_top($title, $minimal = false)
{
  global $wces_this, $wces_path, $LOGIN_PROMPTING, $wces_ns4, $QSID, $ASID;
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
  $uni = login_getuni();
  $status = login_getstatus();

  if ($user_id)
  {
    page_link("Log off $uni","login/logout.php$QSID",true);
    if ($fake = login_getfakeuni())
      page_link("Log on as $fake", "administrators/info.php?nofake=1$ASID", true);
  }
  else
    page_link("Log In","index.php$QSID", !$LOGIN_PROMPTING);

  if ($uni)
    page_link("Home", "index.php$QSID", "${wces_path}index.php" == $wces_this || $wces_path == $wces_this ? false : true);

  $nospace = true;

  if ($status & login_administrator)
  {
    if ($nospace) $nospace = false; else page_spacer();
    page_link("Student Usage",    "administrators/susage.php", $QSID);
    page_link("Professor Usage",  "administrators/pusage.php", $QSID);
    page_link("Course Database",  "administrators/info.php", $QSID);
    page_link("Reporting Wizard", "administrators/report.php", $QSID);
    page_link("Mass Mailing",     "administrators/massmail.php", $QSID);
    page_link("Data Import",      "administrators/import.php", $QSID);
  }

  if ($status & login_professor)
  {
    if ($nospace) $nospace = false; else page_spacer();
    if ($status & login_knownprofessor)
    {
      page_link("Upcoming Surveys", "professors/editsurveys.php", $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
      page_link("Survey Results",   "professors/seeresults.php", $QSID);
      //page_link("Survey Responses", "professors/respond/multiclasses.php", $QSID);
      //page_link("Past Averages",    "professors/respond/historygraph.php", $QSID);
      //page_link("Edit Profile",     "professors/infoedit.php", $QSID);
    }
  }

  if (false && ($status & login_student))
  {
    // show classes
  }

  if ($nospace) $nospace = false; else page_spacer();
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
  print("    <p><img src=\"{$wces_path}media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");
}

function page_showhelp($topic)
{
  global $wces_path;
  print("<a href=\"javascript:showhelp('$topic')\"><img src=\"$wces_path/media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
}

?>
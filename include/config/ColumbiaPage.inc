<?

class ColumbiaPage extends WcesPage
{
  var $loggingIn = false;
  
  function printTop($title)
  {
    global $QSID, $WCES_CONFIG_PATH, $WCES_CONFIG_MEDIA_PATH, $WCES_CONFIG_HELP_PATH;
    
    // not neccessary but makes output less cluttered when db_debug is enabled
    //get_surveys();
    
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd">
<html>
<head>
<title><?=$title?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<LINK REL="stylesheet" type="text/css" href="<?=$WCES_CONFIG_MEDIA_PATH?>style.css">
<script>
<!--

function showhelp(topic)
{
  var remote = open("<?=$WCES_CONFIG_HELP_PATH?>?help=" + topic,"help","height=435,width=435,alwaysLowered=0,alwaysRaised=0,channelmode=0,dependent=0,directories=0,fullscreen=0,hotkeys=1,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=1,toolbar=0,z-lock=0");
  remote.focus();
}

// -->
</script>
<?=html_head()?>
</head>
<body class=main><?=html_body()?>
<? if ($this->mode != WCES_PAGE_BARE) { ?>
<table width="100%" border=0 cellspacing=0 cellpadding=0>
<tr>
  <td colspan=2 bgcolor="#000000"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width=1 height=3 alt=""></td>
</tr>
<tr>
  <td colspan=2 bgcolor="#68ACFF"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width=1 height=15 alt=""></td>
</tr>
<tr>
  <td colspan=2>
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr bgcolor="#FFFDEC">
      <td><a href="<?=$WCES_CONFIG_PATH?>index.php<?=$QSID?>"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>page/Left_Logo.gif" width=164 height=72 border=0 alt=Home></a></td>
      <td><img src="<?=$WCES_CONFIG_MEDIA_PATH?>page/Middle_Header.gif" width=396 height=72 alt="Columbia University Web Based Course Evaluation System"></td>
      <td bgcolor="#FFFDEC" align=right><img src="<?=$WCES_CONFIG_MEDIA_PATH?>page/Right_Logo.gif" width=113 height=72 alt=Crown></td>
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
    <a href="<?=$WCES_CONFIG_PATH?>about/feedback.php<?=$QSID?>">Feedback</a>
  </td>
</tr>
<tr>
  <td colspan=2 bgcolor="#000000"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width=1 height=3 alt=""></td>
</tr>
<? if ($this->mode == WCES_PAGE_PRINTABLE) { ?>
</table>
<? } else { ?>
<tr>
  <td valign=top bgcolor="#DEDEDE" class=sidelinks>
<? $this->printLinks(); ?>
  </td>
  <td valign="top" width="99%" background="<?=$WCES_CONFIG_MEDIA_PATH?>page/Background_Logo.gif">
    <table width="100%" border=0 cellspacing=0 cellpadding=0>
    <tr>
      <td valign="top">
<? } // $this->mode != WCES_PAGE_PRINTABLE ?>
<? } // $this->mode != WCES_PAGE_BARE ?>
<div class=pagecontents>
<!-- BEGIN PAGE CONTENT -->
<?
  }

  function printBottom()
  {
    global $WCES_CONFIG_MEDIA_PATH;
?>
<!-- END PAGE CONTENT -->
</div>
<? if ($this->mode != WCES_PAGE_BARE) { ?>
<? if ($this->mode == WCES_PAGE_PRINTABLE) { ?>
<table width="100%" border=0 cellpadding=0 cellspacing=0>
<? } else { ?>
      </td>
      <td background="<?=$WCES_CONFIG_MEDIA_PATH?>page/Right_LengthBanner.gif"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width="44" height="8"></td>
    </tr>
    </table>
  </td>
</tr>
<? } // $this->mode != WCES_PAGE_PRINTABLE ?>
  <tr>
    <td colspan=2 bgcolor="#000000"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width="1" height="3"></td>
  </tr>
</table>
<? } // $this->mode != WCES_PAGE_BARE ?>
</body>
</html>
<?
  }

  function printLinks()
  {
    global $wces_this, $WCES_CONFIG_PATH, $wces_ns4, $QSID, $ASID;      

    $login =& LoginInstance();

    $uni = $login->get('uni');
    $user_id = $login->get('user_id');
    $status = $login->get('status');
  
    if ($user_id)
    {
      $this->printLink("Log off $uni","login/logout.php$QSID",true);
      if ($fake = $login->get('fake_uni'))
        $this->printLink("Log on as $fake", "administrators/info.php?nofake=1&user_id=$user_id$ASID", true);
    }
    else
      $this->printLink("Log In","index.php$QSID", !$this->loggingIn);
      
    if ($user_id)
      $this->printLink("Home", "index.php$QSID", "{$WCES_CONFIG_PATH}index.php" == $wces_this || $WCES_CONFIG_PATH == $wces_this ? false : true);
  
    if ($status & LOGIN_ADMIN)
    {
      $this->printSpacer();
      $this->printLink("Edit Surveys",     "administrators/surveys.php",  $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
      $this->printLink("Student Usage",    "administrators/susage.php", $QSID);
      $this->printLink("Professor Usage",  "administrators/pusage.php", $QSID);
      $this->printLink("Enrollment Database", "administrators/info.php", $QSID);
      $this->printLink("Reporting Wizard", "administrators/report_choose.php", $QSID);
      $this->printLink("Mass Mailing",     "administrators/massmail.php", $QSID);
      $this->printLink("Data Import",      "administrators/import.php", $QSID);
      $this->printLink("Question Periods", "administrators/questionperiods.php", $QSID);
      $this->printLink("Survey Remove",    "administrators/surveyslasher.php", $QSID);
    }
  
    if ($status & LOGIN_DEPT_ADMIN)
    {
      $this->printSpacer();
      $this->printLink("Reporting Wizard", "administrators/report.php", $QSID);
    }
  
    if ($status & LOGIN_PROFESSOR)
    {
      if ($status & LOGIN_KNOWN_PROFESSOR)
      {
        $this->printSpacer();
        $this->printLink("Upcoming Surveys", "professors/surveys.php", $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
        $this->printLink("Survey Results",   "professors/seeresults.php", $QSID);
        $this->printLink("Edit Profile",     "professors/profile.php", $QSID);
        $this->printLink("Comment On Results", "professors/new_prof.php", $QSID);
        //$this->printLink("Survey Responses", "professors/respond/multiclasses.php", $QSID);
        //$this->printLink("Past Averages",    "professors/respond/historygraph.php", $QSID);
        //$this->printLink("Edit Profile",     "professors/infoedit.php", $QSID);
      }
    }
  
    if ($status & LOGIN_STUDENT)
    {
      $this->printSpacer();
      $survey_listing = get_surveys();
      $wces_closed = !$survey_listing;
  
      if ($survey_listing)
      {
        $n = pg_numrows($survey_listing);
        for($i = 0; $i < $n; ++$i)
        {
          extract(pg_fetch_array($survey_listing,$i,PGSQL_ASSOC));
          if ($surveyed) break;
          $cn = format_class($name,'%n');
          if (!$cn) $cn = format_class($name, '%c');
          $this->printLink($cn, "students/survey.php?topic_id=$topic_id", $ASID, null, "");
        }
      }
      $this->printLink("TA Registration", "students/taregister.php", $QSID);
    }
  
    $this->printSpacer();
    $this->printLink("System Overview", "about/overview.php", $QSID);
    $this->printLink("Feedback", "about/feedback.php", $QSID);
    $this->printLink("Credits", "about/credits.php", $QSID);
  }

  function printLink($caption, $link, $suffix = null, $prefix = null, $params='style="white-space: nowrap;"')
  {
    global $wces_this;
    
    if (!isset($prefix)) $prefix = $GLOBALS['WCES_CONFIG_PATH'];
  
    if (is_string($suffix))
      $islink = strpos($wces_this, "$prefix$link") !== 0;
    else
    { // if suffix is not a string it is interpreted as a boolean
      // that determines whether or not to display a hyperlink
      $islink = (bool)$suffix;
      $suffix = "";
    }
 
    if ($islink)
      print("    <p><a href=\"$prefix$link$suffix\" class=navlinks $params>$caption</a></p>\n");
    else
      print("    <p class=navlinks $params>$caption</p>\n");
  }
  
  function printSpacer()
  {
    global $WCES_CONFIG_MEDIA_PATH;
    static $first = true;
    if ($first)
      $first = false;
    else
      print("    <p><img src=\"{$WCES_CONFIG_MEDIA_PATH}0x000000.gif\" align=absmiddle width=140 height=2 alt=\"\"></p>\n");
  }

  function printHelp($topic)
  {
    global $WCES_CONFIG_HELP_PATH;
    print("<a href=\"javascript:showhelp('$topic')\"><img src=\"{$WCES_CONFIG_HELP_PATH}help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
  }   
}

$wces_this = $server_url->toString(false,true,true);
$wces_ns4 = isset($_SERVER["HTTP_USER_AGENT"]) && substr($_SERVER["HTTP_USER_AGENT"],0,9) == "Mozilla/4" && strpos($_SERVER["HTTP_USER_AGENT"], "compatible;") === false;


?>
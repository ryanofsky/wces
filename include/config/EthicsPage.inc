<?

class EthicsPage extends WcesPage
{
  var $loggingIn = false;
  
  function printTop($title)
  {
    global $QSID, $WCES_CONFIG_PATH, $WCES_CONFIG_MEDIA_PATH, $WCES_CONFIG_HELP_PATH;
    
    // not neccessary but makes output less cluttered when db_debug is enabled
    //get_surveys();
    
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
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
<script language=javascript>
function setCellColor(id, color) {
	document.all[id].bgColor = color;
}
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
      <td background="<?=$WCES_CONFIG_MEDIA_PATH?>page/Right_LengthBanner.gif"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width=44 height=8 alt=""></td>
    </tr>
    </table>
  </td>
</tr>
<? } // $this->mode != WCES_PAGE_PRINTABLE ?>
  <tr>
    <td colspan=2 bgcolor="#000000"><img src="<?=$WCES_CONFIG_MEDIA_PATH?>nada.gif" width=1 height=3 alt=""></td>
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
     
    $this->printLink("Home", "index.php$QSID", "{$WCES_CONFIG_PATH}index.php" == $wces_this || $WCES_CONFIG_PATH == $wces_this ? false : true);
  
    $this->printSpacer();
    $this->printLink("Take Survey", "survey.php", $QSID);
    $this->printLink("Edit Survey", "edit.php", $wces_ns4 ? "/ns4?auto=1$ASID" : $QSID);
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
      print("<hr noshade>\n");
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

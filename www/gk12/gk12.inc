<?

require_once("wbes/server.inc");

$wces_this = $server_url->path;

$gk_path = "/gk12/";
$wces_path = "/wces/";

function page_top($title, $minimal = false)
{
  global $wces_this, $server_media, $LOGIN_PROMPTING, $gk_path, $wces_path;
  $bgcolor = $minimal ? "FFFFFF" : "0065AD";
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd"> 
<html>
<head>
<title><?=$title?></title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
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
<body background="<?=$server_media?>/0x<?=$bgcolor?>.gif" topmargin="0" leftmargin="0" rightmargin="0" marginwidth="0" marginheight="0" link="black" alink="#000000" vlink="#003256">
<table width="100%"<? if (!$minimal) { ?> height="100%"<? } ?> border=0 cellpadding=0 cellspacing=0>
<tr><td colspan=2 height=15 bgcolor="#0065AD"><img src="<?=$server_media?>/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
<tr>
  <td colspan=2 bgcolor="#FFE051" background="<?=$server_media?>/0xFFE051.gif" height=54>
    <a href="<?=$gk_path?>index.php"><img src="<?=$server_media?>/orangecrown.gif" width=50 height=54 border=0 align="absmiddle" alt="SEAS"></a>
    <font face="Trebuchet MS, Arial, Helvetica, sans-serif" size=6 why="netscape4sucks">
    <span class="title">Course Evaluations</span>
    </font>
  </td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$server_media?>/0xC0C0C0.gif"><img src="<?=$server_media?>/nada.gif" width=150 height=15 alt="[ spacer ]"></td>
  <td width="100%" height=15 bgcolor="#0065AD"><img src="<?=$server_media?>/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<? if ($minimal) { ?>
</table>
<? } else { ?>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$server_media?>/0xC0C0C0.gif" valign=top>
  <div class=spaced>
<?
  page_link("Home", "index.php", "${gk_path}index.php" == $wces_this || $gk_path == $wces_this);
  
  page_link("Survey Form", "survey.php");
  page_link("Survey Results", "results.php");
  page_link("Survey Builder", "builder.php");

  
?>
  </div>
  </td>
  <td width="100%" height="100%" bgcolor="#FFFFFF" background="<?=$server_media?>/0xFFFFFF.gif" valign=top rowspan=2><?
} // if minimal ?>
  <div class=spaced>

<!-- BEGIN PAGE CONTENT -->

<?
}

function page_bottom($minimal = false)
{
  global $gk_path, $oracle_root;
?>

<!-- END PAGE CONTENT -->

  </div>
<? if ($minimal) { ?>
<table width="100%" border=0 cellpadding=0 cellspacing=0>
<? } else { ?>
  </td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$server_media?>/0xC0C0C0.gif" align=center valign=bottom height=70><img src="<?=$server_media?>/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<? } // if minimal ?>
<tr><td colspan=2 height=15 bgcolor="#0065AD"><img src="<?=$server_media?>/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
</table>

</body>
</html>
<?
}

function page_link($caption,$link,$islink = 0)
{
  global $gk_path, $wces_this;
  if (((boolean)$islink) !== $islink)
    $islink = "$gk_path$link" != $wces_this;
  if ($islink)  
    print("    <p><a href=\"$gk_path$link\" class=navlinks>$caption</a></p>\n");
  else
    print("    <p class=navlinks>$caption</p>\n");
}

function page_spacer()
{
  global $gk_path;
  print("    <p><img src=\"$server_media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");
}

function page_showhelp($topic)
{
  global $gk_path;
  print("<a href=\"javascript:showhelp('$topic')\"><img src=\"$server_media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
}

?>
<?

$testroot = "/test";

function test_showhelp($topic)
{
  global $testroot;
  print("<a href=\"javascript:showhelp('$topic')\"><img src=\"$testroot/media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a>");
}

function test_top($title, $status="", $classid=0)
{
  global $testroot;
  $page = server_getpage();
?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" "http://www.w3.org/TR/REC-html40/loose.dtd"> 
<html>
<head>
<title><?=$title?></title>
<LINK REL="stylesheet" type="text/css" href="<?=$testroot?>/media/style.css">
<script>
<!--
var remote = open("<?=$testroot?>/remote.html","remote","height=120,width=200,alwaysLowered=0,alwaysRaised=0,channelmode=0,dependent=0,directories=0,fullscreen=0,hotkeys=1,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=1,toolbar=0,z-lock=0");
if (remote.opener == null) remote.opener = window;
remote.opener.name = "bigwindow";

function showhelp(topic)
{
  var remote = open("<?=$testroot?>/help.php?help=" + topic,"help","height=400,width=400,alwaysLowered=0,alwaysRaised=0,channelmode=0,dependent=0,directories=0,fullscreen=0,hotkeys=1,location=0,menubar=0,resizable=1,scrollbars=0,status=0,titlebar=1,toolbar=0,z-lock=0");
  remote.focus();
}


// -->
</script>
</head>
<body background="<?=$testroot?>/media/0x0065AD.gif" topmargin="0" leftmargin="0" rightmargin="0" marginwidth="0" marginheight="0" link="black" alink="#0065AD" vlink="#0065AD">
<table width="100%" height="100%" border=0 cellpadding=0 cellspacing=0>
<tr><td colspan=2 height=15><img src="<?=$testroot?>/media/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
<tr>
  <td colspan=2 bgcolor="#FFE051" background="<?=$testroot?>/media/0xFFE051.gif" height=54>
    <a href="<?$testroot?>?status=<?=$status?>"><img src="<?=$testroot?>/media/orangecrown.gif" width=50 height=54 border=0 align="absmiddle" alt="SEAS"></a>
    <font face="Trebuchet MS, Arial, Helvetica, sans-serif" size=6 why="netscape4sucks">
    <span class="title">Course Evaluations</span>
    </font>
  </td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$testroot?>/media/0xC0C0C0.gif"><img src="<?=$testroot?>/media/nada.gif" width=150 height=15 alt="[ spacer ]"></td>
  <td width="100%" height=15><img src="<?=$testroot?>/media/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="media/0xC0C0C0.gif" valign=top>
  <div class=spaced>
<?
  if ($status)
  {
    print("    <p><a href=\"${testroot}/logout.php\" class=navlinks>Log off rey4</a></p>\n");
  }  
  else
  {
    if ($page == "$testroot/index.php" || $page == $testroot || $page == "$testroot/")
      print("    <p class=navlinks>Log In</p>\n");
    else
      print("    <p><a href=\"$testroot/index.php\" class=navlinks>Log In</a></p>\n");
  }  
?>

<?
  $options = array(
    "System Overview" => "${testroot}/overview.php",
    "Feedback" => "${testroot}/feedback.php",
    "Credits" => "${testroot}/credits.php"
  );

  foreach($options as $label => $link)
  {
    if ($link == $page)
      print("    <p class=navlinks>$label</p>\n");
    else  
      print("    <p><a href=\"$link?status=$status\" class=navlinks>$label</a></p>\n");
  }

if ($status == "student")
{
  $options = array(
    "Email Settings" => "${testroot}/student/optout.php"
  );  
}
else if ($status == "professor")
  $options = array(
    "Upcoming Surveys" => "$testroot/professor/edit.php",
    "Survey Results" => "$testroot/professor/seeresults.php",
    "Edit Profile" => "$testroot/professor/info.php"
  );
else if ($status == "administrator")
{
  $options = array(
    "Student Usage" => "$testroot/administrator/susage.php",
    "Professor Usage" => "$testroot/administrator/pusage.php",
    "Survey Results" => "$testroot/administrator/results.php",
    "Email Services" => "$testroot/administrator/email.php",
    "Database Editor" => "$testroot/administrator/dedit.php",
    "Fake Logon" => "$testroot/administrator/fake.php"
  );
}
else
  $options = array();

if (count($options))
{
  print("    <p><img src=\"$testroot/media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");

  if ($page == "${testroot}/index.php")
    print("    <p class=navlinks>" . ucfirst($status) . " Home</p>\n");
  else
    print("    <p><a href=\"${testroot}/index.php?status=$status\" class=navlinks>" . ucfirst($status) . " Home</a></p>\n");
  
  foreach($options as $label => $link)
  {
    if ($link == $page)
      print("    <p class=navlinks>$label</p>\n");
    else  
      print("    <p><a href=\"$link\" class=navlinks>$label</a></p>\n");
  }
}

if ($status == "student")
{
  print("    <p><img src=\"$testroot/media/0x000000.gif\" align=absmiddle width=140 height=2 alt=\"[ spacer ]\"></p>\n");
  
  print("    <p class=navlinks>Surveys<br>\n");
  $options = array(
    1 => "COMS3210 Discrete Math",
    2 => "COMS3156 Software Engineering"
  );

  foreach($options as $number => $label)
  {
    if ($number == $classid)
      print("    <div class=hanging><img src=\"$testroot/media/dot.gif\" width=10 height=7 alt=\"[ bullet ]\">$label</div>\n");
    else
      print("    <div class=hanging><img src=\"$testroot/media/dot.gif\" width=10 height=7 alt=\"[ bullet ]\"><a href=\"$testroot/student/survey.php?classid=$number\">$label</a></div>\n");
  }
}
?>
  </div>
  </td>
  <td width="100%" height="100%" bgcolor="#FFFFFF" background="<?=$testroot?>/media/0xFFFFFF.gif" valign=top rowspan=2>
  <div class=spaced>

<!-- BEGIN PAGE CONTENT -->

<?
}

function test_bottom()
{
  global $testroot;
?>

<!-- END PAGE CONTENT -->

  </div>
  </td>
</tr>
<tr>
  <td bgcolor="#C0C0C0" background="<?=$testroot?>/media/0xC0C0C0.gif" align=center valign=bottom height=70><img src="<?=$testroot?>/media/nada.gif" width=1 height=15 alt="[ spacer ]"><br><a href="/oracle/"><img src="<?=$testroot?>/media/oraclebutton.jpg" width=130 height=40 border=0 alt="Oracle"></a><br><img src="<?=$testroot?>/media/nada.gif" width=1 height=15 alt="[ spacer ]"></td>
</tr>
<tr><td colspan=2 height=15><img src="<?=$testroot?>/media/nada.gif" width=1 height=15 alt="[ spacer ]"></td></tr>
</table>
</body>
</html>
<?
}
<?

require_once("wbes/general.inc");
require_once("wbes/server.inc");
$page = $server_url->path;
param($frame); param($help);

?>
<html>
<head>
<title>Help</title>
<LINK REL="stylesheet" type="text/css" href="media/style.css">
</head>
<?
if($frame == "top")
{
?>
<body bgcolor="#FFE051"><p align=center><strong>Course Evaluation Online Help</strong></p></body>
<?  
}
else if($frame == "bottom")
{
?>
<body bgcolor="#0065AD">
<script language="JavaScript">
<!--

var happy = document.images && window.Image;

// fix for problematic Netscape 3 string object
if (happy && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

function swap(img)
{
  if(happy)
  {
    var i = document.images[img];
    var temp = i.src;
    i.src = i.flipimage.src;
    i.flipimage.src = temp;
  };
}

function AttachImage(img, filename)
{
  if(happy)
  {
    var i = document.images[img];
    var p = i.src.lastIndexOf("/");
    var q = i.src.lastIndexOf("\\");
    i.flipimage = new Image();  
    i.flipimage.src = i.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
  };
}
// -->
</script>
<p align=right><a target=main href="<?=$page?>?frame=main&help=contents"
onmouseover="swap('b1')" onmouseout="swap('b1')"><img name=b1
src="media/help/contents.gif" alt="Contents" width=20 height=20
border=0></a><a target=main href="<?=$page?>?frame=main&help=feedback"
onmouseover="swap('b2')" onmouseout="swap('b2')"><img name=b2
src="media/help/reply.gif" alt="Reply" width=20 height=20
border=0></a><a href="javascript:void(top.close())"
onmouseover="swap('b3')" onmouseout="swap('b3')"><img name=b3
src="media/help/close.gif" alt="Close" width=20 height=20
border=0></a></p>
<script language="javascript">
<!--
  AttachImage("b1","contents1.gif");
  AttachImage("b2","reply1.gif");
  AttachImage("b3","close1.gif");
// -->
</script>
</body>
<?  
}
else if ($frame == "main")
{
  if ($help == "login")
  {
?>
<p>You need to have a valid CUNIX ID and password use the course evaluation system.</p>
<p align=center><img src="media/help/login.gif"></p>
<p>Students and professors
can use the <a href="http://www.columbia.edu/acis/accounts/create/current.html" target="_blank">AcIS Account Maintenance</a>
page to create and manage CUNIX accounts.</p>
<?
  }
  else if ($help == "contents")
  {
?>
<p><strong>Table Of Contents</strong></p>
<ul>
  <li><a href="help.php?frame=main&help=login">Login Page</a></li>    
</ul>
<?
  }
  else if ($help == "feedback")
  {
?>
  <p>Thank You for visiting the SEAS Oracle and WCES. Send us your feedback so we can make improvements to the site.</p>
<form>
<p><strong>Email Address (Optional):</strong><br><input type=text name=email size=30></p>
<p><strong>Comments:</strong><br><textarea name=comments rows=7 cols=30></textarea></p>
<p><input type=submit name=submit value="Submit"></p>
</form>
<?    
  }
  else
  {
    print("Topic '$help' not found.");
  }
}
else
{
?>
<frameset rows="23,*,20" border=0>
  <frame name=top  marginheight=0  src="<?=$page?>?frame=top">
  <frame name=main src="<?=$page?>?frame=main&help=<?=$help?>">
  <frame name=bot  marginheight=0 src="<?=$page?>?frame=bottom">
</frameset>
<?
}
?>
</html>
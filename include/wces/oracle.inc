<?
require_once("wbes/server.inc");

$oracle_root = "/oracle/";

function oracle_top($title,$headimage)
{
  global $oracle_root;

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<!-- This page was created by Mark Sainz & Alexander Haubold for Columbia University, 1999-->
<html>
<head>
<title><?=$title?></title>

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

<style type=text/css>
<!--
A:link       { FONT-WEIGHT: bold; COLOR: #483d8b; TEXT-DECORATION: none }
A:visited    { FONT-WEIGHT: bold; COLOR: #483d8b; TEXT-DECORATION: none }
A:active     { COLOR: #ffa500; TEXT-DECORATION: none }
A:hover      { COLOR: #ff0000 }
A.ff         { COLOR: #000000; TEXT-DECORATION: none }
A.ff:visited { COLOR: #000000; TEXT-DECORATION: none }
A.ff:hover   { COLOR: #ff0000; TEXT-DECORATION: none }
body         { font-family: Verdana,Trebuchet MS, Arial, Helvetica, sans-serif; }
p            { font-family: Verdana,Trebuchet MS, Arial, Helvetica, sans-serif; }

-->
</style>

</head>
<body text=#000000 vlink=#5d478b alink=#ff0000 link=#0000ee background="<?=$oracle_root?>media/tile.jpg">
<table cellspacing=0 cellpadding=5 width="100%" border=0>
<tr>
  <td align=center><img src="<?=$oracle_root?>media/oracle_logo.gif" width=228 height=50></td>
  <td align=center><?=$headimage ? "<img src=\"${oracle_root}media/$headimage\">" : '&nbsp;'?></td>
</tr>
<tr>
  <td valign=top height="50%">
    <table cellspacing=0 cellpadding=0 width=235 border=0>
    <tr><td><a href="<?=$oracle_root?>aboutus.php"    onmouseover="swap('b1')" onmouseout="swap('b1')"><img name="b1" src="<?=$oracle_root?>media/aboutus_off.gif"    alt="About Us" width=235 height=87 border=0></a></td></tr>
    <tr><td><a href="<?=$oracle_root?>survey.php"     onmouseover="swap('b2')" onmouseout="swap('b2')"><img name="b2" src="<?=$oracle_root?>media/survey_off.gif"     alt="Survey" width=235 height=49 border=0></a></td></tr>
    <tr><td><a href="<?=$oracle_root?>index.php"      onmouseover="swap('b4')" onmouseout="swap('b4')"><img name="b4" src="<?=$oracle_root?>media/home_off.gif"       alt="Home" width=235 height=64 border=0></a></td></tr>
    </table>

<script language="javascript">
<!--
  AttachImage("b1","aboutus_on.gif");
  AttachImage("b2","survey_on.gif");
  //AttachImage("b3","background_on.gif");
  AttachImage("b4","home_on.gif");
// -->
</script>

  </td>
  <td rowspan=2>
<?

};

function oracle_bottom()
{
  global $oracle_root;
?>

  </td>
</tr>
<tr>
  <td valign=top height="50%">
    <table border=10 bordercolor="#677BA4" bordercolorlight="#B2C5DF" bordercolordark="#122761" cellpadding=10 cellspacing=1>
    <tr><td><p align=center><a href="<?=$oracle_root?>oracle_enter.html"><img src="<?=$oracle_root?>media/comein.gif" width=150 height=35 border=0></a></p></td></tr>
    <tr><td><p align=center><a href="<?=$oracle_root?>oracle_enter.html">Consult the oracle</a>.</p></td></tr>

<noscript>
    <tr><td>
      <p><b>Warning: </b>Your browser does not support javascript
      or does not have javascript enabled. If you are using Netscape or Internet Explorer,
      you can follow <a href="http://www.chami.com/tips/internet/021398I.html">these
      instructions</a> to enable javascript. Without javascript, you may have difficulties
      navigating some parts of the oracle web page.</b></p>
    </td></tr>
</noscript>

    </table>
  </td>
</tr>
</table>

<?

}

?>
<?
require_once("gk12.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_text.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");
?>
<html>
<head>
<title>GK12 Fellows Survey</title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>

<body bgcolor="#6699CC" link="#FFCC00">

<?

$db = server_mysqlinit();
mysql_select_db("wces",$db);

if (isset($HTTP_POST_VARS["submit"]))
{
  $d = addslashes(serialize($HTTP_POST_VARS));
  db_exec("INSERT INTO cheesy(dump) VALUES ('$d')", $db, __FILE__, __LINE__);
  print("Your responses have been saved.");
}
else
{
  if (isset($surveyid))
    $result = db_exec("SELECT dump FROM cheesy WHERE cheesyid = '" . addslashes($surveyid) . "'", $db, __FILE__, __LINE__);
  else  
    $result = db_exec("SELECT dump FROM cheesy AS ch, currentsurvey AS c WHERE c.id = ch.cheesyid", $db, __FILE__, __LINE__);
      
  if (mysql_num_rows($result) == 0)    
    print("<h2>Survey not found!</h2>");
    
  $survey = unserialize(mysql_result($result,0));
  
  print("<form name=f method=post><br>\n");
  
  foreach(array_keys($survey->components) as $i)
  {
    $c = &$survey->components[$i];
    $w = $c->getwidget("gk12_$i","f", WIDGET_POST);
    $w->loadvalues();
    print("<div>");
    $w->display();
    print("</div>\n<br>\n");
  }
  
  print('<input type=hidden name=surveyid value="$surveyid">');
  print('<input type=submit name=submit value="Submit Responses" class=tinybutton>');
  
  print("</form>\n");
}

?>
</body>
</html>
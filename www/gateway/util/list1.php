<?

require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/database.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");

require_once("wces/report_page.inc");
require_once("wces/report_generate.inc");
require_once("info.php");

$replacements = array
(
//  array(935,1062,'choice_question_responses'),
//  array(936,1064,'choice_question_responses'),
//  array(938,1068,'choice_question_responses'),
//  array(939,1070,'choice_question_responses'),
//  array(940,1071,'choice_question_responses'),
//  array(941,1072,'choice_question_responses'),
//  array(942,1073,'choice_question_responses'),
//  array(943,1075,'choice_question_responses'),
//  array(944,1076,'choice_question_responses'),
//  array(945,1077,'choice_question_responses')
//  array(946,1104,'textresponse_responses')
//  array(978,1062,'choice_question_responses'),
//  array(979,1064,'choice_question_responses'),
//  array(981,1068,'choice_question_responses'),
//  array(982,1070,'choice_question_responses'),
//  array(983,1071,'choice_question_responses'),
//  array(984,1072,'choice_question_responses'), 
//  array(985,1073,'choice_question_responses')
//  array(986,1104,'textresponse_responses')
//  array(1054,1075,'choice_question_responses'),
//  array(1055,1076,'choice_question_responses'), 
//  array(1056,1077,'choice_question_responses')
//  array(901,1086,'choice_responses'),
//  array(900,1085,'choice_question_responses'),  
//  array(905,1088,'choice_responses'),
//  array(904,1087,'choice_question_responses'),  
//  array(914,1094,'choice_responses'),
//  array(907,1089,'choice_question_responses'),  
//  array(910,1090,'choice_question_responses'),  
//  array(911,1091,'choice_question_responses'),  
//  array(912,1092,'choice_question_responses'),  
//  array(913,1093,'choice_question_responses'),  
//  array(916,1096,'choice_responses'),
//  array(915,1095,'choice_question_responses'),  
//  array(923,1098,'choice_responses'),
//  array(921,1097,'choice_question_responses'),  
//  array(926,1101,'choice_responses'),
//  array(925,1100,'choice_question_responses')
//  array(989,1085,'choice_question_responses'),
//  array(990,1087,'choice_question_responses'),
//  array(991,1089,'choice_question_responses'),
//  array(992,1090,'choice_question_responses'),
//  array(993,1091,'choice_question_responses'), 
//  array(994,1092,'choice_question_responses'),  
//  array(995,1093,'choice_question_responses'),    
//  array(996,1097,'choice_question_responses'),    
//  array(997,1100,'choice_question_responses') 
  array(924,1099,'textresponse_responses')
);

wces_connect();

$up = true;

foreach ($replacements as $r)
{
  list($old, $new, $table) = $r;
  
  $q1 = "UPDATE $table SET revision_id = $new WHERE revision_id = $old";
  $q2 = "SELECT response_id FROM $table WHERE revision_id = $old";  
  if ($up)
    pg_go($q1, $wces, __FILE__, __LINE__);
  
  $r = pg_go($q2, $wces, __FILE__, __LINE__);
  $n = pg_numrows($r);
  print("$q1<br>$q2<br>\n<br>\nresponse_id<br>\n");
  for ($i = 0; $i < $n; ++$i)
  {
    $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
    print("$row[response_id]<br>");
  }
  print("<br>\n");
  



        
}




$r = pg_go("
    SELECT s.response_id
    FROM survey_responses AS s
    INNER JOIN dp_response_saves AS v USING (response_id)
    WHERE s.question_period_id IN (19, 99902) AND v.save_id = 302 AND s.topic_id = 3570
    ORDER BY s.date
", $wces, __FILE__, __LINE__);

$n = pg_numrows($r);

for ($i = 0; $i < $n; ++$i)
{
  $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
  print("UPDATE survey_responses SET topic_id = 3569 WHERE topic_id = 3570 AND response_id = $row[response_id];<br>");
}

?>
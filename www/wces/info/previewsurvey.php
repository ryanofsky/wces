<%
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/wces.inc");
  require_once("wces/oldquestions.inc");

  page_top("Survey Preview","1000");
  
  $db = wces_connect();

  wces_Findquestionsets($db,"qs",$classid);
  $result = db_exec("SELECT questionsetid FROM qs", $db, __FILE__, __LINE__);
  $questionsets = array();
  while($row = mysql_fetch_assoc($result))
  $questionsets[] = $row["questionsetid"];
  db_exec("DROP TABLE qs", $db, __FILE__, __LINE__);
    
  print("<p><strong><a href=\"classinfo.php?classid=$classid\">Back</a></strong></p>");

  if (count($questionsets) > 0)
  {
    print("<form>");
    foreach($questionsets as $k => $qid)
    { 
      $q = new OldQuestionSet($db,$qid, 0, 0, "preview$k","prv",WIDGET_GET);
      $q->loadvalues();
      $q->display();
    }  
    print("</form>");
  }
  else
  {
    print("<p><b>No question sets found</b></p>");
  }
  print("<p><strong><a href=\"classinfo.php?classid=$classid\">Back</a></strong></p>");
   
  page_bottom();
%>

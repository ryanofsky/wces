<%
  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wces/oldquestions.inc");
  login_protect(login_professor);
  $profid = login_getprofid();

  param($previewclass);
  param($editclass);
  param($cancel);
  param($save);

  page_top("Professors Page","0010");

print("<p>This function is not available now.</p>");
page_bottom();
exit();



  $db = wces_connect();
  $profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");
  
  function mydie($str)
  {
    print("<font color=red><b>Internal Error: $str</b></font>");
    page_bottom();
    exit();
  };

 function listclasses()
  {
    global $db, $profid,$profname;
    print("<p>Choose a class from the list below to edit its custom questions. Or, choose the 'All My classes' link to create a custom question set that will be applied to all of your classes.</p>");
    print("<h3>List of Classes for $profname</h3>\n<UL>\n");

    wces_Findclasses($db,"currentclasses");
    $classes = db_exec("
    SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, cl.name AS clname, s.code as scode
    FROM currentclasses as cc
    INNER JOIN classes AS cl USING (classid)
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    WHERE cl.professorid = '$profid'",$db,__FILE__,__LINE__);

    $first = true;    
    while ($class = mysql_fetch_array($classes))
    {
      $first = $classid = $section = $year = $semester = $code = $name = $scode = $clname = "";
      extract($class);
      if ($clname) $clname = "- $clname";
      print ("  <LI>" . ucfirst($semester) . " $year  - $scode$code <i>$name$clname</i> (section $section) [ <A HREF=\"editsurveys.php?editclass=$classid\">Edit</A> | <A HREF=\"editsurveys.php?previewclass=$classid\">Preview</A> ]</LI>\n");
    }
    if ($first)
      print("None of your classes are being surveyed this semester.");
    else
      print("<li>All My Classes [ <a href=\"editsurveys.php?editclass=all\">Edit</a> | <a href=\"editsurveys.php?previewclass=all\">Preview</a> ]</li>\n");  
    print("</UL>\n");
  }
  
  function getquestionsetid($db,$editclass,$profid)
  {
    if ($editclass == "all")
    {
      $result = db_exec("
      SELECT g.questionsetid FROM groupings AS g
      LEFT JOIN answersets AS a USING(questionsetid)
      WHERE g.linktype = 'professors' AND g.linkid = $profid AND a.answersetid IS NULL", $db, __FILE__, __LINE__);
    }
    else
    {
      $ec = addslashes($editclass);
      $result = db_exec("
      SELECT g.questionsetid FROM groupings AS g
      LEFT JOIN answersets AS a USING(questionsetid)
      WHERE g.linktype = 'classes' AND g.linkid = '$ec' AND a.answersetid IS NULL", $db, __FILE__, __LINE__);    
    }
    $rows = mysql_num_rows($result);

    if ($rows == 0)
      return 0;
    else if ($rows == 1)
      return mysql_result($result,0);
    else
      mydie("<p><b>Database Error: Duplicate Question Sets Found. Please contact <a href=\"mailto:wces@columbia.edu\">wces@columbia.edu</a></b></p>");
  }      

  function getclassname($classid)
  {
    global $db, $profname;
    
    if ($classid == "all")
      return $profname;
    else
    {
      $ec = addslashes($classid);
      $result = db_exec("SELECT cl.section, c.code, s.code as scode
      FROM classes AS cl
      INNER JOIN courses AS c USING (courseid)
      INNER JOIN subjects AS s USING (subjectid)
      WHERE cl.classid = '$ec'", $db, __FILE__, __LINE__);
      $row = mysql_fetch_assoc($result);
      return $row["scode"] . $row["code"] . " Section " . $row["section"];
    }
  }  
 
  function editquestions()
  {
    global $db,$editclass,$profid,$profname,$ABETQUESTIONS, $server_wcespath;      
   
    $classname = getclassname($editclass);

    $questionsetid = getquestionsetid($db,$editclass,$profid);
    $questionset = $questionsetid ? 
      db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid),0) : 
      array("displayname" => ("Custom Questions from Professor " . db_getvalue($db,"professors",Array("professorid" => $profid),"name")));
%>

<form action="editsurveys.php" method="get" name=form>
<input name=editclass type="hidden" value="<%=$editclass%>">

<h2>Custom Questions For <%=$classname%></h2>

<% if ($editclass != "all") { %>

<h3>ABET Questions</h3>
<p>The ABET questions accept a rating from 1-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>
<table cellpadding=0 cellspacing=2>
<%
  $a = isset($questionset["ABET"]) ? explode(",",$questionset["ABET"]) : array();
  foreach($ABETQUESTIONS as $k => $v)
    print('  <tr><td bgcolor="#DDDDDD" background="' . $server_wcespath . 'media/0xDDDDDD.gif" width="100%"><p style="margin-left: 30px; text-indent: -30px"><input id="ABET' . $k . '" name="ABET' . $k . '" value=1 type=checkbox style="width: 30px"' . (is_array($a) && in_array($k,$a) ? " checked" : "") . '><label for="ABET' . $k . '"><b>' . $v . "</b></label></p></td></tr>\n");
%>
</table>
<hr>
<h3>Custom Questions</h3>
<% } %>

<p>From this page, you can edit the text of up to 10 ratings
style questions and up to 2 free response questions. On the ratings questions,
students get to choose from responses that vary between 'Excellent' and
'Disastrous.' The free response questions allow students to write up to a page
of text in response to your questions.</p>

<table>
<tr>
  <td valign=top>Heading:</td>
  <% if (!$questionset["displayname"]) $questionset["displayname"] = "Custom Question from Professor " . $profname; %>
  <td><textarea name=heading wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["displayname"])%></textarea></td>
</tr>
<% for($i=1; $i<=10; ++$i) { %>
<tr>
  <td valign=top>Ratings Question <%=$i%>:</td>
  <td><textarea name=ratings<%=$i%> wrap=virtual rows=3 cols=80><%=isset($questionset["MC$i"]) ? htmlspecialchars($questionset["MC$i"]) : ""%></textarea></td>
</tr>
<% } %>
<% for($i=1; $i<=2; ++$i) { %>
<tr>
  <td valign=top>Free Response <%=$i%>:</td>
  <td><textarea name=freeresponse<%=$i%> wrap=virtual rows=3 cols=80><%=isset($questionset["FR$i"]) ? htmlspecialchars($questionset["FR$i"]) : ""%></textarea></td>
</tr>
<% } %>
<tr><td colspan=2><input type=submit name=save value="Submit Changes"> <input type=submit name=cancel value="Cancel"><td></tr>
</table>
</form>
    <%
  };

  function savequestions()
  {
    global $db, $profid, $editclass, $HTTP_GET_VARS, $ABETQUESTIONS;
    
    $questionsetid = getquestionsetid($db,$editclass,$profid);
    
    $delete = true;
    if ($HTTP_GET_VARS["freeresponse1"] || $HTTP_GET_VARS["freeresponse2"])
      $delete = false;
    
    if ($delete)
      foreach($ABETQUESTIONS as $k => $v)
        if ($HTTP_GET_VARS["ABET$k"]) $delete = false;
  
    for($i = 1; $delete && $i <= 10; ++$i)
      if ($HTTP_GET_VARS["ratings$i"]) $delete = false;

    if ($delete)
    {
      if ($questionsetid)
      {
        db_delete($db,"groupings",Array("questionsetid" => $questionsetid));
        db_delete($db,"questionsets",Array("questionsetid" => $questionsetid));
        print("<p><b>The question set has been deleted</b></p><hr>");
      }
      else
      {
        print("<p><b>The question set is empty. Changes have not been saved.</b></p><hr>");        
      };
    }
    else
    {
      $update = array();
      $update["FR1"] = $HTTP_GET_VARS["freeresponse1"];
      $update["FR2"] = $HTTP_GET_VARS["freeresponse2"];
      $update["displayname"] = $HTTP_GET_VARS["heading"];
      for($i = 1; $i <= 10; ++$i)
        $update["MC$i"] = $HTTP_GET_VARS["ratings$i"];
      
      if ($editclass != "all")
      {
        $first = true; $s = "";
        foreach($ABETQUESTIONS as $k => $v)
        if ($HTTP_GET_VARS["ABET$k"])
        {  
          if ($first) $first = false; else $s .= ",";
          $s .= $k;
        }  
        $update["ABET"] = $s;
      }

      if ($questionsetid)
      {
        db_updatevalues($db, "questionsets", Array("questionsetid" => $questionsetid), $update);
        print("<p><b>Question Set '" . htmlspecialchars($update["displayname"]) . "' has been saved</b></p><hr>");        
      }
      else
      {
        $update["type"] = "private";
        $questionsetid = db_addrow($db,"questionsets",$update);
        if ($editclass == "all")
          $groupid = db_addrow($db,"groupings",Array("linkid" => $profid, "linktype" => "professors", "questionsetid" => $questionsetid));
        else
          $groupid = db_addrow($db,"groupings",Array("linkid" => $editclass, "linktype" => "classes", "questionsetid" => $questionsetid));
        print("<p><b>Question Set Saved ($questionsetid, $groupid)</b></p>");      
      }
    }
  };
 
  function isvalidclass($classid)
  {
    global $db, $profid;
    return ($classid == "all" || db_getvalue($db,"classes",array("professorid" => $profid, "classid" => $classid),"classid"));
  }

  function previewquestions()
  {
    global $previewclass, $db, $profid;
    $classname = getclassname($previewclass);
    print("<h2>Preview of Questions for $classname (<a href=\"editsurveys.php\">Back</a>)</h2>\n");
    
    if ($previewclass == "all")
    {
      $q = getquestionsetid($db,$previewclass,$profid);
      $questionsets = $q ? array($q) : array();
    }
    else
    {
      wces_Findquestionsets($db,"qs");
      $result = db_exec("SELECT questionsetid FROM qs WHERE classid = '$previewclass'", $db, __FILE__, __LINE__);
      $questionsets = array();
      while($row = mysql_fetch_assoc($result))
        $questionsets[] = $row["questionsetid"];
      db_exec("DROP TABLE qs", $db, __FILE__, __LINE__);
    }  
    
    
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
    print("<h2><a href=\"editsurveys.php\">Back</a></h2>");
  }
 
  if ($editclass)
  {
    if (isvalidclass($editclass))
    {  
      if ($cancel)
        listclasses();
      else if ($save)
      {
        savequestions();
        listclasses();
      }
      else
        editquestions();  
    }    
    else
		{
      print("<p><b>Invalid Class Number($editclass)</b></p>");
      listclasses();
    }  
  }
  else if ($previewclass)
  {
    if (isvalidclass($previewclass))
      previewquestions();
    else
    {
      print("<p><b>Invalid Class Number($editclass)</b></p>");
      listclasses();
    }  
  }
  else
    listclasses();
  
  page_bottom();
%>
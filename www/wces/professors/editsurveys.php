<%
  require_once("page.inc");
  require_once("database.inc");
  login_protect(login_professor);
  $profid = login_getprofid();

  page_top("Professors Page","0010");

  $db = wces_connect();
  $profname = db_getvalue($db,"Professors",Array("professorid" => $profid),"name");
  
  function mydie($str)
  {
    print("<font color=red><b>Internal Error: $str</b></font>");
    page_bottom();
    exit();
  };

  function listclasses()
  {
    global $db, $profid,$profname;
    print("<p>Choose a class from the list below to edit its custom questions. Or, choose the 'All My Classes' link to create a custom question set that will be applied to all of your classes.</p>");
    print("<h3>List of Classes for $profname</h3>\n<UL>\n");
    $classes = mysql_query("SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode FROM Classes as cl LEFT JOIN Courses AS c USING (courseid) LEFT JOIN Subjects AS s USING (subjectid) WHERE cl.professorid = '$profid' AND cl.year = '2000' AND cl.semester='fall'",$db);
    while ($class = mysql_fetch_array($classes))
    {
      extract($class);
      print ("  <LI><A HREF=\"editsurveys.php?editclass=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</A></LI>\n");
    }
    print("<LI><A HREF=\"editsurveys.php?editclass=all\">All My Classes</A></LI>");
    print("</UL>\n");
  }
  
  function editquestions()
  {
    global $db, $editclass,$profid,$profname;      
   
    if ($editclass == "all")
      $questionsetid = db_getvalue($db,"Groupings",Array("linkid" => $profid, "linktype" => "professors"),"questionsetid");
    else
      $test = db_getvalue($db,"Classes",Array("classid" => $editclass),"classid");
      if (!$test) mydie("Class #$editclass is invalid");
      $questionsetid = db_getvalue($db,"Groupings",Array("linkid" => $editclass, "linktype" => "classes"),"questionsetid");      

      db_getrow($db,"QuestionSets",Array("questionsetid" => $questionsetid),0) : 
      array("displayname" => ("Custom Questions from Professor " . db_getvalue($db,"Professors",Array("professorid" => $profid),"name")));
%>
<h3>Instructions</h3>
style questions and up to 2 free response questions. On the ratings questions,
students get to choose from responses that vary between 'Excellent' and
'Disastrous.' The free response questions allow students to write up to a page
of text in response to your questions.</p>

<form action="editsurveys.php" method="get" name=form>
<input name=editclass type="hidden" value="<%=$editclass%>">
<table>
<tr>
  <td valign=top>Heading:</td>
  <td><textarea name=heading wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["displayname"])%></textarea></td>
</tr>  
<tr>
  <td valign=top>Ratings Question 1:</td>
  <td><textarea name=ratings1 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC1"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 2:</td>
  <td><textarea name=ratings2 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC2"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 3:</td>
  <td><textarea name=ratings3 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC3"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 4:</td>
  <td><textarea name=ratings4 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC4"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 5:</td>
  <td><textarea name=ratings5 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC5"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 6:</td>
  <td><textarea name=ratings6 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC6"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 7:</td>
  <td><textarea name=ratings7 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC7"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 8:</td>
  <td><textarea name=ratings8 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC8"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 9:</td>
  <td><textarea name=ratings9 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC9"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Ratings Question 10:</td>
  <td><textarea name=ratings10 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["MC10"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Free Response 1:</td>
  <td><textarea name=freeresponse1 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["FR1"])%></textarea></td>
</tr>
<tr>
  <td valign=top>Free Response 2:</td>
  <td><textarea name=freeresponse2 wrap=virtual rows=3 cols=80><%=htmlspecialchars($questionset["FR2"])%></textarea></td>
</tr>
<tr><td colspan=2><input type=submit name=save value="Submit Changes"> <input type=submit name=cancel value="Cancel"><td></tr>
</table>
</form>
    <%
  }
  
  function savequestions()
  {
    global $db, $editclass, $profid, $heading, $ratings1, $ratings2, $ratings3, $ratings4, $ratings5, $ratings6, $ratings7, $ratings8, $ratings9, $ratings10, $freeresponse1, $freeresponse2;
    if (isset($heading) && isset($ratings1) && isset($ratings2) && isset($ratings3) && isset($ratings4) && isset($ratings5) && isset($ratings6) && isset($ratings7)  && isset($ratings8) && isset($ratings9) && isset($ratings10) && isset($freeresponse1) && isset($freeresponse2))
    {
      {
        $questionsetid = db_getvalue($db,"Groupings",Array("linkid" => $profid, "linktype" => "professors"),"questionsetid");
        if (!$questionsetid && !$delete)
        {
          if ($questionsetid === false) mydie ("Duplicate Question Sets for Professor #$profid have been found. Please contact an administrator with this information.");
          $questionsetid = db_addrow($db,"QuestionSets",Array("type" => "private","displayname" => "Custom Questions from Professor $profname"));
          $groupid = db_addrow($db,"Groupings",Array("linkid" => $profid, "linktype" => "professors", "questionsetid" => $questionsetid));
        }
      }
      else
      {
        $test = db_getvalue($db,"Classes",Array("classid" => $editclass),"classid");
        if (!$test) mydie("Class #$editclass is invalid");
      
        $questionsetid = db_getvalue($db,"Groupings",Array("linkid" => $editclass, "linktype" => "classes"),"questionsetid");
        if (!$questionsetid && !$delete)
        {
          if ($questionsetid === false) mydie ("Duplicate Question Sets for Class #$editclass have been found. Please contact an administrator with this information.");
          $questionsetid = db_addrow($db,"QuestionSets",Array("type" => "private","displayname" => "Custom Questions from Professor $profname"));
          $groupid = db_addrow($db,"Groupings",Array("linkid" => $editclass, "linktype" => "classes", "questionsetid" => $questionsetid));
        }
      };
        {
          db_delete($db,"QuestionSets",Array("questionsetid" => $questionsetid));
          print("<p><b>The question set has been deleted</b></p><hr>");
        }
        else
      }
      else
      {
        print("<p><b>Question Set '" . htmlspecialchars($heading) . "' has been saved</b></p><hr>");
      }
    else
      print("<p><b>Invalid Form Data. Please click back to try again, or contact an administrator if you continue to experience difficulties.</b></p><hr>");
  };
  
  if ($editclass)
  {
    if (isset($cancel))
      listclasses();
    else if (isset($save))
      savequestions();
    else
      editquestions();  
  }
  else
    listclasses();
  
  page_bottom();
%>
<%
  require_once("wces/page.inc");
  require_once("wces/database.inc");
  login_protect(login_professor);
  $profid = login_getprofid();

  page_top("professors Page","0010");
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
    print("<h3>Preview Surveys</h3>\n");
    print("<p>Choose a class from the list below to preview its custom survey questions</p><UL>\n");
    $classes = mysql_query("SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode FROM classes as cl LEFT JOIN courses AS c USING (courseid) LEFT JOIN subjects AS s USING (subjectid) WHERE cl.professorid = '$profid' AND cl.year = '2001' AND cl.semester='spring'",$db);
    if (mysql_num_rows($classes) <= 0) print("  <LI><I>No classes found</I></LI>");
    while ($class = mysql_fetch_array($classes))
    {
      extract($class);
      print ("  <LI><A HREF=\"previewsurveys.php?previewclass=$classid\">" . ucfirst($semester) . " $year  - $scode$code <i>$name</i> (section $section)</A></LI>\n");
    }
    print("</UL>\n");
  }
  
  function showquestionset($questionset)
  {
    global $count;
    ++$count;
    extract($questionset);
    print("<h4>$displayname</h4>\n");
    print("<form>\n");
    for($i = 1; $i <= 10; ++$i)
    {
      $index = "MC" . $i;
      $text = $questionset[$index];
      if ($text)
      {
%>
  <p><%=$text%></p>
  <blockquote>
  <input type=radio name="q<%=$questionsetid . $index%>" id="q<%=$questionsetid . $index%>a" value="excellent"><label for="q<%=$questionsetid . $index%>a">excellent</label>
  <input type=radio name="q<%=$questionsetid . $index%>" id="q<%=$questionsetid . $index%>b" value="very good"><label for="q<%=$questionsetid . $index%>b">very good</label>
  <input type=radio name="q<%=$questionsetid . $index%>" id="q<%=$questionsetid . $index%>c" value="satisfactory"><label for="q<%=$questionsetid . $index%>c">satisfactory</label>
  <input type=radio name="q<%=$questionsetid . $index%>" id="q<%=$questionsetid . $index%>d" value="poor"><label for="q<%=$questionsetid . $index%>d">poor</label>
  <input type=radio name="q<%=$questionsetid . $index%>" id="q<%=$questionsetid . $index%>e" value="disastrous"><label for="q<%=$questionsetid . $index%>e">disastrous</label>
  </blockquote>
<%
      }  
    };  
    for($i = 1; $i <= 2; ++$i)
    {
      $index = "FR" . $i;
      $text = $questionset[$index];
      if ($text)
      {
%>
  <p><%=$text%></p>
  <blockquote><textarea name="q<%=$questionsetid . $index%>" rows=8 cols=50></textarea></blockquote>
<%
      }  
    };
    print("</form>\n");
  };
  
  $count = 0;
  function showclass($classid)
  {
    global $db,$count;
    print("\n<hr>\n");
    
    $test = db_getvalue($db,"classes",Array("classid" => $classid),"classid");
    if (!$test) { print ("<p><i>Invalid Class Number ($classid)</i></p>"); return; };
    
    $profid = db_getvalue($db,"classes",Array("classid" => $classid),"professorid");
    if ($profid)
    {
      $qsetids = db_getcolumn($db,"groupings",Array("linkid" => $profid, "linktype" => "professors"), "questionsetid");
      foreach($qsetids as $qsetid)
        showquestionset(db_getrow($db,"questionsets",Array("questionsetid" => $qsetid),0));
    };
    
    $qsetids = db_getcolumn($db,"groupings",Array("linkid" => $classid, "linktype" => "classes"), "questionsetid");
      foreach($qsetids as $qsetid)
        showquestionset(db_getrow($db,"questionsets",Array("questionsetid" => $qsetid),0));
        
    if ($count == 0) print ("<p><i>No Custom Questions Found</i></p>");  
  };  
  
  listclasses();
  if ($previewclass) showclass($previewclass);
  page_bottom();
%>








<%
  require_once("page.inc");
  require_once("login.inc");
  
    
  
  $y = mysql_query("CREATE TEMPORARY TABLE currentclasses (classid INTEGER NOT NULL, PRIMARY KEY(classid))",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.classid
    WHERE g.linktype = 'classes' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes as cl ON g.linkid = cl.courseid
    WHERE g.linktype = 'courses' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Classes AS cl ON g.linkid = cl.professorid
    WHERE g.linktype = 'professors'  && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.subjectid INNER JOIN Classes as cl ON c.courseid = cl.courseid
    WHERE g.linktype = 'subjects' && cl.year = 2000 && cl.semester = 'fall'",$db);
  $y = mysql_query("REPLACE INTO currentclasses (classid) SELECT cl.classid FROM Groupings AS g INNER JOIN Courses AS c  ON g.linkid = c.departmentid INNER JOIN Classes as cl ON c.courseid = cl.courseid 
    WHERE g.linktype = 'departments' && cl.year = 2000 && cl.semester = 'fall'",$db);

  $classes = mysql_query(
  "SELECT cl.classid, cl.section, cl.year, cl.semester, c.code, c.name, s.code AS scode 
  FROM currentclasses AS cc
  LEFT JOIN Classes AS cl ON cc.classid = cl.classid

  {
    $found = true;
      print ("  <li>$scode$code <i>$name</i> (section $section)</LI>\n");
  print("</ul>");
%>
<?

require_once("wces/oracle.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wbes/general.inc");

wces_connect();

param($mode);
param($searchfor);
param($searchin);

?>
<head>
<title>List</title>
<style>
<!--

body
{
  scrollbar-3dlight-color: #6699CC; 
  scrollbar-darkshadow-color:black;
  scrollbar-face-color: #6699CC;
  scrollbar-arrow-color:black;
  scrollbar-highlight-color:#AFD2F5;
  scrollbar-shadow-color: #3D5C7A;
  scrollbar-base-color:tomato;
  scrollbar-track-color: #B5CFE8; 
  font-family: Arial, Helvetica, sans-serif;
}
p  { font-family: Arial, Helvetica, sans-serif; }
li { font-family: Arial, Helvetica, sans-serif; }
ul { font-family: Arial, Helvetica, sans-serif; list-style-type: square; list-style-position: outside; }
a  { text-decoration: none; }
a:hover { text-decoration: underline; ) 

-->
</style>
</head>
<body bgcolor="#6699CC" link="#000000" alink="#444444" vlink="#444444" topmargin=0 leftmargin=0 rightmargin=0 marginwidth=0 marginheight=0>
<base target=info>
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

<?

function db_makesearch($text,$fields)
{
  $text = trim(ereg_replace("[\001-\040]+", " ", $text));
  $arr = explode(" ",$text);
  $str = '';
  foreach($arr as $t)
  foreach($fields as $f)
  {
    if ($str) $str .= " OR ";
    $str .= "$f ILIKE '%" . addslashes($t) . "%'";
  }
  return $str;
}

if ($mode == "search")
{
  ?>
  <p align=center>
  <a href="oracle_listpane.php?mode=courses" target="_self" onmouseover="swap('courses')" onmouseout="swap('courses')"><img src="<?=$oracle_root?>media/courseraised.jpg" name=courses border=0></a>
  <a href="oracle_listpane.php?mode=professors" target="_self" onmouseover="swap('profs')" onmouseout="swap('profs')"><img src="<?=$oracle_root?>media/profraised.jpg" name=profs border=0></a>
  <img src="<?=$oracle_root?>media/searchsunk.jpg" name=search border=0>
  <script>AttachImage('courses','courselit.jpg'); AttachImage('profs','proflit.jpg')</script>
  </p>
  
  <form method=get target="_self">
  <input type=hidden name=mode value=search>
  <table align=center width="100%">
  <tr>
    <td nowrap><strong>Search for</strong></td>
    <td colspan=2 width="100%"><input name=searchfor type=text size=18 style="width: 100%" value="<?=htmlspecialchars($searchfor)?>"></td>
  </tr>
  <tr>
    <td><strong>in</strong></td>
    <td width="100%"><select name=searchin style="width: 100%"><option value=1<?=$searchin==1?" selected":""?>>Course Names</option><option value=2<?=$searchin==2?" selected":""?>>Professor Names</option></select></td>
    <td><input type=submit name=submit value="Go"></td>
  </tr>
  </table>
  </form>
<?
  if ($searchfor)
  {
    if ($searchin == 2)
    {
      $search = db_makesearch($searchfor, array("u.firstname", "u.lastname"));
      $result = pg_go("
        SELECT u.user_id AS id, (u.lastname || ', ' || u.firstname) AS name
        FROM ($select_classes) AS l
        INNER JOIN enrollments AS e ON e.class_id = l.class_id AND e.status = 3
        INNER JOIN users AS u USING (user_id)
        WHERE $search
        GROUP BY u.user_id, u.lastname, u.firstname
        ORDER BY lastname, firstname
      ",$wces,__FILE__,__LINE__);
    }  
    else // $searchin == 1
    {
      $search = db_makesearch($searchfor, array("cl.name","c.name"));
      $result = pg_go("
        SELECT cl.course_id AS id, get_course(cl.course_id) AS course_info
        FROM ($select_classes) AS l
        INNER JOIN classes AS cl USING (class_id)
        INNER JOIN courses AS c USING (course_id)
        WHERE $search
        GROUP BY cl.course_id
      ", $wces, __FILE__, __LINE__);
    }

    $n = pg_numrows($result);
    if ($n == 0)
      print("<p><b>No matches found</b></p>");
    else
    {
      if ($searchin == 2)
      {
        for ($i = 0; $i < $n; ++$i)
        {
          extract(pg_fetch_array($result, $i, PGSQL_ASSOC));
          print("<li><a href=\"oracle_infopane.php?user_id=$id\">$name</a></li>\n");
        } 
      }
      else // $searchin == 1 
      {
        for ($i = 0; $i < $n; ++$i)
        {
          extract(pg_fetch_array($result, $i, PGSQL_ASSOC));
          print("<li><a href=\"oracle_infopane.php?course_id=$id\">" . format_course($course_info) . "</a></li>\n");
          
        }
      }
      
    }
  }
}
else if ($mode == "professors")
{
  ?>
  <p align=center>
  <a href="oracle_listpane.php?mode=courses" target="_self" onmouseover="swap('courses')" onmouseout="swap('courses')"><img src="<?=$oracle_root?>media/courseraised.jpg" name=courses border=0></a>
  <img src="<?=$oracle_root?>media/profsunk.jpg">
  <a href="oracle_listpane.php?mode=search" target="_self" onmouseover="swap('search')" onmouseout="swap('search')"><img src="<?=$oracle_root?>media/searchraised.jpg" name=search border=0></a>
  </p>

  <script>AttachImage('courses','courselit.jpg'); AttachImage('search','searchlit.jpg');</script>
  <? 

  $result = pg_go("
    SELECT DISTINCT u.user_id, u.firstname, u.lastname, d.department_id, d.code, d.name
    FROM ($select_classes) AS l
    INNER JOIN enrollments AS e ON e.class_id = l.class_id AND e.status = 3
    INNER JOIN users AS u ON u.user_id = e.user_id
    LEFT JOIN departments AS d USING (department_id)
    ORDER BY d.code, d.department_id, u.lastname
  ", $wces, __FILE__, __LINE__);

  $users = new pg_segmented_wrapper($result, "department_id");
  while($users->row)
  {
    extract($users->row);
    if ($users->split)
    {
      $dept = $department_id ? "$name ($code)" : "<i>Unknown</i>"; 
      print("<h5>$dept</h5>\n<font size=-1>\n<ul>\n");
    }
    print("  <li><a href=\"oracle_infopane.php?user_id=$user_id\">$lastname, $firstname</a></li>\n");
    $users->advance();
    if ($users->split) print("</ul>\n</font>\n");
  }
}
else // $mode == "courses"
{
  ?>
  <p align=center>
  <img src="<?=$oracle_root?>media/coursesunk.jpg">
  <a href="oracle_listpane.php?mode=professors" target="_self" onmouseover="swap('profs')" onmouseout="swap('profs')"><img src="<?=$oracle_root?>media/profraised.jpg" name=profs border=0></a>
  <a href="oracle_listpane.php?mode=search" target="_self" onmouseover="swap('search')" onmouseout="swap('search')"><img src="<?=$oracle_root?>media/searchraised.jpg" name=search border=0></a>
  </p>
  <script>AttachImage('profs','proflit.jpg'); AttachImage('search','searchlit.jpg')</script>
  <?  

  $result = pg_go("
    SELECT DISTINCT d.department_id, d.code, d.name, c.course_id, get_course(c.course_id) AS course_info
    FROM ($select_classes) AS l
    INNER JOIN classes AS cl USING (class_id)
    INNER JOIN courses AS c USING (course_id)
    LEFT JOIN departments AS d ON d.department_id = c.guess_department_id
    ORDER BY d.name, course_info
  ", $wces, __FILE__, __LINE__);
  
  $classes = new pg_segmented_wrapper($result, "department_id");
  while($classes->row)
  {
    
    extract($classes->row);
    if ($classes->split)
    {
      $dept = $department_id ? "$name ($code)" : "<i>Unknown</i>"; 
      print("<h5>$dept</h5>\n<font size=-1>\n<ul>\n");
    }
    print("  <li><a href=\"oracle_infopane.php?course_id=$course_id\">" . format_ccourse($course_info, "<i>%c</i><br>%n") . "</a></li>\n");
    $classes->advance();
    if ($classes->split) print("</ul>\n</font>\n");
  }
}

?>

</body>








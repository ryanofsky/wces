<?

require_once("wces/login.inc");
require_once("wces/prespond.inc");

login_protect(login_professor);

$user_id = login_getuserid();
$profname = login_getname();

function listclasses()
{
  global $user_id, $profname, $wces, $select_classes;

  $uid = (int) login_getuserid();
  wces_connect();

  $result = pg_go("
    SELECT cl.course_id, get_course(cl.course_id) AS course_info
    FROM enrollments AS e
    INNER JOIN ($select_classes) AS l USING (class_id)
    INNER JOIN classes AS cl USING (class_id)
    WHERE e.user_id = $uid AND e.status = 3
    GROUP BY cl.course_id
    ORDER BY course_info
  ", $wces, __FILE__, __LINE__);

  $classes = new pg_wrapper($result);

  print("<p>Choose a course to review its results.</p>\n");
  print("<ul>\n");
  if ($classes->rows == 0)
  {
    print("  <li><i>No courses found</i></li>");
  }
  else
  {
    while($classes->row)
    {
      extract($classes->row);
      print("  <li><a href=\"new_prof.php?course_id=$course_id\">" . format_ccourse($course_info, "<i>%c</i> %n") . "</a></li>\n");
      $classes->advance();
    }
  }
  print("</ul>\n");
}

param($course_id);
param($save);
$course_id = (int) $course_id;

page_top("Survey Results");
if ($course_id)
{
  if ($save)
  {
    saveResults($user_id, $course_id);  
    listclasses();
  }
  else
  {
    print("<p><a href=new_prof.php>Back</a></p>\n");
    showResults($user_id, $course_id, true);
  }
}
else
  listclasses();
page_bottom();

?>
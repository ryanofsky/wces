<?

require_once("wces/login.inc");
require_once("wces/prespond.inc");

$user_id = login_getuserid();
$profname = login_getname();

function listclasses()
{
  global $user_id, $profname, $wces, $select_classes;

  wces_connect();

  $result = pg_go("
    SELECT p.user_id, p.course_id, get_course(p.course_id) AS course_info, u.firstname, u.lastname
    FROM presps AS p
    INNER JOIN users AS u USING (user_id)
  ", $wces, __FILE__, __LINE__);

  $classes = new pg_wrapper($result);

  print("<p>Choose a professor response.</p>\n");
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
      print("  <li><a href=\"new_admin.php?course_id=$course_id&user_id=$user_id\">" 
        . format_ccourse($course_info, "<i>%c</i> %n")
        . "</a>"
        . " from $firstname $lastname</li>\n");
      $classes->advance();
    }
  }
  print("</ul>\n");
}

param($course_id);
param($user_id);
param($save);
$course_id = (int) $course_id;
$user_id = (int) $user_id;

page_top("Survey Results");
if ($course_id && $user_id)
{
    print("<p><a href=new_admin.php>Back</a></p>\n");
    showResults($user_id, $course_id, false);
}
else
  listclasses();
page_bottom();

?>
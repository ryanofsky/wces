<?

require_once("wces/login.inc");
require_once("wces/prespond.inc");

login_protect(login_professor);

$user_id = login_getuserid();
$profname = login_getname();

define('ProfResponseMenu_choose', 1);

class ProfResponseMenu extends StatefullWidget
{
  function ProfResponseMenu($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->event->shortName(ProfResponseMenu_choose, "course_id");
  }

  function & handleEvent($event, $param, $isNew)
  {
    global $base_branch_id, $user_id, $factories;
    if ($event == ProfResponseMenu_choose)
    {
      $user_id = login_getuserid();
      ProfResponse::DumpScript();
      $this->response =& new ProfResponse($param, $user_id, false, 'response', $this);
      $this->loadChild($this->response, $isNew);
      return $this->response;
    }
  }
  
  function printVisible()
  {
    global $user_id, $profname, $wces, $select_classes, $ASID;
  
    if (isset($this->response->message))
      print($this->response->message);
    
    $uid = (int)login_getuserid();
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
      print("  <li><i>No courses found</i></li>");
    else
    {
      while($classes->row)
      {
        extract($classes->row);
        print('  <li><a href="'
          . $this->event->getUrl(ProfResponseMenu_choose, $course_id)
          ."$ASID\">" . format_ccourse($course_info, "<i>%c</i> %n")
          . "</a></li>\n");
        $classes->advance();
      }
    }
    print("</ul>\n");    
  }
}

$f =& new Form();
$l =& new ProfResponseMenu('menu', $f);
$f->loadState();

page_top("Survey Results");

print("<form method=post action=\"$f->pageName\">\n$ISID\n");
$f->display();
$l->display();
print("</form>\n");

page_bottom();

?>
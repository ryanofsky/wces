<?

require_once("wces/login.inc");
require_once("wces/prespond.inc");

LoginProtect(LOGIN_PROFESSOR);

$user_id = LoginValue('user_id');
$profname = LoginValue('name');

class ProfResponseMenu extends ParentWidget
{
  function ProfResponseMenu($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->shortName("course_id");
  }

  function loadState($new)
  {
    ParentWidget::loadState($new);
    if ($new) return;
    
    $course_id = (int)$this->readValue('course_id');
    if ($course_id)
    {
      global $user_id;
      ProfResponse::DumpScript();
      $this->init =& new InitializerWidget('init', $this);
      $this->response =& new ProfResponse($course_id, $user_id, false, 
        'response', $this->init);
      $this->response->modal = true;
      $this->loadChild($this->init);
      if ($this->response->done) unset($this->modalChild);
    }
  }  
  
  function printVisible()
  {
    global $user_id, $profname, $wces, $select_classes, $ASID;
  
    if (isset($this->response->message))
      print($this->response->message);
    
    wces_connect();
  
    $result = pg_go("
      SELECT cl.course_id, get_course(cl.course_id) AS course_info
      FROM enrollments_p AS e
      INNER JOIN ($select_classes) AS l USING (class_id)
      INNER JOIN classes AS cl USING (class_id)
      WHERE e.user_id = $user_id
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
          . $this->getUrl(array('course_id' => $course_id))
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

print("<form method=post>\n$ISID\n");
$f->display();
$l->display();
print("</form>\n");

page_bottom();

?>

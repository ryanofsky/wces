<?

require_once("wces/login.inc");
require_once("wces/prespond.inc");

define('ProfResponseList_choose', 1);

class ProfResponseList extends StatefullWidget
{
  function ProfResponseList($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->event->shortName(ProfResponseList_choose, array('course_id', 'user_id'));
  }

  function & handleEvent($event, $param, $isNew)
  {
    global $base_branch_id, $user_id, $factories;
    if ($event == ProfResponseList_choose)
    {
      $course_id = $param[0];
      $user_id = $param[1];
      ProfResponse::DumpScript();
      $this->response =& new ProfResponse($course_id, $user_id, true, 'response', $this);
      $this->loadChild($this->response, $isNew);
      return $this->response;
    }
  }
  
  function printModalChildren()
  {
    print("<p><a href=new_admin.php>Back</a></p>\n");
    StatefullWidget::printModalChildren(); 
  } 
    
  function printVisible()
  { 
    global $user_id, $profname, $wces, $select_classes, $ASID;

    $result = pg_go("
      SELECT p.user_id, p.course_id, get_course(p.course_id) AS course_info, u.firstname, u.lastname
      FROM presps AS p
      INNER JOIN users AS u USING (user_id)
    ", $wces, __FILE__, __LINE__);
  
    $classes = new pg_wrapper($result);
  
    if (isset($this->response->message))
      print($this->response->message);

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
        print('  <li><a href="'
          . $this->event->getUrl(ProfResponseList_choose, array($course_id, $user_id))
          . "$ASID\">" . format_ccourse($course_info, "<i>%c</i> %n")
          . "</a>"
          . " from $firstname $lastname</li>\n");
        $classes->advance();
      }
    }
    print("</ul>\n");
  }
}

$f =& new Form();
$l =& new ProfResponseList('menu', $f);
$f->loadState();

page_top("Professor Responses");

print("<form method=post action=\"$f->pageName\">\n$ISID\n");
$f->display();
$l->display();
print("</form>\n");

page_bottom();

?>
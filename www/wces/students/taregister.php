<?

require_once("wces/login.inc");

LoginProtect(LOGIN_STUDENT);

class TaRegister extends ParentWidget
{
  function TaRegister($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->shortName('drop_class');
    $this->shortName('add_class');
    $this->search =& new TextBox(0, 30, '', 'search', $this);
    $this->search->shortName('','search');
  }

  function loadState($new)
  {
    ParentWidget::loadState($new);
    if ($new) return;
    global $wces;
    
    if ($add = (int)$this->readValue('add_class'))
    {
      wces_connect();
      $user_id = (int)LoginValue('user_id');
      pg_go("
        SELECT enrollment_add_ta($user_id, $add)
      ", $wces, __FILE__, __LINE__);
    }

    if ($drop = (int)$this->readValue('drop_class'))
    {
      wces_connect();
      $user_id = (int)LoginValue('user_id');
      pg_go("
        SELECT enrollment_drop_ta($user_id, $drop)
      ", $wces, __FILE__, __LINE__);
    }
  }

  function printVisible()
  {
    global $wces;
    wces_connect();
    if (isset($this->search->text))
      $this->form->addUrlVar($this->search->name(), $this->search->text);
    
    $user_id = (int)LoginValue('user_id');
    $r = pg_go("
      SELECT class_id, get_class(class_id) AS class,
        get_profs(class_id) AS profs
      FROM enrollments
      WHERE user_id = $user_id AND status & 2 <> 0
    ", $wces, __FILE__, __LINE__);

    $n = pg_numrows($r);

    if ($n == 0)
    {
      print("<p>You are currently not registered as a TA for any classes. "
        . "Use the search below to find classes by name or number to register "
        . "yourself as a TA for.</p>");
    }
    else
    {
      print("<p>You are registered as a TA for the classes listed below. You "
        . "can use the links to drop registrations.");

      print("<ul>\n");
      for ($i = 0; $i < $n; ++$i)
      {
        $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
        print("<p>" . format_class($row['class']) . "<br>\n");
        if ($row['profs'])
          print(format_profs($row['profs'], false, "Professor ", "<br>\n"));
        print("<a href=\"".$this->getUrl(array('drop_class'=>$row['class_id']))
          . "\">Drop this Registration</a></p>\n");
      }
      print("</ul>\n");
      
      print("<p>You can use the search below to find new classes to register "
        . "yourself as a TA for.</p>");      
    }
    
    $this->search->display();
    print(" <input type=submit value=\"Search Classes\">\n");

    $question_period_id = get_question_period();
    
    if (!isset($this->search->text)) return;
    
    $terms = parse_search($this->search->text);
    
    if (!count($terms))
      $where = "";
    else
    {
      $where = "";
      foreach($terms as $term)
      {
        if ($where) $where .= "AND "; else $where .= "WHERE ";    
        $search = addslashes(pg_reg_escape($term));
        $where .= "t.info ~* '$search' ";
      }
    } 

    $r = pg_go("
      SELECT * FROM
      ( SELECT class_id, get_class(class_id) AS info
        FROM wces_topics
        WHERE question_period_id = $question_period_id
      ) AS t    
      $where
      ORDER BY info
    ", $wces, __FILE__, __LINE__);
    
    $n = pg_numrows($r);
    
    print("<p><i>$n results found</i></p>");
    
    print("<ul>\n");
    for ($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_row($r, $i, PGSQL_ASSOC);

      print("<p>" . format_class($row['info']) . "<br>\n");
      print("<a href=\"".$this->getUrl(array('add_class'=>$row['class_id']))
        . "\">Register as a TA for this Class</a></p>\n");
    }
    print("</ul>");
  }
}

function parse_search($search)
{
  $terms = array();
  $n = strlen($search);
  $term = "";
  $inquote = false;
  $escaped = false;

  for ($i = 0; $i < $n; ++$i)
  {
    $c = $search[$i];
    if ($escaped)
    {
      $term .= $c;
      $escaped = false;
    }
    else if ($c == '\\')
    {
      $escaped = true;
    }
    else if ($c == '"')
    {
      $inquote = !$inquote;
    }
    else if ($c == ' ' && !$inquote)
    {
      if (strlen($term))
      {
        $terms[] = $term;
        $term = "";
      }
    }
    else
    {
      $term .= $c;
    }
  }
  if (strlen($term))
    $terms[] = $term;

  return $terms;
}

$f =& new Form();
$t =& new TaRegister('ta', $f);
$f->loadState();

page_top("TA Registration");

print("<form name=\"$f->formName\">\n");
$f->display();
$t->display();
print("</form>");

page_bottom();

?>
<?
  
  require_once("wces/oracle.inc");
  require_once("wbes/postgres.inc");
  require_once("wces/wces.inc");
  require_once("wces/login.inc");
  $server_isproduction = 0;

LoginProtect(LOGIN_ADMIN);

// use for primitive values only
function swap(&$a, &$b)
{
  $t = $a;
  $a = $b;
  $b = $t;
}

class ProfessorMerge extends ParentWidget
{
  var $message = "";
  
  function ProfessorMerge($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    ProfessorMerge::DumpScript();
  } 
  
  function loadState($new)
  {
    ParentWidget::LoadState($new);
    if ($new) return;

    $user_ids = $this->readValue('user_ids');
    if (is_array($user_ids))
    {
      if (count($user_ids) == 2)
      {
        list($u1, $u2) = array_map('intval', $user_ids);
        if ($u2 < $u1) swap($u1, $u2);
        
        global $wces;
        wces_connect();
        
        $r = pg_go("SELECT professor_merge($u1, $u2)", $wces, __FILE__, __LINE__);
        if (!$r || !pg_result($r, 0, 0))
          $this->message = "<p><font color=red>professor_merge($u1, $u2) failed</font></p>";
        else
          $this->message = "<p><font color=blue>professor_merge($u1, $u2) succeeded</font></p>";
      }
      else if (count($user_ids) > 2)
        $this->message = "<p><font color=red>You can only merge two users at a time.</font></p>";
    }
  }

  function printVisible()
  {
    $floater = $this->name('floater');
?>
<div id=<?=$floater?> style="position:absolute">
<input type=submit value=Merge>
</div>
<script>initFloater('<?=$floater?>')</script>
<?

    print($this->message);

    global $wces, $ASID;
    wces_connect();

    $result = pg_go("
      SELECT u.user_id, u.firstname, u.lastname, u.uni
      FROM ( SELECT DISTINCT e.user_id FROM wces_topics AS l
        INNER JOIN enrollments_p AS e USING (class_id)) AS w
      INNER JOIN users AS u USING(user_id)
      ORDER BY upper(u.lastname), u.lastname, upper(u.firstname), u.firstname
    ", $wces, __FILE__, __LINE__);
    
    $n = pg_numrows($result);
    
    print("<table>\n");

    
    $prev_name = NULL;
    
    $color = 0;
  
    
    $field = $this->name('user_ids[]');  
    $next = pg_fetch_array($result, 0);
    $prev_name = null;
  
    for($i = 1; isset($next); ++$i)
    {
      $row = $next;
      $next = $i < $n ? pg_fetch_array($result, $i) : null;

      $same_as_next = isset($next) && strcasecmp($row['lastname'], $next['lastname']) == 0;
      $same_as_prev = isset($prev_name) && strcasecmp($prev_name, $row['lastname']) == 0;
      $prev_name = (string)$row['lastname'];
      
      if ($same_as_next && !$same_as_prev) ++$color;
      
      extract($row);
      $bg = $same_as_next || $same_as_prev ? 
        (" bgcolor=" . ($color % 2 == 0 ? "yellow" : "orange")) : "";
      print("<tr>\n  <td><input type=checkbox name=$field value=$user_id></td>\n  <td$bg>");
      $u = strlen($row['uni']) ? " ($row[uni])" : "";
      print("<a href=\"/wces/administrators/info.php?user_id=$user_id&hooks=1$ASID\" target=lb>$lastname, $firstname$u</a></td>\n</tr>\n");
    };
  
    print("</table>\n");
  }

  function DumpScript()
  {
    static $dumped = false;
    if ($dumped) return false; else $dumped = true;

    $str = <<<EOD
<script type="text/javascript" language="JavaScript">

var gright = 170;
var gbottom = 40;

function initFloater(elem)
{
  window.floater = document.getElementById(elem);
  centerFloater();
  window.onresize = centerFloater;
  window.onscroll = centerFloater;
}

function centerFloater()
{
  pageWidth = document.body.offsetWidth - 4;
  pageHeight = document.body.offsetHeight - 2;
  window.floater.style.left = document.body.scrollLeft + (pageWidth - gright) / 2;
  window.floater.style.top = document.body.scrollTop  + (pageHeight - gbottom) / 2;
}
</script>
EOD;

    html_head_append($str);
  }
}

$f =& new Form('f');
$m =& new ProfessorMerge('m', $f);
$f->loadState();

$page =& WcesPageInstance();
$page->mode = WCES_PAGE_BARE;

$page->printTop('Professor Merge');
print("<form name=$f->formName method=post>\n");
$m->display();
print("</form");

$page->printBottom();

?>


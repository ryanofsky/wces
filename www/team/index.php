<?
require_once("team/page.inc");
require_once("team/login.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

login_protect(login_any);

define("TeamQ_cat",    1);
define("TeamQ_logoff", 3);

class TeamQ extends FormWidget
{
  var $action;
  var $team;
  
  var $users = array();
  var $questions = array();
  var $categories = array();
  var $user_id;
  var $resultmode = false;
  
  function TeamQ($user_id, $team, $prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
    $this->user_id = $user_id;
    $this->team = $team;
  }
  
  function getquestions($category)
  {
    $team = $this->team;
    $q = array();
    $r = db_exec("SELECT name, question_id FROM questions WHERE category_id = $category ORDER BY ORDINAL", $team, __FILE__, __LINE__);
    while($cat = mysql_fetch_assoc($r))
      $q[$cat['name']] = $cat['question_id'];
    return $q;
  }
  
  function loadvalues()
  {
    $team = $this->team;
    $user_id = $this->user_id;
    $this->action->loadvalues();

    $a = mysql_result(db_exec("SELECT COUNT(*) FROM answers WHERE user_id = $this->user_id", $this->team, __FILE__, __LINE__),0);
    $q = mysql_result(db_exec("
      SELECT COUNT(*)
      FROM questions AS q
      INNER JOIN users AS u ON u.user_id = $this->user_id
      INNER JOIN memberships AS m1 USING (user_id)
      INNER JOIN memberships AS m2 USING (team_id)
      INNER JOIN users AS u2 USING (user_id)
    ", $this->team, __FILE__, __LINE__),0);
    //print ("a = $a, q = $q");
    if ($a == $q)
    {
      if ($this->action->action == TeamQ_logoff)
      {
        login_logout();
        $this->user_id = 0;
      }      
      
      $this->resultmode = true;
      return;
    }
    
    $this->category = (int)$this->loadattribute("category");
    
    $r = db_exec("
      SELECT 
      q.question_id, u2.user_id
      FROM users AS u1
      INNER JOIN memberships AS m1 USING (user_id)
      INNER JOIN memberships AS m2 USING (team_id)
      INNER JOIN users AS u2 USING (user_id)
      INNER JOIN questions AS q ON q.category_id = $this->category
      WHERE u1.user_id = $this->user_id
    ", $team, __FILE__, __LINE__);

    while($row = mysql_fetch_assoc($r))
    {
      $v = (int)$this->loadattribute("q{$row[question_id]}u{$row[user_id]}");
      if (1 <= $v && $v <= 5)
      {
        db_exec("
          REPLACE INTO answers (user_id, question_id, who, response)
          VALUES ($this->user_id, $row[question_id], $row[user_id], $v)
        ", $this->team, __FILE__, __LINE__);
      }
    };  

    if ($this->action->action == TeamQ_cat)
    {
      $this->category = $this->action->object;
    }
    else if ($this->action->action == TeamQ_logoff)
    {
      login_logout();
      $this->user_id = 0;
    }
  }
  
  function showresults()
  {
    $team = $this->team;
    $user_id = $this->user_id;

    print("<h3>Survey Results</h3>\n");

    $r = db_exec("
      SELECT IF (u.lastname IS NULL, u.cunix, CONCAT(u.firstname, ' ', u.lastname)) AS name, t.name AS tname, t.team_id 
      FROM users AS u
      INNER JOIN memberships AS m USING (user_id)
      INNER JOIN teams AS t USING (team_id)
      WHERE u.user_id = $user_id
    ", $team, __FILE__, __LINE__);
    
    $row = mysql_fetch_assoc($r);
    
    $team_id = $row['team_id'];
    print("User: <b>$row[name]</b><br>\n");
    print("Team: <b>$row[tname]</b><br>\n");
    db_exec("
      CREATE TEMPORARY TABLE bob AS
      SELECT c.category_id, q.ordinal, q.name, IFNULL(a1.response,0) AS self, AVG(a2.response) AS others
      FROM categories AS c
      INNER JOIN questions AS q USING (category_id)
      INNER JOIN teams AS t ON t.team_id = $team_id
      LEFT JOIN answers AS a1 ON a1.user_id = $user_id AND a1.who = $user_id AND a1.question_id = q.question_id
      LEFT JOIN answers AS a2 ON a2.who = $user_id AND a2.question_id = q.question_id AND a2.user_id <> $user_id
      GROUP BY q.question_id
    ", $team, __FILE__, __LINE__);
    
    $a = db_exec("
      SELECT c.category_id, c.name, AVG(b.self) AS self, AVG(b.others) AS others
      FROM categories AS c
      INNER JOIN bob AS b USING (category_id)
      GROUP BY c.category_id
      ORDER BY c.ordinal
    ", $team, __FILE__, __LINE__);
    
    function printrow($text, $self, $others, $b)
    {
      if ($b)
        printf("<tr><td><b>$text</b></td><td><b>%.2f</b></td><td><b>%.2f</b></td></tr>\n", $self, $others);
      else
        printf("<tr><td>$text</td><td>%.2f</td><td>%.2f</td></tr>\n", $self, $others);
    }
    
    print("<table cellpadding=2>\n");
    printf("<tr><td>&nbsp;</td><td><i>Self</i></td><td><i>Others</i></td></tr>\n");
    while($ra = mysql_fetch_assoc($a))
    {
      $b = db_exec("
        SELECT name, self, others FROM bob WHERE category_id = $ra[category_id] ORDER BY ordinal
      ", $team, __FILE__, __LINE__);
    
      printrow($ra['name'], $ra['self'], $ra['others'], true);
    
      while($rb = mysql_fetch_assoc($b))
      {
        printrow($rb['name'], $rb['self'], $rb['others'], false);
      }
    }
    print("</table>\n");
  
  
  }
  
  function showstatus()
  {
    $r = db_exec("
      SELECT c.category_id, c.name, COUNT(*) AS total, COUNT(distinct a.who, a.question_id) AS filled
      FROM categories AS c
      INNER JOIN questions as q USING (category_id)
      INNER JOIN users AS u ON u.user_id = $this->user_id
      INNER JOIN memberships AS m1 USING (user_id)
      INNER JOIN memberships AS m2 USING (team_id)
      INNER JOIN users AS u2 USING (user_id)
      LEFT JOIN answers AS a ON a.question_id = q.question_id AND a.user_id = u.user_id AND a.who = u2.user_id
      GROUP BY c.category_id
      ORDER BY c.category_id
    ", $this->team, __FILE__, __LINE__);
      
    print("<table border=1>\n");
    print("<tr><td><i>Category</i></td><td><i>Fields Completed</i></td><td><i>Fields Total</i></td></tr>\n");
    while($row = mysql_fetch_assoc($r))
    {
      $f = $row[filled];
      if ($f != $row['total']) $f = "<font color=red>$f</font>";
      print("<tr><td>$row[name]</td><td>$f</td><td>$row[total]</td></tr>");
    }
    print("</table>\n");  
  }
  
  function display()
  {
    global $server_media;
    $team = $this->team;
 
    if (!$this->user_id)
    {
      print("You are now logged out.");
      return;    
    }
    
    if ($this->resultmode)
    {
      print("<p align=center>");
      $this->action->display("Log Out", TeamQ_logoff);
      print("</p>\n");      
      $this->showresults();
      return;
    }
    
    $this->printattribute("category", $this->category);
    $choices = array(1 => "Never", 2 => "Rarely", 3 => "Sometimes", 4 => "Frequently", 5 => "Always");
    
    $r = db_exec("SELECT category_id, name FROM categories ORDER BY ORDINAL", $team, __FILE__, __LINE__);
    $first = true;
    
    if ($this->category == 0)
      print("<strong>Overview</strong>");
    else
      $this->action->display("Overview", TeamQ_cat, 0);
    
    print (" | ");
      
    while($cat = mysql_fetch_assoc($r))
    {
      if ($cat['category_id'] == $this->category)
      {
        print("<strong>$cat[name]</strong>");
      }
      else
      {
        $this->action->display($cat['name'], TeamQ_cat, $cat['category_id']);
      }  
      print (" | ");
    };
    //$this->action->display("Save", TeamQ_logoff);
    //print (" | ");
    $this->action->display("Log Out", TeamQ_logoff);
    
    if ($this->category == 0)
    {
?>
<h4>About Team Developer</h4>
<p>
   Working in teams requires four core skills:  communication,
   collaboration, leadership, and decision making.  You and other
   team members are the source of these skills.  The team's ability
   to perform at high levels depends upon each individual and the
   ontributions he or she makes.  Information gathered with the Team
   Developer surveys can be used to guide the growth of any team and
   its members.  Each person for whom a survey is completed receives
   a Team Developer report comparing his or her own responses to the
   combined observations of other team members.  The report also
   provides the recipient with a list of specific actions he or she
   can take to become a high-performing team member.  Individual
   ratings always remain confidential, as team members will see only
   averages of others' responses.
</p>
<h4>Rating Instructions</h4>
  
<p>
   Think about all of the situations in which you and the individuals
   you are rating work together.  First you will rate yourself on how
   frequently you engage in the behavior shown at the top of each
   section.  Then you will rate each of your fellow team members on
   how frequently you have observed him or her engaged in the same
   behavior. Response options are provided below each survey item.
</p>
   Remember:
   <ul>
     <li> This information will remain confidential, so be completely
       candid.
     <li> Base your responses on actual behavior.
     <li> Don't worry about mistakes; you will be able to make
       corrections after you have rated each group member.
   </ul>
   
   The survey is divided up into four pages, one for each of the core skills. You can jump to any part of the survey using the navigation buttons at the top of each page. You can also use the "Next" and "Previous" buttons at the bottom of each page to advance through the survey sequentially. Pressing any button on any page saves your responses. So, if you log out in the middle of the survey and log back in, you responses would be saved and you could pick up where you left off.   
   
<h4>Survey Status</h4>    
<?  
     
$this->showstatus();

?>
<p>The table above shows how much of the survey you have filled out. A red number in a category indicates that there are still questions left in that category that haven't been completed. </p>

<p>On each page of the survey, any incomplete fields will appear in <b>bold</b>.</p>
<?      
    }
    else if ($this->category <= 4)
    {
      $r = db_exec("
        SELECT 
        q.name AS qname, q.question_id, u2.user_id, a.response,
        IF (u1.user_id = u2.user_id, 1, 0) AS isme,
        IF (u1.user_id = u2.user_id, '<i>Yourself</i>', IF(u2.lastname IS NOT NULL, CONCAT(u2.firstname, ' ', u2.lastname), u2.cunix)) AS name
        FROM users AS u1
        INNER JOIN memberships AS m1 USING (user_id)
        INNER JOIN memberships AS m2 USING (team_id)
        INNER JOIN users AS u2 USING (user_id)
        INNER JOIN questions AS q ON q.category_id = $this->category
        LEFT JOIN answers AS a ON a.user_id = u1.user_id AND a.question_id = q.question_id AND a.who = u2.user_id
        WHERE u1.user_id = $this->user_id
        ORDER BY q.ordinal, isme DESC, name
      ", $team, __FILE__, __LINE__);
      
      $lastq = -1;
      for(;;)
      {
        $row = mysql_fetch_assoc($r);
  
        if ((!$row || $row['question_id'] != $lastq) && lastq >= 0)
          print("\n</tbody>\n</table>\n");
        
        if (!$row) break;
  
        if ($row['question_id'] != $lastq)
        {
?>    	
<hr>
<p><strong><?= $row['qname'] ?></strong></p>
<table bordercolor=black cellspacing=0 cellpadding=3 border=0 RULES="groups" FRAME=box STYLE="border: none">
<tbody>
<tr>
  <td bgcolor=black background="<?=$server_media?>/0x000000.gif">&nbsp;</td>
<? foreach($choices as $choice) { ?>
  <td bgcolor=black background="<?=$server_media?>/0x000000.gif" align=center><font color=white><STRONG><?=$choice?></STRONG></font></td>
<? } ?>  
</tr>
<?
          $lastq = $row['question_id'];
          $arow = true;
        }
        $name = "{$this->prefix}_q{$row[question_id]}u{$row[user_id]}";
        $arow = !$arow; $color = $arow ? 'bgcolor="#FFFFFF" background="' . $server_media . '/0xFFFFFF.gif"' : 'bgcolor="#EEEEEE" background="' . $server_media . '/0xEEEEEE.gif"';
        $text = $row['name'];
        if (!$row['response']) $text = "<b>$text</b>";
        print("<tr>\n<td $color>$text</td>\n");
        print("<td $color align=center>");
        $sep = "</td>\n<td $color align=center>";      
        $first = true;
        foreach($choices as $i => $choice)
        {
          if ($first) $first = false; else print($sep);
          $id = $this->prefix . '_q' . $questionno . '_' . $i;
          $checked = $i === $this->answers[$questionno] ? " checked" : "";
          $checked = $row['response'] == $i ? " checked" : "";
          print("<input type=radio name=$name value=$i id=$id$checked>");
        }  
        print("</td></tr>");
      }
    }
    else
    {
      $a = mysql_result(db_exec("SELECT COUNT(*) FROM answers WHERE user_id = $this->user_id", $team, __FILE__, __LINE__),0);
      $q = mysql_result(db_exec("
        SELECT COUNT(*)
        FROM questions AS q
        INNER JOIN users AS u ON u.user_id = $this->user_id
        INNER JOIN memberships AS m1 USING (user_id)
        INNER JOIN memberships AS m2 USING (team_id)
        INNER JOIN users AS u2 USING (user_id)
      ", $team, __FILE__, __LINE__),0);
      //print ("a = $a, q = $q");
      if ($a == $q)
      {
        print("<h4>Survey Complete</h4>");
        print("<p>Thank you for filling completing the survey. You can now log out, or go back and review your responses.</p>");
      }
      else
      {
        print("<h4>Survey Not Complete</h4>");
        $this->showstatus();
        print("<p>Some sections of the survey are still not complete. Use the buttons at the top of this page to go back to a specific section. Questions that are not complete appear in <b>bold</b>.</p>");
      }
    }
    
    print("<hr>\n");
    print("<p align=left>");
    if ($this->category > 0)
    {
      $this->action->display("< Previous", TeamQ_cat, $this->category - 1);
      print(" ");
    };
    if ($this->category < 4)
    {
      $this->action->display("Next >", TeamQ_cat, $this->category + 1);
    }
    else if ($this->category == 4)
    {
      $this->action->display("Finish", TeamQ_cat, $this->category + 1);
    }    
    
    print("</p>");
    
  }
};

$team = team_connect();
$tq = new TeamQ(login_getuserid(), $team, "tq", "f", WIDGET_POST);
$tq->loadvalues();

page_top("Team Developer");
?>
<form method=post name=f>
<? $tq->display(); ?>
</form>
<? page_bottom(); ?>

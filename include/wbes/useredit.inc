<?

class UserEntry
{
  function UserEntry($first = "", $last = "", $ssn = "", $teams = "1")
  {
    $this->first = $first;
    $this->last = $last;
    $this->ssn = $ssn;
    $this->teams = $teams;
  }
  
}

define("UserList_insert", 1);

class UserList extends FormWidget
{
  var $users = array();
  var $action;
  
  function UserList($prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
  }

  function save()
  {
    global $db,$topic_id;
    if (!isset($db)) $db = wces_connect();
    
    db_exec("DELETE FROM tusers WHERE topicid = $topic_id", $db, __FILE__, __LINE__);
    db_exec("DELETE FROM teams WHERE topicid = $topic_id", $db, __FILE__, __LINE__);
    db_exec("DELETE FROM tuserassoc WHERE topicid = $topic_id", $db, __FILE__, __LINE__);
    
    $teams = array();
    reset($this->users);
    while(list($o,$u) = each($this->users))
    {
      if ($u->first || $u->last)
      {
        db_exec("INSERT INTO tusers (firstname,lastname,ssn,topicid) VALUES ('" . addslashes($u->first) . "','" . addslashes($u->last) . "','" . addslashes($u->ssn) . "', $topic_id)", $db, __FILE__, __LINE__);
        $userid = mysql_insert_id($db);
        $n = "";
        $l = strlen($u->teams);
        for($i=0;$i<=$l;++$i)
        {
          $c = $u->teams[$i];
          $ca = ord($c);
          $is_num = (48 <= $ca && $ca <= 57) || $ca == 46;
          if (strlen($n) && !$is_num)
          {
            $team = (int)$n;
            if (!isset($teams[$team]))
            {
              db_exec("INSERT INTO teams (teamno, topicid) VALUES ($team, $topic_id)", $db, __FILE__, __LINE__);
              $teams[$team] = mysql_insert_id($db);
            }
            db_exec("INSERT INTO tuserassoc (tuserid, teamid, topicid) VALUES ($userid, $teams[$team], $topic_id)", $db, __FILE__, __LINE__);
            $n = "";
          }
          else
            $n .= $c;
        }  
      }
    }
  }
  
  function dumpscript()
  {
    ActionButton::dumpscript();
?>
<script>
<!--
  
  function UserList_addrows(position, form, actionelement)
  {
    var r = prompt("How many rows do you want to insert?",1)|0;
    if (r)
      ActionButton_sgo("<?=UserList_insert?>", r + "," + position, form, actionelement);
  }
   
// -->
</script>
<?
  }
  
  function loadvalues()
  {  
    global $db,$topic_id;
    $this->action->loadvalues();
    $rows = (int)$this->loadattribute("rows");
    if(!$rows)
    {
      if (!isset($db)) $db = wces_connect();
      
      $result = db_exec("
        SELECT u.tuserid, u.firstname, u.lastname, u.ssn, t.teamno
        FROM tusers AS u
        LEFT JOIN tuserassoc AS a USING (tuserid)
        LEFT JOIN teams AS t USING (teamid)
        WHERE u.topicid = $topic_id AND a.topicid = $topic_id AND t.topicid = $topic_id
        ORDER BY u.ord, u.tuserid, t.teamno
      ", $db, __FILE__, __LINE__);
      
      $lu = 0; $u = false;
      while($row = mysql_fetch_assoc($result))
      {
        if ($row['tuserid'] <> $lu)
        {
          if ($u) $this->users[] = $u;
          $u = new UserEntry();
          $u->first = $row['firstname'];
          $u->last = $row['lastname'];
          $u->ssn = $row['ssn'];
          $u->teams = $row['teamno'];
        }
        else
        {
          $u->teams .= ", $row[teamno]";
        }
        $lu = $row['tuserid'];
      }
      if ($u) $this->users[] = $u;
      $nu = new UserEntry();
      if ($u) $nu->teams = "";
      array_push($this->users,$nu,$nu,$nu,$nu);
    }
    else
    {
      $this->users = array();
      for($i = 0; $i < $rows; ++$i)
      {
        $u = new UserEntry
        (
          $this->loadattribute("r{$i}_first"),
          $this->loadattribute("r{$i}_last"),
          $this->loadattribute("r{$i}_pass"),
          $this->loadattribute("r{$i}_team")
        );
        $this->users[] = $u;
      }
    }
    
    if ($this->action->action == UserList_insert)
    {
      $a = array_map("intval", explode(",",$this->action->object));
      if (count($a) == 2)
      {
        $r = new UserEntry();
        for($i = 0; $i < $a[0]; ++$i)
          array_splice($this->users, $a[1], 0, array($r));
      }
      $this->action->action = 0;
    }
  }
  
  function display()
  {
    global $server_media;
    $rows = count($this->users);
    $this->printattribute("rows",$rows);
    $this->action->display();
?>
<table>
<tr>
  <td rowspan=2><small><b>First Name</b></small></td>
  <td rowspan=2><small><b>Last Name</b></small></td>
  <td rowspan=2><small><b>4 Digits SSN</b></small></td>
  <td rowspan=2><small><b>Team(s)</b></small></td>
  <td><img src="<?=$server_media?>/nada.gif" width=1 height=8></td>
</tr>
<?
    for($i=0;;++$i)
    {
?>
<tr>
  <td rowspan=2><a href="javascript:void(UserList_addrows(<?=$i?>,'<?=$this->form?>','<?=$this->action->prefix?>'));"><img src="insert.gif" width=15 height=13 border=0 alt="Insert Rows"></a></td>
</tr>
<?
      if ($i >= $rows) break;
      $u = &$this->users[$i];
      //print("<script>alert('" . serialize($u) . "');</script>");
?>        
<tr>
  <td rowspan=2><input type=text class=plainbox name=<?= "{$this->prefix}_r{$i}_first"?> value="<?= htmlspecialchars($u->first) ?>"></td>
  <td rowspan=2><input type=text class=plainbox name=<?= "{$this->prefix}_r{$i}_last" ?> value="<?= htmlspecialchars($u->last) ?>"></td>
  <td rowspan=2><input type=text class=plainbox name=<?= "{$this->prefix}_r{$i}_pass" ?> value="<?= htmlspecialchars($u->ssn) ?>"></td>
  <td rowspan=2><input type=text class=plainbox name=<?= "{$this->prefix}_r{$i}_team" ?> value="<?= htmlspecialchars($u->teams) ?>"></td>
</tr>
<?
    }
?>
<tr>
  <td colspan=4><img src="<?=$server_media?>/nada.gif" width=1 height=8></td>
</tr>
</table>
<?
  }
}

?>
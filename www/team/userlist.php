<?

require_once("wces/page.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

class UserEntry
{
  function UserEntry($first = "", $last = "", $password = "", $teams = "1")
  {
    $this->first = $first;
    $this->last = $last;
    $this->password = $password;
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
    $this->action->loadvalues();
    $rows = (int)$this->loadattribute("rows");
    if(!$rows)
    {
      $u = new UserEntry();
      $this->users = array($u, $u, $u, $u, $u);
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
  <td rowspan=2><small><b>Password</b></small></td>
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
  <td rowspan=2><input type=text class=plainbox name=<?= "{$this->prefix}_r{$i}_pass" ?> value="<?= htmlspecialchars($u->password) ?>"></td>
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



page_top("users");
print("<form name=f method=post>\n");
$d = new UserList("f","f",WIDGET_POST);
//$d->users = array
//(
//  new UserEntry("First", "Last", "234", "1"),
//  new UserEntry("First", "Last", "343", "1"),
//  new UserEntry("First", "Last", "567", "1"),
//  new UserEntry("First", "Last", "637", "2"),
//  new UserEntry("First", "Last", "853", "2")
//);
$d->loadvalues();
$d->dumpscript();
$d->display();

print("<input type=submit name=submitjkhlkj value=Submit>");
print("</form>\n");

page_bottom();

?>
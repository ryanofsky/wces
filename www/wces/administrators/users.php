<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/useredit.inc");

param('user_id');

login_protect(login_administrator | login_deptadmin);

page_top("Question Periods");

print("<p><a href=\"info.php?user_id=$user_id\">Back</a></p>");

$server_isproduction = false;

if (isset($user_id)) $user_id = (int)$user_id; else $user_id = 0;

print('<form enctype="multipart/form-data" method=post><input type=hidden name=MAX_FILE_SIZE value=1048576>');
print("<input type=hidden name=user_id value=$user_id>");
print($ISID);

$f =& new Form("f");
$q =& new UserEditor($user_id, "ue", $f);

$f->loadState();
$f->display();

if (!$q->done)
  $q->display();
else
  print($q->message);

print("</form>\n");
page_bottom();

?>

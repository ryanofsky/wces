<?

require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");
require_once("wces/useredit.inc");

param('user_id');

login_protect(login_professor);

$f =& new Form("f");
$q =& new UserEditor(login_getuserid(), "ue", $f);
$q->show_admin = false;
$f->loadState();

if ($q->saved) login_refresh();
if ($q->done) redirect($wces_path);

page_top("User Edit");

if (isset($user_id)) $user_id = (int)$user_id; else $user_id = 0;

print('<form enctype="multipart/form-data" method=post><input type=hidden name=MAX_FILE_SIZE value=1048576>');
print($ISID);

$f->display();
$q->display();

page_bottom();

?>
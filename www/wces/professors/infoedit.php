<?

require_once("wces/login.inc");
require_once("wces/page.inc");
require_once("wces/profinfo.inc");

login_protect(login_professor);
$professorid = login_getprofid();

$db = wces_connect();
$pe = new ProfessorEditor($db, $professorid, false, "pe", "f", WIDGET_POST);
$pe->loadvalues();

page_top("Professor Information Editor","0010");

?>
<form name="f" method="post" enctype="multipart/form-data">
<input type="hidden" name="MAX_FILE_SIZE" value="1000">
<? $pe->display(); ?>
</form>
<?

page_bottom();

?>
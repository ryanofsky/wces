<?

require_once('wces/login.inc');
require_once('wces/page.inc');

login_protect(login_administrator);

require_once('wces/TopicEditor.inc');

function SimpleTop($title)
{
  global $server_media;
?>
<head>
<title><?=$title?></title>
<LINK REL="stylesheet" type="text/css" href="<?=$server_media?>/style.css">
</head>
<?
}

function SimpleBottom()
{}

$f =& new Form("f");
$t =& new TopicEditor(false, "topics", $f);
$f->loadState();

if ($t->barePage)
  list($pageTop, $pageBottom) = array("SimpleTop", "SimpleBottom");
else
  list($pageTop, $pageBottom) = array("page_top", "page_bottom");

$pageTop("Survey Builder");
print("<form name=$f->formName method=post action=\"$f->pageName\">");
print($AISID);
$f->display();
$t->display();
print("</form>");
$pageBottom();

?>
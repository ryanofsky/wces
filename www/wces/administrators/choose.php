<?
require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_administrator | login_deptadmin);
page_top("Reporting Wizard");
?>
<p>Which semesters do you need data from?</p>
<ul>
  <li><a href="report.php<?=$QSID?>">Spring 2002 or later</a></li>
  <li><a href="old/report.php<?=$QSID?>">Fall 2001 or earlier</a></li>
</ul>
<? page_bottom("Reporting Wizard"); ?>
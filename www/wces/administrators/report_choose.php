<?
require_once("wces/login.inc");
require_once("wces/page.inc");
LoginProtect(LOGIN_ADMIN | LOGIN_DEPT_ADMIN);
page_top("Reporting Wizard");
?>
<p>Which semesters do you need data from?</p>
<ul>
  <li><a href="report.php<?=$QSID?>">Spring 2002 or later</a></li>
  <li><a href="report_old.php<?=$QSID?>">Fall 2001 or earlier</a></li>
</ul>
<? page_bottom("Reporting Wizard"); ?>
<?
 require_once("wces/page.inc");
 require_once("wces/login.inc");
 require_once("wces/report_widget.inc");
 $depts = login_getdepts();
 if (!$depts) login_protect(login_administrator);
 
 $report = new Report("report","wiz",WIDGET_POST);
 $report->loadvalues();
 page_top("Administrative Report",$report->hidemenus);
?>

<form name="wiz" method="post">
<? $report->display(); ?>
</form>

<? page_bottom($report->hidemenus); ?>
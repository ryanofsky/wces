<?
  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/report_widget.inc");
  
  login_protect(login_administrator | login_deptadmin);
  
  $seconds = 3600;
  session_cache_limiter('public');
  header('Cache-Control: public');
  header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
  header('Last-Modified: ' . gmdate('D, d M Y H:i:s', getlastmod()) . ' GMT');
  
  $report = new Report("report","wiz",WIDGET_POST);
  $report->loadvalues();
  
  page_top("Administrative Report",$report->hidemenus);
?>

<form name="wiz" method="post">
<? $report->display(); ?>
</form>

<? page_bottom($report->hidemenus); ?>
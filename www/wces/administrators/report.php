<?

  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/report_widget.inc");
  login_protect(login_administrator | login_deptadmin);
 
  $f =& new Form('wiz');
  $report =& new Report('report', $f);
  //DebugBreak();
  $f->loadState();

  $seconds = 3600;
  session_cache_limiter('public');
  header('Cache-Control: public');
  header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
  header('Last-Modified: ' . gmdate('D, d M Y H:i:s', getlastmod()) . ' GMT');

  page_top("Administrative Report",$report->hideMenus);
 
  print("<form name={$f->formName} action=\"{$f->pageName}\" method=post>\n");
  print($ISID);
  $f->display();
  $report->display();
  print("</form>");

  page_bottom($report->hideMenus);

?>
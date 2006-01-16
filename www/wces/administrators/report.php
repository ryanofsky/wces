<?

  require_once("wces/page.inc");
  require_once("wces/login.inc");
  require_once("wces/report_widget.inc");

  LoginProtect(LOGIN_ADMIN | LOGIN_DEPT_ADMIN);
 
  $f =& new Form('wiz');
  $t =& new InitializerWidget('init', $f);
  $report =& new Report('report', $t);
  $f->loadState();

  $seconds = 3600;
  session_cache_limiter('public');
  header('Cache-Control: public');
  header('Pragma: public');
  header('Expires: ' . gmdate('D, d M Y H:i:s', time() + $seconds) . ' GMT');
  header('Last-Modified: ' . gmdate('D, d M Y H:i:s', getlastmod()) . ' GMT');

  page_top("Reporting Wizard",$report->hideMenus);
 
  print("<form name={$f->formName} action=\"{$f->pageName}\" method=post>\n");
  print($ISID);
  $f->display();
  $t->display();
  $report->display();
  print("</form>");

  page_bottom($report->hideMenus);

?>

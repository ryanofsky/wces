<?
  require_once("team/login.inc");
  require_once("team/page.inc");
  login_logout();

  page_top("Logged Out");
  print("You are now logged out.");
  page_bottom();
?>
<?
  require_once("wces/login.inc");
  require_once("wces/page.inc");
  $login =& LoginInstance();
  $login->logout();

  page_top("Logged Out");
  print("You are now logged out.");
  page_bottom();
?>
<?
  require_once("wces/general.inc");
  require_once("test/test.inc");
  param($status);
  test_top("Log Out",$status);
  print("<h3>You are now logged out.</h3>");
  test_bottom();
?>
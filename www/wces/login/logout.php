<%
  require_once("login.inc");
  require_once("page.inc");
  login_logout();

  page_top("Logged Out");
  print("You are now logged out.");
  page_bottom();
%>
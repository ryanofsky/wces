<%
  require_once("wces/page.inc");
  login_protect(login_professor);
  $profid = login_getprofid();
  
  page_top("professors Page","0010");

  $db = wces_connect();
  $profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");
%>
<h3><%=$profname%></h3>
<p>Welcome to the professors page.</p>
<p>Currently, these options are available:</p>
<ul>
<li><a href="<%=$server_wcespath%>professors/editsurveys.php">Edit Custom Questions</a></li>
<li><a href="<%=$server_wcespath%>professors/previewsurveys.php">Preview Surveys</a></li>
<li><a href="<%=$server_wcespath%>professors/seeresults.php">See Survey Results</a></li>
</ul>
<p>Remember to <a href="<%=$server_wcespath%>login/logout.php">log out</a> when you are done.</p>
<%
  page_bottom();
%>



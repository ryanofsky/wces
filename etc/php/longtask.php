<%
require_once("page.inc");
require_once("taskwindow.inc");

page_top("A Long Task");
taskwindow_start("Progress Window");

taskwindow_print("Here are the results of your request:<br>");
flush();

for ($i = 0; $i <= 10000000; ++$i)
  if ($i % 100000 == 0)
  {
    taskwindow_print("$i<br>");
    flush();
  };  

taskwindow_print("Done.");
taskwindow_end();
page_bottom();
%>
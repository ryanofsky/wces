<?
require_once("wces/page.inc");
require_once("wces/taskwindow.inc");

page_top("A Long Task");
taskwindow_start("Progress Window", true);

taskwindow_print("Here are the results of your request:<br>");
flush();

for ($i = 0; $i <= 2000000; ++$i)
  if ($i % 100000 == 0)
  {
    taskwindow_print("$i<br>");
    flush();
  };  

taskwindow_print("Done.");
taskwindow_end();
page_bottom();
?>
<?
require_once("page.inc");
page_top("Demo");

?>
<p align=center><a href="survey.php">Click here to take survey</a></p>
<p align=center><a href="results.php">Click here to see the results</a></p>
<p align=center><a href="builder.php<?=$wces_ns4 ? "/ns4" : ""?>">Click here to access the survey builder</a></p>
<? page_bottom(); ?>
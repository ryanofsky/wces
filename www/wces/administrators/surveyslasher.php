<?
require_once("wbes/server.inc");
require_once("wbes/wbes.inc");
require_once("wbes/postgres.inc");

require_once("wces/page.inc");

page_top("Remove classes");

//first get a database connection.
global $wbes;
wbes_connect();


if(!isset($HTTP_POST_VARS['input']))
    {
?>

<h3>Following is a list off the classes currently being surveyed.</h3>
<br>
<h4>Please check the boxes for those classes you do not wish to be surveyed,
 then press submit. They will then be deleted.</h4>
<br>

<?
  $result = pg_query("
    SELECT question_period_id, displayname, year, semester
    FROM semester_question_periods
    WHERE question_period_id = (SELECT get_anext_question_period())
  ", $wces, __FILE__, __LINE__);
  extract(pg_fetch_array($result,0,PGSQL_ASSOC));
  
    $sql = "SELECT topic_id, d.name AS name, section, year, s.code || ' ' || d.code AS code FROM wces_topics t, classes c, courses d, subjects s";
    $sql = $sql . " WHERE c.class_id = t.class_id and c.year = $year and c.semester = $semester and c.course_id = d.course_id and d.subject_id = s.subject_id and t.category_id IS NOT NULL ORDER BY code, section";
   
    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
?>

<form method=POST>
<table border=0 cellpadding=0 cellspacing=2 width="100%" bgcolor="#ffffff">
<tr>
    <td bgcolor="#555599" width=60><h3><font color="#ffffff">Delete</font></h3></td>
    <td bgcolor="#555599"><h3><font color="#ffffff">Name</font></h3></td>
    <td bgcolor="#555599" width=80><h3><font color="#ffffff">Section</font></h3></td>
    <td bgcolor="#555599" width=80><h3><font color="#ffffff">Year</font></h3></td>
    <td bgcolor="#555599" width=80><h3><font color="#ffffff">Code</font></h3></td>
</tr>

<?
    $counter = 0;
    for($wr = new pg_wrapper($result); $wr->row; $wr->advance())
	{
	$counter++;
	$color_odd = "#ccccff";
	$color_even = "#aaaaee";
		
	?>
	<tr>
	    <td bgcolor="<?= $color_odd ?>"><input type=checkbox name="<?= $wr->row['topic_id'] ?>" value="remove"></td>
	    <td bgcolor="<?= $color_even ?>"><?= $wr->row['name'] ?></td>
	    <td bgcolor="<?= $color_odd ?>"><?= $wr->row['section'] ?></td>
	    <td bgcolor="<?= $color_even ?>"><?= $wr->row['year'] ?></td>
	    <td bgcolor="<?= $color_odd ?>"><?= $wr->row['code'] ?></td>
	</tr>
	
	<?
	}
?>
</table>

<input type=hidden name="input" value="exists">
<input type=submit name="OK" value="OK">

</form>

</body>
<?
    }
else
    {    
    $elements = null;
    
    while(list($key, $value) = each($HTTP_POST_VARS))
	{
	//print("Got Key: " . $key . " Value: " . $value . "<br>\n");
	if(!strcmp($value, "remove"))
	    {
	    if($elements == null)
		{$elements = $key;}
	    else
		{$elements = $elements . ", " . $key;}
	    }
	}

    $sql = "UPDATE wces_topics SET category_id = NULL WHERE topic_id IN(" . $elements . ")";
?>    
    
    <h3>Input Accepted.</h3>
    <br>
    SQL: <?= $sql ?>
    <br>
    Executing SQL now.
    <br><br>
    <?
    global $wbes;
    wbes_connect();
    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
    ?>
    Execution done.<br>
    
    
    
    
    
    
    

<?    
    }
page_bottom();    
    
?>
    
    

<?
require_once("wbes/server.inc");
require_once("wbes/wbes.inc");
require_once("wbes/postgres.inc");

require_once("wces/page.inc");

login_protect(login_administrator);

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
 <br><br>
 <h4>
 You may update classes by checking the update box and then modifying the data in
 any fields you wish. Obviously you should refrain from both updating and deleting a single
 class. :-)
 </h4>

<br>

<?
    $sql = "SELECT topic_id, d.name AS name, section, year, s.code || ' ' || d.code AS code FROM wces_topics t, classes c, courses d, subjects s";
    $sql = $sql . " WHERE c.class_id = t.class_id and c.course_id = d.course_id and d.subject_id = s.subject_id and t.category_id IS NOT NULL ORDER BY code, section";
   
    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
?>

<form method=POST>
<table border=0 cellpadding=0 cellspacing=2 width="100%" bgcolor="#ffffff">
<tr>
    <td bgcolor="#555599" width=60><h3><font color="#ffffff">Delete</font></h3></td>
    <td bgcolor="#555599" width=60><h3><font color="#ffffff">Update</font></h3></td>
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
	    <td bgcolor="<?= $color_odd ?>">
		Delete: <input type=checkbox name="<?= $wr->row['topic_id'] ?>" value="remove_this">
	    </td>
	    <td bgcolor="<?= $color_even ?>">
		Update: <input type=checkbox name="<?= $wr->row['topic_id'] ?>_update" value="update_this">
	    </td>
	    <td bgcolor="<?= $color_odd ?>">
		<input type=text name="<?= $wr->row['topic_id'] ?>_name" 
		value="<?= $wr->row['name'] ?>" size=50>
	    </td>
	    <td bgcolor="<?= $color_even ?>"><?= $wr->row['section'] ?></td>
	    <td bgcolor="<?= $color_odd ?>"><?= $wr->row['year'] ?></td>
	    <td bgcolor="<?= $color_even ?>"><?= $wr->row['topic_id'] ?></td>
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
    $otherData = null;
    
    
    while(list($key, $value) = each($HTTP_POST_VARS))
	{
	if(!strcmp($value, "remove_this"))
	    {
	    if($elements == null)
		{$elements = $key;}
	    else
		{$elements = $elements . ", " . $key;}
	    }
	else
	    {
	    //not one to be deleted, lets add it to our own hash table.
	    
	    if($otherData == null)
		{$otherData = array($key => $value);}
	    else
		{$otherData[$key] = $value;}
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
    if($elements != null)
	{$result = pg_query($sql, $wbes, __FILE__, __LINE__);}
    ?>
    
    Execution done.<br>
    
    Now performing updates.
    <br><br>




    <?


    
    
    print("<BR><BR>NOW DOING UPDATE STUFF.<BR><BR>");
    
    
    
    
       
    while(list($key, $value) = each($otherData))
	{
	//print("[2] Got other data: $key ===> $value\n<br>");
		
	if(!strcmp($value, "update_this"))
	    {
	    print("<br>Doing an update.\n<br>");
	    $Id = substr($key, 0, strpos($key, "_"));
	    
	    print("Got an ID: $Id\n<br>");
	    
	    $name = $otherData[$Id . "_name"];
	    /*
	    $section = $otherData[$Id . "_section"];
	    $year = $otherData[$Id . "_year"];
	    $code = $otherData[$Id . "_code"];
	    */
	    
	    print("Got Name: $name\n<br>");
	    /*
	    print("Got Section: $section\n<br>");
	    print("Got Year: $year\n<br>");
	    print("Got Code: $code\n<br>");
	    
	    $code1 = substr($code, 0, strpos($code, " "));
	    $code2 = substr($code, (strpos($code, " ") + 1));
	    
	    print("Broke down code, CODE1: $code1\n<br>");
	    print("CODE2: $code2\n<br>");
	    */
	    $sql = "SELECT c.course_id, c.class_id, d.subject_id FROM wces_topics t, classes c, courses d WHERE t.topic_id = '$Id' and c.class_id = t.class_id and c.course_id = d.course_id";
	    
	    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
	    
	    $wr = new pg_wrapper($result);
	    
	    if($wr->row == null)
		{print("ERROR, no row for that topic ID: $Id\n<br>");}
	    
	    $class_id = $wr->row['class_id'];
	    $course_id = $wr->row['course_id'];
	    $subject_id = $wr->row['subject_id'];
	    
	    print("Now doing updates.\n<br>");
	    /*

	    $sql = "UPDATE classes SET section = $section, year = $year WHERE class_id = '$class_id'";
	    
	    print("SQL 1: $sql \n<br><br>");

	    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
	    
	    $sql = "UPDATE courses SET name = '$name', code = $code2 WHERE course_id = '$course_id'";
	    */
	    $sql = "UPDATE courses SET name = '$name' WHERE course_id = '$course_id'";

	    print("SQL 2: $sql \n<br><br>");
	    
	    $result = pg_query($sql, $wbes, __FILE__, __LINE__);
	    }
	}




    }
page_bottom();    
    
?>
    
    

<?
require_once("wbes/server.inc");
require_once("wbes/wbes.inc");
require_once("wbes/postgres.inc");

require_once("wces/page.inc");

param('question_period_id');

if (!empty($question_period_id)) set_question_period($question_period_id);

LoginProtect(LOGIN_ADMIN);

page_top("Remove classes");

//first get a database connection.
global $wbes;
wbes_connect();

$first = true;
$question_period_id = get_question_period();

if (!$question_period_id) {
?><p>No question periods found. Click <a href="<?=$wces_path?>administrators/questionperiods.php">here</a> to create one.</p><?
} else {

print("<p><b>Question Periods:</b> ");
foreach(get_question_periods() as $id => $name)
{
  if ($first) $first = false; else print(" | ");

  if ($id == $question_period_id)
    print("<span style=\"white-space: nowrap\">$name</span>");
  else
    print("<a href=\"$_SERVER[PHP_SELF]?question_period_id=$id$ASID\" style=\"white-space: nowrap\">$name</a>\n");
}
print("</p>");



if(!isset($_POST['input']))
    {
?>

<h3>Following is a list off the classes being surveyed.</h3>
<br>
<h4>Please check the boxes for those classes you do not wish to be surveyed,
 then press submit. They will then be deleted.</h4>

<br>

<?
    $sql = "SELECT topic_id, d.name AS name, section, year, s.code || ' ' || d.code AS code FROM wces_topics t, classes c, courses d, subjects s";
    $sql = $sql . " WHERE t.question_period_id = $question_period_id AND NOT t.cancelled AND c.class_id = t.class_id and c.course_id = d.course_id and d.subject_id = s.subject_id ORDER BY code, section";
   
    $result = pg_go($sql, $wbes, __FILE__, __LINE__);
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
	    <td bgcolor="<?= $color_even ?>" align=center>
		<input type=checkbox name="<?= $wr->row['topic_id'] ?>" value="remove_this">
	    </td>
	    <td bgcolor="<?= $color_odd ?>"><?= htmlspecialchars($wr->row['name']) ?></td>
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
    
    
    while(list($key, $value) = each($_POST))
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

    $sql = "UPDATE wces_topics SET cancelled = 't' WHERE topic_id IN(" . $elements . ")";
    
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
	{$result = pg_go($sql, $wbes, __FILE__, __LINE__);}
    ?>
    
    Execution done.<br>
    
    <?




    }

}

page_bottom();    
    
?>
    
    

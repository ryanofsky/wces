<?

require_once("wces/login.inc");
require_once("wces/page.inc");
login_protect(login_professor);

param('lastname');
param('url');

page_top("Professor Search Page");

if (!$lastname)
{
?>
<p>WCES needs to associate your CUNIX id, '<b><?=login_getuni()?></b>' with a professor listing from our database.</p>
<p>After this has been done once, WCES will remember the association for future logins. Associations can be updated at any time by choosing '<b>Update CUNIX Association</b>' from the professors menu on the left side of WCES pages.</p>
<p>Type in your last name in the form below to search for your listing.</p>
<?
}
?>
<form name=profsearch action=profsearch.php method=get>
Last Name: <input name="lastname" type="text" size="20" value="<?=htmlspecialchars($lastname)?>"> <input name="search" type="submit" value="Search">
<input type="hidden" name="url" value="<?=htmlspecialchars($url)?>">
</form>
<?

if ($lastname)
{
  ?>
  <hr><h3>Results</h3>
  <p>Find the listing below that displays your name and classes and follow its 'Use This Listing' link.</p>
  <p><font size=-1">Note: If you see your name listed twice, you should contact an administrator so the listings can be merged. In the meantime, choose the listing with the classes you need to access.</font></p>
  <?

  wces_connect();

  $professors = pg_go("SELECT user_id, lastname, firstname FROM users WHERE flags & 4 = 4 AND lastname ILIKE '%" . addslashes($lastname) . "%' LIMIT 30",$wces,__FILE__,__LINE__);
  $sems = array(0 => "Spring", 1 => "Summer", 2 => "Fall");
  if (($ps = pg_numrows($professors)) > 0)
  {
    print("<TABLE cellSpacing=0 cellPadding=2 border=1>\n");
    for($p = 0; $p < $ps; ++$p)
    {
      extract(pg_fetch_array($professors,$p,PGSQL_ASSOC));
      print("<TR><TD>$firstname $lastname [<a href=\"profbounce.php?user_id=$user_id&url=$url\">Use This Listing</a>]<UL>");
      $classes = pg_go("
        SELECT cl.section, cl.year, cl.semester, c.code, c.name, s.code as scode
        FROM enrollments AS e
        INNER JOIN classes as cl USING (class_id)
        INNER JOIN courses AS c USING (course_id)
        INNER JOIN subjects AS s USING (subject_id)
        WHERE e.user_id = '$user_id' AND status = 3
        ORDER BY cl.year DESC, cl.semester DESC LIMIT 50
      ",$wces,__FILE__,__LINE__);
      
      $cs = pg_numrows($classes);
      for($c = 0; $c < $cs; ++$c)
      {
        extract(pg_fetch_array($classes,$c,PGSQL_ASSOC));
        print ("<LI>$sems[$semester] $year  - $scode$code <i>$name</i> (section $section)</LI>");
      }
      print("</UL><P>&nbsp;</P></TD></TR>\n");
    }
    print("</TABLE>\n<p>&nbsp;</p>\n");
  }
  else
  {
?>
<p><i>No matches found. Please contact an administrator if you need to have your name added to the database.</i></p>
    
<?
  };  
};

page_bottom();

?>






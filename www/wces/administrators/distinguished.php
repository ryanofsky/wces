<?

require_once("wbes/wbes.inc");
require_once("wbes/postgres.inc");
require_once("wces/wces.inc");
require_once("wces/login.inc");

login_protect(login_administrator);

$choice_qitem_id = 1133;
$choice_qrevision_id = 1279;
$choice_citem_id = 1134;
$choice_crevision_id = 1280;
$text_item_id = 1135;
$text_revision_id = 1281;
$year = 2002;
$semester = 2;
$answer = 0;

$db_debug = false;
$server_isproduction = false;

page_top('Distinguished Faculty Nominations', 1);
ob_flush(); flush();

wbes_connect();

pg_go("
  CREATE TEMPORARY TABLE distinguished AS
  SELECT p.class_id, p.user_id, p.students, r.response_id, qr.answer
  FROM 
  ( SELECT t.topic_id, cl.class_id, cl.students, e.user_id
    FROM wces_topics AS t
    INNER JOIN classes AS cl USING (class_id)
    INNER JOIN enrollments AS e ON e.status = 3 AND e.class_id = cl.class_id
    WHERE cl.year = $year AND cl.semester = $semester
  ) AS p
  INNER JOIN survey_responses AS r ON r.topic_id = p.topic_id
  INNER JOIN choice_responses AS cr ON cr.parent = r.response_id
  INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
  WHERE cr.item_id = $choice_citem_id AND cr.revision_id = $choice_crevision_id
    AND qr.item_id = $choice_qitem_id AND qr.revision_id = $choice_qrevision_id
", $wbes, __FILE__, __LINE__);

$sorts = array(0 => "votes", 1 => "ratio");

param('sort');
param('all');
param('r');
param('rr');

$rurl = '';
if ($r) $rurl .= '&r=1';
if ($rr) $rurl .= '&rr=1';

if (!isset($sorts[$sort])) $sort = 0;

$votes = "(SELECT COUNT(DISTINCT t1.response_id)::integer FROM distinguished AS t1 WHERE t1.user_id = sr.user_id AND answer = $answer)";
$responses = "(SELECT COUNT(DISTINCT t2.response_id)::integer FROM distinguished AS t2 WHERE t2.user_id = sr.user_id)";

pg_go("
  CREATE TEMPORARY TABLE summary AS
  SELECT sr.user_id, u.firstname, u.lastname, $votes AS votes, $responses AS responses
  FROM distinguished AS sr
  INNER JOIN users AS u USING (user_id)
  GROUP BY sr.user_id, u.firstname, u.lastname
", $wbes, __FILE__, __LINE__);

$limit = $all ? "" : "LIMIT 20";

$res = pg_go("
  SELECT user_id, responses, votes, votes::real / responses::real AS ratio, firstname, lastname
  FROM summary
  ORDER BY $sorts[$sort] DESC
  $limit
", $wbes, __FILE__, __LINE__);

$result =& new pg_wrapper($res);

print("<table border=1 cellspacing=0 cellpadding=2>\n");
print("<tr>\n");
print("  <td><b>#</b></td>\n");
print("  <td><b>Professor</b></td>\n");
if ($r)
  print("  <td><b>Responses</b></td>\n");

$a = $all ? "&all=1" : "";

if ($sort == 0)
  print("  <td><b>Nominations</b></td>\n");
else
  print("  <td><b><a href=\"$PHP_SELF?sort=0$a$rurl$ASID\">Nominations</a></b></td>\n");

if ($rr)
  if ($sort == 1)
    print("  <td><b>Percent Nominations</b><td>\n");
  else
    print("  <td><b><a href=\"$PHP_SELF?sort=1$a$rurl$ASID\">Percent Nominations</a></b></td>\n");

print("</tr>\n");

while($result->row)
{
  $user_id = $result->row['user_id'];
  print("<tr>\n");
  print("  <td><a href=\"#comments$user_id\"name=row$user_id>" . $result->rkey . "</a></td>\n");
  print("  <td>" . $result->row['firstname'] . ' ' . $result->row['lastname'] . "</td>\n");
  if ($r)
    print("  <td>" . $result->row['responses'] . "</td>\n");
  print("  <td>" . $result->row['votes'] . "</td>\n");
  if ($rr)
    print("  <td>" . sprintf("%.1f%%", $result->row['ratio'] * 100.0) . "</td>\n");
  print("</tr>\n");
  $result->advance();  
}
print("</table>");
if (!$all) print("<p><a href=\"$PHP_SELF?sort=$sort$rurl&all=1\">Show All Professors...</a></p>\n");

$result->rkey = 0;
$result->advance();

while($result->row)
{
  ob_flush(); flush();
  extract($result->row);
  $user_id = $result->row['user_id'];
  $i = $result->rkey;
  print("<hr>\n");
  print("<h3><a href=\"#row$user_id\" name=\"comments$user_id\">$i) $firstname $lastname</a></h3>\n");
  $r = sprintf("%.1f%%", $ratio * 100);
  print("<p>$responses responses, $votes nominations, $r of respondents nominated this professor</p>");
  
  $res = pg_go("
    SELECT d.class_id, get_class(d.class_id) AS class, r.rtext
    FROM distinguished AS d
    INNER JOIN textresponse_responses AS r ON r.parent = d.response_id
    WHERE d.user_id = $user_id AND d.answer = $answer 
      AND r.item_id = $text_item_id
      AND r.revision_id = $text_revision_id
    ORDER BY class, class_id
  ", $wbes, __FILE__, __LINE__);
  
  $tr = new pg_segmented_wrapper($res, 'class_id');
  
  while($tr->row)
  {
    if ($tr->split)
    {
      print("<h4><a href=\"{$wces_path}administrators/info.php?class_id=" . $tr->row['class_id'] . "$ASID\" target=infowindow>" . format_class($tr->row['class']) . "</a></h4>\n");
      print("<ul>\n");
    } 

    print("  <li>");
    print(nl2br(htmlspecialchars($tr->row['rtext'])));
    print("</li>\n");
    
    $tr->advance();
        
    if ($tr->split)
    print("</ul>\n");
  }
  $result->advance();  
}
print("</table>");

page_bottom(1);

?>
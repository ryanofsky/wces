<?
require_once("team/page.inc");
require_once("team/login.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

login_protect(login_any);

define("TeamQ_cat",    1);
define("TeamQ_logoff", 3);

page_top("Team Developer Results");

$team = team_connect();
$user_id = login_getuserid();

$r = db_exec("
  SELECT IF (u.lastname IS NULL, u.cunix, CONCAT(u.firstname, ' ', u.lastname)) AS name, t.name AS tname, t.team_id 
  FROM users AS u
  INNER JOIN memberships AS m USING (user_id)
  INNER JOIN teams AS t USING (team_id)
  WHERE u.user_id = $user_id
", $team, __FILE__, __LINE__);

$row = mysql_fetch_assoc($r);

$team_id = $row['team_id'];
print("User: <b>$row[name]</b><br>\n");
print("Team: <b>$row[tname]</b><br>\n");
db_exec("
  CREATE TEMPORARY TABLE bob AS
  SELECT c.category_id, q.ordinal, q.name, IFNULL(a1.response,0) AS self, AVG(a2.response) AS others
  FROM categories AS c
  INNER JOIN questions AS q USING (category_id)
  INNER JOIN teams AS t ON t.team_id = $team_id
  LEFT JOIN answers AS a1 ON a1.user_id = $user_id AND a1.who = $user_id AND a1.question_id = q.question_id
  LEFT JOIN answers AS a2 ON a2.who = $user_id AND a2.question_id = q.question_id AND a2.user_id <> $user_id
  GROUP BY q.question_id
", $team, __FILE__, __LINE__);

$a = db_exec("
  SELECT c.category_id, c.name, AVG(b.self) AS self, AVG(b.others) AS others
  FROM categories AS c
  INNER JOIN bob AS b USING (category_id)
  GROUP BY c.category_id
  ORDER BY c.ordinal
", $team, __FILE__, __LINE__);

function printrow($text, $self, $others, $b)
{
  if ($b)
    printf("<tr><td><b>$text</b></td><td><b>%.2f</b></td><td><b>%.2f</b></td></tr>\n", $self, $others);
  else
    printf("<tr><td>$text</td><td>%.2f</td><td>%.2f</td></tr>\n", $self, $others);
}

print("<table cellpadding=2>\n");
printf("<tr><td>&nbsp;</td><td><i>Self</i></td><td><i>Others</i></td></tr>\n");
while($ra = mysql_fetch_assoc($a))
{
  $b = db_exec("
    SELECT name, self, others FROM bob WHERE category_id = $ra[category_id] ORDER BY ordinal
  ", $team, __FILE__, __LINE__);

  printrow($ra['name'], $ra['self'], $ra['others'], true);

  while($rb = mysql_fetch_assoc($b))
  {
    printrow($rb['name'], $rb['self'], $rb['others'], false);
  }
}
print("</table>\n");

  




?>
</form>
<? page_bottom(); ?>

<?

require_once("wbes/postgres.inc");
require_once("wbes/wbes.inc");

param($bbid,0);
$bbid = (int)$bbid;

class Revision
{
  var $revision_id = 0;
  var $code = array();
  var $save_id;
  var $merged;
  var $type;
  var $topic_id;
}

$tables = array
(
  1  => "surveys",
  2  => "choice_components",
  3  => "textresponse_components",
  4  => "text_components",
  5  => "text_components",
  6  => "choice_questions",
  7  => "generic_components",
  8  => "subsurvey_components",
  9  => "pagebreak_components",
  10 => "abet_components"
);

$revisions = array();

function getRevisions($startBranch, &$base)
{
  global $revisions, $wbes;
  if ($startBranch)
  {
    $w = "r.parent IS NULL AND branch_id = $startBranch";
  }
  else
  {
    $w = $base ? "r.parent = $base->revision_id" : "r.parent IS NULL";
  }
  $result = pg_go("
    SELECT r.revision_id, r.branch_id, r.revision, r.save_id, r.merged, r.type, b.base_branch_id, b.topic_id
    FROM revisions AS r
    INNER JOIN branches AS b USING (branch_id)
    WHERE $w
    ORDER BY r.branch_id, r.revision
  ", $wbes, __FILE__, __LINE__);

  $n = pg_numrows($result);
  for($i = 0; $i < $n; ++$i)
  {
    $row = pg_fetch_array($result, $i, PGSQL_ASSOC);
    $r = new Revision();
    $r->revision_id = (int)$row['revision_id'];
    if ($base) $r->code = $base->code;
    array_push($r->code, (int)$row['branch_id'], (int)$row['revision']);
    $r->save_id = (int)$row['save_id'];
    $r->merged = (int)$row['merged'];
    $r->type = (int)$row['type'];
    $r->topic_id = (int)$row['topic_id'];
    array_push($revisions,$r);
    getRevisions(false, $r);
  }
}

function getBaseBranch($bbid)
{
  $result = pg_go("
    SELECT r.revision_id, r.branch_id, r.revision, r.save_id, r.merged, r.type, b.base_branch_id, b.topic_id
    FROM revisions AS r
    INNER JOIN branches AS b USING (branch_id)
    
    ORDER BY r.branch_id, r.revision
  ", $wbes, __FILE__, __LINE__);
}

wbes_connect();

if ($bbid)
{
  $r = NULL;
  getRevisions($bbid, $r);

  reset($revisions);
  while(list($key) = each($revisions))
  {
    $r = &$revisions[$key];
    $table = $tables[$r->type];
    print("<h4>Revision " .implode($r->code, ".") . "</h4>\n");
    print("<p>save_id = $r->save_id</p>");
    print("<p>type = $r->type ($table)</p>");
    print("<p>merged = $r->merged</p>");
    print("<p>topic_id = $r->topic_id</p>");
    $result = pg_go("SELECT * FROM $table WHERE revision_id = $r->revision_id",$wbes, __FILE__, __LINE__);
    pg_show($result);
    $result = pg_go("SELECT item_id FROM list_items WHERE revision_id = $r->revision_id ORDER BY ordinal", $wbes, __FILE__, __LINE__);
    if (0 < ($n = pg_numrows($result)))
    {
      print("<p><b>List Items (base_branch_id's): </b>");
      $first = true;
      for($i = 0; $i < $n; ++$i)
      {
        $id = (int)pg_result($result,$i,0);
        if ($first) $first = false; else print(", ");
        print("<a href=\"revs.php?bbid=$i\">$i</a>");
      }
      print("</p>");
    }
  }
}
else
{
  print("<p>Choose a base branch</p>");
  print("<ul>\n");
  $result = pg_go("SELECT branch_id FROM branches WHERE parent IS NULL ORDER BY branch_id", $wbes, __FILE__, __LINE__);
  $n = pg_numrows($result);
  for ($i = 0; $i < $n; ++$i)
  {
    $id = (int)pg_result($result, $i, 0);
    print("  <li><a href=\"revs.php?bbid=$id\">$id</a></li>\n");
  }
  print("</ul>\n");
}

?>
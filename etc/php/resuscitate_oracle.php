<?

require_once("wces/wces.inc");
require_once("wces/database.inc");

class Survey {}
class Choice {}

$qs = 1; // question set id
$qp = 9; // question period id 
$choices = array('a', 'b', 'c', 'd', 'e');
$questions = array(1,2,3,4,5,6,7,8,9,10);

$allquestions = $allchoices = "";
foreach($questions as $q)
{
  if ($allquestions) $allquestions .= ", ";
  $allquestions .= "MC$q";
  foreach($choices as $c)
  {
    if ($allchoices) $allchoices .= ", ";
    $allchoices .= "MC$q$c";
  } 
}

$nchoices = count($choices);
$nquestions = count($questions);

$db = wces_oldconnect();
$result = db_exec("SELECT $allquestions FROM questionsets WHERE questionsetid = $qs", $db, __FILE__, __LINE__);
$oldbase = mysql_fetch_assoc($result);

$result = db_exec("SELECT classid, dump FROM cheesyresponses WHERE questionperiodid = $qp order by classid", $db, __FILE__, __LINE__);
$lastclassid = 0;
$row = mysql_fetch_assoc($result);

$db_debug = true;
db_exec("DELETE FROM answersets WHERE questionperiodid = $qp AND questionsetid = $qs", $db, __FILE__, __LINE__);
$db_debug = false;
for(;;)
{
  if (!$lastclassid || $lastclassid != $row['classid'])
  {
    $count = 0;
    $map = array(); // map of prefixes to index in questions array
    $dist = array_pad(array(), $nchoices * $nquestions, 0);
    $topicid = 0;
    $classid = $row['classid'];

    $r = mysql_fetch_assoc(db_exec("
      SELECT c.dump, g.topicid
      FROM cheesyclasses AS c
      LEFT JOIN groupings AS g ON g.linkid = $classid AND g.linktype = 'classes'
      WHERE c.questionperiodid = $qp AND c.classid IN ($classid, 0)
      ORDER BY c.classid DESC LIMIT 1
    ", $db, __FILE__, __LINE__));
    $survey = unserialize($r['dump']);
    $topicid = (int)$r['topicid'];
    
    print("<hr>Survey for class $classid topic $topicid<br>\n");
    foreach(array_keys($survey->components) as $key)
    {
      $item = &$survey->components[$key];
      $type = get_class($item);
      if ($type == "choice")
      {
        $item->revision_id = false;
        $item->question_ids = array_pad(array(),count($item->questions),false);
  
        foreach($item->questions as $qkey => $qtext)
        {
          $prefix = "_{$key}_q{$qkey}";
          foreach($questions as $questionidx => $question)
          {
            if ($oldbase["MC$question"] == $qtext)
            {
              print("mapped to $prefix to $questionidx<br>\n");
              $map[$prefix] = $questionidx; 
              break;
            }
          }
        }
      }
    }
  }
  
  ++$count;
  $response = unserialize($row['dump']);
  
  // find the row prefix
  $prefix = "";
  foreach($response as $k => $v)
  {
    if (substr($k,0,8) == "student_")
    {
      $prefix = "student"; // old
      break;
    }
    else if (substr($k,0,7) == "survey_")
    {
      $prefix = "survey"; // new
      break;
    }
  }

  foreach($map as $qname => $question)
  {
    if (isset($response["$prefix$qname"]))
    {
      $v = $response["$prefix$qname"];
      if (is_numeric($v) && isset($choices[$v]))
      {
        $choice = $choices[$v];
        print("$prefix$qname ($question) = '$choice ($v)'<br>\n");
        ++$dist[$nchoices * $question + $v];
      }
      else
        print("$prefix$qname = <i>empty</i><br>\n");
    }
  }
    
  $lastclassid = $row['classid'];
  $row = mysql_fetch_assoc($result);
  if (!$row || $lastclassid != $row['classid'])
  {
    $acols = $avals = "";
    foreach($questions as $qidx => $q)
    foreach($choices as $cidx => $c)
    {
      $acols .= ", MC$q$c";  
      $avals .= ", " . $dist[$nchoices * $qidx + $cidx];
    }
    
    db_exec("
      INSERT INTO answersets (questionsetid, questionperiodid, classid, topicid, responses $acols)
      VALUES ($qs, $qp, $classid, $topicid, $count $avals)
    ", $db, __FILE__, __LINE__);   
    
    if (!$row) break;
  }
}

print("<p>Done.</p>");


?>
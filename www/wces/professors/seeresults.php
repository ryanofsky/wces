<?

//  SELECT cl.classid, CONCAT(s.code, c.code) AS code, cl.section, c.name, p.name, u.cunix
//  FROM cheesyclasses AS cc
//  INNER JOIN classes AS cl USING (classid)
//  INNER JOIN courses AS c USING (courseid)
//  INNER JOIN subjects AS s USING (subjectid)
//  LEFT JOIN professors AS p ON p.professorid = cl.professorid
//  LEFT JOIN users AS u USING (userid);
//
// CREATE TEMPORARY TABLE pu AS
// SELECT cl.classid, CONCAT(s.code, c.code) AS code, cl.section, cl.students c.name, p.name, u.cunix, u.lastlogin, IF(u.lastlogin > '2001-11-15', 1, 0) AS tl, IF(cc.classid IS NOT NULL, 1, 0) AS tl
// FROM groupings AS g
// INNER JOIN classes AS cl ON cl.classid = g.linkid AND g.linktype = 'classes'
// INNER JOIN courses AS c USING (courseid)
// INNER JOIN subjects AS s USING (subjectid)
// LEFT JOIN cheesyclasses AS cc WHERE cc.classid = cl.classid AND cc.questionperiodid = $questionperiodid
// LEFT JOIN professors AS p ON p.professorid = cl.professorid
// LEFT JOIN users AS u USING (userid)
// ORDER BY tl DESC, code, cl.section

//  SELECT CONCAT(s.code, c.code) AS code, cl.section AS sec, c.name, p.name, IFNULL(u.cunix,'') AS cunix, COUNT(*) AS res, cl.students AS stu
//  FROM cheesyresponses AS cr
//  INNER JOIN classes AS cl USING (classid)
//  INNER JOIN courses AS c USING (courseid)
//  INNER JOIN subjects AS s USING (subjectid)
//  LEFT JOIN professors AS p ON p.professorid = cl.professorid
//  LEFT JOIN users AS u USING (userid)
//  GROUP BY cr.classid
//  ORDER BY code, section;

  require_once("wces/page.inc");
  require_once("wces/database.inc");
  require_once("wces/report_page.inc");

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_text.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");
  
  login_protect(login_professor);
  $profid = (int)login_getprofid();

function listclasses()
{
  global $profid,$profname,$db;

  print("<h3>$profname - Survey Responses</h3>\n");

  $questionperiods = db_exec("
    SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
    cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT cr.userid, cr.classid) AS hasanswers
    FROM classes AS cl
    INNER JOIN groupings AS g ON cl.classid = g.linkid AND g.linktype = 'classes'
    INNER JOIN questionperiods AS qp ON cl.year = qp.year AND cl.semester = qp.semester
    INNER JOIN courses AS c ON c.courseid = cl.courseid
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN cheesyresponses AS cr ON (cr.classid = cl.classid AND cr.questionperiodid = qp.questionperiodid)
    WHERE qp.questionperiodid > 7 AND qp.questionperiodid < 9 AND cl.professorid = '$profid'
    GROUP BY qp.questionperiodid, cl.classid
    ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
  ", $db, __FILE__, __LINE__);
  
  $count = $lqp = 0; $first = true;
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
    extract($questionperiod);

    if ($questionperiodid != $lqp)
    {
      if ($first)
        $first = false;
      else
      {
        if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");      
        print("</ul>");
      } 
      $count = 0;
      $lqp = $questionperiodid;
      print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
      print("<ul>\n");
    }

    if ($clname) $name .= " - $clname";
    if ($hasanswers)
    {
      ++$count;
      print("  <li><a href=\"seeresults.php?nquestionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
    }
    else
      print("  <li>$scode$code$section <i>$name</i> (No Responses Available)</li>");
  }
  
  if (!$first)
  {
    print("</ul>");
  } 

  $questionperiods = db_exec("
    SELECT qp.questionperiodid, qp.semester, qp.year, qp.description,
    cl.classid, s.code as scode, c.code, cl.section, c.name, cl.name as clname, COUNT(DISTINCT a.answersetid) AS hasanswers
    FROM questionperiods AS qp
    INNER JOIN classes AS cl ON cl.year = qp.year AND cl.semester = qp.semester
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    LEFT JOIN answersets AS a ON (a.classid = cl.classid AND a.questionperiodid = qp.questionperiodid)
    WHERE qp.questionperiodid <= 7 AND cl.professorid = '$profid'
    GROUP BY qp.questionperiodid, cl.classid
    ORDER BY qp.year DESC, qp.semester DESC, qp.questionperiodid DESC, hasanswers DESC, s.code, c.code, cl.section
  ", $db, __FILE__, __LINE__);
  
  $count = $lqp = 0; $first = true;
  while($questionperiod = mysql_fetch_assoc($questionperiods))
  {
    $questionperiodid = $semester = $year = $description = "";
    $classid = $scode = $code = $section = $name = $clname = $hasanswers = "";
    extract($questionperiod);

    if ($questionperiodid != $lqp)
    {
      if ($first)
        $first = false;
      else
      {
        if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");      
        print("</ul>");
      } 
      $count = 0;
      $lqp = $questionperiodid;
      print("<h4>" . ucfirst($semester) . " $year - $description</h4>\n");
      print("<ul>\n");
    }

    if ($clname) $name .= " - $clname";
    if ($hasanswers)
    {
      ++$count;
      print("  <li><a href=\"seeresults.php?questionperiodid=$questionperiodid&classid=$classid\">$scode$code$section <i>$name</i></a></li>");  
    }
    else
      print("  <li>$scode$code$section <i>$name</i> (No Responses Available)</li>");
  }
  
  if (!$first)
  {
    if ($count > 0) print("  <li><a href=\"?questionperiodid=$lqp\">All Classes Combined</a></li>\n</ul>");      
    print("</ul>");
  } 
  
};  
  
function showresults($db,$questionperiodid,$classid)
{
  global $profid;
  
  print('<h3><a href="seeresults.php">Back</a></h3><hr>');
  
  $sqloptions = array ("standard" => true, "custom" => true);
  $groups = Array("classes" => $classid ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $header = $ratings = $listclasses = $listprofessors = $abet = $responses = $tas = true;
  $sort = array("classes","questionperiods","professors","courses","departments");
  $criteria = array("professors" => array($profid), "classes" => $classid ? array($classid) : false, "topics" => false, "questionperiods" => array($questionperiodid), "departments" => false, "courses" => false);
  
  report_makequeries($db, $sqloptions, $criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet, 
    $responses, $tas);
  
  $displayoptions = array("pies" => true);

  $outhtml = "<br>"; $text = false;

  report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
    $listprofessors, $ratings, $abet, $responses, $tas);
    
  print($outhtml);
}

///////////////////////////////////////////////////////////////////////////////

function happycsv(&$str)
{
  $l = strlen($str);
  $str = str_replace(array("\r", "\n", '"'), array(" ", " ", '""'), $str);
  if (strlen($str) != $l || strpos($str,",") !== false)
    $str = "\"$str\"";
}

function getv(&$str)
{
  if (isset($str)) return $str; else return "";
}

function getrating(&$component, $index)
{
  if (!is_numeric($index)) return "";
  
  $count = (int)count($component->choices);
  if ($component->is_numeric  || $component->first_number || $component->last_number)
  {
    if ($count <= 1)
      return (int)$component->first_number;
    else  
      return (double)$index / (double)($count-1.0) * ($component->last_number - $component->first_number) + $component->first_number;
  }
  else if (isset($component->choices[$index]))
    return $component->choices[$index];
  else if ($component->other_choice && $index == $count)
    return "";
  else
    return $index;
}

function printcsv($survey,$ck,$results)
{
  header("Content-Type: application/octet-stream");
  //header("Content-Type: text/plain");
  $first = true;
  foreach($ck as $k)
  {
    $c = &$survey->components[$k];
    if (get_class($c) == "textresponse")
    {
      if ($first) $first = false; else print(",");
      $str = $c->text; happycsv($str);
      print($str);
    }
    else if (get_class($c) == "choice")
    {
      foreach($c->questions as $str)
      {
        if ($first) $first = false; else print(",");
        happycsv($str);
        print($str);
      }  
    }
  }
  print("\n");
  
  while($row = mysql_fetch_assoc($results))
  {
    $result = unserialize($row["dump"]);
    if (is_array($result))
    {
      $first = true;
      foreach($ck as $k)
      {
        $prefix = "student_${k}_";
        
        $c = &$survey->components[$k];
        if (get_class($c) == "textresponse")
        {
          if ($first) $first = false; else print(",");
          $str = getv($result["${prefix}response"]);
          happycsv($str);
          print($str);
        }
        else if (get_class($c) == "choice")
        {
          foreach($c->questions as $q => $str)
          {
            if ($first) $first = false; else print(",");
            
            //print("${prefix}q${q}\n\n");
            $v = getv($result["${prefix}q${q}"]);
            if (is_array($v))
            {
              $str = "";
              foreach($v as $index)
              {
                $rat = getrating($c, $index);
                if ($rat)
                {
                  if ($str) $str .= "|";
                  $str .= $rat;
                }
              }  
            }
            else
              $str = getrating($c, $v);
            
            $other = getv($result["${prefix}q${q}_" . count($c->choices)]);
            if ($other)
            {
                if ($str) $str .= "|";
                $str .= $other;
            }
            
            happycsv($str);
            print($str);
          }  
        }
      }
    }
    else
    {  
      print("bad result set");
    }
    print("\n"); 
  }  
}

function calc_mode($distribution)
{
  $max = $maxv = false;
  foreach($distribution as $score => $people)
  if ($max === false || $people >= $maxv) 
  {
    $max = $score;
    $maxv = $people;
  }
  return $max;
}

function calc_avg($distribution)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $score * $people;
  }
  return $sum == 0 ? 0 : $wsum / $sum;
}

function calc_sd($distribution, $avg)
{
  $sum = $wsum = 0;
  foreach($distribution as $score => $people)
  {
    $sum += $people;
    $wsum += $people * pow(($score - $avg),2);
  }
  return $sum == 0 ? 0 : sqrt($wsum/$sum);
}

function nshowresults($db,$questionperiodid,$classid,$showcsv)
{
  global $profid, $wces_path;
  
  $surveyrow = db_exec("
    SELECT cc.dump
    FROM cheesyclasses AS cc
    WHERE cc.classid IN ($classid,0) AND cc.questionperiodid = $questionperiodid
    ORDER BY cc.classid DESC LIMIT 1
  ", $db, __FILE__, __LINE__);
  if (mysql_num_rows($surveyrow) != 1) die("Survey '$surveyid' not found");
  
  $surveyarr = mysql_fetch_assoc($surveyrow);
  $date = $surveyarr["date"];
  $survey = unserialize($surveyarr["dump"]);
  
  if (get_class($survey) != "survey") die("Invalid Survey Data");
  
  $ck = array_keys($survey->components);

  $results = db_exec("
    SELECT cr.dump
    FROM cheesyresponses AS cr
    INNER JOIN classes AS cl USING (classid)
    WHERE cr.classid = $classid AND cr.questionperiodid = $questionperiodid AND cl.professorid = $profid
    ORDER BY RAND()
  ", $db, __FILE__, __LINE__);

  if ($showcsv)
    return printcsv($survey,$ck,$results);

  print('<h3><a href="seeresults.php">Back</a></h3><hr>');
  
  $qpi = db_exec("SELECT year, semester, description FROM questionperiods WHERE questionperiodid = $questionperiodid", $db, __FILE__, __LINE__);  
  extract(mysql_fetch_assoc($qpi));

  $cli = db_exec("
    SELECT CONCAT(s.code, c.code, ' ', c.name, ' Section ', cl.section) cname, p.name AS pname, cl.students, COUNT(*) AS responses
    FROM classes AS cl 
    INNER JOIN courses AS c USING (courseid)
    INNER JOIN subjects AS s USING (subjectid)
    INNER JOIN professors AS p ON p.professorid = cl.professorid
    INNER JOIN cheesyresponses AS cr ON cr.classid = cl.classid AND cr.questionperiodid = $questionperiodid
    WHERE cl.classid = $classid
    GROUP BY cl.classid
  ", $db, __FILE__, __LINE__);
  
  extract(mysql_fetch_assoc($cli));  
  
?><br><h4>On this page</h4>
 <table border=0>
 <tr><td>Question Period:</td><td><b><?=ucwords($semester)?> <?=$year?> <?=$description?></b></td></tr>
 <tr><td>Department:</td><td><b>Computer Science (COMS)</b></td></tr>
 <tr><td>Professor:</td><td><b><a href="<?=$wces_path?>administrators/info.php?professorid=<?=$profid?>"><?=$pname?></a></b></td></tr>
 <tr><td>Class:</td><td><b><a href="<?=$wces_path?>administrators/info.php?classid=<?=$classid?>"><?=$cname?></a></b></td></tr>
 </table>
 <h4>Response Statistics</h4><table border=0>
 <tr><td>Total Students:</td><td><b><?=$students?></b></td></tr>
 <tr><td>Students Evaluated:</td><td><b><?=$responses?></b></td></tr>
 </table>
 <img src="<?=$wces_path?>media/graphs/susagegraph.php?blank=<?=$students-$responses?>&filled=<?=$responses?>" width=200 height=200><img src="<?=$wces_path?>media/graphs/susagelegend.gif" width=147 height=31>
 <h4>Results Summary</h4>
<?

  $resp = array();
  
  while($row = mysql_fetch_assoc($results))
    $resp[] = unserialize($row['dump']);

    $first = true;
  foreach($ck as $k)
  {
    $c = &$survey->components[$k];
    $prefix = "student_${k}_";
    
    if (get_class($c) == "textresponse")
    {
      print("<h5>$c->text</h5>\n");
      print("<ul>");
      reset($resp);
      $first = true;
      while(list($rk) = each($resp))
      {
        $v = $resp[$rk]["{$prefix}response"];
        if ($v)
        {
          if ($first) {$first = false; print("  <li>"); } else print("</li>\n  <li>");
          print(nl2br(htmlspecialchars($v)));
        }  
      }
      if (!$first) print("</li>\n"); else print("<blockquote><i>None</i></blockquote>");
      print("</ul>");
    }
    else if (get_class($c) == "choice")
    {
      if ($c->is_numeric)
      {
        print("($c->first_number = $c->first_label, $c->last_number = $c->last_label)<br>");
      }
      print("<table border=1 cellspacing=0 cellpadding=2>\n");
      print("<thead style=\"page-break-inside: avoid\">\n");
      print("<tr>\n");
      print("<td>&nbsp;</td>\n");
      $values = array();
      if ($c->is_numeric)
      {
        $showstats = true;
        $d = $first_number < $last_number ? 1 : -1;
        $r = abs($list_number - $first_number);
        for($i=0; $i <= $r; ++$i)
        {
          $values[$i] = $first_number + $d * $i;
          print($values[$i]);
        }  
      }
      else
      {
        $showstats = is_numeric($c->first_number) && is_numeric($c->last_number) ? true : false;
        foreach($c->choices as $ci => $ct)
        {       
          $str = $ct;
          if ($showstats)
          {
            $values[$ci] = $c->first_number + ($ci / (count($c->choices)-1)) * ($c->last_number - $c->first_number);
            $str .= sprintf(" (%.1f)",$values[$ci]);
          }
          else 
            $values[$ci] == false;
              
          print("  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n");
        }
        if ($c->other_choice)
        {
          $str = $c->other_choice;
          $values[] = false;
          print("  <td><div style=\"writing-mode:tb-rl; white-space: nowrap\"><b>$str</b></div></td>\n");
        }
        if ($showstats)
        {
          print("  <td><b>Avg</b></td>\n  <td><b>Mode</b></td>\n  <td><b>SD</b></td>\n");  
        }
      }  
      print("</tr>\n");
 
 
      foreach($c->questions as $q => $str)
      {
        print("<tr>\n");
        print("  <td>$str</td>\n");
 
        // array_fill function apparently missing on oracle (?)
        $sums = array_pad(array(),count($c->choices),0);
 
        reset($resp);
        while(list($rk) = each($resp))
        {
          $result = &$resp[$rk];
          $v = getv($result["${prefix}q${q}"]);
          
          if (is_array($v))
          {
            foreach($v as $vi)
              $sums[$vi] += 1;
          }
          else    
            $sums[$v] += 1;
        }
 
        $dist = false;
        foreach($values as $vi => $vk)
        {
          if ($showstats) $dist[$vk] = $sums[$vi];
          print("  <td>$sums[$vi]</td>\n");
        }
        
        if ($showstats)
        {
          $a = report_avg($dist);
          printf("  <td>%.1f</td>\n  <td>%.1f</td>\n  <td>%.1f</td>\n",$a,report_mode($dist),report_sd($dist,$a));
        }
        

        print("</tr>\n");  
      }  
      print("</table>\n");
    }
  }
  print("\n");

  $url = "seeresults.php/results.csv?nquestionperiodid=$questionperiodid&classid=$classid";
 
?>
 <h4>Results Download</h4>
 <p>Download the raw survey responses as a spreadsheet.</p>
 <blockquote><a href="<?=$url?>"><img src="<?=$wces_path?>media/report/download.gif" width=16 height=16 border=0 alt="results.csv" align=absmiddle></a> <a href="<?=$url?>">results.csv</a></blockquote>
<?



}



///////////////////////////////////////////////////////////////////////////////

$db = wces_connect();
$profname = db_getvalue($db,"professors",Array("professorid" => $profid),"name");

param($nquestionperiodid);
param($questionperiodid);
param($classid);

$nquestionperiodid = (int) $nquestionperiodid;
$questionperiodid = (int) $questionperiodid;
$classid = (int) $classid;

$showcsv = $server_url->xpath ? true : false;

if (!$showcsv)
  page_top("Survey Results");



if ($questionperiodid)
  showresults($db,$questionperiodid,$classid);
else if ($nquestionperiodid && $classid)
  nshowresults($db,$nquestionperiodid,$classid,$showcsv);
else
  listclasses();

if (!$showcsv)
  page_bottom();

?>









<?

// Functions to display fall 2001 results, which were stored in an intermediate format

function cheesy_showresults($db, $questionperiodid, $classid, $profid, $showcsv)
{
  global $wces_path;
  
  $result = db_exec("
    SELECT * FROM classes WHERE classid = $classid AND professorid = $profid
  ", $db, __FILE__, __LINE__);

  if (mysql_num_rows($result) != 1)
  {
    print("Class not found");
    return;    
  }
  
  $surveyrow = db_exec("
    SELECT cc.dump
    FROM cheesyclasses AS cc
    WHERE cc.classid IN ($classid,0) AND cc.questionperiodid = $questionperiodid 
    ORDER BY cc.classid DESC LIMIT 1
  ", $db, __FILE__, __LINE__);

  if (mysql_num_rows($surveyrow) != 1)
  { 
    print("Survey not found");
    return;
  }
  
  $surveyarr = mysql_fetch_assoc($surveyrow);
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
 <tr><td>Professor:</td><td><b><a href="<?=$wces_path?>administrators/info.php?professorid=<?=$professor_id?>"><?=$pname?></a></b></td></tr>
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
    print("<p>");

    $c = &$survey->components[$k];
    $prefix = "survey_${k}_";
    $prefix2 = "student_${k}_";

    if (get_class($c) == "textresponse")
    {
      print("<h5>" . make_html($c->is_html, $c->text) . "</h5>\n");
      print("<ul>");
      reset($resp);
      $first = true;
      while(list($rk) = each($resp))
      {
        $v = isset($resp[$rk]["{$prefix}response"]) ? $resp[$rk]["{$prefix}response"] : $resp[$rk]["{$prefix2}response"];
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
        print("($c->first_number = " . make_html($c->is_html, $c->choices[0]) . ", $c->last_number = " . make_html($c->is_html, $c->choices[1]) . ")<br>");
      }
      print("<table border=1 cellspacing=0 cellpadding=2>\n");
      print("<thead style=\"page-break-inside: avoid\">\n");
      print("<tr>\n");
      print("<td>&nbsp;</td>\n");
      $values = array();
      if ($c->is_numeric)
      {
        $showstats = true;
        $d = $c->first_number < $c->last_number ? 1 : -1;
        $r = abs($c->last_number - $c->first_number);
        for($i=0; $i <= $r; ++$i)
        {
          $values[$i] = $c->first_number + $d * $i;
          printf("<td>%.1f</td>", $values[$i]);
        }
      }
      else
      {
        $showstats = is_numeric($c->first_number) && is_numeric($c->last_number) ? true : false;
        foreach($c->choices as $ci => $ct)
        {
          $str = make_html($c->is_html, $ct);
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
          $str = make_html($c->is_html, $c->other_choice);
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
        print("  <td>" . make_html($c->is_html, $str) . "</td>\n");

        $sums = array_pad(array(), count($c->choices), 0);

        reset($resp);
        while(list($rk) = each($resp))
        {
          $result = &$resp[$rk];
          $v = getr($result, "${prefix}q${q}", "${prefix2}q${q}");
          if (is_array($v))
          {
            foreach($v as $vi)
              $sums[$vi] += 1;
          }
          else if (isset($v))
            @$sums[$v] += 1;
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
    print("</p>");
  }
  print("\n");

  /////////////////////////////////////////////////////////////////////////////
  // TA SECTION

  $sqloptions = array ("standard" => false, "custom" => false);
  $groups = Array("classes" => $classid ? true : false, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $ratings = $listclasses = $listprofessors = $abet = $responses = false;
  $tas = $header = true;

  $sort = array("classes","questionperiods","professors","courses","departments");
  $criteria = array("professors" => array($profid), "classes" => $classid ? array($classid) : false, "topics" => false, "questionperiods" => array($questionperiodid), "departments" => false, "courses" => false);

  legacy_report_makequeries($db, $sqloptions, $criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet,
    $responses, $tas);

  $displayoptions = array("pies" => false);

  $outhtml = "<br>"; $text = false;

  legacy_report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
    $listprofessors, $ratings, $abet, $responses, $tas, true);

  print($outhtml);

  /////////////////////////////////////////////////////////////////////////////

  $url = "seeresults.php/results.csv?nquestionperiodid=$questionperiodid&classid=$classid";
}

function make_html($is_html, $text)
{
  return $is_html ? $text : htmlspecialchars($text);
}

function getr(&$arr, $index1, $index2)
{
  if (isset($arr[$index1]))
    return $arr[$index1];
  if (isset($arr[$index2]))
    return $arr[$index2];
  return NULL;
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
        $prefix = "survey_${k}_";
        $prefix2 = "student_${k}_";

        $c = &$survey->components[$k];
        if (get_class($c) == "textresponse")
        {
          if ($first) $first = false; else print(",");
          $str = getr($result,"${prefix}response","${prefix2}response");
          happycsv($str);
          print($str);
        }
        else if (get_class($c) == "choice")
        {
          foreach($c->questions as $q => $str)
          {
            if ($first) $first = false; else print(",");
            $v = getr($result,"${prefix}q${q}","${prefix2}q${q}");
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

            $other = getr($result,"${prefix}q${q}_" . count($c->choices),"${prefix2}q${q}_" . count($c->choices));

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

function happycsv(&$str)
{
  $l = strlen($str);
  $str = str_replace(array("\r", "\n", '"'), array(" ", " ", '""'), $str);
  if (strlen($str) != $l || strpos($str,",") !== false)
    $str = "\"$str\"";
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

?>
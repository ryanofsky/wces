<?
require_once("gk12.inc");
require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");
require_once("wbes/component_textresponse.inc");
require_once("wbes/component_heading.inc");
require_once("wbes/component_text.inc");
require_once("wbes/survey.inc");
require_once("wces/database.inc");
require_once("widgets/basic.inc");

$db = server_mysqlinit();
mysql_select_db("wces",$db);

function ohno($text)
{
  page_top("Error");
  print($text);
  page_bottom();
  exit();
}

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


if (isset($surveyid))
{
  //header("Content-Type: application/octet-stream");
  header("Content-Type: text/plain");
  
  $surveyrow = db_exec("SELECT date, dump FROM cheesy WHERE cheesyid = '" . addslashes($surveyid) . "'", $db, __FILE__, __LINE__);
  if (mysql_num_rows($surveyrow) != 1) ohno("Survey '$surveyid' not found");
  
  $surveyarr = mysql_fetch_assoc($surveyrow);
  $date = $surveyarr["date"];
  $survey = unserialize($surveyarr["dump"]);
  
  if (get_class($survey) != "survey") ohno("Invalid Survey Data");
  
  $nextrow = db_exec("SELECT date FROM cheesy WHERE date > $date AND left(dump,1) != 'a' ORDER BY date LIMIT 1", $db, __FILE__, __LINE__);
  if (mysql_num_rows($nextrow) == 1)
    $bdate = mysql_result($nextrow,0);
  else
    $bdate = "20100101000000";

  $ck = array_keys($survey->components);
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


  $results = db_exec("SELECT cheesyid, dump FROM cheesy WHERE $date <= date AND date < $bdate AND left(dump,1) = 'a' ORDER BY date", $db, __FILE__, __LINE__);
  while($row = mysql_fetch_assoc($results))
  {
    //print( $row["cheesyid"] . "\n\n");
    $result = unserialize($row["dump"]);
    if (is_array($result))
    {
      $first = true;
      foreach($ck as $k)
      {
        $prefix = "gk12_${k}_";
        
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
else
{
  page_top("GK12 Main");

  print("<p>Click a link below to download a CSV file containing the results of a survey.</p>");

  $surveys = db_exec("select cheesyid, date_format(date, '%c-%e-%Y %l:%i %p') as pdate, date from cheesy where left(dump,1) != 'a' order by date", $db, __FILE__, __LINE__);
  $ldate = 0;
  $survey = true;
  $counts = false;

  print("<ul>\n");
  for(;;)
  {
    $survey = mysql_fetch_assoc($surveys);

    $date = $survey ? $survey["date"] : false;

    if ($ldate)
    {
      if ($date)
        $counts = db_exec("select count(*) AS results from cheesy where $ldate <= date and date < $date and left(dump,1) = 'a'", $db, __FILE__, __LINE__);  
      else
        $counts = db_exec("select count(*) AS results from cheesy where $ldate <= date and left(dump,1) = 'a'", $db, __FILE__, __LINE__);  
      
      $count = mysql_fetch_assoc($counts);
      $results = $count["results"];
      print("($results results)</li>\n");
    }  
  
    if (!$survey) break;
    
    $surveyid = $survey["cheesyid"];
    $pdate = $survey["pdate"];
    print("  <li>Survey saved at <a href=\"results.php/results.csv?surveyid=$surveyid\">$pdate</a> ");
    $ldate = $date;
  }
  print("</ul>\n");
  page_bottom();
}

?>
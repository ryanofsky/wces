<?
require_once("page.inc");
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
  
  $surveyrow = db_exec("SELECT date, dump FROM poorsurveys WHERE surveyid = '" . addslashes($surveyid) . "'", $db, __FILE__, __LINE__);
  if (mysql_num_rows($surveyrow) != 1) ohno("Survey '$surveyid' not found");
  
  $surveyarr = mysql_fetch_assoc($surveyrow);
  $date = $surveyarr["date"];
  $survey = unserialize($surveyarr["dump"]);
  
  if (get_class($survey) != "survey") ohno("Invalid Survey Data");
  
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


  $results = db_exec("SELECT responseid AS cheesyid, dump FROM poorresponses WHERE surveyid = $surveyid ORDER BY date", $db, __FILE__, __LINE__);
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

  $surveys = db_exec("
    select s.surveyid, date_format(s.date, '%c-%e-%Y %l:%i %p') as pdate, count(*) as results
    from poorsurveys AS s
    inner join poorresponses AS r USING (surveyid)
    group by s.surveyid
    order by s.date desc", $db, __FILE__, __LINE__);
  $ldate = 0;
  $survey = true;
  $counts = false;

  print("<ul>\n");
  if (mysql_num_rows($surveys) == 0)
    print("<li><i>No results found</i></li>");
  else
  while($row = mysql_fetch_assoc($surveys))
  {
    print("<li>Survey saved at <a href=\"results.php/results.csv?surveyid=$row[surveyid]\">$row[pdate]</a> ($row[results] results)</li>\n");
  }
  print("</ul>\n");
  page_bottom();
}

?>
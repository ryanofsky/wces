<?

/*

CREATE TABLE presps
(
  user_id INTEGER NOT NULL REFERENCES users,
  course_id INTEGER NOT NULL REFERENCES courses,
  data TEXT
);

*/

require_once("wces/SimpleResults.inc");
require_once("widgets/basic.inc");
require_once("wces/report_help.inc");
require_once("jpgraph/jpgraph.php");

$graph_media = $server_media;
//$graph_media = "http://oracle.seas.columbia.edu/wces/media";

function putJava1()
{
?>
<script language="JavaScript">
<!--
var happy = document.images && window.Image;
var GraphSources = new Array();
var GraphImages = new Array();

function cacheImage(number, src)
{
  if(!happy) return;
  GraphSources[number] = src;
}

function showImage(name, number)
{
  if (!happy) return;
  var i = document.images[name];
  i.src = GraphSources[number];
}
// -->
</script>
<? 
}

function putJava2()
{
?>
<script>
<!--
if (happy)
for(i in GraphSources)
{
  var img = new Image();
  img.src = GraphSources[i];
  GraphImages[i] = img;
};
// -->
</script>
<?
}

function GetParams(&$results, &$classes)
{
  global $graph_media;

  $params = array();
  LineGraphSize($params, $classes);
  
  $params2 = array();
  LineGraphSize($params2, $results->base_choices);

  if ($params2['width'] > $params['width']) $params['width'] = $params2['width'];
  if ($params2['height'] > $params['height']) $params['height'] = $params2['height'];

  return $params;
}

function GetLineGraph($params, $datax, $datay, $title, $number, $initSrc)
{
  global $graph_media;
  $y1 = $params['graphMargin'];
  $y2 = $params['graphMargin'] + $params['graphHeight'];

  $n = count($datax);
  $halfwidth = 0.5 * (float)$params['graphWidth'] / (float)($n-1);
  $right = (float)($params['width'] - $params['graphMargin']);
  
  $start = $number * $n;
  
  $str = "<map name=map$number>\n";
  for($i = 0; $i < $n; ++$i)
  {
    $x = (float)$right - $params['graphWidth']*($n-$i-1)/($n-1);
    $x1 = round($x - $halfwidth);
    $x2 = round($x + $halfwidth);
    $gn = $start + $i;
    $str .= "<area shape=rect coords=\"$x1,$y1,$x2,$y2\" href=\"javascript:\" onmouseover=\"showImage('graph$number', $gn)\">\n";
  }
  print("</map>");
 
  return "$str<img src=\"$graph_media/graphs/rline.php"
    . "?datax=" . urlencode(serialize($datax)) 
    . "&datay=" . urlencode(serialize($datay))
    . "&title=" . urlencode($title)
    . "&width=$params[width]&height=$params[height]"
    . "\""
    . " width=$params[width] height=$params[height]"
    . " usemap=\"#map$number\" border=0>"
    . "\n<img src=\"$initSrc\""
    . " width=$params[width] height=$params[height]"
    . " name=graph$number border=0>";
};

function MakeBarGraph(&$params, $datax, $datay, $title)
{
  global $graph_media;

  return "$graph_media/graphs/rbars.php"
    . "?datax=" . urlencode(serialize($datax)) 
    . "&datay=" . urlencode(serialize($datay))
    . "&title=" . urlencode($title)
    . "&width=$params[width]&height=$params[height]";
}

function saveResults($user_id, $course_id)
{
  global $wces, $HTTP_POST_VARS;
    
  wces_connect();
  
  $where = "WHERE user_id = $user_id AND course_id = $course_id";
  
  $data = addslashes(serialize($HTTP_POST_VARS));
  
  $r = pg_go("SELECT EXISTS (SELECT * FROM presps $where)", $wces, __FILE__, __LINE__);
  if (pg_result($r,0,0) == 't')
  {
    $sql = "UPDATE presps SET data = '$data' $where";
  }
  else
  {
    $sql = "INSERT INTO presps (user_id, course_id, data) VALUES ($user_id, $course_id, '$data')";
  }
  
  pg_go($sql, $wces, __FILE__, __LINE__);
  
  print("<font color=blue>Changes saved.</font>");
  
};

function showResults($user_id, $course_id, $collect)
{
  global $wces, $select_classes;
  wces_connect();
  
  $r = pg_go("SELECT data FROM presps WHERE user_id = $user_id AND course_id = $course_id", $wces, __FILE__, __LINE__);
  $data = pg_numrows($r) == 1 ? unserialize(pg_result($r, 0, 0)) : array();
  
  print("<table><tr><td>");
  
  print("<form method=post>\n");
  
  putJava1();
  
  $r = pg_go("SELECT get_course($course_id)", $wces, __FILE__, __LINE__);
  $course = pg_result($r, 0, 0);
  print("<h2>" . format_course($course) . "</h2>\n");

  $r = pg_go("
    SELECT cl.class_id, get_class(cl.class_id) AS cl, get_profs(cl.class_id) AS profs
    FROM classes AS cl
    INNER JOIN ($select_classes) AS l USING (class_id)
    WHERE cl.course_id = $course_id
    ORDER BY cl.year DESC, cl.semester DESC
  ", $wces, __FILE__, __LINE__);

  $profinfo = array();
  $classinfo = array();

  $n = pg_numrows($r);
  for($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_array($r, $i, PGSQL_ASSOC));
    $classes[] = (int)$class_id;
    $classinfo[$class_id] = explode_class($cl);
    $profinfo[$class_id] = format_profs($profs, true, "", "", ",");
  };

  print("<p>Past Professors:</p>\n<ul>\n");
  foreach($classes as $cl)
    print("  <li>" . format_class($classinfo[$cl], "%t Section %s") . ": " . $profinfo[$cl] .  "</li>\n");
  print("</ul>");
  
  $s = new SimpleResults($classes, true);
  $linex = array();
  foreach(array_reverse($classes) as $class_id)
    $linex[] = format_class($classinfo[$class_id], "%t");

  $params = GetParams($s, $linex);
  
  $gn = 0;
  foreach($s->base_questions as $key => $value)
  {
    print("<hr>\n");
    print("<h3>$value</h3>\n");
    $lc = count($s->base_choices) - 1;
    printf("<p>%s = %.1f, %s = %.1f</p>\n", $s->base_choices[0], $s->base_values[0], $s->base_choices[$lc], $s->base_values[$lc]);

    print("<script>\n<!--\n");
    $datax = $s->base_choices;
    foreach(array_reverse($classes) as $class_id)
    {
       $title = format_class($classinfo[$class_id], "%t Section %s");
       $datay = $s->distributions[$class_id][$key];
       $grs = addslashes(MakeBarGraph($params, $datax, $datay, $title));
       print("cacheImage($gn, '$grs');\n");
       ++$gn;
    }
    print("// -->\n</script>\n");
    
    $liney = array();
    
    foreach(array_reverse($classes) as $class_id)
    {
       $dist = &$s->distributions[$class_id][$key];
       $vdist = array();
       foreach($s->base_values as $k => $v)
         $vdist[$v] = $dist[$k];
       $liney[] = report_avg($vdist);
    }     

    print(GetLineGraph($params, $linex, $liney, "Past Averages", $key, $grs));
    print("<br>");
    
    $text = isset($data["respond$key"]) ? $data["respond$key"] : "";
    if ($collect)
    {
      print("<div align=center><textarea rows=4 cols=60 name=respond$key>$text</textarea></div>");
    }
    else
    {
      if (!$text) $text = "<i>none</i>";
      print("<p><b>Comments from professor:</b> $text</p>");
    };
  }

  putJava2();
  
  if ($collect)
    print("<hr><p align=center><input type=submit name=save value=\"Save Changes\"></p>\n");
  
  print("</td></tr></table>");

  print("</form>\n");

};

?>
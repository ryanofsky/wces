<?

/*

CREATE TABLE presps
(
  user_id INTEGER NOT NULL REFERENCES users,
  course_id INTEGER NOT NULL REFERENCES courses,
  data TEXT
);

//TODO: use widget name functions to give images and javascript variables unique names

*/

require_once("wces/SimpleResults.inc");
require_once("widgets/basic.inc");
require_once("wces/report_generate.inc");
require_once("wces/report_page.inc");
require_once("wces/report_help.inc");
require_once("jpgraph/jpgraph.php");

$graph_media = $server_media;
//$graph_media = "http://oracle.seas.columbia.edu/wces/media";

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

define('ProfResponse_save', 1);
define('ProfResponse_cancel', 2);

class ProfResponse extends ParentWidget
{
  var $course_id;
  var $user_id;
  var $readOnly;
  var $done = false;
  var $event;
  var $data = null;
  
  // array of class_id's for this course sorted by semester
  var $classes;
  
  // SimpleResults object for current course
  var $simpleResults = null;
  
  // class_id's of this course's classes which were taught by other professors
  var $otherClasses;
  
  // Information about classes
  var $classInfo;
  
  // array of for responses indexed by SimpleResults question key 
  var $responses = array();
  var $responseBoxes = array();  
  
  function ProfResponse($course_id, $user_id, $readOnly, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);    
    $this->course_id = (int)$course_id;
    $this->user_id = (int)$user_id;
    $this->readOnly = (bool)$readOnly;
    $this->event =& new EventWidget('event', $this);
    $this->userOnly =& new CheckBox('userOnly', $this);
  }
    
  function saveResponses()
  {
    global $wces;
    
    $data = array();
    foreach (array_keys($this->simpleResults->base_questions) as $key)
      $data["respond$key"] = $this->responseBoxes[$key]->text;
    $data = addslashes(serialize($data));
    
    $where = "WHERE user_id = $this->user_id AND course_id = $this->course_id";
    
    $r = pg_go("SELECT EXISTS (SELECT * FROM presps $where)", $wces, __FILE__, __LINE__);
    if (pg_result($r,0,0) == 't')
      $sql = "UPDATE presps SET data = '$data' $where";
    else
      $sql = "INSERT INTO presps (user_id, course_id, data) VALUES ($this->user_id, $this->course_id, '$data')";
    
    return (bool)pg_go($sql, $wces, __FILE__, __LINE__);
  }
 
  function getResults($isNew)
  {
    global $wces, $select_classes;
    wces_connect();
    
    if (isset($this->simpleResults)) return;
    
    $r = pg_go("
      SELECT cl.class_id, get_class(cl.class_id) AS cl, 
        EXISTS (SELECT * FROM enrollments AS e WHERE e.user_id = $this->user_id 
          AND class_id = l.class_id AND status = 3) as prof
      FROM classes AS cl
      INNER JOIN ($select_classes) AS l USING (class_id)
      WHERE cl.course_id = $this->course_id
      ORDER BY cl.year DESC, cl.semester DESC
    ", $wces, __FILE__, __LINE__);
    
    // Gather information into arrays
    
    $this->classInfo = array();
    $this->otherClasses = array();
    
    $n = pg_numrows($r);
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($r, $i, PGSQL_ASSOC));
      if ($prof == 't')
        $add = true;
      else
      {
        $this->otherClasses[] = (int)$class_id;
        $add = !$this->userOnly->checked;
      }      
      
      if ($add)
      {
        $classes[] = (int)$class_id;
        $this->classInfo[$class_id] = explode_class($cl);
      }
    };
    $this->simpleResults =& new SimpleResults($classes, true);        
    $this->classes = $classes;
    
    if ($this->readOnly || $isNew)
    {
      $r = pg_go("
        SELECT data FROM presps 
        WHERE user_id = $this->user_id AND course_id = $this->course_id
      ", $wces, __FILE__, __LINE__);
      
      $data = pg_numrows($r) == 1 ? unserialize(pg_result($r, 0, 0)) : array();
    }
     
    foreach (array_keys($this->simpleResults->base_questions) as $key)
    {
      $name = "respond$key";
      if ($this->readOnly)
        $this->responses[$key] = $data[$name];
      else
      {
        $c =& new TextBox(4, 60, '', $name, $this);
        $this->loadChild($c, $isNew);
        if ($isNew) $c->text = isset($data[$name]) ? $data[$name] : '';
        $this->responseBoxes[$key] =& $c;
      }
    }          
  }
  
  function loadInitialState()
  {
    ParentWidget::loadInitialState();
    $this->getResults(true);
  }
  
  function loadState()
  {
    ParentWidget::loadState();
    $event = $this->event->event;
    if ($event == ProfResponse_save)
    {
      $this->done = true;
      $this->getResults(false);
      if ($this->saveResponses())
        $this->message = "<font color=blue>Responses saved.</font>";
      else
        $this->message = "<font color=red>Error saving responses.</font>";
    }
    else if ($event == ProfResponse_cancel)
    {     
      $this->message = "<font color=red>No changes saved.</font>";
      $this->done = true; 
    }    
    
    if (!$this->done) $this->getResults(false);
  }
  
  function printSummary()
  {
    global $wces, $wces_semesters;
    wces_connect();
    
    $sr =& $this->simpleResults;
    
    // figure out which classes to scan
    $result = pg_go("
      SELECT class_id, year, semester FROM classes 
      WHERE class_id in (" . implode(',', $sr->class_ids) .") 
      ORDER BY year DESC, semester DESC
    ", $wces, __FILE__, __LINE__);
    
    $classes = array();
    $year = $semester = null;
    $n = pg_numrows($result);
    for($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_row($result, $i, PGSQL_ASSOC);
      if ($i == 0) 
      { $year = $row['year']; 
        $semester = $row['semester'];
      }
      else if ($year != $row['year'] || $semester != $row['semester'])
        break;
      $classes[] = $row['class_id'];
    }
    
    print("<h4>Summary of results for $wces_semesters[$semester] $year:</h4>\n");
    
    $l = count($sr->base_choices) - 1;
    printf("<p><i>%.1f = %s, %.1f = %s</i></p>\n", $sr->base_values[0],
      $sr->base_choices[0], $sr->base_values[$l], $sr->base_choices[$l]);
    print("<table>\n");
    foreach ($sr->base_questions as $i => $qtext)
    {
      $vdist = array();
      foreach ($sr->base_values as $k => $v)
        $vdist[$v] = 0;
      foreach ($classes as $class_id)
      {
        $dist = &$sr->distributions[$class_id][$i];
        foreach($sr->base_values as $k => $v)
          $vdist[$v] += $dist[$k];
      }
      $avg = report_avg($vdist);
      printf("<tr><td>$qtext</td><td>%.2f</td><td nowrap>", $avg);
      print(report_meter(round($avg * 20)));
      print("</td></tr>\n");
    }
    print("</table>");
  }
  
  function printVisible()
  {
    global $wces, $select_classes;
    
    // get information from database
    wces_connect();
  
    $r = pg_go("SELECT get_course($this->course_id)", $wces, __FILE__, __LINE__);
    $course = pg_result($r, 0, 0);

    $r = pg_go("
      SELECT firstname, lastname FROM users WHERE user_id = $this->user_id
    ", $wces, __FILE__, __LINE__);
    extract(pg_fetch_row($r, 0, PGSQL_ASSOC));
    
    print("<h3>" . format_course($course) . "<br>Professor $firstname $lastname</h3>\n");
    
    // Begin output
    $this->printSummary();
    
    print("<h4>Past Results and Professor Comments</h4>");
    print("<table><tr><td>");
    print("<p><i>FIXME: Add instructions for this section here.</i></p>");
    
    if (count($this->otherClasses) > 0)
    {
      if (!$this->userOnly->checked)
      {
        print("<font size=-1>");
        print("<p><i>Note:</i> The following class sections for <i>"  
          . format_course($course)
          . "</i> were taught by other professors:</p>\n");
        print("<ul>\n");
        foreach($this->otherClasses as $clid)
          print("  <li>" . format_class($this->classInfo[$clid],
            "%t Section %s") . "</li>\n"); 
        print("</ul>\n");
        print("</font>\n");
      }
      print("<div>");
      $this->userOnly->display('onclick="this.form.submit();"');
      print("&nbsp;");
      if ($this->readOnly)
        $this->userOnly->displayLabel("Hide class sections not taught by $firstname $lastname");
      else
        $this->userOnly->displayLabel("Hide class sections not taught by me.");
        
        
      print("&nbsp;");
      $this->event->displayNoJavascriptRefresh("Go");
      print("</div>\n");
    }
    
    $linex = array();
    foreach(array_reverse($this->classes) as $class_id)
      $linex[] = format_class($this->classInfo[$class_id], "%t");

    $s =& $this->simpleResults;
    $params = GetParams($s, $linex);
    $gn = 0;

    print("<table><tr><td>\n");
    foreach($s->base_questions as $key => $value)
    {
      print("<hr>\n");
      print("<h3>$value</h3>\n");
      $lc = count($s->base_choices) - 1;
      printf("<p>%s = %.1f, %s = %.1f</p>\n", $s->base_choices[0], $s->base_values[0], $s->base_choices[$lc], $s->base_values[$lc]);
    
      print("<script>\n<!--\n");
      $datax = $s->base_choices;
      foreach(array_reverse($this->classes) as $class_id)
      {
        $title = format_class($this->classInfo[$class_id], "%t Section %s");
        $datay = $s->distributions[$class_id][$key];
        $grs = addslashes(MakeBarGraph($params, $datax, $datay, $title));
        print("cacheImage($gn, '$grs');\n");
        ++$gn;
      }
      print("// -->\n</script>\n");
      
      $liney = array();
      
      foreach(array_reverse($this->classes) as $class_id)
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
      if (!$this->readOnly)
      {
        print("<div>Comments:</div>\n");
        print("<div align=center>");
        $this->responseBoxes[$key]->display();
        print("</div>");
      }
      else
      {
        $text = $this->responses[$key];
        if (!$text) $text = "<i>none</i>";
        print("<p><b>Comments from professor:</b> $text</p>");
      };
    }
    
    $this->printImageLoad();
    
    if (!$this->readOnly)
    {
      print("<div>");
      $this->event->displayButton("Save", ProfResponse_save);
      $this->event->displayButton("Cancel", ProfResponse_cancel);
      print("</div>\n");    
    }
    
    print("</td></tr></table>");
  }

  function DumpScript()
  {
    static $printed = false;
    if ($printed) return; else $printed = true;
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
  
  function printImageLoad()
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
};
  
?>
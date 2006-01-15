<?

require_once('widgets/widgets.inc');

class UsageOptions extends Widget
{
  var $category_id = null;
  var $question_period_id = null;
  var $sort = null;

  function UsageOptions($sortChoices, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->sortChoices = $sortChoices;
    $this->shortName('category_id');
    $this->shortName('question_period_id');
    $this->shortName('sort');
  }

  function loadState($new)
  {
    if (empty($new)) 
    {
      $this->sort = $this->readValue('sort');
      $this->category_id = $this->readValue('category_id');
      $this->question_period_id = $this->readValue('question_period_id');
    }
    $this->getChoices();
  }

  function getChoices()
  {
    global $wces;
    wces_connect();

    // generate lists of options
    $this->questionPeriodChoices = get_question_periods();

    $this->categoryChoices = pg_select_array("
      SELECT 0 AS id, 'None'
      UNION
      SELECT c.category_id, c.name
      FROM categories AS c
      ORDER BY id
    ", $wces, __FILE__, __LINE__);

    if (!isset($this->sort) || !isset($this->sortChoices[$this->sort])) $this->sort = 0;
    if (!isset($this->category_id) || !isset($this->categoryChoices[$this->category_id])) $category_id = 0;
    if (isset($this->question_period_id) && isset($this->questionPeriodChoices[$this->question_period_id]))
      set_question_period($this->question_period_id);
    else
      $this->question_period_id = get_question_period();
  }

  function display()
  {
    $this->printValue('category_id', $this->category_id, WIDGET_URL);
    if (is_array($this->sortChoices))
      $this->printValue('sort', $this->sort, WIDGET_URL);

    print("<table>\n");

    print("<tr><td valign=top><i>Filtering:</i></td><td>");
    $this->printMenu($this->categoryChoices, 'category_id', $this->category_id);
    print("</td></tr>\n");

    if (is_array($this->sortChoices))
    {
      print("<tr><td valign=top><i>Sort by:</i></td><td>");
      $this->printMenu($this->sortChoices, 'sort', $this->sort);
      print("</td></tr>\n");
    }

    print("<tr><td valign=top><i>Period:</i></td><td>");
    $this->printMenu($this->questionPeriodChoices, 'question_period_id', $this->question_period_id);
    print("</td></tr>\n");

    print("</table>");
  }

  function printMenu($choices, $fieldName, &$current)
  {
    $first = true;
    foreach ($choices as $value => $text)
    {
      if ($first) $first = false; else print(' | ');
      if ($value == $current)
        print("<span style=\"white-space: nowrap\">$text</span>");
      else
      {
        $params = '';
        $url = $this->getUrl(array($fieldName => $value));
        print("<a href=\"$url\" style=\"white-space: nowrap\">$text</a>");
      }
    }
  }
}

class StudentUsage extends ParentWidget
{
  var $options;

  var $sortChoices = array("Course Code", "Number of surveys left", "Number of surveys completed");
  var $sortSql = array("cl", "students - responses DESC", "responses DESC");

  function StudentUsage($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->options =& new UsageOptions($this->sortChoices, 'options', $this);
  }

  function printVisible()
  {
    global $wces, $wces_path, $ASID;

    if (!$this->options->question_period_id)
    {
      print("<p>No question periods found. Click <a href=\"{$wces_path}administrators/questionperiods.php\">here</a> to create one.</p>");
      return;
    }

    $this->options->display();

    $times = array();
    $times["begin"] = microtime();

    $cat = $this->options->category_id ? "AND t.category_id = {$this->options->category_id}" : "";

    pg_go("
      CREATE TEMPORARY TABLE surveycounts AS
      SELECT t.class_id, COUNT(DISTINCT response_id)::INTEGER AS responses
      FROM wces_topics AS t
      INNER JOIN classes AS cl USING (class_id)
      LEFT JOIN responses_survey AS s USING (topic_id)
      WHERE t.question_period_id = {$this->options->question_period_id} AND NOT t.cancelled $cat
      GROUP BY t.class_id
    ", $wces, __FILE__, __LINE__);

    $times["count_responses"] = microtime();

    pg_go("
      CREATE TEMPORARY TABLE surveyclasses AS
      SELECT cc.class_id, COALESCE(cl.students,0) AS students, COALESCE(cc.responses,0) AS responses
      FROM surveycounts AS cc
      INNER JOIN classes AS cl USING (class_id)
    ",$wces,__FILE__, __LINE__);

    $times["count_enrollments"] = microtime();

    $y = pg_go("SELECT SUM(students) as students, SUM(responses) as responses FROM surveyclasses",$wces,__FILE__, __LINE__);

    $times["sum_counts"] = microtime();

    extract(pg_fetch_array($y,0,PGSQL_ASSOC));

?>
<h3>Aggregate Student Usage</h3>
Total number of surveys: <b><?=$students?></b><br>
Number of surveys completed: <b><?=$responses?></b><br>
<img src="<?=$wces_path?>media/graphs/susagegraph.php?blank=<?=$students - $responses?>&filled=<?=$responses?>" width=200 height=200>
<img src="<?=$wces_path?>media/graphs/susagelegend.gif" width=147 height=31><br>
<h3>Individual Class Usage</h3>
<p><font size="-1">Sorted by number of surveys that haven't been filled out</font></p>

<table border=1>
<tr>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/surveys_completed.gif" width=16 height=151 border=0 alt="Surveys Completed"></td>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/surveys_left.gif" width=16 height=94 border=0 alt="Surveys Left"></td>
  <td align=center valign=bottom><img src="<?=$wces_path?>media/report/total_surveys.gif" width=16 height=105 border=0 alt="Total Surveys"></td>
  <td align=center valign=bottom>Class Information</td>
</tr>
<?

    $classes = pg_go("
      SELECT CASE WHEN students < responses THEN responses ELSE students END AS students, responses, get_class(class_id) AS cl, get_profs(class_id) AS p
      FROM surveyclasses
      ORDER BY {$this->sortSql[$this->options->sort]}
    ",$wces, __FILE__, __LINE__);

    $times["get_names"] = microtime();

    $n = pg_numrows($classes);

    for($i=0; $i<$n; ++$i)
    {
      extract(pg_fetch_array($classes,$i,PGSQL_ASSOC));
      print("<tr>\n  <td>$responses</td>\n  <td>" . ($students - $responses)  . "</td>\n  <td>$students</td>\n");
      $classinfo = format_class($cl, "%c %n Section %s", true) . format_profs($p, true, "<br>Professor ");
      print("  <td>$classinfo</td>\n");
      print("</tr>\n");
    }
    print("</table>\n");
    @ob_flush(); flush();

    $r = pg_go("
      SELECT i.user_id, u.uni, CASE WHEN i.responses = 0 THEN 0 WHEN i.classes <= i.responses THEN 2 ELSE 1 END AS level
      FROM
        (SELECT e.user_id, COUNT (DISTINCT t.topic_id) AS classes, COUNT(DISTINCT s.topic_id) AS responses
        FROM wces_topics AS t
        INNER JOIN enrollments AS e ON e.class_id = t.class_id AND e.status & 1 <> 0
        LEFT JOIN responses_survey AS s ON s.user_id = e.user_id AND s.topic_id = t.topic_id
        WHERE t.question_period_id = {$this->options->question_period_id} AND NOT t.cancelled $cat
        GROUP BY e.user_id) AS i
      INNER JOIN users AS u USING (user_id)
      ORDER BY level DESC, random();
    ", $wces, __FILE__, __LINE__);

    $students =& new pg_segmented_wrapper($r, 'level');

    $times["get_individual_students"] = microtime();

    print("<h3>Individual Student Usage</h3>\n");

    $levels = array
    (
      2 => "Students who completed all of their surveys",
      1 => "Students who completed at least one of their surveys",
      0 => "Students who completed none of their surveys"
    );

    while ($students->row)
    {
      extract($students->row);
      if ($students->split)
      {
        print("<h4>$levels[$level]</h4>\n");
        print("<blockquote>\n");
      }

      print("<a href=\"${wces_path}administrators/info.php?user_id=$user_id$ASID\">$uni</a>");

      $students->advance();

      if ($students->split)
        print("</blockquote>");
      else
        print(", ");
    }

    $times["print_individual_students"] = microtime();
    printtimes($times);

  }
}

class ProfessorUsage extends ParentWidget
{
  var $options;

  function ProfessorUsage($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->options =& new UsageOptions(false, 'options', $this);
  }

  function printVisible()
  {
    global $wces, $wces_path;

    if (!$this->options->question_period_id)
    {
      print("<p>No question periods found. Click <a href=\"{$wces_path}administrators/questionperiods.php\">here</a> to create one.</p>");
      return;
    }

    $this->options->display();

    $cat = $this->options->category_id ? " AND t.category_id = {$this->options->category_id}" : '';
    pg_go("
      CREATE TEMPORARY TABLE tcl AS
      SELECT t.class_id, cl.students, u.user_id, u.lastname, u.uni,
        u.firstname, u.lastlogin, get_class(t.class_id) AS class_info,
        CASE WHEN specialization_modified(t.item_id, t.specialization_id)
        THEN 1 ELSE 0 END AS customized,
        CASE WHEN (select timestamp 'now'  - interval '30 days') < u.lastlogin
        THEN 1 ELSE 0 END AS loggedin
      FROM wces_topics AS t
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN enrollments_p AS e ON e.class_id = cl.class_id
      INNER JOIN users AS u USING (user_id)
      WHERE t.question_period_id = {$this->options->question_period_id} AND NOT t.cancelled $cat;
    ",$wces,__FILE__, __LINE__);

    pg_go("
      CREATE TEMPORARY TABLE tst AS
      SELECT class_id, CASE WHEN customized <> 0 THEN 0 WHEN sum(loggedin) > 0 THEN 1
        ELSE 2 END AS status
      FROM tcl
      GROUP BY class_id, customized;
    ",$wces,__FILE__, __LINE__);

    pg_go("
      CREATE TEMPORARY TABLE tpr AS
      SELECT user_id, CASE WHEN SUM(customized) > 0 THEN 0 WHEN loggedin <> 0 THEN 1
        ELSE 2 END AS status
      FROM tcl
      GROUP BY user_id, loggedin;
    ",$wces,__FILE__, __LINE__);

    $profCount = (int)pg_result(pg_go("SELECT COUNT(*) FROM tpr",$wces,__FILE__,__LINE__), 0, 0);
    $profCustomized = (int)pg_result(pg_go("SELECT COUNT(*) FROM tpr WHERE status = 0", $wces, __FILE__, __LINE__), 0, 0);
    $profLoggedIn = (int)pg_result(pg_go("SELECT COUNT(*) FROM tpr WHERE status = 1", $wces, __FILE__, __LINE__), 0, 0);
    $profNotLoggedIn = $profCount - $profLoggedIn;
    $profLoggedInNotCustomized = $profLoggedIn - $profCustomized;
    print("<h3>Aggregate Usage</h3>\n");

    print("Number of professors: <b>$profCount</b><br>\n");
    print("Number of professors who logged in during the customization period: <b>$profLoggedIn</b><br>\n");
    print("Number of professors who created custom surveys: <b>$profCustomized</b><br>\n");
    print("<img src=\"{$wces_path}media/graphs/pusagegraph.php?neverloggedin=$profNotLoggedIn&custom=$profCustomized&nocustom=$profLoggedInNotCustomized\" width=200 height=200><img src=\"{$wces_path}media/graphs/pusagelegend.gif\" width=133 height=49><br>\n");
    print("<p>&nbsp;</p>\n");
    print("<h3>Individual Class Usage</h3>\n");

    $result = pg_go("
      SELECT tst.status, tcl.class_id, tcl.students, tcl.lastname, tcl.firstname, tcl.class_info, tcl.user_id,
        to_char(tcl.lastlogin,'YYYY-MM-DD') AS lastlogin
      FROM tcl INNER JOIN tst USING (class_id)
      ORDER BY tst.status, tcl.class_info, tcl.class_id
    ", $wces, __FILE__, __LINE__);

    pg_go("DROP TABLE tcl, tst, tpr", $wces, __FILE__, __LINE__);

    $r =& new pg_segmented_wrapper($result, array('status', 'class_id'));

    print("<table cellspacing=1 cellpadding=1 border=1>\n");
    print("<tr><td><b>Code</b></td><td><b>Name</b></td><td><b>Size</b></td><td><b>Professor</b></td><td><b>Last Login</b></td></tr>\n");
    while ($r->row)
    {
      if ($r->split[0])
      {
        print("<tr><td colspan=5><strong><font color=\"#0030E7\">");
        if ($r->row['status'] == 0)
          print("Classes with custom surveys:");
        else if ($r->row['status'] == 1)
          print("Classes with professors who have recently logged in:");
        else
          print("Classes with professors who haven't recently logged in:");
        print("</font></strong></td></tr>\n");
      }

      if ($r->split[1])
      {
        $rows = 0;
        $pr = '';
      }
      else
        $pr .= '<tr>';

      ++$rows;

      extract($r->row);
      $pr .= "<td><a href=\"{$wces_path}administrators/info.php?user_id=$user_id\">$firstname $lastname</a></td>";
      $pr .= $lastlogin ? "<td>$lastlogin</td>" : "<td><i>never</i></td>";
      $pr .= "</tr>\n";

      $r->advance();

      if ($r->split[1])
      {
        $rows = $rows == 1 ? '' : " rowspan=$rows";
        print("<tr><td$rows>");
        print(format_class($class_info, "%c %s"));
        print("</td><td$rows>");
        print(format_class($class_info, "%n"));
        print("</td><td$rows>$students</td>");
        print($pr);
      }
    }
    print("</table>\n");
  }
}

?>

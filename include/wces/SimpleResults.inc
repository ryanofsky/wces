<?

require_once("wbes/postgres.inc");
require_once("wces/wces.inc");

// SimpleResults exists to provides a uniform interface for
// retrieving responses to base questions

//$select_classes = "SELECT DISTINCT class_id FROM wces_topics WHERE make_public";
$select_classes = "SELECT class_id FROM wces_topics AS tsct WHERE tsct.make_public AND EXISTS (SELECT * FROM question_periods AS tscq WHERE tscq.question_period_id = tsct.question_period_id AND tscq.oracledate < (SELECT NOW()))";
$set_question_periods = "(2,23,28)";
$base_item_id = 14; // comments
$oracle_item_id = 13; // multiple choice except for quality of text question
$oracle_quality_of_text = 305; // multiple choice for quality of text question

function & array_get(&$array, $index)
{
  if ($index >= 0)
    return $array[$index];
  else
    return $array[count($array)+$index];
}

class SimpleResults
{
  var $class_ids; // an array of class_id's to retrieve results for
  var $responses = array(); // array (indexed by class_id) of number of survey responses for each class
  var $students = array(); // array (indexed by class_id) of number of students each class
  var $base_choices = array("excellent", "very good", "satisfactory", "poor", "disastrous");
  var $base_values = array(5.0, 4.0, 3.0, 2.0, 1.0);  // values corresponding to choices
  var $base_questions = array(); // array of questions texts
  var $distributions = array(); // array (indexed by class_id) of array of distributions
  var $comments = array(); // array (indexed by class_id) of array of comments

  function SimpleResults($class_ids, $includeComments)
  {
    global $wces, $set_question_periods, $oracle_item_id, $oracle_quality_of_text, $base_item_id;
    wces_connect();

    $class_ids = array_unique($class_ids);
    $classes = implode(',', $class_ids);
    $this->class_ids = $class_ids;

    ///////////////////////////////////////////////////////////////////////////
    // NEW RESPONSE STATISTICS

    $r = pg_go("
      SELECT cl.class_id, cl.students, COUNT(DISTINCT r.user_id) AS responses
      FROM classes AS cl
      INNER JOIN wces_topics AS t USING (class_id)
      INNER JOIN responses_survey AS r ON r.topic_id = t.topic_id AND r.user_id IS NOT NULL
      WHERE cl.class_id IN ($classes) AND t.question_period_id IN $set_question_periods
      GROUP BY cl.class_id, cl.students
    ", $wces, __FILE__, __LINE__);

    $n = pg_numrows($r);
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($r, $i, PGSQL_ASSOC));
      $this->addStatistic($class_id, $students, $responses);
    }

    ///////////////////////////////////////////////////////////////////////////
    // NEW MULTIPLE CHOICE RESULTS

    $r = pg_go("
      SELECT t.class_id, first(c.choices) AS cchoices, q.qtext, choice_dist(qr.answer) AS rdist
      FROM wces_topics AS t
      INNER JOIN responses_survey AS r ON r.topic_id = t.topic_id
      INNER JOIN responses_choice AS cr ON cr.parent = r.response_id
      INNER JOIN revisions AS crv ON crv.revision_id = cr.revision_id
      INNER JOIN components_choice AS c ON c.component_id = crv.component_id
      INNER JOIN responses_choice_question AS qr ON qr.parent = cr.response_id
      INNER JOIN revisions AS qrv ON qrv.revision_id = qr.revision_id
      INNER JOIN components_choice_question AS q ON q.component_id = qrv.component_id
      WHERE t.class_id IN ($classes) AND t.question_period_id IN $set_question_periods AND cr.item_id IN ($oracle_item_id, $oracle_quality_of_text)
      GROUP BY t.class_id, q.qtext, qr.revision_id
      ORDER BY qr.revision_id
    ", $wces, __FILE__, __LINE__);

    $base_choices_count = count($this->base_choices);

    $n = pg_numrows($r);
    for ($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($r, $i, PGSQL_ASSOC));
      $choices = pg_explode($cchoices);
      if (count($choices) < $base_choices_count
          || $choices[0] != $this->base_choices[0]
          || $choices[$base_choices_count - 1] != $this->base_choices[$base_choices_count - 1]
         ) continue;

      $this->addBase($class_id, $qtext, array_pad(pg_explode($rdist), $base_choices_count, 0));
    }

    ///////////////////////////////////////////////////////////////////////////
    // NEW COMMENTS

    if ($includeComments)
    {
      $r = pg_go("
        SELECT DISTINCT t.class_id, tr.revision_id, c.ctext, c.flags, tr.rtext
        FROM wces_topics AS t
        INNER JOIN responses_survey AS r ON r.topic_id = t.topic_id
        INNER JOIN responses_text_question AS tr ON tr.parent = r.response_id
        INNER JOIN revisions AS rv ON rv.revision_id = tr.revision_id
        INNER JOIN components_text_question AS c USING (component_id)
        WHERE t.class_id IN ($classes) AND t.question_period_id IN $set_question_periods AND tr.item_id = $base_item_id
      ", $wces, __FILE__, __LINE__);

      $n = pg_numrows($r);
      for($i = 0; $i < $n; ++$i)
      {
        extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
        $this->addComment($class_id, $rtext);
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    // OLD CLASS_ID's

    // if we don't need to look in the old database, just return now
    if (count($this->distributions) >= count($class_ids)) return;

    $old_new_map = array();
    $r = pg_go("SELECT oldid, newid FROM temp_class WHERE newid IN ($classes)", $wces, __FILE__, __LINE__);
    $n = pg_numrows($r);
    for($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_array($r, $i, PGSQL_ASSOC);
      $old_new_map[(int)$row['oldid']] = (int)$row['newid'];
    }

    if (count($old_new_map) == 0) return;

    $old_classes = implode(',', array_keys($old_new_map));

    ///////////////////////////////////////////////////////////////////////////
    // OLD RESULTS

    $db = wces_oldconnect();
    $answer_criteria = "a.questionperiodid IN (1,2,4,5,7,9) AND a.topicid IN (1,2,4) AND a.questionsetid = '1'";
    $sql_columns = "a.classid, c.students, a.responses, a.FR1";

    $choices = array("a","b","c","d","e");
    for($i = 1; $i <= 10; ++$i)
    {
      $sql_columns .= ", q.MC$i";
      foreach($choices as $choice)
        $sql_columns .= ", a.MC$i$choice";
    };

    $r = db_exec("
      SELECT $sql_columns
      FROM answersets AS a
      INNER JOIN classes as c USING (classid)
      LEFT JOIN questionsets as q ON (a.questionsetid = q.questionsetid)
      WHERE a.classid IN ($old_classes) AND $answer_criteria
    ", $db, __FILE__, __LINE__);
    while($row = mysql_fetch_assoc($r))
    {
      $class_id = $old_new_map[(int)$row['classid']];
      $this->addStatistic($class_id, $row['students'], $row['responses']);
      $this->comments[$class_id] = array_map("stripcslashes", explode("\t", $row['FR1']));
      for($i = 1; $i <= 10; ++$i)
      {
        $qtext = $row["MC$i"];
        if (!$qtext) continue;
        $dist = array($row["MC{$i}a"], $row["MC{$i}b"], $row["MC{$i}c"], $row["MC{$i}d"], $row["MC{$i}e"]);
        $this->addBase($class_id, $qtext, $dist);
      }
    }
  }

  function addStatistic($class_id, $students, $responses)
  {
    $this->students[$class_id] = (int)$students;
    $this->responses[$class_id] = (int)$responses;
  }

  function addComment($class_id, $ctext)
  {
    if (!isset($this->comments[$class_id]))
      $this->comments[$class_id] = array();
    array_push($this->comments[$class_id], $ctext);
  }

  var $base_questions_map = array(); // reverse of $this->base_questions array

  function addBase($class_id, $qtext, $dist)
  {
    if ($qtext == "Quality of Text")
    {
      $qtext = "Course: Quality of Text";
      $dist = array_slice($dist, 0, 5);
    }
    if (isset($this->base_questions_map[$qtext]))
      $index = $this->base_questions_map[$qtext];
    else
    {
      $index = array_push($this->base_questions, $qtext) - 1;
      $this->base_questions_map[$qtext] = $index;
    }

    if (!isset($this->distributions[$class_id]))
      $this->distributions[$class_id] = array();

    $this->distributions[$class_id][$index] = $dist;
  }
};

/*
$s = new SimpleResults(array(20585), true);
print("<pre>");
print_r($s);
print("</pre>");
*/
?>
<?

require_once('wces/page.inc');

login_protect(login_administrator);

param($question_period_id);
$question_period_id = (int)$question_period_id;

if ($question_period_id)
{
  wces_connect();

  $fun = substr(uniqid('fun'), 0, 31);

  pg_query("
    CREATE TEMPORARY TABLE dumbness AS
    SELECT r.topic_id, cr.revision_id AS crevision_id, qr.revision_id AS qrevision_id, min(qr.oid::integer) as ordinal, avg(qr.answer), count(distinct r.response_id)
    FROM survey_responses AS r
    INNER JOIN choice_responses AS cr ON cr.parent = r.response_id
    INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
    WHERE r.question_period_id = $question_period_id 
    GROUP BY r.topic_id, cr.revision_id, qr.revision_id
  ", $wces, __FILE__, __LINE__);
  
  pg_query("
    CREATE TEMPORARY TABLE punches AS
    SELECT DISTINCT ON (crevision_id, qrevision_id) crevision_id, qrevision_id, ordinal
    FROM dumbness ORDER BY crevision_id, qrevision_id, ordinal;
  ", $wces, __FILE__, __LINE__);
  
  pg_query("
    CREATE FUNCTION $fun(INTEGER, INTEGER) RETURNS INTEGER AS '
      SELECT ordinal FROM punches WHERE crevision_id = $1 AND qrevision_id = $2
    ' LANGUAGE 'sql';
    
    UPDATE dumbness SET ordinal = $fun(crevision_id, qrevision_id);
  ", $wces, __FILE__, __LINE__);
  
  $r = pg_query("
    SELECT d.crevision_id, d.qrevision_id, d.ordinal, c.first_number,
      c.last_number, c.flags, c.choices, q.qtext
    FROM punches AS d
    INNER JOIN choice_components AS c ON c.revision_id = d.crevision_id
    INNER JOIN choice_questions AS q ON q.revision_id = d.qrevision_id
    INNER JOIN branches AS b ON b.branch_id = c.branch_id
    WHERE b.topic_id = 1 AND c.first_number IS NOT NULL AND c.last_number IS NOT NULL
    ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);
  
  $questions = new pg_wrapper($r);
  
  $r = pg_query("
    SELECT * FROM dumbness ORDER BY ordinal
  ", $wces, __FILE__, __LINE__);

  $results = new pg_wrapper($r);

  $r = pg_query("SELECT displayname FROM semester_question_periods WHERE question_period_id = $question_period_id", $wces, __FILE__, __LINE__);
  if (pg_numrows($r) != 1) die ("Invalid question period $question_period_id"); 
  $qp = pg_result($r, 0, 0);

  $r = pg_query("
    SELECT t.topic_id, s.code AS scode, c.code AS ccode, cl.semester, cl.year, u.lastname
    FROM (SELECT DISTINCT topic_id FROM dumbness) AS d
    INNER JOIN wces_topics AS t USING (topic_id)
    INNER JOIN classes AS cl USING (class_id)
    INNER JOIN courses AS c ON c.course_id = cl.course_id
    INNER JOIN subjects AS s USING (subject_id)
    LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND status = 3
    LEFT JOIN users AS u USING (user_id)
    ORDER BY s.code, c.code
  ", $wces, __FILE__, __LINE__);

  $classes = new pg_wrapper($r);


  $r = pg_query("
    SELECT DISTINCT ON (topic_id) topic_id, count
    FROM dumbness
  ", $wces, __FILE__, __LINE__);

  $counts = new pg_wrapper($r);

  $semesters = array("S", "U", "F", "W");

  $topics = array();

  $r1 = $r2 = $r3 = $r4 = '';
  while ($classes->row)
  {
    extract($classes->row);
    $topics[] = $topic_id;
    $r1 .= "\t$scode $ccode";
    $sem = isset($semesters[$semester]) ? $semesters[$semester] : '?';
    $r2 .= "\t$sem$year";
    $r3 .= "\t$lastname";
    $r4 .= "\taverage";
    $classes->advance();
  }

//  pg_show($questions->result);
//  pg_show($results->result);

  header("Content-type: text/plain");
  //print("<pre>");

  print("Thayer School Course Evaluations -- Grand Summary\n");
  print("\t$qp\n$r1\n$r2\n$r3\nQuestion$r4\n");

  while ($questions->row)
  {
    extract($questions->row); 
    
    if ($flags & 1) // is_numeric
      $last = abs($last_number - $first_number);
    else
      $last = count(pg_explode($choices)) - 1;

    print("$qtext");
    $averages = array();
    while($results->row && $results->row['ordinal'] == $ordinal)
    {
      $averages[$results->row['topic_id']] = $results->row['avg'] 
        / $last * ($last_number - $first_number) + $first_number;
      $results->advance();
    } 
    
    foreach($topics as $topic)
    {
      if (isset($averages[$topic]))
        printf("\t%.2f", $averages[$topic]);
      else
        print("\t?"); 
    }
    print("\n");
    
    $questions->advance();  
  }
  
  $responses = array();
  while($counts->row)
  {
    $responses[$counts->row['topic_id']] = $counts->row['count'];
    $counts->advance();
  }
  
  print("\nRespondents");
  
  foreach($topics as $topic)
  {
    if (isset($responses[$topic]))
      print("\t$responses[$topic]");
    else
      print("\t?"); 
  }
  print("\n");
  
}
else
{
  page_top('Text Report');

  print("<p>Choose a question period:</p>\n");
  print("<ul>\n");
  wces_connect();
  $result = pg_query("
    SELECT question_period_id, displayname FROM semester_question_periods ORDER BY begindate
  ", $wces, __FILE__, __LINE__);
  
  $n = pg_numrows($result);
  
  for ($i = 0; $i < $n; ++$i)
  {
    extract(pg_fetch_row($result, $i, PGSQL_ASSOC));
    print("  <li><a href=\"$PHP_SELF?question_period_id=$question_period_id\">$displayname</li>\n"); 
  }
  
  print("</ul>\n");
  
  page_bottom();
}



?>
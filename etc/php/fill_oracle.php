<?

// script to convert saved responses in the new high-tech database
// into the answersets format that oracle can understand

// ABORT, ABORT!
// script is only half-written. I stopped work, because I suddenly
// realize it is more worthwile to just port oracle over to the new
// database to bring results back to the mysql database
//
// reasons for doing this:
// 
// 1) It needs to by done anyway if we are every going to get rid
//    of the mysql database
// 2) Sort of related to the other task getting old results
//    to display in the reporting wizard
// 3) The old database format doesn't support multiple professors
//    per class and some of the classes that were evaluated this
//    term did in fact have >1 professor.

define("DEPARTMENT", 1);
define("COURSE", 2);
define("SUBJECT", 3);

// copy a row from the new departments, subjects, or courses tables
// into the old database, and update the temporary links
// this function assumes that the row is NOT already in the old database

// $newid is the row id in the new database
// $entity_type is the type of row
// $othercols is an associative array of attributes and values to include in
//   the insert statement to the old
function move_over($newid, $entity_type, $othercols = array())
{
  $entities = array
  (
    DEPARTMENT => array
    (
      "new_table_name" => "departments",
      "old_table_name" => "departments",
      "temp_table_name" => "temp_dept",
      "new_id_name" => "department_id",
      "old_id_name" => "departmentid",
      "columns" => array("code", "name")
    ),

    SUBJECT => array
    (
      "new_table_name" => "subjects",
      "old_table_name" => "subjects",
      "temp_table_name" => "temp_dept",
      "new_id_name" => "subject_id",
      "old_id_name" => "subjectid",
      "columns" => array("code", "name")
    ),

    COURSE => array
    (
      "new_table_name" => "courses",
      "old_table_name" => "courses",
      "temp_table_name" => "temp_dept",
      "new_id_name" => "course_id",
      "old_id_name" => "courseid",
      "columns" => array("code", "name")
    )
  );

  if (isset($entities[$entity_type]))
    extract($entities[$entity_type]);
  else
    die("Invalid entity type: $entity_type at" . __FILE__ . ":" . __LINE__);

  $colnames = implode($columns, ", ");

  $r = pg_query("SELECT $colnames FROM $new_table_name WHERE $new_id_name = $newid", $pg, __FILE__, __LINE__);
  if (pg_numrows($r) != 1) die ("Inconsistent information about $new_id_name $newid at" . __FILE__ . ":" . __LINE__);

  $data = pg_fetch_assoc($r, 0, PGSQL_ASSOC);
  $data = array_merge($data, $othercols); // merge in custom values
  aarray_map("quot", $data)

  $datanames = implode(array_keys($data), ", ");
  $datavals = implode(array_values($data), ", ");

  db_exec("INSERT INTO $old_table_name ($datanames) VALUES ($datavals)", $my, __FILE__, __LINE__)
    or die("insert failed at" . __FILE__ . ":" . __LINE__);

  $oldid = mysql_insert_id($my);

  pg_query("INSERT INTO $temp_table_name (oldid, newid) VALUES ($oldid, $newid)", $pg, __FILE__, __LINE__)
    or die("insert failed at" . __FILE__ . ":" . __LINE__);

  return $oldid;
};

$pg_question_period_id = 2;
$my_question_period_id = 10;

$my_question_set_id = 1;

$pg = wces_connect();
$my = wces_oldconnect();

// delete old responses
$result = db_exec("DELETE FROM answersets WHERE my_question_period_id = $qp AND questionsetid = $my_question_set_id", $my, __FILE__, __LINE__);

// first, import new class data from new database to old
$result = pg_query("
  SELECT t.class_id
  FROM wces_topics AS t
  EXCEPT
  SELECT newid FROM temp_class
", $pg, __FILE__, __LINE__);

// loop through all the classes that don't exist in the old database
$n = pg_numrows();
for($i = 0; $i < $n; ++$i)
{
  $class_id = (int)pg_result($result, $i, 0);

  $r = pg_query("
    SELECT cl.name,
      d.department_id, do.oldid AS old_department_id,
      s.subject_id, so.oldid AS old_subject_id,
      c.course_id, co.oldid AS old_course_id
    FROM classes AS cl
    INNER JOIN courses AS c USING (course_id)
    INNER JOIN subjects AS s USING (subject_id)
    LEFT JOIN departments AS d ON d.department_id = cl.department_id
    LEFT JOIN temp_course AS co ON co.newid = c.course_id
    LEFT JOIN temp_subject AS so ON so.newid = s.subject_id
    LEFT JOIN temp_department AS do ON do.newid = d.department_id
    WHERE cl.class_id = $class_id;
  ", $pg, __FILE__, __LINE__);

  if (pg_numrows($r) != 1) die("Inconsistent information about class_id $class_id at" . __FILE__ . ":" . __LINE__);

  extract(pg_fetch_assoc($r, 0, PGSQL_ASSOC));

  if (!$old_department_id)
  {
    if ($department_id) 
      $old_department_id = move_over($department_id, DEPARTMENT);
    else
      $old_department_id = "NULL";
  }

  if (!$old_subject_id)
    $old_subject_id = move_over($department_id, SUBJECT);

  if (!$old_course_id)
    $old_course_id = move_over($course_id, COURSE, array("subjectid" => $old_subject_id));

  $r = pg_query("
    SELECT section, year, semester, division FROM classes WHERE class_id = $class_id
  ", $pg, __FILE__, __LINE__);
  
  array
  (
    section , year, semester, divisioncode

  
  course_id, department_id FROM classes WHERE class_id = $class_id", $pg, __FILE__, __LINE__);



}

// next, import all categories (old: topics) that don't exist in the old database

?>
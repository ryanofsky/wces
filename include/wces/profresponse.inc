<?

require_once("wces/database.inc");

//---------------------------------------------------------------
// BEGIN FUNCTION LIST

function classinfo($classID)
{
  global $db, $connection;

  $class = array();

  $result = db_exec("
    SELECT 
      cl.year                         AS year,
      cl.semester                     AS sem,
      cl.section                      AS section,
      cl.courseid                     AS courseID,
      cl.students                     AS total_students,
      cl.professorid                  AS professorID,
      cl.departmentid                 AS departmentID,
      c.name                          AS name,
      c.code                          AS code,
      SUBSTRING_INDEX(p.name,' ',-1)  AS professor_name,
      p.name                          AS professor_fullname,
      qp.questionperiodid             AS questionperiodID,
      d.code                          AS department_code
    FROM classes AS cl
    INNER JOIN courses AS c USING (courseid)
    LEFT JOIN professors AS p ON p.professorid = cl.professorid
    LEFT JOIN departments AS d ON d.departmentid = cl.departmentid
    LEFT JOIN questionperiods AS qp ON cl.year = qp.year AND cl.semester = qp.semester AND qp.description = 'Final Evaluations'
    WHERE classid = '$classID' AND cl.courseid = c.courseid
    LIMIT 1", $db, __FILE__, __LINE__);
    
  $myrow = mysql_fetch_assoc($result);
  mysql_free_result($result);
    
  if ($myrow) return $myrow;
  
  return $class;
  
} //classinfo

//---------------------------------------------------------------
// getQuestion(int $answersetID, "MC1" etc)
// will return plain text of question

function getQuestion($answersetID, $whichQ)
{
  global $db;
  
  if (!$answersetID || !$whichQ) return "";
  
  $result = db_exec("
    SELECT qs.$whichQ
    FROM questionsets AS qs, answersets AS a
    WHERE qs.questionsetid = a.questionsetid AND a.answersetid='$answersetID'
    LIMIT 1
  ", $db, __FILE__, __LINE__);

  $text = mysql_result($result,0);
  mysql_free_result($result);

  return $text;
} //getQuestion


//---------------------------------------------------------------
// function getLink(int $answersetID, "MC1" etc)
// will return link in form of answerset-question i.e. 1444-MC1
//
// Mark I am not quite sure what format you were intending to use
// for the links table because there is no sample data in there.
// The following code just finds the previous answerset by
// looking for a previous class with the same professor and
// same courseid that was given the same questionset.
//
// The function can be modified to use the links table simply
// by replacing these two lines
//
// $newqs = $oldqs;
// $newq = $oldq;  
//
// with lines that are able to find a mapping between
// different questions and questionsets.

function getLink($answersetID, $oldq)
{
  global $db;

  $oldqs = QSfromAS($answersetID);
 
  $newqs = $oldqs;  //      <-------  Update these lines to use the links table
  $newq = $oldq;    //      <-------  Update these lines to use the links table

  $r = db_exec("
    SELECT a2.answersetid, a2.questionsetid
    FROM answersets AS a1
    INNER JOIN classes AS cl1 ON cl1.classid = a1.classid
    INNER JOIN classes AS cl2 ON cl2.professorid = cl1.professorid AND cl2.courseid = cl1.courseid
    INNER JOIN answersets AS a2 ON a2.classid = cl2.classid
    INNER JOIN questionperiods AS qp1 ON qp1.questionperiodid = a1.questionperiodid
    INNER JOIN questionperiods AS qp2 ON qp2.questionperiodid = a2.questionperiodid
    WHERE
      a1.answersetid = '$answersetID' 
      AND
      a2.answersetid <> '$answersetID' 
      AND
      a2.questionsetid IN ('$newqs','$oldqs')
      AND
      (
        qp2.year < qp1.year
        OR
        (
          qp2.year = qp1.year
          AND 
          qp2.semester < qp1.semester
        )  
      )
      AND
      qp2.description <> 'Midterm Evaluations'
    ORDER BY qp2.year DESC, qp2.semester DESC
  ", $db, __FILE__, __LINE__);
      
  if (mysql_num_rows($r) == 0)
    return "-";
  else
  {
    $a = mysql_fetch_assoc($r);
    if ($a["questionsetid"] == $oldqs)
      return $a["answersetid"] . "-" . $oldq;
    else // $a["questionsetid"] == $newqs
      return $a["answersetid"] . "-" . $newq;
  }  
 
  //$result = db_exec("SELECT $whichQ FROM links WHERE answersetid='$answersetID' LIMIT 1", $db, __FILE__, __LINE__);
  //$returnMe = mysql_result($result,0);
  //mysql_free_result($result);
  //return $returnMe;
}

function QSfromAS($asID) // blunt but effective
{
  global $db;
  $r = db_exec("SELECT questionsetid FROM answersets WHERE answersetid = '$asID'", $db, __FILE__, __LINE__);
  return mysql_result($r,0);
}

//---------------------------------------------------------------
// function getRawNums(array $AID, "MC1" etc)
// will return an array of raw numbers

function getRawNums($AID, $whichQ)
{
  global $db;

  $temp_str = $whichQ."a,".$whichQ."b,".$whichQ."c,".$whichQ."d,".$whichQ."e";
  $as_str = "answersetid IN (" . implode(", ", $AID) . ")";
  $result = db_exec("SELECT $temp_str FROM answersets WHERE $as_str", $db, __FILE__, __LINE__);

  $MCA = $MCB = $MCC = $MCD = $MCE = 0;
 
  while ($myrow = mysql_fetch_assoc($result))
  {
    $MCA += $myrow[$whichQ."a"];
    $MCB += $myrow[$whichQ."b"];
    $MCC += $myrow[$whichQ."c"];
    $MCD += $myrow[$whichQ."d"];
    $MCE += $myrow[$whichQ."e"];
  }
  mysql_free_result($result);

  return array($MCA,$MCB,$MCC,$MCD,$MCE);
} // getRawNums


//-----------------------------------------------------------------
// function get array_of_answersetIDs

function getArrayOfAnswersetIDs($professorID,$last_sem,$last_year)
{
  global $db;
  $result = db_exec("
    SELECT a.answersetid
    FROM classes AS cl
    INNER JOIN answersets AS a USING (classid)
    INNER JOIN questionperiods AS qp USING (questionperiodid)
    WHERE 
      cl.professorid  = '$professorID' AND
      cl.semester     = '$last_sem'    AND 
      cl.year         = '$last_year'   AND 
      a.questionsetid = 1              AND
      qp.description  = 'Final Evaluations'      
  ", $db, __FILE__, __LINE__);
  
  $return = NULL;
  while($row = mysql_fetch_assoc($result))
    $return[] = $row["answersetid"];
  
  return $return ? $return : array(-1);
}

?>

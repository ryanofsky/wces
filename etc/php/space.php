<?

require_once("wces/wces.inc");

$db = wces_connect();
$result = db_exec("select answersetid, FR1, FR2 from answersets where questionperiodid = 7",$db,__FILE__,__LINE__);

    $db_debug = true;
while($row = mysql_fetch_assoc($result))
{
  $FR1 = explode("\t",$row["FR1"]);
  if ($FR1)
  {
    foreach ($FR1 as $key => $value)
    {
      $FR1[$key] = trim($value);
      if(!$FR1[$key]) unset($FR1[$key]);
    }
    $answersetid = $row["answersetid"];
    $FR1 = addslashes(implode("\t",$FR1));
    db_exec("UPDATE answersets SET FR1 = '$FR1' WHERE answersetid = $answersetid", $db, __FILE__,__LINE__);
  }  
  
  $FR2 = $row["FR2"] ? explode("\t",$row["FR2"]) : "";
  if (is_array($FR2) && count($FR2) > 0)
  {
    foreach ($FR2 as $key => $value)
    {
      $FR2[$key] = trim($value);
      if(!$FR2[$key]) unset($FR2[$key]);
    }
    $answersetid = $row["answersetid"];
    $FR2 = addslashes(implode("\t",$FR2));
    db_exec("UPDATE answersets SET FR2 = '$FR2' WHERE answersetid = $answersetid", $db, __FILE__,__LINE__);
  }  
}


//UPDATE answersets SET FR1 = 'Believe, this has to have credit hours\' rank =4, at least.\tThank you for not requireing feedback on TA\'s...been waiting two years for that.\n\nBut, not all courses have textbooks, especially labs. The rating for textbook is meaningless.\tThe labs themselves were well put together and they worked. They demonstrated some advanced concepts and newly discovered phenomena and yet were performable in the alotted time. This is a great feat. However, I have some misgivings which are outlined below, possibly in an overcritical tone, but I think it reflects a feeling of frustration shared by the graduate students...\n\nFor graduate students it is practical to learn how to write a laboratory report directed to peers assuming some level of background knowledge, with the aim to clearly explain any new concepts. It is a waste of our time to write with the aim of convincing the reader that we can understand a lab or perform data analysis, as we would not be here if we could not. We need to learn the communication skills that will be practical in futur research and/or learn new techniques of data analysis. Perhaps the faculty does not believe that we are smart enough to understand what is going on a lab or how to ananlyse data and wishes that we prove ourselves?? Such a proof would be foiled by the fact that those whose performance was best may simply be those who were most willing to invest time in getting another A on a piece of paper whose meaning would fade with time...\n\noops, a little harsh, but very honest..' WHERE answersetid = 1513;



?>
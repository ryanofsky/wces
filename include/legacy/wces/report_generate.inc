<?

require_once("legacy/wces/report_page.inc");

function legacy_makereport($db,&$wizard,$stdout,&$html,&$text)
{
  global $wces_path;    
      
  $sqloptions = array
  (
    "standard" => $wizard->includestandard->checked || ($wizard->includetext->checked && !$wizard->includecustom->checked),
    "custom" => $wizard->includecustom->checked
  );

  $groups = Array
  (
    "classes" => $wizard->groupclasses->checked,
    "courses" => $wizard->groupcourses->checked,
    "professors" => $wizard->groupprofessors->checked,
    "departments" => $wizard->groupdepartments->checked,
    "questionperiods" => $wizard->groupquestionperiods->checked
  );

  $header = true;
  $ratings = ($wizard->includestandard->checked || $wizard->includecustom->checked) ? true : false;
  $listclasses = ($wizard->groupclasses->checked || $wizard->bighead->checked) ? true : false;
  $listprofessors = ($wizard->groupprofessors->checked || $wizard->bighead->checked) ? true : false;
  
  $abet = $wizard->includeabet->checked ? true : false;
  $responses = $wizard->includetext->checked ? true : false;
  $tas = $wizard->includeta->checked ? true : false;
  $sort = array();
  foreach ($wizard->order->items as $order)
    array_push($sort, $wizard->ordernames[$order]);
    
  $pages = legacy_report_makequeries($db, $sqloptions, $wizard->criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet, 
    $responses, $tas);
  
  $displayoptions = array("pies" => $wizard->includepies->checked);

  $page = 0;
  if ($stdout || $html)
  {
    $outhtml = "<font size=1>[ $page / $pages ]</font>\n";
    $outhtml .= "<h4>Report Information</h4>\n";
    $outhtml .= "<table border=0><tr><td>Administrator ID: </td><td><b>" . login_getuni() . "</b></td></tr><tr><td>Date:</td><td><b>" . date("l, F j, Y") . "</b></td></tr><tr><td>Time:</td><td><b>" . date("g:i:s A") . "</b></td></tr></table>";
    $outhtml .= "<h4>Report Specification</h4><pre>" . htmlspecialchars($wizard->summarize()) . "</pre>";
      
    if ($html) $html = $outhtml;
    if ($stdout) print ($outhtml);
  }
    
  $line = "---------------------------------------------------------------------------\n";
  if ($text)
  {
    $text = "${line}[ $page / $pages ]\n\n";
    $text .= "legacy_Report Information\n";
    $text .= " - Administrator ID: " . login_getuni() . "\n";
    $text .= " - Date: " . date("l, F j, Y") . "\n";
    $text .= " - Time: " . date("g:i:s A") . "\n";
  }
    
  $outhtml = false;
  for($page = 1; $page <= $pages; ++$page)
  {
    if ($html || $stdout)
    {
      $outhtml = '<hr style="page-break-before:always">' . "\n";
      $outhtml .= "<font size=1>[ $page / $pages ]</font>\n";
    }
     
    if ($text)
      $text .= "\n${line}[ $page / $pages ]\n\n";
    
    legacy_report_makepage($text, $outhtml, $displayoptions, $groups, $header, $listclasses,
      $listprofessors, $ratings, $abet, $responses, $tas);
    
    if ($stdout) print($outhtml);
    if ($html) $html .= $outhtml;
  }
}

function legacy_report_profemail($db,&$wizard,$subject,$message,$debug)
{
  $sqloptions = array
  (
    "standard" => $wizard->includestandard->checked || ($wizard->includetext->checked && !$wizard->includecustom->checked),
    "custom" => $wizard->includecustom->checked
  );
  $displayoptions = array("pies" => false);
  $groups = array("classes" => true, "courses" => false, "professors" => true, "departments" => true, "questionperiods" => true);
  $sort = array("professors","questionperiods","classes");

  $header = true;
  $ratings = ($wizard->includestandard->checked || $wizard->includecustom->checked) ? true : false;
  $listclasses = true;
  $listprofessors = true;
  $abet = $wizard->includeabet->checked ? true : false;
  $responses = $wizard->includetext->checked ? true : false;
  $tas = $wizard->includeta->checked ? true : false;
  
  legacy_report_makequeries($db, $sqloptions, $wizard->criteria, $groups,
    $sort, $header, $listclasses, $listprofessors, $ratings, $abet, 
    $responses, $tas);

  $professors = db_exec("
    SELECT COUNT(DISTINCT ml.masterid) AS number, ml.professorid, p.name, IFNULL(p.email,IFNULL(u.email,'')) AS address
    FROM mastertable AS ml
    LEFT JOIN professors AS p ON p.professorid = ml.professorid
    LEFT JOIN users AS u ON u.userid = p.userid
    GROUP BY ml.professorid
    ORDER BY ml.masterid", $db, __FILE__, __LINE__);

  $line = "---------------------------------------------------------------------------\n";  
  $html = false;
  if (!$debug) print("<h3>Sending Email...</h3>");
  print("<p>");
  while($professor = mysql_fetch_assoc($professors))
  {
    $name = $professor["name"];
    $address = $professor["address"];
    $email = "";
    if ($message) $email .= wordwrap(str_replace("%profname%", $name, $message),76) . "\n\n$line";
    $email .= "\nA graphical version of this report is available online at\n";
    $email .= "http://oracle.seas.columbia.edu/wces/professors/\n";

    $pages = $professor["number"];
    $professorid = $professor["professorid"];
    
    for($page=1; $page <= $pages; ++$page)
    {
      if ($professorid != $listprofessors->row["professorid"])
      {
        printarray($listprofessors->row,"listprofessors");
        debugout("<b>SYNCHRONIZATION ERROR (" . $professor["professorid"] . " != ". $listprofessors->row["professorid"] . ")</b>");
        exit();
      }  
      $email .= "\n${line}Page $page of $pages\n\n";        
      legacy_report_makepage($email, $html, $displayoptions, $groups, $header, $listclasses,
        $listprofessors, $ratings, $abet, $responses, $tas);      
    }
    
    $from = "wces@columbia.edu";
    
    if (!$address) print("<b><font color=red>No email address found for Professor $name</font></b><br>\n");

    if ($debug)
    {
      print("<h4>Professor $name (<a href=\"mailto:$address\">$address</a>)</h4>\n");
      print("<p><b>$subject</b></p>\n");
      print("<ul><pre>$email</pre></ul>");
    }  
    else
    {
      $address = "rey4@columbia.edu"; //FOR DEBUGGING
      if (mail($address,$subject,$email, "To: $address\nFrom: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion()))
      {
        print("<font size=\"-1\">Successfully sent report to Professor $name ($address, $professorid)</font><br>\n");
      }
      else
      {
      	print("<font size=\"-1\" color=red><b>Problem sending report to Professor $name ($address, $professorid)</b></font><br>\n");
      }
    }  
  }
  print("</p>\n<h3>Done.</h3>\n");
  
}

?>
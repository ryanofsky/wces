<%
require_once("wces/page.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

$MassEmail_students = array
(
  "all" => "All Students",
  "dumb" => "Students who haven't filled out any surveys",
  "bad" => "Students who haven't filled out all of their surveys",
  "good" => "Students who completed all of their surveys"
);

define("MassEmail_send",1);
define("MassEmail_preview",2);

class MassEmail extends FormWidget
{
  var $from;
  var $replyto;
  var $to;
  var $text;
  
  var $action;
  var $errors;
  var $db;
  var $form;
  
  function MassEmail($db,$prefix, $form, $formmethod)
  {
    $this->db = $db;
    $this->FormWidget($prefix, $form, $formmethod);
    
    $this->form = new Form("${prefix}_form", $form, $formmethod);
    $this->from = new TextBox(0,60,"", "${prefix}_from", $form, $formmethod);
    $this->replyto = new TextBox(0,60,"", "${prefix}_replyto", $form, $formmethod);
    $this->subject = new TextBox(0,60,"", "${prefix}_subject", $form, $formmethod);
    $this->text = new TextBox(15,60,"", "${prefix}_text", $form, $formmethod);
    $this->action = new ActionButton("${prefix}_action", $form, $formmethod);
    $this->errors = array();
  }
  
  function loadvalues()
  {
    global $MassEmail_students;
    
    $this->form->loadvalues();
    
    if ($this->form->isstale)
    {
      $this->from->loadvalues();
      $this->replyto->loadvalues();
      $this->subject->loadvalues();
      $this->text->loadvalues();
      $this->action->loadvalues();
      $this->to = $this->loadattribute("to");
    }
    else
    {
      $userid = login_getuserid();
      $name = ucwords(strtolower(db_getvalue($this->db,"ldapcache", array("userid" => $userid), "cn")));
      $email = db_getvalue($this->db,"users", array("userid" => $userid), "email");
      if ($email)
      {
        if ($name)
          $this->from->text = "$name <$email>";
        else
          $this->from->text = $email;  
      }
      $this->to = "";
      $this->replyto->text = "wces@columbia.edu";
      $this->subject->text = "WCES Reminder";
      $this->text->text = "Dear %studentname%,\n\nCome to http://oracle.seas.columbia.edu/ so you can rate these %nmissingclasses% classes:\n\n%missingclasses%\n\nWin prizes!";
      
    }  

    if (!emailvalid($this->from->text))
      $this->errors[] = "Invalid FROM address";

    if (!emailvalid($this->replyto->text))
      $this->errors[] = "Invalid REPLY-TO address";
       
    if (!isset($MassEmail_students[$this->to]))
      $this->errors[] = "Invalid TO selection";
      
    if (!trim($this->text->text))
      $this->errors[] = "You must enter text to send";  
      
    if (!$this->subject->text)
      $this->errors[] = "Subject is blank.";  
    
    $this->action->loadvalues();  
  }
  
  function display()
  {
    global $MassEmail_students;
    $prefix = $this->prefix;
    $this->form->display();

    $go = $this->action->action == MassEmail_preview || $this->action->action == MassEmail_send;
    
    if ($go && count($this->errors) == 0)
    {
      $this->action->display("Back");
      print("<hr>");
      $this->domail($this->action->action == MassEmail_send);
      print("<hr>");
      $this->action->display("Back");
      
      $this->from->display(true);
      $this->replyto->display(true);
      $this->subject->display(true);
      $this->text->display(true);
      $this->preserveattribute("to");
    }
    else
    {
      if ($go)
      {
        print("<p><strong>The following errors were found:</strong></p>\n<ul>\n");
        foreach($this->errors as $error)
          print("  <li>$error</li>\n");
        print("</ul>\n");
      }
%>
<p>From this page you can send customized emails to students in classes being surveyed this semester.</p>
<p>Use the <strong>To:</strong> drop down box to send emails to a specific group of students.</p>
<p>The following variables can be used in the subject and message body. They will be replaced by student data taken from the WCES database.</p>
<div style="background: #EEEEEE">
<pre>
  <strong>%studentname%</strong>      - The full name of the student.
  <strong>%allclasses%</strong>       - A list of the student's classes that have surveys available.
  <strong>%missingclasses%</strong>   - A list of classes that the student has not filled out surveys for.
  <strong>%finishedclasses%</strong>  - A list of classes that the student has filled out surveys for.
  <strong>%nallclasses%</strong>      - The number of the student's classes that have surveys available.
  <strong>%nmissingclasses%</strong>  - The number of classes that the student has not filled out surveys for.
  <strong>%nfinishedclasses%</strong> - The number of classes that the student has filled out surveys for.
</pre>
</div>
<hr>
<table>
  <tr>
    <td valign=top align=right><STRONG>From:</STRONG></td>
    <td><% $this->from->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Reply To:</STRONG></td>  
    <td><% $this->replyto->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>To:</STRONG></td>  
    <td><%
      print("<select name=\"${prefix}_to\">");
      foreach($MassEmail_students as $key => $label)
      {
        $selected = $key == $this->to ? " selected" : "";
        print("<option value=\"$key\"$selected>$label</option>");
      }  
      print("</select>");  
    %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Subject:</STRONG></td>
    <td><% $this->subject->display(); %></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Text:</STRONG></td>
    <td><% $this->text->display(); %></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td><% $this->action->display("Send", MassEmail_send); %> <% $this->action->display("Preview", MassEmail_preview); %>  </td>
  </tr>
</table>
<%  
    }
  }
  
  function domail($send = false)
  {
    $db = $this->db;
   
    $questionperiodid = wces_Findquestionsetsta($db,"qsets");        

    db_exec("CREATE TEMPORARY TABLE studclasses( userid INTEGER NOT NULL, classid INTEGER NOT NULL, surveyed INTEGER, PRIMARY KEY(userid, classid) )", $db, __FILE__, __LINE__);
    db_exec("
      REPLACE INTO studclasses(userid, classid, surveyed)
      SELECT e.userid, e.classid, IF(cs.userid IS NULL,0,1)
      FROM qsets AS qs
      INNER JOIN enrollments AS e ON e.classid = qs.classid
      INNER JOIN users AS u ON e.userid = u.userid AND u.isprofessor = 'false'
      INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
      LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
      LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
      
      GROUP BY e.userid, e.classid
      ", $db, __FILE__, __LINE__);

    db_exec("CREATE TEMPORARY TABLE recipients( userid INTEGER NOT NULL, bob INTEGER, PRIMARY KEY(userid) )", $db, __FILE__, __LINE__);
    
    // mysql wants bob because it gives an error when I try to put
    // COUNT(DISTINCT sc.classid) directly in the HAVING clause

    if ($this->to == "good")
      $having = "HAVING bob = SUM(sc.surveyed)";
    else if ($this->to == "dumb")
      $having = "HAVING 0 = SUM(sc.surveyed)";
    else if ($this->to == "bad")
      $having = "HAVING bob > SUM(sc.surveyed) AND SUM(sc.surveyed) > 0";
    else
      $having = "";

    db_exec("REPLACE INTO recipients(userid,bob) SELECT userid, COUNT(DISTINCT sc.classid) AS bob FROM studclasses AS sc GROUP BY sc.userid $having",$db, __FILE__, __LINE__);
    
    $users = db_exec("
      SELECT u.cunix AS cunix, sc.userid AS userid, u.email AS email, ld.cn AS name, COUNT(DISTINCT sc.classid) AS surveys, SUM(sc.surveyed) AS surveyed
      FROM studclasses AS sc
      INNER JOIN recipients AS r ON u.userid = r.userid
      INNER JOIN users AS u ON (u.userid = sc.userid)
      LEFT JOIN ldapcache AS ld ON (ld.userid = sc.userid)
      GROUP BY sc.userid
      ORDER BY sc.userid
    ", $db, __FILE__, __LINE__);
    
    $classes = db_exec("
      SELECT sc.userid AS cluserid, cl.section AS section, c.code AS code, c.name AS cname, s.code AS scode, p.name as pname, sc.surveyed as surveyed
      FROM studclasses AS sc
      INNER JOIN recipients AS r ON sc.userid = r.userid
      INNER JOIN classes AS cl ON cl.classid = sc.classid
      INNER JOIN courses AS c ON c.courseid = cl.courseid
      INNER JOIN subjects AS s ON s.subjectid = c.subjectid
      LEFT JOIN professors AS p ON p.professorid = cl.professorid
      GROUP BY sc.userid, sc.classid
      ORDER BY sc.userid
    ", $db, __FILE__, __LINE__);
    
    $total = mysql_num_rows($users);
    $sofar = 0;
    
    if ($send) 
    {
      taskwindow_start("Progress Window", false);
      print("<h3>Sending...</h3>");
    }  
    
    while($user = mysql_fetch_assoc($users))
    {
      $studentname = ucwords(strtolower($user["name"]));
      $nallclasses = $user["surveys"];
      $nfinishedclasses = $user["surveyed"];
      $nmissingclasses = $nallclasses - $nfinishedclasses;
      $missingclasses = "";
      $finishedclasses = "";

      for($i=0; $i < $nallclasses; ++$i)
      {
        $classe = mysql_fetch_assoc($classes);
        if ($classe["surveyed"]) $thelist = &$finishedclasses; else $thelist = &$missingclasses;
        if ($thelist) $thelist .= "\n\n";
        $thelist .= " * " .  str_replace("\n", "\n   ", wordwrap($classe["scode"] . $classe["code"] . ' ' . $classe["cname"] . ' Section ' . $classe["section"].  ($classe["pname"] ? "\nProfessor " . $classe["pname"] : ''), 70, "\n"));
      }
      
      $allclasses = $finishedclasses;
      if ($missingclasses) 
      {
        if ($allclasses) $allclasses .= "\n\n";
        $allclasses .= $missingclasses;
      }

      $fields = array("studentname", "nallclasses", "nfinishedclasses", "nmissingclasses", "missingclasses", "finishedclasses", "allclasses");
      $text = $this->text->text;
      foreach($fields as $field)
        $text = str_replace("%${field}%", ${$field}, $text);
      $text = wordwrap($text, 75);
      
      ++$sofar;
      $email = $user["email"];
      //$email = "rey4@columbia.edu";
      
      $address = $email ? ($studentname ? "$studentname <$email>" : $email) : "";
      $from = $this->from->text;
      $replyto = $this->replyto->text;

      if ($send)
      {
        if ($address)
        {
          taskwindow_cprint("[ $sofar  /  $total  ] Sending to " . htmlspecialchars($address) . " <br>\n");
          mail($address, $this->subject->text, $text, "From: $from\nReply-To: $replyto\nX-Mailer: PHP/" . phpversion());
        }  
        else
        {
          taskwindow_cprint("<font color=red>[ $sofar / $total ] Missing email address for UNI " . $user["cunix"] . "</font><br>\n");
          print("<p><strong><font color=red>Missing email address for UNI " . $user["cunix"] . "</font></strong></p>\n");
        }  
        if ($sofar % 5 == 0) { taskwindow_flush(); }  
      }
      else
      {  
        print("<pre>\n");
        print("<b>Subject:</b> " . htmlspecialchars($this->subject->text) . "\n");  
        print("<b>From:</b> " . htmlspecialchars($from) . "\n");  
        print("<b>Reply To:</b> " . htmlspecialchars($replyto) . "\n");  
        print("<b>To:</b> " . htmlspecialchars($address) . "\n\n");  
        print(htmlspecialchars($text));
        print("</pre>\n<hr>\n");
      }  
    }
    if ($send)
    {
      taskwindow_end("Progress Window");
      print("<h3>Done.</h3>");
    }  
    if ($send) db_addrow($db, "sentmails", array("sfrom" => $this->from->text, "sto" => $this->to, "replyto" => $this->replyto->text, "title" => $this->subject->text, "body" => $this->text->text));
  }
}

login_protect(login_administrator);
page_top("Mass Emailer");


$db = wces_connect();

$mm = new MassEmail($db,"mm","f",WIDGET_POST);
$mm->loadvalues();

print("<form name=f method=post action=massmail.php>\n");
$mm->display();
print("</form>\n");

page_bottom();
%>
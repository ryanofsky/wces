<?
require_once("wces/page.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
login_protect(login_administrator);

$MassEmail_students = array
(
  "all" => "All Students",
  "dumb" => "Students who completed no surveys",
  "bad" => "Students who completed some surveys",
  "good" => "Students who completed all of their surveys",
  "prof" => "Professors"
);

define("MassEmail_send",1);
define("MassEmail_preview",2);

class MassEmail extends FormWidget
{
  var $from;
  var $replyto;
  var $to;
  var $text;
  var $survey_category_id;

  var $action;
  var $errors;
  var $form;

  function MassEmail($prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);

    $this->form = new Form("${prefix}_form", $form, $formmethod);
    $this->from = new TextBox(0,60,"", "${prefix}_from", $form, $formmethod);
    $this->replyto = new TextBox(0,60,"", "${prefix}_replyto", $form, $formmethod);
    $this->subject = new TextBox(0,60,"", "${prefix}_subject", $form, $formmethod);
    $this->text = new TextBox(15,60,"", "${prefix}_text", $form, $formmethod);
    $this->action = new ActionButton("${prefix}_action", $form, $formmethod);
    $this->errors = array();
    $this->survey_category_id = 0;
  }

  function loadvalues()
  {
    global $MassEmail_students, $wces;

    $this->form->loadvalues();

    if ($this->form->isstale)
    {
      $this->from->loadvalues();
      $this->replyto->loadvalues();
      $this->subject->loadvalues();
      $this->text->loadvalues();
      $this->action->loadvalues();
      $this->to = $this->loadattribute("to");
      $this->survey_category_id = $this->loadattribute("survey_category_id");
    }
    else
    {
      wces_connect();

      $user_id = login_getuserid();
      $name = login_getname();
      $name = ucwords(strtolower($name));
      $email = pg_result(pg_query("SELECT email FROM users WHERE user_id = $user_id", $wces, __FILE__, __LINE__),0,0);
      if ($email)
      {
        if ($name)
          $this->from->text = "$name <$email>";
        else
          $this->from->text = $email;
      }
      $this->to = "";
      $this->replyto->text = "registrar@thayer.dartmouth.edu";
      $this->subject->text = "WCES Reminder";
	/*
      $this->text->text = "Dear %name%,\n\nCome to http://eval.thayer.dartmouth.edu/ so you can rate these %nmissingclasses% classes:\n\n%missingclasses%\n\nWin prizes!";
	*/


     $default = "You are receiving this message because you are enrolled in one or more courses at Thayer School of Engineering this term.  We would greatly appreciate if you could take a few moments to fill out course evaluations for the classes you are taking with us. \n\n";

	$default = $default . "The course evaluations are available online.  To submit evaluations, simply point your web browser at: http://eval.thayer.dartmouth.edu/ \n\n When you arrive log in and then follow the directions on the page to evaluate your courses. \n\n";

	$default = $default . "In consideration of the time it takes to complete the course evaluations, and in appreciation of your input, Thayer School will be giving away a Palm Pilot (Palm M105).  Every student who completes the course evaluations for all of their Thayer School classes this term will automatically be entered into a drawing for this prize.  The drawing will take place on 11-Mar-2002, and the winner will be notified via email. \n\n";

	$default = $default . "We would greatly appreciate your assistance! Your evaluations are very important to us, since they help us to measure and improve the quality of the courses we offer here at Thayer School.";



	$this->text->text = $default;

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
    global $MassEmail_students, $wces;
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
      $this->preserveattribute("survey_category_id");
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
?>
<p>From this page you can send customized emails to students and professors in classes being surveyed this semester.</p>
<p>Use the <strong>To:</strong> drop down box to select who the recipients will be.</p>
<p>The following variables can be used in the subject and message body. They will be replaced by with user information taken from the WCES database:</p>
<div style="background: #EEEEEE">
<pre>
  <strong>%name%</strong>             - The full name of the recipient.
  <strong>%classes%</strong>          - A list of the recipient's classes that have surveys available.
  <strong>%nclasses%</strong>         - The number of the recipient's classes that have surveys available.
</pre>
</div>  
<p>These variables work for emails to students only, not professors:</p>  
<div style="background: #EEEEEE">
<pre>
  <strong>%missingclasses%</strong>   - A list of classes that the student has not filled out surveys for.
  <strong>%finishedclasses%</strong>  - A list of classes that the student has filled out surveys for.
  <strong>%nmissingclasses%</strong>  - The number of classes that the student has not filled out surveys for.
  <strong>%nfinishedclasses%</strong> - The number of classes that the student has filled out surveys for.
</pre>
</div>
<hr>
<table>
  <tr>
    <td valign=top align=right><STRONG>From:</STRONG></td>
    <td><? $this->from->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Reply To:</STRONG></td>
    <td><? $this->replyto->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>To:</STRONG></td>
    <td><?
      print("<select name=\"${prefix}_to\">");
      foreach($MassEmail_students as $key => $label)
      {
        $selected = $key == $this->to ? " selected" : "";
        print("<option value=\"$key\"$selected>$label</option>");
      }
      print("</select><br>\n");

      wces_connect();
      $survey_categories = pg_query("SELECT survey_category_id, name FROM survey_categories", $wces, __FILE__, __LINE__);
      $n = pg_numrows($survey_categories);

      print("<select name=\"${prefix}_survey_category_id\">");
      print("<option value=\"0\"$selected>All Class Categories</option>");
      for($i=0; $i<$n; ++$i)
      {
        extract(pg_fetch_array($survey_categories,$i,PGSQL_ASSOC));
        $selected = $survey_category_id == $this->survey_category_id ? " selected" : "";
        print("<option value=\"$survey_category_id\"$selected>$name Students</option>");
      }
      print("</select>\n");
    ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Subject:</STRONG></td>
    <td><? $this->subject->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Text:</STRONG></td>
    <td><? $this->text->display(); ?></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td><? $this->action->display("Send", MassEmail_send); ?> <? $this->action->display("Preview", MassEmail_preview); ?>  </td>
  </tr>
</table>
<?
    }
  }

  // TODO: domail() is a very quick and dirty port to postgres, needs to be optimized and tested for bugs

  function domail($send = false)
  {
    global $wces;
    
    wces_connect();
    if ($send)
    {
      $user_id = login_getuserid();
      $from = addslashes($this->from->text);
      $reply_to = addslashes($this->replyto->text);
      $mail_to = addslashes($this->to) . " " . addslashes($this->survey_category_id);
      $subject = addslashes($this->subject->text);
      $body = addslashes($this->text->text);

      pg_query("
        INSERT INTO sent_mails (user_id, mail_from, reply_to, mail_to, subject, body)
        VALUES ($user_id, '$from', '$reply_to', '$mail_to', '$subject', '$body');
      ", $wces, __FILE__, __LINE__); 
      
    }
    
    $result = pg_query("
      SELECT question_period_id, displayname, year, semester
      FROM semester_question_periods
      WHERE question_period_id = (SELECT get_question_period())
    ", $wces, __FILE__, __LINE__);
    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
    
    $cat = $this->survey_category_id ? "AND t.category_id = $this->survey_category_id" : "";

    $status = $this->to == "prof" ? 3 : 1;

    $result = pg_query("
      CREATE TEMPORARY TABLE studclasses AS
      SELECT e.class_id, e.user_id, " . ($status == 1 ? "CASE WHEN COUNT(DISTINCT s.response_id) > 0 THEN 1 ELSE 0 END" : "1") . " AS surveyed
      FROM wces_topics AS t
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = $status " . ($status == 1 ? "
      LEFT JOIN survey_responses AS s ON (s.user_id = e.user_id AND s.topic_id = t.topic_id AND s.question_period_id = $question_period_id)" : "") . "
      WHERE cl.year = $year AND cl.semester = $semester $cat
      GROUP BY e.class_id, e.user_id
    ", $wces, __FILE__, __LINE__);      

    if ($this->to == "good")
      $having = "HAVING SUM(sc.surveyed) >= COUNT(DISTINCT sc.class_id)";
    else if ($this->to == "dumb")
      $having = "HAVING SUM(sc.surveyed) = 0";
    else if ($this->to == "bad")
      $having = "HAVING 0 < SUM(sc.surveyed) AND SUM(sc.surveyed) < COUNT(DISTINCT sc.class_id)";
    else
      $having = "";

    pg_query("
      CREATE TEMPORARY TABLE recipients AS
      SELECT user_id, COUNT(DISTINCT sc.class_id) AS surveys, SUM(sc.surveyed) AS surveyed
      FROM studclasses AS sc GROUP BY sc.user_id
      $having
    ",$wces, __FILE__, __LINE__);

    $users = pg_query("
      SELECT r.user_id, r.surveys, r.surveyed, u.uni AS cunix, u.email, u.firstname || ' ' || u.lastname AS name
      FROM recipients AS r
      INNER JOIN users AS u USING (user_id)
      GROUP BY r.user_id, r.surveys, r.surveyed, u.uni, u.email, u.firstname, u.lastname
      ORDER BY r.user_id
    ", $wces, __FILE__, __LINE__);

    $classes = pg_query("
      SELECT sc.user_id AS cluser_id, sc.surveyed, cl.class_id, cl.section, c.code, c.name AS cname, s.code AS scode, p.firstname || ' ' || p.lastname as pname
      FROM recipients AS r
      INNER JOIN studclasses AS sc USING (user_id)
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN courses AS c USING (course_id)
      INNER JOIN subjects AS s USING (subject_id)
      LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND status = 3
      LEFT JOIN users AS p ON p.user_id = e.user_id
      GROUP BY sc.user_id, cl.class_id, e.user_id, cl.section, c.code, c.name, s.code, p.firstname, p.lastname, sc.surveyed
      ORDER BY sc.user_id
    ", $wces, __FILE__, __LINE__);

    $total = pg_numrows($users);
    $cltotal = pg_numrows($classes);
    $sofar = 0;

    if ($send)
    {
      taskwindow_start("Progress Window", false);
      print("<h3>Sending...</h3>");
    }
    else
    {
      print("<h5>Previewing $total messages.</h5>");
    }

    $class_num = 0;
    $class_row = pg_fetch_array($classes,$class_num,PGSQL_ASSOC);
    $class_count = pg_numrows($classes);
    for($userno = 0; $userno < $total; ++$userno)
    {
      $user = pg_fetch_array($users,$userno,PGSQL_ASSOC);
      $s_name = ucwords(strtolower($user["name"]));
      $s_nclasses = $user["surveys"];
      $s_nfinishedclasses = $user["surveyed"];
      $s_nmissingclasses = $s_nclasses - $s_nfinishedclasses;
      $s_missingclasses = "";
      $s_finishedclasses = "";
      $class_str = "";
      
      while($class_row && $user['user_id'] == $class_row['cluser_id'])
      {
        $next_row = ++$class_num < $class_count ? pg_fetch_array($classes,$class_num,PGSQL_ASSOC) : NULL;

        if (!$class_str) $class_str = $class_row["scode"] . $class_row["code"] . ' ' . $class_row["cname"] . ' Section ' . $class_row["section"];
        $class_str .= ($class_row["pname"] ? "\nProfessor " . $class_row["pname"] : '');
        
        if ($class_row['class_id'] != $next_row['class_id'] || $user['user_id'] != $class_row['cluser_id'])
        {
          if ($class_row["surveyed"]) $cl = &$s_finishedclasses; else $cl = &$s_missingclasses;
          if ($cl) $cl .= "\n\n";
          $cl .= " * " .  str_replace("\n", "\n   ", wordwrap($class_str, 70, "\n"));
          $class_str = "";
        }
        $class_row = $next_row;
      }

      $s_classes = $s_finishedclasses;
      if ($s_missingclasses)
      {
        if ($s_classes) $s_classes .= "\n\n";
        $s_classes .= $s_missingclasses;
      }

      $names = array("%name%", "%classes%", "%nclasses%", "%missingclasses%", "%finishedclasses%", "%nmissingclasses%", "%nfinishedclasses%");
      $vals = array($s_name, $s_classes, $s_nclasses, $s_missingclasses, $s_finishedclasses, $s_nmissingclasses, $s_nfinishedclasses);
      $text = wordwrap(str_replace($names, $vals, $this->text->text), 75);

      ++$sofar;
      $email = $user["email"];
      $address = $email ? ($s_name ? "$s_name <$email>" : $email) : "";
      $from = $this->from->text;
      $replyto = $this->replyto->text;

      if ($send)
      {
        if ($address)
        {
          taskwindow_cprint("[ $sofar  /  $total  ] Sending to " . htmlspecialchars($address) . " <br>\n");
          //$email = "rey4@columbia.edu"; // debug
          mail($email, $this->subject->text, $text, "From: $from\nReply-To: $replyto\nTo: $address\nX-Mailer: PHP/" . phpversion());
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
  }
}

page_top("Mass Emailer");

$mm = new MassEmail("mm","f",WIDGET_POST);
$mm->loadvalues();

print("<form name=f method=post action=massmail.php>$ISID\n");
$mm->display();
print("</form>\n");

page_bottom();
?>
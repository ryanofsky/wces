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
      $this->replyto->text = "admin@thayer.dartmouth.edu";
      $this->subject->text = "WCES Reminder";
	/*
      $this->text->text = "Dear %studentname%,\n\nCome to http://eval.thayer.dartmouth.edu/ so you can rate these %nmissingclasses% classes:\n\n%missingclasses%\n\nWin prizes!";
	*/


     $default = "You are receiving this message because you are enrolled in one or more courses at Thayer School of Engineering this term.  We would greatly appreciate if you could take a few moments to fill out course evaluations for the classes you are taking with us. \n\n";

	$default = $default . "The course evaluations are available online.  To submit evaluations, simply point your web browser at: http://eval.thayer.dartmouth.edu/ \n\n When you arrive log in and then follow the directions on the page to evaluate your courses. \n\n";

	$default = $default . "In consideration of the time it takes to complete the course evaluations, and in appreciation of your input, Thayer School will be giving away a Palm Pilot (Palm M105).  Every student who completes the course evaluations for all of their Thaye School classes this term will automatically be entered into a drawing for this prize.  The drawing will take place on 11-Mar-2002, and the winner will be notified via email. \n\n";

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
      print("<option value=\"0\"$selected>All Students</option>");
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
    
    $result = pg_query("
      SELECT question_period_id, displayname, year, semester
      FROM semester_question_periods
      WHERE question_period_id = (SELECT get_question_period())
    ", $wces, __FILE__, __LINE__);
    extract(pg_fetch_array($result,0,PGSQL_ASSOC));
    
    $cat = $this->survey_category_id ? "AND t.category_id = $this->survey_category_id" : "";

    $result = pg_query("
      CREATE TEMPORARY TABLE studclasses AS
      SELECT e.class_id, e.user_id, CASE WHEN COUNT(DISTINCT s.response_id) > 0 THEN 1 ELSE 0 END AS surveyed
      FROM wces_topics AS t
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status = 1
      LEFT JOIN survey_responses AS s ON (s.user_id = e.user_id AND s.topic_id = t.topic_id AND s.question_period_id = $question_period_id)
      WHERE cl.year = $year AND cl.semester = $semester $cat
      GROUP BY e.class_id, e.user_id
    ", $wces, __FILE__, __LINE__);      

    if ($this->to == "good")
      $having = "HAVING bob <= SUM(sc.surveyed)";
    else if ($this->to == "dumb")
      $having = "HAVING 0 = SUM(sc.surveyed)";
    else if ($this->to == "bad")
      $having = "HAVING bob > SUM(sc.surveyed) AND SUM(sc.surveyed) > 0";
    else
      $having = "";

    pg_query("
      CREATE TEMPORARY TABLE recipients AS
      SELECT user_id, COUNT(DISTINCT sc.class_id) AS bob
      FROM studclasses AS sc GROUP BY sc.user_id
      $having
    ",$wces, __FILE__, __LINE__);

    $users = pg_query("
      SELECT u.uni AS cunix, sc.user_id AS userid, u.email, u.firstname || ' ' || u.lastname AS name, COUNT(DISTINCT sc.class_id) AS surveys, SUM(sc.surveyed) AS surveyed
      FROM studclasses AS sc
      INNER JOIN recipients AS r USING (user_id)
      INNER JOIN users AS u USING (user_id)
      GROUP BY sc.user_id, u.uni, u.email, u.firstname, u.lastname
      ORDER BY sc.user_id
    ", $wces, __FILE__, __LINE__);

    $classes = pg_query("
      SELECT sc.user_id AS cluserid, cl.section, c.code, c.name AS cname, s.code AS scode, p.firstname || ' ' || p.lastname as pname, sc.surveyed
      FROM studclasses AS sc
      INNER JOIN recipients AS r USING (user_id)
      INNER JOIN classes AS cl ON cl.class_id = sc.class_id
      INNER JOIN courses AS c USING (course_id)
      INNER JOIN subjects AS s USING (subject_id)
      LEFT JOIN enrollments AS e ON e.class_id = cl.class_id AND status = 3
      LEFT JOIN users AS p ON p.user_id = e.user_id
      GROUP BY sc.user_id, sc.class_id, cl.section, c.code, c.name, s.code, p.firstname, p.lastname, sc.surveyed
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

    $classno = 0;
    $classe = pg_fetch_array($classes,$classno,PGSQL_ASSOC);

    for($userno = 0; $userno < $total; ++$userno)
    {
      $user = pg_fetch_array($users,$userno,PGSQL_ASSOC);
      $studentname = ucwords(strtolower($user["name"]));
      $nallclasses = $user["surveys"];
      $nfinishedclasses = $user["surveyed"];
      $nmissingclasses = $nallclasses - $nfinishedclasses;
      $missingclasses = "";
      $finishedclasses = "";

      while($user['userid'] == $classe['cluserid'])
      {
        if ($classe["surveyed"]) $thelist = &$finishedclasses; else $thelist = &$missingclasses;
        if ($thelist) $thelist .= "\n\n";
        $thelist .= " * " .  str_replace("\n", "\n   ", wordwrap($classe["scode"] . $classe["code"] . ' ' . $classe["cname"] . ' Section ' . $classe["section"].  ($classe["pname"] ? "\nProfessor " . $classe["pname"] : ''), 70, "\n"));
        $classe = pg_fetch_array($classes,++$classno,PGSQL_ASSOC);
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

      $address = $email ? ($studentname ? "$studentname <$email>" : $email) : "";
      $from = $this->from->text;
      $replyto = $this->replyto->text;

      if ($send)
      {
        if ($address)
        {
          taskwindow_cprint("[ $sofar  /  $total  ] Sending to " . htmlspecialchars($address) . " <br>\n");
          $address = "admin@thayer.dartmouth.edu"; // debug
          // mail($address, $this->subject->text, $text, "From: $from\nReply-To: $replyto\nX-Mailer: PHP/" . phpversion());
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
    // todo: make a new table to keep track of sent mails
    //if ($send) db_addrow($db, "sentmails", array("sfrom" => $this->from->text, "sto" => $this->to, "replyto" => $this->replyto->text, "title" => $this->subject->text, "body" => $this->text->text));
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
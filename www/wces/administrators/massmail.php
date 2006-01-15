<?
require_once("wces/page.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
require_once("widgets/SqlBox.inc");
require_once("wces/TopicEditor.inc");

LoginProtect(LOGIN_ADMIN);

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
define("MassEmail_back",3);
define("MassEmail_choose", 4);
define("MassEmail_load", 5);

class MassEmail extends ParentWidget
{
  var $from;
  var $replyto;
  var $to;
  var $text;
  var $category_id;

  var $event;
  var $errors;
  var $form;

  function MassEmail($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->from =& new TextBox(0,60,"", "from", $this);
    $this->replyto =& new TextBox(0,60,"", "replyto", $this);
    $this->subject =& new TextBox(0,60,"", "subject", $this);
    $this->text =& new TextBox(15,60,"", "text", $this);

    $this->question_periods =& new QuestionPeriodSelector('', 'question_period', $this);

    $this->topics =& new SqlBox(null, true, 'topics', $this);
    $this->topics->format = "format_class";
    $this->topics->selected = array(0);

    $this->event =& new EventWidget("event", $this);
    $this->errors = array();
    $this->category_id = 0;
  }

  function loadInitialState()
  {
    global $MassEmail_students, $wces, $server_feedback;
    wces_connect();

    $user_id = LoginValue('user_id');
    $name = LoginValue('name');
    $name = ucwords(strtolower($name));
    $email = pg_result(pg_go("SELECT email FROM users WHERE user_id = $user_id", $wces, __FILE__, __LINE__),0,0);
    if ($email)
    {
      if ($name)
        $this->from->text = "$name <$email>";
      else
        $this->from->text = $email;
    }
    $this->to = "";
    $this->replyto->text = $server_feedback;
    $this->subject->text = "WCES Reminder";
    $this->text->text = "Dear %name%,\n\nCome to http://oracle.seas.columbia.edu/ so you can rate these %nmissingclasses% classes:\n\n%missingclasses%\n\nWin prizes!";
  }

  function loadState($new)
  {
    assert(isset($new));
    ParentWidget::loadState($new);
    if ($new) return $this->loadInitialState();
    
    global $wces;
    $this->to = $this->readValue("to");
    $this->category_id = (int)$this->readValue("category_id");

    if ($this->event->event == MassEmail_load && $this->event->param)
    {
      $sent_mail_id = (int)$this->event->param;
      $r = pg_go("
        SELECT sent_mail_id, mail_from, reply_to, mail_to, subject, body, category_id, question_period_id
        FROM sent_mails WHERE sent_mail_id = $sent_mail_id
      ", $wces, __FILE__, __LINE__);
      assert(pg_numrows($r) == 1);
      
      $row = pg_fetch_row($r, 0, PGSQL_ASSOC);
      
      $this->from->text = $row['mail_from'];
      $this->replyto->text = $row['reply_to'];
      $this->subject->text = $row['subject'];
      $this->text->text = $row['body'];
      $this->to = $row['mail_to'];
      $this->category_id = (int)$row['category_id'];
      if ($row['question_period_id'])
        set_question_period($row['question_period_id']);
      
      $r = pg_go("
        SELECT topic_id
        FROM sent_mails_topics WHERE sent_mail_id = $sent_mail_id
      ", $wces, __FILE__, __LINE__);        
      $n = pg_numrows($r);
      $s = array();
      for ($i = 0; $i < $n; ++$i)
        $s[] = pg_result($r, $i, 0);
      $this->topics->selected = count($s) ? $s : array(0);
    }
  }

  function checkErrors()
  {
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
  }

  function printVisible()
  {
    global $MassEmail_students, $wces;

    $go = $this->event->event == MassEmail_preview || $this->event->event == MassEmail_send;

    if ($go && count($this->errors) == 0)
    {
      $this->event->displayButton("Back", MassEmail_back);
      print("<hr>");
      $this->domail($this->event->event == MassEmail_send);
      print("<hr>");
      $this->event->displayButton("Back", MassEmail_back);

      $this->from->displayHidden();
      $this->replyto->displayHidden();
      $this->subject->displayHidden();
      $this->text->displayHidden();
      $this->question_periods->displayHidden();
      $this->topics->displayHidden();
      $this->printValue("to", $this->to);
      $this->printValue("category_id", $this->category_id);
    }
    else if ($this->event->event == MassEmail_choose)
    {
      $this->from->displayHidden();
      $this->replyto->displayHidden();
      $this->subject->displayHidden();
      $this->text->displayHidden();
      $this->question_periods->displayHidden();
      $this->topics->displayHidden();      
      $this->printValue("to", $this->to);
      $this->printValue("category_id", $this->category_id);

      print('Open a previously sent message to edit and resend it, or choose '
        .  '"Cancel" to return to the mass mailing form.');
      print("<p>");
      $this->event->displayButton('Cancel', MassEmail_back);
      print("</p>");
      
      $r = pg_go("
        SELECT category_id AS id, name FROM categories
      ", $wces, __FILE__, __LINE__);
      
      $cats = array();
      
      $n = pg_numrows($r);
      for($i = 0; $i < $n; ++$i)
      {
        $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
        $cats[$row['id']] = $row['name']; 
      }
      
      $r = pg_go("
        SELECT m.sent_mail_id, EXTRACT(EPOCH FROM m.sent) AS date,
          m.mail_from, m.reply_to, m.mail_to, m.subject, m.body, u.uni, c.name AS cat, q.displayname
        FROM sent_mails AS m
        LEFT JOIN users AS u USING (user_id)
        LEFT JOIN question_periods AS q USING (question_period_id)
        LEFT JOIN categories AS c ON c.category_id = m.category_id
        ORDER BY m.sent DESC
      ", $wces, __FILE__, __LINE__);
      
      $n = pg_numrows($r);
      
      for($i = 0; $i < $n; ++$i)
      {
        $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
        
        $toa = explode(" ", $row['mail_to']);

        $to = $row['cat'];
        @$to .= ' ' . $MassEmail_students[$row['mail_to']];
        
        print("<hr>\n");
        print("<p>");
        $this->event->displayButton("Open this email.", MassEmail_load, $row['sent_mail_id']);
        print("</p>\n<pre>");
        print("<b>Question Period:</b> " . htmlspecialchars($row['displayname']) . "\n");
        print("<b>Date:</b> " . date('l, F j, Y g:i a', $row['date']) . "\n");
        print("<b>Sent By:</b> " . htmlspecialchars($row['uni']) . "\n");
        print("<b>To:</b> " . htmlspecialchars($to) . "\n");
        print("<b>From:</b> " . htmlspecialchars($row['mail_from']) . "\n");
        print("<b>Reply To:</b> " . htmlspecialchars($row['reply_to']) . "\n");
        print("<b>Subject:</b> " . htmlspecialchars($row['subject']) . "\n\n");
        print(htmlspecialchars(wordwrap($row['body'])));
        print("</pre>\n");
      }
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
    <td>&nbsp;</td>
    <td><? $this->event->displayButton("Load a previously sent email...", MassEmail_choose); ?></td>
  </tr>
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
      print("<select name=\"" . $this->name('to') . "\">");
      foreach($MassEmail_students as $key => $label)
      {
        $selected = $key == $this->to ? " selected" : "";
        print("<option value=\"$key\"$selected>$label</option>");
      }
      print("</select><br>\n");

      wces_connect();
      $survey_categories = pg_go("SELECT category_id, name FROM categories", $wces, __FILE__, __LINE__);
      $n = pg_numrows($survey_categories);

      print("<select name=\"" . $this->name('category_id') . "\">");
      print("<option value=0$selected>All Class Categories</option>");
      for($i=0; $i<$n; ++$i)
      {
        extract(pg_fetch_array($survey_categories,$i,PGSQL_ASSOC));
        $selected = $category_id == $this->category_id ? " selected" : "";
        print("<option value=$category_id$selected>$name Classes</option>");
      }
      print("</select><br>\n");
      
      $this->question_periods->display();

      $question_period_id = get_question_period();

      $this->topics->sql = "
        SELECT 0 AS id, 'All Classes' AS name, 0 as ordinal
      ";

      if ($question_period_id)
      {
        $this->topics->sql .= "
          UNION
          SELECT topic_id, get_class(class_id), 1
          FROM wces_topics
          WHERE question_period_id = " . get_question_period() . "
          ORDER BY ordinal, name
        ";
      }
      $this->topics->display("size=4 multiple");
      
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
    <td><? $this->event->displayButton("Send", MassEmail_send); ?> <? $this->event->displayButton("Preview", MassEmail_preview); ?>  </td>
  </tr>
</table>
<?
    }
  }

  
  function doMail($send = false)
  {
    global $wces;

    $topics = $this->topics->selected;
    assert(is_array($topics));
    if (in_array(0, $topics)) $topics = array();
    
    wces_connect();
    if ($send)
    {
      $user_id = LoginValue('user_id');
      $from = addslashes($this->from->text);
      $reply_to = addslashes($this->replyto->text);
      $mail_to = addslashes($this->to);
      $category_id = $this->category_id ? (int)$this->category_id : 'NULL';
      $subject = addslashes($this->subject->text);
      $body = addslashes($this->text->text);
      $question_period_id = get_question_period();

      $r = pg_go("
        BEGIN;
        INSERT INTO sent_mails (user_id, mail_from, reply_to, mail_to, subject, body, question_period_id, category_id)
        VALUES ($user_id, '$from', '$reply_to', '$mail_to', '$subject', '$body', $question_period_id, $category_id);
        SELECT currval('sent_mail_ids');
      ", $wces, __FILE__, __LINE__); 
      
      $sent_mail_id = pg_result($r, 0, 0);
      
      foreach($topics AS $topic_id)
      {
        pg_go("
          INSERT INTO sent_mails_topics (sent_mail_id, topic_id)
          VALUES ($sent_mail_id, $topic_id);
        ", $wces, __FILE__, __LINE__); 
      }

      pg_go("COMMIT;", $wces, __FILE__, __LINE__); 
    }
    
    $cat = $this->category_id ? "AND t.category_id = $this->category_id" : "";
    $topic = count($topics) ? ("AND t.topic_id IN (" . implode(",", $topics) . ")") : "";
    
    if ($this->to == "prof")
    {
      $je = "INNER JOIN enrollments_p AS e ON e.class_id = cl.class_id";
      $surveyed = "1";
      $responses = "";
    }
    else
    {
      $je = "INNER JOIN enrollments AS e ON e.class_id = cl.class_id AND e.status & 1 <> 0";
      $surveyed = "CASE WHEN COUNT(DISTINCT s.user_id) > 0 THEN 1 ELSE 0 END";
      $responses = "LEFT JOIN responses_survey AS s ON s.user_id = e.user_id AND s.topic_id = t.topic_id";
    }
    
    $result = pg_go("
      CREATE TEMPORARY TABLE studclasses AS
      SELECT e.class_id, e.user_id, $surveyed AS surveyed
      FROM wces_topics AS t
      INNER JOIN classes AS cl USING (class_id)
      $je
      INNER JOIN users AS u ON u.user_id = e.user_id AND u.flags & 128 = 0
      $responses
      WHERE t.question_period_id = " . get_question_period() . " $cat $topic
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

    pg_go("
      CREATE TEMPORARY TABLE recipients AS
      SELECT user_id, COUNT(DISTINCT sc.class_id) AS surveys, SUM(sc.surveyed) AS surveyed
      FROM studclasses AS sc GROUP BY sc.user_id
      $having
    ",$wces, __FILE__, __LINE__);

    $users = pg_go("
      SELECT r.user_id, r.surveys, r.surveyed, u.uni AS cunix, u.email, u.firstname || ' ' || u.lastname AS name
      FROM recipients AS r
      INNER JOIN users AS u USING (user_id)
      GROUP BY r.user_id, r.surveys, r.surveyed, u.uni, u.email, u.firstname, u.lastname
      ORDER BY r.user_id
    ", $wces, __FILE__, __LINE__);

    $classes = pg_go("
      SELECT sc.user_id AS cluser_id, sc.surveyed, cl.class_id, cl.section, c.code, c.name AS cname, s.code AS scode, p.firstname || ' ' || p.lastname as pname
      FROM recipients AS r
      INNER JOIN studclasses AS sc USING (user_id)
      INNER JOIN classes AS cl USING (class_id)
      INNER JOIN courses AS c USING (course_id)
      INNER JOIN subjects AS s USING (subject_id)
      LEFT JOIN enrollments_p AS e ON e.class_id = cl.class_id
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
    $class_count = pg_numrows($classes);
    $class_row = $class_num < $class_count ? pg_fetch_array($classes,$class_num,PGSQL_ASSOC) : NULL;
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
        
        // handle classes with more than one professor
        if ($class_row['class_id'] != $next_row['class_id'] || $user['user_id'] != $next_row['cluser_id'])
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
          
          // XXX: need to add Errors-To: header to set the address that
          // postfix uses as the FROM address when connecting to the
          // recieving SMTP server, and which also gets tacked
          // on as the message's Return-Path
          
          safe_mail($address, $this->subject->text, $text, "From: $from\nReply-To: $replyto\nX-Mailer: PHP/" . phpversion());
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
$f =& new Form('f');
$t =& new InitializerWidget('init', $f);
$mm =& new MassEmail("mm", $t);
$f->loadState();

print("<form name=f method=post event=massmail.php>$ISID\n");
$f->display();
$t->display();
$mm->display();
print("</form>\n");

page_bottom();
?>

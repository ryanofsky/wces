<?
require_once("wces/page.inc");
require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

define("MassEmail_send",1);
define("MassEmail_preview",2);

function db_show($result,$name = "Result", $showtypes = false)
{
  if ($result)
  {
    $rows = mysql_num_rows($result);
    $cols = mysql_num_fields($result);
    if ($cols)
    {
      print ("<table border=1>\n");
      print("<tr><td colspan=$cols>$name</td></tr>\n");
      print("<tr>\n");
      for($c=0; $c < $cols; ++$c)
        print("  <td><b>" . htmlspecialchars(mysql_field_name($result,$c)) . " (" . htmlspecialchars(mysql_field_type($result,$c)) . ")</b></td>\n");
      print("</tr>\n");
      
      while($row = mysql_fetch_row($result))
      {
        print("<tr>\n"); 
        for($c = 0; $c < $cols; ++$c)
        {
          if (!isset($row[$c]))
            $str = "<i><small><small>null</small></small></i>";
          else if (!$showtypes)
            $str = htmlspecialchars($row[$c]);
          else
            $str = htmlspecialchars($row[$c]) . " <small><small><i>" . gettype($row[$c]) . "</i></small></small>";  
          print("  <td>$str</td>\n");
        }
        print("</tr>\n");  
      }  
      print ("</table>\n");
      mysql_data_seek($result, 0);
      return;
    }
  }  
  print("<p><b>No Result</b></p>\n");
}

class MassEmail extends FormWidget
{
  var $from;
  var $replyto;
  var $to;
  var $text;
  var $topicid;
  
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
    $this->cc = new TextBox(0,60,"", "${prefix}_cc", $form, $formmethod);
    $this->subject = new TextBox(0,60,"", "${prefix}_subject", $form, $formmethod);
    $this->text = new TextBox(15,60,"", "${prefix}_text", $form, $formmethod);
    $this->action = new ActionButton("${prefix}_action", $form, $formmethod);
    $this->errors = array();
    $this->topicid = 0;
  }
  
  function loadvalues()
  {
    global $MassEmail_students;
    
    $this->form->loadvalues();
    
    if ($this->form->isstale)
    {
      $this->from->loadvalues();
      $this->replyto->loadvalues();
      $this->cc->loadvalues();
      $this->subject->loadvalues();
      $this->text->loadvalues();
      $this->action->loadvalues();
      $this->topicid = $this->loadattribute("topicid");
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
      $this->text->text = "Dear %profname%,\n\nCome to http://www.esurveys.columbia.edu/wces/ so you can see survey responses for these %nclasses% classes:\n\n%classes%\n\n";
    }  

    if (!emailvalid($this->from->text))
      $this->errors[] = "Invalid FROM address";

    if (!emailvalid($this->replyto->text))
      $this->errors[] = "Invalid REPLY-TO address";
       
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
    $db = $this->db;
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
      $this->preserveattribute("topicid");
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
<p>From this page you can send customized emails to professors for classes being surveyed this semester.</p>
<p>Use the <strong>To:</strong> drop down box to send emails to a specific group of professors.</p>
<p>The following variables can be used in the subject and message body. They will be replaced by professor data taken from the WCES database.</p>
<div style="background: #EEEEEE">
<pre>
  <strong>%profname%</strong> - The full name of the professor.
  <strong>%classes%</strong>  - A list of the professor's classes that have survey results available.
  <strong>%nclasses%</strong> - The number of the professor's classes that have survey results available.
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
    <td>
    <?
      print("<select name=\"${prefix}_topicid\">");
      $topics = db_exec("SELECT topicid, name FROM topics", $db, __FILE__, __LINE__);
      print("<option value=\"0\"$selected>All Professors</option>");
      while($topic = mysql_fetch_assoc($topics))
      {
        $topicid = $name = "";
        extract($topic);
        $selected = $topicid == $this->topicid ? " selected" : "";
        print("<option value=\"$topicid\"$selected>$name Professors</option>");
      }
      print("</select>\n");
    ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>CC:</STRONG></td>
    <td><? $this->cc->display(); ?></td>
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
  
  function domail($send = false)
  {
    $db = $this->db;
   
    db_exec("
      CREATE TEMPORARY TABLE pmail 
      SELECT p.professorid, cl.classid, IFNULL(p.email,u.email) AS email,
      p.name, cl.students, COUNT(DISTINCT cr.userid, cr.classid) AS responses,
      s.code AS scode, c.code AS ccode, c.name AS cname, cl.section
      FROM groupings AS g
      INNER JOIN classes AS cl ON g.linkid = cl.classid
      INNER JOIN courses AS c USING (courseid)
      INNER JOIN subjects AS s USING (subjectid)
      LEFT JOIN professors AS p ON p.professorid = cl.professorid      
      LEFT JOIN users AS u USING (userid)
      LEFT JOIN cheesyresponses AS cr ON cr.classid = cl.classid AND questionperiodid = 9
      WHERE g.linktype = 'classes'" .
      ($this->topicid ? "AND g.topicid = $this->topicid" : "") . "
      GROUP BY cl.classid
    ", $db, __FILE__, __LINE__);

//     $result = db_exec("SELECT * FROM pmail", $db, __FILE__, __LINE__);
//     db_show($result);
// 
// $users = db_exec("
//   SELECT professorid, name
//   FROM pmail
//   WHERE (email = '' OR email IS NULL) AND professorid IS NOT NULL
//   GROUP BY professorid
// ", $db, __FILE__, __LINE__);
// 
//     db_show($users);

//  $ds=ldap_connect("ldap.columbia.edu");
//  $r=ldap_bind($ds);
//  
//  $row = 0;
//  while($user = mysql_fetch_assoc($users))
//  {
//    $name = $userid = "";
//    extract($user);
//    $sr=ldap_search($ds,"o=Columbia University,c=US", "cn=$name");  
//    $info = ldap_get_entries($ds, $sr);
//    if (isset($info[0]) && $info["count"] == 1) 
//    {
//      $cn    = addslashes($info[0]["cn"][0]);
//      $title = addslashes($info[0]["title"][0]);
//      $ou    = addslashes($info[0]["ou"][0]);
//      $email = addslashes($info[0]["mail"][0]);
//      if ($email)
//        db_exec("UPDATE professors SET email = '$email' WHERE professorid = '$professorid'", $db, __FILE__, __LINE__);
//      else
//        $email = "<i>no email address found</i>"; 
//      print("<b>$name</b> - $email - $cn<br>\n");
//    }
//    else
//    {
//      print("<b>$name</b> - $info[count] records found<br>\n");
//      //printarray($info,"info");
//    }
//    if ((++$row) % 5 == 0) taskwindow_flush();
//  }
    
    $users = db_exec("
      SELECT email, name, COUNT(DISTINCT classid) AS classes
      FROM pmail
      GROUP BY professorid
      ORDER BY professorid
    ", $db, __FILE__, __LINE__);
    
    $classes = db_exec("
      SELECT scode, ccode, cname, section
      FROM pmail
      ORDER BY professorid, scode, ccode, cname, section
    ", $db, __FILE__, __LINE__);
    
    $total = mysql_num_rows($users);
    $sofar = 0;
    
    if ($send) 
    {
      taskwindow_start("Progress Window", false);
      print("<h3>Sending...</h3>");
    }
    else
      print("<h5>Previewing $total messages.</h5>");
    
    $cc = $this->cc->text ? "Cc: {$this->cc->text}\r\n" : "";
    while($user = mysql_fetch_assoc($users))
    {
      $vprofname = ucwords(strtolower($user["name"]));
      $vnclasses = (int) $user["classes"];
      $vclasses = "";
      for($i=0; $i < $vnclasses; ++$i)
      {
        $classe = mysql_fetch_assoc($classes);
        if ($vclasses) $vclasses .= "\n\n";
        $vclasses .= " * " . str_replace("\n", "\n   ", wordwrap("$classe[scode]$classe[ccode] $classe[cname] Section $classe[section]", 70, "\n"));
      }
      
      $fields = array("%profname%", "%nclasses%", "%classes%");
      $values = array($vprofname, $vnclasses, $vclasses);
      $text = str_replace($fields, $values, $this->text->text);
      $text = wordwrap($text, 75);
      
      ++$sofar;
      $email = $user["email"];
      $name = $user["name"];
      
      $address = $email ? ($name ? "$name <$email>" : $email) : "";
      $from = $this->from->text;
      $replyto = $this->replyto->text;

      if ($send)
      {
        if ($address)
        {
          taskwindow_cprint("[ $sofar  /  $total  ] Sending to " . htmlspecialchars($address) . " <br>\n");
          //$address = "rey4@columbia.edu";
          mail($address, $this->subject->text, $text, "From: $from\r\nReply-To: $replyto\r\n{$cc}X-Mailer: PHP/" . phpversion());
        }  
        else
        {
          taskwindow_cprint("<font color=red>[ $sofar / $total ] Missing email address for " . $user["name"] . "</font><br>\n");
          print("<p><strong><font color=red>Missing email address for " . $user["name"] . "</font></strong></p>\n");
        }  
        if ($sofar % 5 == 0) { taskwindow_flush(); }  
      }
      else
      {  
        print("<pre>\n");
        print("<b>Subject:</b> " . htmlspecialchars($this->subject->text) . "\n");  
        print("<b>From:</b> " . htmlspecialchars($from) . "\n");  
        print("<b>Reply To:</b> " . htmlspecialchars($replyto) . "\n");  
        print("<b>To:</b> " . htmlspecialchars($address) . "\n");  
        print("<b>CC:</b> " . htmlspecialchars($this->cc->text) . "\n\n");  
        print(htmlspecialchars($text));
        print("</pre>\n<hr>\n");
      }  
    }
    if ($send)
    {
      taskwindow_end("Progress Window");
      print("<h3>Done.</h3>");
    }  
    if ($send) db_addrow($db, "sentmails", array("sfrom" => $this->from->text, "sto" => "Professors (topic: $this->topicid)", "replyto" => $this->replyto->text, "title" => $this->subject->text, "body" => $this->text->text));
  }
}

login_protect(login_administrator);
page_top("Mass Emailer");


$db = wces_connect();

$mm = new MassEmail($db,"mm","f",WIDGET_POST);
$mm->loadvalues();

print("<form name=f method=post action=$PHP_SELF>\n$ISID");
$mm->display();
print("</form>\n");

page_bottom();
?>
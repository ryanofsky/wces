<?

require_once("wces/report_wizard.inc");
require_once("wces/report_generate.inc");

//---------------------------------------------------------------------------------------------
// Report Widget

define("Report_wizard",1); // state
define("Report_results",2); // state
define("Report_profemail",3); // state
define("Report_email",4); // state and event
define("Report_print",5); // event
define("Report_revivewizard",6); // event
define("Report_email_send",1);
define("Report_email_cancel",2);

class Report extends TabbedWidget
{
  var $wizard;
  var $hideMenus = false;
  var $currentEvent = null;

  function Report($name, &$parent)
  {
    $this->TabbedWidget($name, $parent);
    EventWidget::DumpScript();
    $this->wizard =& new ReportWizard('wizard', $this);
  }

  function & handleEvent($event, $param, $new)
  {
    switch($event)
    {
      case Report_print:
        $this->wizard->output->printable->checked = true;
      break;

      case Report_wizard;
      return $this->wizard;

      case Report_profemail:
        $this->profemail =& new ProfessorEmail($this->wizard, "profemail", $this);
        $this->loadChild($this->profemail, $new);      
      return $this->profemail;
      
      case Report_email:
        $this->email =& new ReportEmail($this->wizard, "reportemail", $this);
        $this->loadChild($this->email, $new);
      return $this->email;
    };
  }

  function loadState($new)
  {
    assert(isset($new));
    TabbedWidget::loadState($new);
    if ($new)
    {
      $this->eventLoop(Report_wizard, null, true, true);
      return;
    }
    
    $this->hideMenus = !isset($this->currentEvent) && $this->wizard->output->printable->checked;
    if (isset($this->email) && $this->currentEvent == Report_wizard)
      $this->email->generate = true;
  }

  function navigationButton($text, $href, $icon, $width, $height)
  {
    return "<a class=blank href=\"$href\">"
     . "<img src=\"$icon\" border=0 width=$width height=$height alt=\"$text\">"
     . " $text</a> ";
  }

  function display()
  {
    $this->printNavigation();
    TabbedWidget::display();      
  }

  function printNavigation()
  {
    global $wces_path;
    
    if ($this->currentEvent == Report_wizard) return; 

    $this->event->displayField();
    print($this->navigationButton("Back to the Reporting Wizard",
      $this->event->gethref(Report_wizard,0),  
      "{$wces_path}media/report/wizard.gif", 24, 22));
    
    if (isset($this->modalChild))
      $this->modalChild->displayNavigation($this);
    else
    {
      if ($this->wizard->output->printable->checked)
      {
        $str = addslashes($this->navigationButton("Print this report",
          "javascript:void(window.print())",
          "{$wces_path}media/report/print.gif", 20, 20));
  
        print("<script>\n<!--\nif (window.print) document.write('$str');\n"
          . "// -->\n</script>");
      }
      else
        print($this->navigationButton("Printable version of this Report",
          $this->event->gethref(Report_print,0), 
          "{$wces_path}media/report/print.gif", 20, 20));
    }

    print($this->navigationButton("Email this Report",
      $this->event->gethref(Report_email,0),  
      "{$wces_path}media/report/email.gif", 20, 20)); 
    
    print("<br>\n");   
  }

  function printVisible()
  {
    global $wces_path, $db;
    TabbedWidget::printVisible();
    $html = $text = false;
    makereport($this->wizard,true,$html,$text,array('taResponses' => 1));
  }
};

class ProfessorEmail extends ParentWidget
{
  var $profemail_subject;
  var $profemail_message;
  var $profemail_debug;
  var $profemail_event;
  var $done = false;
  var $wizard;
  
  function ProfessorEmail(&$wizard, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->wizard =& $wizard;
    $this->profemail_form = new Form($this->prefix . "_profemail_form", $this->form, $this->formmethod);
    $this->profemail_subject = new TextBox(1,76,"",$this->prefix . "_profemail_subject", $this->form, $this->formmethod);
    $this->profemail_message = new TextBox(8,76,"",$this->prefix . "_profemail_message", $this->form, $this->formmethod);
    $this->profemail_debug = new CheckBox($this->prefix . "_profemail_debug", $this->form, $this->formmethod);
    $this->profemail_event = new EventButton($this->prefix . "_profemail_event", $this->form, $this->formmethod);
  }
  
  function loadInitialState()
  {
    $this->profemail_subject->text = "WCES Evaluation Results";
    $this->profemail_message->text = "Dear %profname%,\n\nThis automated email contains the results of the WCES evaluations for your classes. Send questions or comments to wces@columbia.edu";
    $this->profemail_debug->checked = false;
  }
  
  function displayNavigation(&$widget)
  {
    if ($this->profemail_event->event)
    {
      $this->profemail_event->displayField();
      print($widget->navigationButton("Back to Professor Email", 
        $this->profemail_event->gethref(0,0),
        "{$wces_path}media/report/back.gif",20, 20));
    }
    print("</td></tr></table>\n");        
  }
  
  function printVisible()
  {
    global $wces_path, $db;
    if (!$db) $db = wces_connect();

    $this->profemail_form->display();

    if ($this->profemail_event->event)
    {
      $this->profemail_subject->displayHidden();
      $this->profemail_message->displayHidden();
      $this->profemail_debug->displayHidden();
      report_profemail($db, $this->wizard, $this->profemail_subject->text, $this->profemail_message->text, $this->profemail_debug->checked);
      return;
    }

    print("<h3>Email Report</h3>\n");
    print("<h4>Professors Included</h4>\n");
    $q = $this->wizard->query;
    $q->addcolumn("p.professorid, p.name AS pname, IFNULL(p.email,u.email) AS email, COUNT(DISTINCT cl.classid, qp.questionperiodid) AS pages");
    $q->addjoin("questionperiods AS qp",db_SelectMonster_inner,"qp.questionperiodid","a.questionperiodid");
    $q->addjoin("professors AS p",db_SelectMonster_inner,"p.professorid","cl.professorid");
    $q->addjoin("users AS u",db_SelectMonster_left,"u.userid","p.userid");
    $q->addgroup("p.professorid");
    $q->execute($db);
    print("<ul>\n");
    while($q->advance())
    {
      $email = "Unknown"; $name="Unknown";
      extract($q->row);
      if (!$email) $email = "Unknown";
      print("Professor <a href=\"{$wces_path}administrators/info.php?professorid=$professorid\">$pname</a> - email <a href=\"mailto:$email\">$email</a> - $pages classes<br>");
    };
    print("</ul>\n");
?>
<h4>Message</h4>
<table>
<tr>
  <td valign=top align=right><strong>Subject:</strong></td><td valign=top><? $this->profemail_subject->display(); ?></td>
</tr>
<tr>
  <td valign=top align=right><STRONG>Custom Message:</STRONG></td><td valign=top><? $this->profemail_message->display(); ?></td>
</tr>
<tr>
  <td>&nbsp;</td><td><? $this->profemail_debug->display(); ?> <? $this->profemail_debug->displayLabel("Debug Mode (doesn't actually send emails)"); ?></td>
</tr>
<tr>
  <td>&nbsp;</td><td><? $this->profemail_event->displayButton("Submit",1,0); ?></td>
</tr>
</table>
<?
  }
};

class ReportEmail extends ParentWidget
{
  var $email_to;
  var $email_from;
  var $email_subject;
  var $email_text;
  var $email_event;
  var $generate = true;
  var $done = false;

  function ReportEmail(&$wizard, $name, &$parent)
  {
    $this->wizard =& $wizard;
    $this->ParentWidget($name, $parent);
    $this->email_form    =& new Form("email_form", $this);
    $this->email_to      =& new TextBox(1, 40, "","email_to", $this);
    $this->email_from    =& new TextBox(1, 40, "","email_from", $this);
    $this->email_subject =& new TextBox(1, 80,"","email_subject", $this);
    $this->email_text    =& new TextBox(15, 80,"","email_text", $this);
    $this->email_event   =& new EventWidget("email_event", $this);
  }

  function displayNavigation(&$widget)
  {
    global $wces_path;
    $this->email_event->displayField();
    print($widget->NavigationButton("Back to Results",
      $this->email_event->gethref(Report_email_cancel, null),
      "{$wces_path}media/report/back.gif", 20, 20));
  }

  function loadState($new)
  {
    assert(isset($new));
    ParentWidget::loadState($new);
    if ($new) return;
    
    $this->generate = (bool)$this->readValue('generate');
    if ($this->email_event->event == Report_email_cancel)
      $this->done = true;
    if ($this->generate)      
      $this->email_text->text = null;
  }
  
  function printState()
  {
    if ($this->generate) $this->printValue('generate', 1);
  }

  function printVisible()
  {
    global $db;
    
    if ($this->generate)
    {
      $html = false; $text = "";
      makereport($this->wizard,false,$html,$text,array('taResponses' => 1));
      $this->email_text->text = $text;
      $this->generate = false;
    }    
    
    if ($this->email_event->event == Report_email_send)
    {
       $from = $this->email_from->text;
       $address = $this->email_to->text;
       $subject = $this->email_subject->text;
       $email = $this->email_text->text;
       print("<h3>Sending...</h3>");
       print("<pre>");
       print("<strong>From: $from</strong>\n");
       print("<strong>To: $address</strong>\n");
       print("<strong>Subject: $subject</strong>\n\n");
       print($email);
       print("</pre>");
       mail($address,$subject,$email, "From: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion());
       print("<h3>Done.</h3>");
    }
    else
    {
?>
<table>
  <tr>
    <td valign=top align=right><STRONG>To:</STRONG></td>
    <td><? $this->email_to->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>From:</STRONG></td>
    <td><? $this->email_from->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Subject:</STRONG></td>
    <td><? $this->email_subject->display(); ?></td>
  </tr>
  <tr>
    <td valign=top align=right><STRONG>Text:</STRONG></td>
    <td><? $this->email_text->display(); ?></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
    <td><? $this->email_event->displayButton("Send",Report_email_send,0); ?> <? $this->email_event->displayButton("Cancel",Report_email_cancel,0); ?></td>
  </tr>
</table>
<?
    }
  }
};

?>

<?

require_once("wces/report_wizard.inc");
require_once("wces/report_generate.inc");

//---------------------------------------------------------------------------------------------
// Report Widget

define("Report_wizard",1); // state
define("Report_results",2); // state
define("Report_profemail",3); // state
define("Report_email",4); // state and action
define("Report_print",5); // action
define("Report_revivewizard",6); // action
define("Report_email_send",1);
define("Report_email_cancel",2);
define("Report_email_topcancel",3);
class Report extends FormWidget
{
  var $wizard;
  var $hidemenus;
  var $state;
  var $action;
  
  var $email_form;
  var $email_to;
  var $email_from;
  var $email_subject;
  var $email_text;
  var $email_action;
  
  var $profemail_form;
  var $profemail_subject;
  var $profemail_message;
  var $profemail_debug;
  var $profemail_action; 
  
  function Report($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->action = new ActionButton($this->prefix . "_action",$this->form,$this->formmethod);
    $this->action->loadvalues();
    
    $this->wizard = new ReportWizard($this->prefix . "_wizard",$this->form,$this->formmethod);
    $this->wizard->loadvalues();
    
    if (!$this->wizard->isfinished() || $this->action->action == Report_revivewizard)
      $this->state = Report_wizard;
    else
    {
      if ($this->wizard->output->checked == ReportWizard_output_www)
      {
        $this->email_action = new ActionButton($this->prefix . "_email_action",$this->form,$this->formmethod);
        $this->email_action->loadvalues();
        if ($this->action->action == Report_email && $this->email_action->action != Report_email_cancel && $this->email_action->action != Report_email_topcancel)
          $this->state = Report_email;
        else
          $this->state = Report_results;  
      }  
      else
        $this->state = Report_profemail;  
    }

    if ($this->action->action == Report_print) $this->wizard->printable->checked = true;

    $this->hidemenus = ($this->state == Report_results && ($this->wizard->printable->checked || $this->action->action == Report_print));
    
    if ($this->state == Report_email)
    {
      $this->email_form = new Form($this->prefix . "_email_form", $this->form, $this->formmethod);
      $this->email_to = new TextBox(1,40,"",$this->prefix . "_email_to",$this->form,$this->formmethod);
      $this->email_from = new TextBox(1,40,"",$this->prefix . "_email_from",$this->form,$this->formmethod);
      $this->email_subject = new TextBox(1,80,"",$this->prefix . "_email_subject",$this->form,$this->formmethod);
      $this->email_text = new TextBox(15,80,"",$this->prefix . "_email_text",$this->form,$this->formmethod);
      $this->email_form->loadvalues();
      $this->email_to->loadvalues();
      $this->email_from->loadvalues();
      $this->email_subject->loadvalues();
      $this->email_text->loadvalues();
    }
    else if ($this->state == Report_profemail)
    {
      $this->profemail_form = new Form($this->prefix . "_profemail_form", $this->form, $this->formmethod);
      $this->profemail_subject = new TextBox(1,76,"",$this->prefix . "_profemail_subject", $this->form, $this->formmethod);
      $this->profemail_message = new TextBox(8,76,"",$this->prefix . "_profemail_message", $this->form, $this->formmethod);
      $this->profemail_debug = new CheckBox($this->prefix . "_profemail_debug", $this->form, $this->formmethod);
      $this->profemail_action = new ActionButton($this->prefix . "_profemail_action", $this->form, $this->formmethod);
      $this->profemail_form->loadvalues();
      if ($this->profemail_form->isstale)
      {
        $this->profemail_subject->loadvalues();
        $this->profemail_message->loadvalues();
        $this->profemail_debug->loadvalues();
        $this->profemail_action->loadvalues();
      }
      else
      {
        $this->profemail_subject->text = "WCES Evaluation Results";
        $this->profemail_message->text = "Dear %profname%,\n\nThis automated email contains the results of the WCES evaluations for your classes. Send questions or comments to wces@columbia.edu";
        $this->profemail_debug->checked = false;
      }
    }
  }
  
  function display()
  {
    global $server_wcespath,$db;
  
    if ($this->state == Report_wizard)
      $this->wizard->display(false);
    else
      $this->wizard->display(true);
  
    if ($this->state == Report_email || $this->state == Report_profemail || $this->state == Report_results)
    {
      print("<table><tr><td height=50>\n");
      $this->action->display(false,Report_revivewizard,0);
      ?><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<?=$this->action->gethref(Report_revivewizard,0)?>"><img src="<?=$server_wcespath?>media/report/wizard.gif" border=0 width=24 height=22 alt="Back to the Reporting Wizard"> Back to the Reporting Wizard</a></span> <?
      if ($this->state == Report_results)
      {
        $this->action->display(false,Report_print,0);
        $this->action->display(false,Report_email,0);
        ?><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<?=$this->wizard->printable->checked ? "javascript:void(window.print())" : $this->action->gethref(Report_print,0) ?>"><img src="<?=$server_wcespath?>media/report/print.gif" border=0 width=20 height=20 alt="Printer"><?=$this->wizard->printable->checked ? "Print this report" : "Printable version of this Report"?></a></span> <?
        ?><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<?=$this->action->gethref(Report_email,0)?>"><img src="<?=$server_wcespath?>media/report/email.gif" border=0 width=20 height=20 alt="Email a copy of this report"> Email this Report</a></span> <?
      }
      if ($this->state == Report_profemail && $this->profemail_action->action)
      {
        $this->profemail_action->display(false,0,0);
        ?><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<?=$this->profemail_action->gethref(0,0)?>"><img src="<?=$server_wcespath?>media/report/back.gif" border=0 width=20 height=20 alt="Back">Back to Professor Email</span> <?
      }
      if ($this->state == Report_email)
      {
        $this->email_action->display(false,Report_email_topcancel,0);
        ?><span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<?=$this->email_action->gethref(Report_email_topcancel,0)?>"><img src="<?=$server_wcespath?>media/report/back.gif" border=0 width=20 height=20 alt="Back">Back to Results</span> <?
      }
      print("</td></tr></table>\n");
    }

    switch($this->state)
    {
      case Report_results:
        if (!$db) $db = wces_connect();
        $html = $text = false;
        makereport($db,$this->wizard,true,$html,$text);
        break;
      case Report_email:
        $this->emailreport();
      break;
      case Report_profemail:
         $this->profemail();
      break;
    }
  }
  
  function emailreport()
  {
    global $db;
    $this->action->display();
    if ($this->email_action->action == Report_email_send)
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
       mail($address,$subject,$email, "To: $address\nFrom: $from\nReply-To: $from\nX-Mailer: PHP/" . phpversion());
       print("<h3>Done.</h3>"); 
    }
    else
    {
       if (!$this->email_form->isstale)
       {
         if (!$db) $db = wces_connect();
         $html = false; $text = true;
         makereport($db,$this->wizard,false,$html,$text);
         $this->email_text->text = $text;
       }  
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
    <td><? $this->email_action->display("Send",Report_email_send,0); ?> <? $this->email_action->display("Cancel",Report_email_cancel,0); ?></td>
  </tr>
</table>
<?
    }
  }
  
  function profemail()
  {
    global $server_wcespath,$db;
    if (!$db) $db = wces_connect();

    $this->profemail_form->display();
    
    if ($this->profemail_action->action)
    {
      $this->profemail_subject->display(true); // true means hidden
      $this->profemail_message->display(true); 
      $this->profemail_debug->display(true);
      report_profemail($db,$this->wizard, $this->profemail_subject->text, $this->profemail_message->text, $this->profemail_debug->checked);
    }  
    else
    {
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
        print("Professor <a href=\"{$server_wcespath}administrators/info.php?professorid=$professorid\">$pname</a> - email <a href=\"mailto:$email\">$email</a> - $pages classes<br>");
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
  <td>&nbsp;</td><td><? $this->profemail_debug->display(); ?> <? $this->profemail_debug->displaylabel("Debug Mode (doesn't actually send emails)"); ?></td>
</tr>
<tr>  
  <td>&nbsp;</td><td><? $this->profemail_action->display("Submit",1,0); ?></td>
</tr>
</table>  
<?      
    };
  }
};  
?>
<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

/**
* Array containing the text of the ABET questions. Used by the professor
* custom question editor and by the QuestionSet widget.
* $var array
*/

$ABETQUESTIONS = array(
   1 => "Design experiments",
   2 => "Analyze and interpret data",
   3 => "Conduct experiments",
   4 => "Analyze and interpret data",
   5 => "Design a system, component, or process to meet desired needs",
   6 => "Function on multidisciplinary teams",
   7 => "Identify or formulate engineering problems",
   8 => "Solve engineering problems",
   9 => "Understand ethical responsibilities",
  10 => "Understand the impact of engineering solutions in a global/societal context",
  11 => "Use modern engineering tools",
  12 => "Communicate using oral presentations",
  13 => "Communicate using written reports",
  14 => "Pilot test a component prior to implementation",
  15 => "Use text materials to support project design",
  16 => "Integrate knowledge of mathematics, science, and engineering in engineering solutions",
  17 => "Apply knowledge of contemporary issues to engineering solutions",
  18 => "Recognize need to engage in lifelong learning"
);

/**
* Array which maps each numerical column of the TA database to the text of a question.
* $var array
*/

$TAQUESTIONS = array
(
  "overall" => "Overall Quality",
  "knowledgeability" => "Knowledgeability",
  "approachability" => "Approachability",
  "availability" => "Availability",
  "communication" => "Communication"
);

/**
* Array which maps the form values used by the TARate widget to their text labels.
* $var array
*/

$TAVALUES = array
(
  "5" => "excellent",
  "4" => "very good",
  "3" => "satisfactory",
  "2" => "poor",
  "1" => "disastrous",
  "0" => "n/a"
);

define("OldSurvey_save",1);
define("OldSurvey_cancel",2);
define("OldSurvey_taedit",3);
define("OldSurvey_tadelete",4);

class OldSurvey extends FormWidget
{
//public:

  var $finished; 

//private:
  var $pagename;
  var $db;
  var $classid;
  var $userid;
  var $questionperiodid;
  var $questionsets;

  var $action; // ActionButton widget
  var $form; // Form widget
  var $showerrors; //boolean
  
  var $tas;
  var $tawidget;
  
  function OldSurvey($db, $classid, $userid, $pagename, $prefix, $form, $formmethod)
  {
    $this->db = $db;
    $this->classid = $classid;
    $this->userid = $userid;
    $this->pagename = $pagename;
    $this->FormWidget($prefix, $form, $formmethod);

    $this->form = new Form("${prefix}_form",$form,$formmethod);
    $questionperiodid = wces_Findquestionsetsta($db,"qsets",$classid);

    if($userid)
    {
      $classes = db_exec(
      "SELECT qs.questionsetid
      FROM enrollments AS e 
      INNER JOIN qsets AS qs ON e.classid = qs.classid
      INNER JOIN questionsets AS q ON q.questionsetid = qs.questionsetid
      LEFT JOIN answersets AS a ON a.questionsetid = q.questionsetid AND a.classid = e.classid AND a.questionperiodid = '$questionperiodid'
      LEFT JOIN completesurveys AS cs ON cs.userid = e.userid AND cs.answersetid = a.answersetid
      WHERE e.userid = '$userid' AND e.classid = '$classid' AND cs.userid IS NULL
      LIMIT 50", $db, __FILE__, __LINE__);
    }
    else
    {
      $classes = db_exec("SELECT questionsetid FROM qsets", $db, __FILE__, __LINE__);
    }
    
    $this->questionsets = array();
    while($row = mysql_fetch_assoc($classes))
    {
      $questionsetid = $row["questionsetid"];
      $this->questionsets[] = new OldQuestionSet($db, $questionsetid, $classid, $questionperiodid, "${prefix}_Q$questionsetid", $form, $formmethod);
    }  

    if ($userid)
    {
      $this->tas = 0 < mysql_result(
        db_exec("SELECT IF(MAX(tarate) = 'yes' AND tac.classid IS NULL,1,0) AS dota FROM qsets LEFT JOIN tacomplete AS tac ON tac.classid = qsets.classid AND questionperiodid = '$questionperiodid' AND userid = '$userid'", $db, __FILE__, __LINE__), 0
        ) ? array() : false;
    }
    else
    {
      $this->tas = 0 < mysql_result(
        db_exec("SELECT IF(MAX(tarate) = 'yes',1,0) AS dota FROM qsets", $db, __FILE__, __LINE__), 0
        ) ? array() : false;
    }
    $this->activeta = false;
    $this->tawidget = false;
    
    db_exec("DROP TABLE qsets", $db, __FILE__, __LINE__);
    
    $this->questionperiodid = $questionperiodid;
    
    $this->action = new ActionButton("${prefix}_action",$form,$formmethod);
    $this->finished = false;
    $this->showerrors = false;
  }
  
  function loadvalues()
  {
    $this->form->loadvalues();
    
    foreach($this->questionsets as $key => $value)
      $this->questionsets[$key]->loadvalues();

    if (is_array($this->tas))
    {
      $this->tas = unserialize($this->loadattribute("tas"));
      if (!is_array($this->tas)) $this->tas = array();
    }

    $this->action->loadvalues();   
    $this->handleactions();  
  }

  function handleactions()
  {
    if ($this->action->action == OldSurvey_save)
    {
      $this->action->action = "";	
      $this->action->object = "";

      $this->showerrors = true;

      foreach($this->questionsets as $key => $v)
        if (count($this->questionsets[$key]->errors) > 0) return false;
        
      $failure = false;
      foreach($this->questionsets as $key => $v)
      if (!$this->questionsets[$key]->save($this->userid))
      {
        print("<p><strong>Failed to save question set #" . $this->questionsets[$key]->questionsetid . "</strong></p>");
        $failure = true;	
      }

      if(is_array($this->tas))
      {
        if (!db_addrow($this->db, "tacomplete",array("userid" => $this->userid, "classid" => $this->classid, "questionperiodid" => $this->questionperiodid)))
        {
          print("<p><strong>Failed to save TA completion record.</strong></p>");
          $failure = true;
        }
        else
        {
          
          if (!($tauserid = db_addrow($this->db,"tausers",array())))
          {
            $failure = true;
            print("<p><strong>Failed to obtain TA user id</strong></p>");
          }  
          else
          {
            foreach($this->tas as $ta)
            {
              if (!$ta->validate())
              {
                $failure = true;
                print("<p><strong>TA responses did not validate.</strong></p>\n");
              }
              else
              {
                if (!$ta->save($this->db, $this->classid, $this->questionperiodid, $tauserid))
                {
                  $failure = true;
                  print("<p><strong>Failed to save results for TA '" . $ta->name . "'</strong></p>");	
                }
              }
            }  
          }  
        }    
      }
      if ($failure) print('<p><strong><font size="+2"><font color=red>Warning:</font> Errors were encountered during the save process. Please report this to <a href="mailto:' . $server_feedback . '">' . $server_feedback . '</a>. Include your user name and text of any error messages that preceded this one.</font></strong></p>' . "\n");
      $this->finished = true;
    }
    else if ($this->action->action == OldSurvey_cancel)
    {
      $this->finished = true;
      $this->action->action = "";	
      $this->action->object = "";
    }
    else if ($this->action->action == OldSurvey_taedit)
    {
      $ta = isset($this->tas[$this->action->object]) ? $this->tas[$this->action->object] : false;
      $this->tawidget = new TAWidget($ta, $this->prefix . "_tawidget", $this->form, $this->formmethod);
      $this->tawidget->loadvalues();

      if ($this->tawidget->save)
      {
        if (isset($this->tas[$this->action->object]))
          $this->tas[$this->action->object] = $this->tawidget->tarating;  
        else
          array_push($this->tas, $this->tawidget->tarating);
      }
      
      if ($this->tawidget->save && !$this->tawidget->finished)
      {
        $this->tawidget = new TAWidget(false, $this->prefix . "_tawidget", $this->form, $this->formmethod);
        $this->action->object = "new";
      }  
      else if ($this->tawidget->finished)
      {
        $this->tawidget = false;
        $this->action->action = "";	
        $this->action->object = "";
      }
    }
    else if ($this->action->action == OldSurvey_tadelete)
    {
      if ($this->tas[$this->action->object])
        array_splice($this->tas, $this->action->object, 1);
      $this->action->action = "";	
      $this->action->object = "";
    }
    else
    {
      $this->action->action = "";	
      $this->action->object = "";
    }
  }
 
  function display()
  {
    global $wces_path, $TAQUESTIONS, $TAVALUES;
    
    if (count($this->questionsets) == 0 && !is_array($this->tas))
    {  
      print("<p><strong>No new surveys can be found for this class.</strong></p>");
      return;
    }  
  
    $this->form->display();
    $this->action->display();
    if (is_array($this->tas)) $this->printattribute("tas",serialize($this->tas));
    
    if (!$this->tawidget)
    {
      if ($this->form->isstale) $this->optionbar();
      if ($this->showerrors)
      {
        $errortext = "";
        foreach($this->questionsets as $questionset)
          foreach($questionset->errors as $error)
            $errortext .= "  <li>$error</li>\n";
        if ($errortext) print("<p><strong>The form is not complete. The missing fields are highlighted in <font color=red>red</font>.</strong></p>\n<ul>\n$errortext</ul>\n");
      }    
      if ($this->action->action) $this->action->display();
      foreach($this->questionsets as $key => $value)
        $this->questionsets[$key]->display(false, $this->showerrors);
      
      if (is_array($this->tas))
      {
        print("<h4>TA Ratings</h4>");
        if (count($this->tas) == 0)
        {
          print("<p>If your class has teaching assistants, you can use this section of the survey to rate them.</p>");
          $this->action->display("Rate a TA...",OldSurvey_taedit,"new");
        }
        else
        {
          print("<p>You can rate an additional TA by clicking the <i>Rate Another TA</i> button. You can also edit or delete ratings that you have already entered.</p>");
          $this->action->display("Rate another TA...",OldSurvey_taedit,"new");
          print("<table>\n");
          foreach($this->tas as $key => $value)
          {
            print("<tr>\n  <td>");

            $taname = $value->name ? $value->name : "ALL TAs";
            print("<table cellspacing=0 cellpadding=1 border=1 bordercolor=black RULES=NONE FRAME=BOX>\n");
            print("<tr><td colspan=2 bgcolor=black background=\"${wces_path}media/0x000000.gif\"><font color=white><strong>$taname</strong></font></td></tr>\n");
            foreach($TAQUESTIONS as $answerkey => $question)
            {
              print("<tr>\n");
              print("  <td valign=top bgcolor=\"0xEEEEEE\" background=\"${wces_path}media/0xEEEEEE.gif\">$question:</td>\n");
              print("  <td bgcolor=white background=\"${wces_path}media/0xFFFFFF.gif\">" . $TAVALUES[$value->answers[$answerkey]] . "</td>\n");
              print("</tr>");
            };
            if ($value->comments)
            {
              print("<tr>\n");
              print("  <td valign=top bgcolor=\"0xEEEEEE\" background=\"${wces_path}media/0xEEEEEE.gif\">Comments:</td>\n");
              print("  <td bgcolor=white background=\"${wces_path}media/0xFFFFFF.gif\">" . htmlspecialchars(nl2br($value->comments)) . "</td>\n");
              print("</tr>");                
            };
            print("</table>\n");
            
            print("</td>\n  <td>");
            $this->action->display("Edit...", OldSurvey_taedit, $key, 'style="width: 100%"');
            print("<br>");
            $this->action->display("Delete", OldSurvey_tadelete, $key);
            print("  </td>\n</tr>\n");
            
          }
          print("</table>\n");
        }
      }
      print("<p>&nbsp;</p>\n");
      $this->optionbar();
    }  
    else
    {
      $this->tawidget->display();
      foreach($this->questionsets as $qs)
        $qs->display(true);
    }
  }
  
  function optionbar()
  {
    global $wces_path;
    if ($this->userid)
    {
?>
<table width="100%" cellpadding=5 cellspacing=0>
<tr>
  <td align=center bgcolor=black background="<?=$wces_path?>media/0x000000.gif">
    <? $this->action->display("Save responses", OldSurvey_save, false, ""); ?>
    <? $this->action->display("Return to class list without saving", OldSurvey_cancel, false, ""); ?>
  </td>
</tr>
</table>
<?  
    }
  }
}

class TARating
{
  var $name; //string response
  var $comments; //string response
  var $answers; // array of 

  function TARating($name, $comments, $answers)
  {
    $this->name = $name;
    $this->comments = $comments;
    $this->answers = $answers;
  }
  
  function save($db, $classid, $questionperiodid, $tauserid)
  {
    global $TAVALUES, $TAQUESTIONS;
    
    $a = array("tauserid" => $tauserid, "questionperiodid" => $questionperiodid, "classid" => $classid, "name" => $this->name, "comments" => $this->comments);
    foreach($TAQUESTIONS as $column => $text)
      $a[$column] = $this->answers[$column];
    return db_addrow($db,"taratings",$a);  	
  }
  
  function validate()
  {
    global $TAVALUES, $TAQUESTIONS;
    foreach($TAQUESTIONS as $column => $text)
      if (!isset($TAVALUES[$this->answers[$column]])) 
        return false;
    return true;    
  }
}

define("TAWidget_save",1);
define("TAWidget_saveand",2);
define("TAWidget_cancel",3);

class TAWidget extends FormWidget
{
  var $finished;
  var $save;
    
  var $tarating;
  var $errors;
  var $action;
  var $showerrors;

  function TAWidget($ta, $prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->tarating = $ta ? $ta : new TARating("","",array());
    $this->errors = array();
    $this->save = $this->finished = false;
    $this->action = new ActionButton("${prefix}_action", $form, $formmethod);
    $this->showerrors = false;
  }	
  
  function loadvalues()
  {
    global $TAQUESTIONS, $TAVALUES;

    foreach($TAQUESTIONS as $fieldname => $text)
    {
      $val = $this->loadattribute("ta$fieldname");
      if (isset($TAVALUES[$val]))
      	$this->tarating->answers[$fieldname] = $val;
      else
        $this->errors[$fieldname] = "Missing response for <i>$text</i>";
    }
    
    ifassign(trim($this->loadattribute("comments")), $this->tarating->comments);
    ifassign(trim($this->loadattribute("name")), $this->tarating->name);
    $this->action->loadvalues();

    $this->finished = $this->action->action == TAWidget_save || $this->action->action == TAWidget_cancel;
    
    if ($this->action->action == TAWidget_save || $this->action->action == TAWidget_saveand)
    {
     if (count($this->errors) == 0)
       $this->save = true;
     else
     {
       $this->showerrors = true;
       $this->finished = false;
     }  
    }  
  }

  function display($hidden = false)
  {
    global $TAQUESTIONS, $TAVALUES;
    $prefix = $this->prefix;
    
    if ($this->showerrors)
    {
      print("<p><strong>The form is not complete. This missing fields are listed below and highlighted in <font color-red>red</font>.</strong></p>");
      print("<ul>\n");
      foreach($this->errors as $error)
        print("  <li>$error</li>\n");
      print("</ul>\n");
    }
    
    print("<p><strong>Enter a TA Name:</strong> <input name=\"${prefix}_name\" size=25 value=\"" . htmlspecialchars($this->tarating->name) . "\"><br>\n");
    print("<font size=-1>(Leave blank to rate all TA's together)</font></p>");
      
    foreach($TAQUESTIONS AS $key => $text)
    {
      if ($this->showerrors && isset($this->errors[$key])) $text = "<font color=red>$text</font>";
      print("<p><b>$text</b></p>\n");
      print("<blockquote>\n");
      foreach($TAVALUES AS $vkey => $vtext)
      {
        $selected = isset($this->tarating->answers[$key]) && $vkey == $this->tarating->answers[$key] ? " checked" : "";
        print("<input type=radio name=\"${prefix}_ta$key\" id=\"${prefix}_ta$key$vkey\" value=\"$vkey\"$selected><label for=\"${prefix}_ta$key$vkey\">$vtext</label>\n");
      }
      print("</blockquote>\n");
    }
    print("<p><b>Comments</b></p>\n");
    print("<blockquote><textarea name=\"${prefix}_comments\" rows=8 cols=50 wrap=virtual>" . htmlspecialchars($this->tarating->comments) . "</textarea></blockquote>\n");
    
    print("<p>");
    $this->action->display("Save",TAWidget_save); print(" ");
    $this->action->display("Save and rate another TA",TAWidget_saveand); print(" ");
    $this->action->display("Cancel",TAWidget_cancel);
    print("</p>");
  }
}

class OldQuestionSet extends FormWidget
{
  var $errors; // string array containing error messages if question is not filled out properly, indexed by question name;
  var $questionsetid, $classid, $questionperiodid; // database identifiers

  var $db; //database handle
  var $action;
 
  var $displayname;
  var $mc;
  var $fr;
  var $abet;
  var $answers;
  var $errors;
  
  function OldQuestionSet($db, $questionsetid, $classid, $questionperiodid, $prefix, $form, $formmethod)
  {
    $this->db = $db;
    $this->questionsetid = $questionsetid;
    $this->classid = $classid;
    $this->questionperiodid = $questionperiodid;
    $this->FormWidget($prefix, $form, $formmethod);
    
    $this->displayname = "";
    $this->mc = array();
    $this->fr = array();
    $this->abet = array();
    $this->answers = array();
    $this->errors = array();
  }
  
  function loadvalues()
  {
    global $ABETQUESTIONS;
    
    $db = $this->db;
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $this->questionsetid), 0);
    
    $this->displayname = $questionset["displayname"];
    
    $choices = array(1 => "a", 2 => "b", 3 => "c", 4 => "d", 5 => "e");
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = $this->prefix . "_MC$i";
      if ($questionset["MC$i"])
      {
        $this->mc[$fieldname] = $questionset["MC$i"];
        $this->answers[$fieldname] = $this->loadattribute("MC$i");
        if (!in_array($this->answers[$fieldname], $choices))
          $this->errors[$fieldname] = "Missing response for ratings question #$i under '" . $this->displayname . "'";
      }
    }

    for($i = 1; $i <= 2; ++$i)
    {
      $fieldname = $this->prefix . "_FR$i";
      $text = $questionset["FR$i"];
      if ($text)
      { 
        $this->fr[$fieldname] = $text;
        $this->answers[$fieldname] = trim($this->loadattribute("FR$i"));
      }  
    }

    if ($questionset["ABET"])
    {
      $choices = array(0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e", 5 => "f");
      $abet = explode(",", $questionset["ABET"]);
      foreach ($abet as $v)
      {
        $fieldname = $this->prefix . "_ABET$v";
        if (isset($ABETQUESTIONS[$v]))
        {
          $this->abet[$fieldname] = $ABETQUESTIONS[$v];
          $this->answers[$fieldname] = $this->loadattribute("ABET$v");
          if (!in_array($this->answers[$fieldname], $choices))
            $this->errors[$fieldname] = "Missing response for ABET Question #" . ($v);
        }
      }  
    }
  }
  
  function display($hidden = false, $showerrors = false)
  {
    global $wces_path;
    if ($hidden)
    {
      foreach($this->answers as $key => $value)
        print("<input type=hidden name=\"$key\" value=\"" . htmlspecialchars($value) . "\">\n");
    }
    else
    {
      if (count($this->mc) + count($this->fr) > 0)
      {
        print("<h4>" . $this->displayname . "</h4>\n");
        
        foreach($this->mc as $fieldname => $text)
        {
          $val = $this->answers[$fieldname];
          $text = $showerrors && isset($this->errors[$fieldname]) ? "<font color=red>" . htmlspecialchars($text) . "</font>" : htmlspecialchars($text);
?>
<p><b><?=$text?></b></p>
<blockquote>
  <input type=radio name="<?=$fieldname?>" id="<?=$fieldname?>a" value="a" <?=$val == "a" ? "checked" : ""?>><label for="<?=$fieldname?>a">excellent</label>
  <input type=radio name="<?=$fieldname?>" id="<?=$fieldname?>b" value="b" <?=$val == "b" ? "checked" : ""?>><label for="<?=$fieldname?>b">very good</label>
  <input type=radio name="<?=$fieldname?>" id="<?=$fieldname?>c" value="c" <?=$val == "c" ? "checked" : ""?>><label for="<?=$fieldname?>c">satisfactory</label>
  <input type=radio name="<?=$fieldname?>" id="<?=$fieldname?>d" value="d" <?=$val == "d" ? "checked" : ""?>><label for="<?=$fieldname?>d">poor</label>
  <input type=radio name="<?=$fieldname?>" id="<?=$fieldname?>e" value="e" <?=$val == "e" ? "checked" : ""?>><label for="<?=$fieldname?>e">disastrous</label>
</blockquote>
<?
        }  
    
        foreach($this->fr as $fieldname => $text)
        {
?>
<p><b><?=$text?></b></p>
<blockquote><textarea name="<?=$fieldname?>" rows=8 cols=50 wrap=virtual><?=htmlspecialchars($this->answers[$fieldname])?></textarea></blockquote>
<?
        }  
      };
  
      if (count($this->abet) > 0)
      {  
?>
<h4>ABET Questions</h4>
<table bordercolor=black cellspacing=0 cellpadding=3 border=0 RULES="groups" FRAME=box STYLE="border: none">
<thead>
<tr>
<td colspan=2 align=left STYLE="border: none"><b>To what degree did this course enhance your ability to ...</b></td>
<td colspan=9 align=right STYLE="border: none"><b>(0 = <i>not at all</i>, 5 = <i>a great deal</i>)</b></td>
</tr>
</thead>
<tbody>
<tr>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>0</STRONG></font></td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>1</STRONG></font></td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>2</STRONG></font></td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>3</STRONG></font></td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>4</STRONG></font></td>
  <td bgcolor=black background="<?=$wces_path?>media/0x000000.gif" align=center><font color=white><STRONG>5</STRONG></font></td>
</tr>
<?
        $choices = array(0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e", 5 => "f");
        $row = true;
        foreach ($this->abet as $fieldname => $text)
        {
          $text = $showerrors && isset($this->errors[$fieldname]) ? "<font color=red>" . htmlspecialchars($text) . "</font>" : htmlspecialchars($text);
          $val = $this->answers[$fieldname];
          $row = !$row; $color = $row ? 'bgcolor="#FFFFFF" background="' . $wces_path . 'media/0xFFFFFF.gif"' : 'bgcolor="#EEEEEE" background="' . $wces_path . 'media/0xEEEEEE.gif"';
          print("<tr><td colspan=5 $color>$text</td>");
          foreach($choices as $choice)
          {
            $selected = ($val == $choice) ? " checked" : "";
            print("<td align=center $color><input name=\"$fieldname\" value=\"$choice\" type=radio$selected></td>");
          }  
          print("</tr>\n");
        }
        print("\n</tbody>\n</table>\n");
      }
    }  
  }

  function save($userid)
  {
    if (!is_array($this->errors) || count($this->errors) != 0)
      return false;
    
    $db = $this->db;
    
    $sql = "UPDATE answersets SET responses = responses + 1";
    
    $choices = array(1 => "a", 2 => "b", 3 => "c", 4 => "d", 5 => "e");
    
    foreach($this->mc as $fieldname => $text)
    {
      $field = "";
      simplesplit('_',$fieldname,$null,$field);
      $choice = $this->answers[$fieldname];
      if (in_array($choice,$choices) && $field)
        $sql .= ", $field$choice = $field$choice + 1";
      else
        return false;
    };

    foreach($this->fr as $fieldname => $text)
    {
      $field = "";
      simplesplit('_',$fieldname,$null,$field);
      $result = addslashes(addcslashes($this->answers[$fieldname],"\t\\"));
      $sql .= ", $field = concat($field, '\t$result')";
    }

    $choices = array(0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e", 5 => "f");
    foreach ($this->abet as $fieldname => $text)
    {
      $choice = $this->answers[$fieldname];
      $field = "";
      simplesplit('_',$fieldname,$null,$field);
      if (in_array($choice,$choices))
      	$sql .= ", $field$choice = $field$choice + 1";
      else
        return false;
    }        
        
    if (!($userid && $this->questionsetid && $this->classid && $this->questionperiodid))
      return false;
          
    $keys = Array("questionsetid" => $this->questionsetid, "questionperiodid" => $this->questionperiodid, "classid" => $this->classid);
    $answersetid = db_replace($db, "answersets", $keys, false, "answersetid");

    $sql .= " WHERE answersetid = $answersetid";

    if (!db_exec("INSERT INTO completesurveys(userid,answersetid) VALUES ($userid,$answersetid)",$db,__FILE__,__LINE__))
      return false;
      
    if (!db_exec($sql,$db,__FILE__,__LINE__))
    {
      db_exec("DELETE FROM completesurveys(userid,answersetid) VALUES ($userid,$answersetid)", $db, __FILE__, __LINE__);
      return false; 
    }
    else
      return true;
  }
}

?>
<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");

define("OldSurvey_save",1);
define("OldSurvey_cancel",2);
define("OldSurvey_taedit",3);
define("OldSurvey_tadelete",4);
define("TAWidget_save",1);
define("TAWidget_saveand",2);
define("TAWidget_cancel",3);

$db_debug = true;

$TAREVISION = 42;
// insert into revisions(revision_id, type, branch_id, revision) values (42, -1, 40,-1);
//
// CREATE TABLE ta_ratings
// (
//   name TEXT,
//   overall INTEGER,
//   knowledgeability INTEGER,
//   approachability INTEGER,
//   availability INTEGER,
//   communication INTEGER,
//   comments TEXT
// )
// INHERITS (responses);

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

  function save($parent)
  {
    global $TAVALUES, $TAQUESTIONS, $TAREVISION, $wces;
    wces_connect();

    $name = nullquot($this->name);
    $comments = nullquot($this->comments);
    
    $bsql = "INSERT INTO ta_ratings(revision_id, parent, name, comments";
    $msql = ") VALUES ($TAREVISION, $parent, $name, $comments";
    $esql = ")";

    foreach($TAQUESTIONS as $column => $text)
    {
      $bsql .= ", $column";
      $msql .= ", '" . addslashes($this->answers[$column]) . "'";
    }
    return pg_query($bsql . $msql . $esql, $wces, __FILE__, __LINE__);
  }

  function validate()
  {
    global $TAVALUES, $TAQUESTIONS;
    foreach($TAQUESTIONS as $column => $text)
      if (!isset($TAVALUES[$this->answers[$column]]))
        return false;
    return true;
  }
};

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
};

class TASurvey extends FormWidget
{
//public:
  var $modal = false;

//private:

  var $action; // ActionButton widget

  var $tas;
  var $tawidget = false;
  var $errors = array();

  function TASurvey($prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->form = new Form("${prefix}_form",$form,$formmethod);
    $this->tas = array();
    $this->action = new ActionButton("${prefix}_action",$form,$formmethod);
    $this->finished = false;
    $this->showerrors = false;
  }

  function loadvalues()
  {
    $this->tas = @unserialize($this->loadattribute("tas"));
    if (!is_array($this->tas)) $this->tas = array();

    $this->action->loadvalues();
    //DebugBreak();
    if ($this->action->action == OldSurvey_taedit)
    {
      $this->modal = true;
      $n = $this->action->object;
      $ta = isset($this->tas[$n]) ? $this->tas[$n] : false;
      $this->tawidget = new TAWidget($ta, $this->prefix . "_tawidget", $this->form, $this->formmethod);
      $this->tawidget->loadvalues();

      if ($this->tawidget->save)
      {
        if (isset($this->tas[$n]))
          $this->tas[$n] = $this->tawidget->tarating;
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
        $this->action->action = $this->action->object = "";
        $this->modal = false;
        $this->tawidget = false;
      }
    }
    else if ($this->action->action == OldSurvey_tadelete)
    {
      if ($this->tas[$this->action->object])
        array_splice($this->tas, $this->action->object, 1);
      $this->action->object = $this->action->action = "";
    }
  }

  function save($parent)
  {
    global $wces;
    $success = true;

    foreach($this->tas as $k => $ta)
    {
      if (!$ta->validate())
      {
        print("<p><strong>TA response $k did not validate.</strong></p>\n");
        $success = false;
      }
      if (!$ta->save($parent))
      {
        print("<p><strong>Failed to save results for TA '" . $ta->name . "' ($k)</strong></p>");
        $success = false;
      }
    }
    $this->finished = true;
    return $success;
  }


  function display($hidden = false)
  {
    global $wces_path, $TAQUESTIONS, $TAVALUES;

    $this->action->display();
    
    if (is_array($this->tas)) $this->printattribute("tas",serialize($this->tas));
    if ($this->tawidget) $this->tawidget->display();
    if ($this->tawidget ||$hidden) return;

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
          print("  <td valign=top bgcolor=\"#EEEEEE\" background=\"${wces_path}media/0xEEEEEE.gif\">$question:</td>\n");
          print("  <td bgcolor=white background=\"${wces_path}media/0xFFFFFF.gif\">" . $TAVALUES[$value->answers[$answerkey]] . "</td>\n");
          print("</tr>");
        };
        if ($value->comments)
        {
          print("<tr>\n");
          print("  <td valign=top bgcolor=\"#EEEEEE\" background=\"${wces_path}media/0xEEEEEE.gif\">Comments:</td>\n");
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
}

?>
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

  function TARating($name = null, $comments = null, $answers = array())
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
    return pg_go($bsql . $msql . $esql, $wces, __FILE__, __LINE__);
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

class TAWidget extends Widget
{
  var $done = false;
  var $save = false;

  var $tarating;
  var $errors = array();
  var $action;
  var $showerrors = false;

  function TAWidget($name, &$parent)
  {
    $this->FormWidget($name, $parent);
    $this->action = & new ActionButton("action", $this);
  }

  function loadRating($taRating)
  {
    $this->taRating = $taRating;
  }
  
  function saveRating(&$taRating)
  {
    $taRating = $this->taRating;
  }

  function loadValues()
  {
    global $TAQUESTIONS, $TAVALUES;
    Widget::loadValues();

    ifassign(trim($this->loadValue("comments")), $this->tarating->comments);
    ifassign(trim($this->loadValue("name")), $this->tarating->name);
    foreach($TAQUESTIONS as $fieldname => $text)
    {
      $val = $this->loadValue("ta$fieldname");
      if (isset($TAVALUES[$val]))
        $this->tarating->answers[$fieldname] = $val;
      else
        $this->errors[$fieldname] = "Missing response for <i>$text</i>";
    }

    $this->done = $this->action->action == TAWidget_save || $this->action->action == TAWidget_cancel;

    if ($this->action->action == TAWidget_save || $this->action->action == TAWidget_saveand)
    {
     if (count($this->errors) == 0)
       $this->save = true;
     else
     {
       $this->showerrors = true;
       $this->done = false;
     }
    }
  }

  function displayHidden()
  {
    // never implemented
    assert(0);      
  } 
  
  function display()
  {
    global $TAQUESTIONS, $TAVALUES;

    if ($this->showerrors)
    {
      print("<p><strong>The form is not complete. This missing fields are listed below and highlighted in <font color-red>red</font>.</strong></p>");
      print("<ul>\n");
      foreach($this->errors as $error)
        print("  <li>$error</li>\n");
      print("</ul>\n");
    }

    print("<p><strong>Enter a TA Name:</strong> <input name=\"" . $this->fullName("name") . "\" size=25 value=\"" . htmlspecialchars($this->tarating->name) . "\"><br>\n");
    print("<font size=-1>(Leave blank to rate all TA's together)</font></p>");

    foreach($TAQUESTIONS AS $key => $text)
    {
      if ($this->showerrors && isset($this->errors[$key])) $text = "<font color=red>$text</font>";
      print("<p><b>$text</b></p>\n");
      print("<blockquote>\n");
      foreach($TAVALUES AS $vkey => $vtext)
      {
        $selected = isset($this->tarating->answers[$key]) && $vkey == $this->tarating->answers[$key] ? " checked" : "";
        print("<input type=radio name=\"" . $this->fullName("ta$key") . "\" id=\"" . $this->fullName("ta$key$vkey") . "\" value=\"$vkey\"$selected><label for=\"" . $this->fullname("ta$key$vkey") . "\">$vtext</label>\n");
      }
      print("</blockquote>\n");
    }
    print("<p><b>Comments</b></p>\n");
    print("<blockquote><textarea name=\"" . $this->fullName("comments") . "\" rows=8 cols=50 wrap=virtual>" . htmlspecialchars($this->tarating->comments) . "</textarea></blockquote>\n");
    print("<p>");
    $this->action->display("Save",TAWidget_save); print(" ");
    $this->action->display("Save and rate another TA",TAWidget_saveand); print(" ");
    $this->action->display("Cancel",TAWidget_cancel);
    print("</p>");
  }
};

class TASurvey extends Widget
{
//public:
  var $modal = false;
  var $errors = array();
  var $editing = null;

  var $tas;
  var $action, $tawidget; 

  function TASurvey($name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->action = & new ActionButton("action", $this);
    $this->done = false;
    $this->showerrors = false;
  }

  function loadDefaults()
  {
    $this->tas = array();      
  }

  function loadValues()
  {
    Widget::loadValues();
    
    $this->editing = $this->loadValues("editing");
    $this->tas = unserialize($this->loadValue("tas"));
    assert(is_array($this->tas));

    $newLoad = false;
    
    if (!isset($this->editing))
    {
      if ($this->action->action == OldSurvey_taedit)
      {
        $this->editing = $this->action->param;
        $newLoad = true;
      }
      else if ($this->action->action == OldSurvey_tadelete)
      {
        if (isset($this->tas[$this->action->param]))
          array_splice($this->tas, $this->action->object, 1);
      }
    }

    if (isset($this->editing))
    {
      $this->modal = true;    
      $this->tawidget = & new TAWidget($ta, "tawidget", $this);
      $this->tawidget->loadRating(isset($this->tas[$n]) ? $this->tas[$n] : new TaRating);
      $this->loadChild($this->tawidget, $newLoad);
  
      if ($this->tawidget->save)
      {
        if (isset($this->tas[$n]))
          $this->tawidget->saveRating($this->tas[$n]);
        else
          $this->tawidget->saveRating($this->tas[]);
      }
  
      if ($this->tawidget->save && !$this->tawidget->done)
      {
        $this->loadRating(new TaRating());
        $this->editing = 'new';
      }
      else if ($this->tawidget->done)
      {
        $this->modal = false;
      }
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
    $this->done = true;
    return $success;
  }

  function displayHidden()
  {
    Widget::displayHidden();
    $this->printValue('editing', $this->editing);
    $this->printValue("tas",serialize($this->tas));    
  }

  function display()
  {
    global $wces_path, $TAQUESTIONS, $TAVALUES;

    $this->printValue('editing', $this->editing);
    $this->printValue("tas",serialize($this->tas));

    $this->action->display();

    if ($this->modal)
    {
      $this->tawidget->display();
    }
    else
    {
      print("<h4>TA Ratings</h4>");
      if (count($this->tas) == 0)
      {
        print("<p>If your class has teaching assistants, you can use this section of the survey to rate them.</p>");
        $this->action->display("Rate a TA...", OldSurvey_taedit, "new");
      }
      else
      {
        print("<p>You can rate an additional TA by clicking the <i>Rate Another TA</i> button. You can also edit or delete ratings that you have already entered.</p>");
        $this->action->display("Rate another TA...", OldSurvey_taedit, "new");
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
}

?>
<?

// XXX: This version of the TA component does not
// use the lists of TAs stored in the database.
// An incompatible TA component which does do this
// was added on the fall-2002 branch in revision 1.9.2.2
// Changes from that revision need to be merged in here.

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

class TaWidget extends ParentWidget
{
  var $done = false;
  var $save = false;

  var $errors = array();
  var $showErrors = false;

  var $taRating;  
  var $event;

  function TaWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->event =& new EventWidget('event', $this);
  }

  function loadRating($taRating)
  {
    $this->taRating = $taRating;
  }
  
  function saveRating(&$taRating)
  {
    $taRating = $this->taRating;
  }

  function loadState()
  {
    global $TAQUESTIONS, $TAVALUES;
    ParentWidget::loadState();

    $this->taRating->comments  = $this->readValue('comments');
    $this->taRating->name = $this->readValue('name');
    
    foreach($TAQUESTIONS as $fieldname => $text)
    {
      $val = $this->readValue("ta$fieldname");
      if (isset($TAVALUES[$val]))
        $this->taRating->answers[$fieldname] = $val;
      else
        $this->errors[$fieldname] = "Missing response for <i>$text</i>";
    }

    $this->done = $this->event->event == TAWidget_save || $this->event->event == TAWidget_cancel;
    
    if ($this->event->event == TAWidget_save || $this->event->event == TAWidget_saveand)
    {
      if (count($this->errors) == 0)
        $this->save = true;
      else
      {
        $this->showErrors = true;
        $this->done = false;
      }
    }
    print("<h5>done = " . (int)$this->done . "</h5>");
  }

  function displayHidden()
  {
    // never implemented
    assert(0);      
  } 
  
  function display()
  {
    global $TAQUESTIONS, $TAVALUES;

    if ($this->showErrors)
    {
      print("<p><strong>The form is not complete. This missing fields are listed below and highlighted in <font color-red>red</font>.</strong></p>");
      print("<ul>\n");
      foreach($this->errors as $error)
        print("  <li>$error</li>\n");
      print("</ul>\n");
    }

    print("<p><strong>Enter a TA Name:</strong> <input name=\"" . $this->fullName("name") . "\" size=25 value=\"" . htmlspecialchars($this->taRating->name) . "\"><br>\n");
    print("<font size=-1>(Leave blank to rate all TA's together)</font></p>");

    foreach($TAQUESTIONS AS $key => $text)
    {
      if ($this->showErrors && isset($this->errors[$key])) $text = "<font color=red>$text</font>";
      print("<p><b>$text</b></p>\n");
      print("<blockquote>\n");
      foreach($TAVALUES AS $vkey => $vtext)
      {
        $selected = isset($this->taRating->answers[$key]) && $vkey == $this->taRating->answers[$key] ? " checked" : "";
        print("<input type=radio name=\"" . $this->fullName("ta$key") . "\" id=\"" . $this->fullName("ta$key$vkey") . "\" value=\"$vkey\"$selected><label for=\"" . $this->fullname("ta$key$vkey") . "\">$vtext</label>\n");
      }
      print("</blockquote>\n");
    }
    print("<p><b>Comments</b></p>\n");
    print("<blockquote><textarea name=\"" . $this->fullName("comments") . "\" rows=8 cols=50 wrap=virtual>" . htmlspecialchars($this->taRating->comments) . "</textarea></blockquote>\n");
    print("<p>");
    $this->event->displayButton("Save",TAWidget_save); print(" ");
    $this->event->displayButton("Save and rate another TA",TAWidget_saveand); print(" ");
    $this->event->displayButton("Cancel",TAWidget_cancel);
    print("</p>");
  }
};

class TaSurvey extends StatefullWidget
{
//public:
  var $modal = false;
  var $errors = array();

  var $tas = array();
  var $taWidget; 

  function TaSurvey($name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
  }

  function & handleEvent($event, $param, $isNew)
  {
    if ($event == OldSurvey_taedit)
    {
      $this->taWidget =& new TaWidget('taWidget', $this);
      
      $this->taWidget->loadRating(isset($this->tas[$param]) ? $this->tas[$param] : new TaRating);
      print("<h5>isNew = " . (int)$isNew . "</h5>");
      $this->loadChild($this->taWidget, $isNew);
  
      if ($this->taWidget->save)
      {
        if (isset($this->tas[$param]))
          $this->taWidget->saveRating($this->tas[$param]);
        else
          $this->taWidget->saveRating($this->tas[]);
      }
  
      if ($this->taWidget->save && !$this->taWidget->done)
      {
        $this->taWidget->loadRating(new TaRating());
      }

      $this->modal = !$this->taWidget->done;
      return $this->taWidget;
    }
    else if ($event == OldSurvey_tadelete)
    {
      if (isset($this->tas[$this->event->param]))
        array_splice($this->tas, $this->event->param, 1);
    }
  }

  function loadState()
  {
    $this->tas = unserialize($this->readValue('tas'));
    assert(is_array($this->tas));
    StatefullWidget::loadState();
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

  function printState()
  {
    StatefullWidget::printState();
    $this->printValue('tas', serialize($this->tas));    
  }

  function printVisible()
  {
    global $wces_path, $TAQUESTIONS, $TAVALUES;

    print("<h4>TA Ratings</h4>");
    if (count($this->tas) == 0)
    {
      print("<p>If your class has teaching assistants, you can use this section of the survey to rate them.</p>");
      $this->event->displayButton("Rate a TA...", OldSurvey_taedit, "new");
    }
    else
    {
      print("<p>You can rate an additional TA by clicking the <i>Rate Another TA</i> button. You can also edit or delete ratings that you have already entered.</p>");
      $this->event->displayButton("Rate another TA...", OldSurvey_taedit, "new");
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
        $this->event->displayButton("Edit...", OldSurvey_taedit, $key, 'style="width: 100%"');
        print("<br>");
        $this->event->displayButton("Delete", OldSurvey_tadelete, $key);
        print("  </td>\n</tr>\n");
      }
      print("</table>\n");
    }
  }
}

?>
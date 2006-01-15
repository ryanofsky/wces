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

$TAREVISION = 42; $TAITEM = 40; $TABRANCH = 40;
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
  var $user_id = null;
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
    global $TAVALUES, $TAQUESTIONS, $TAREVISION, $TAITEM, $wces;
    wces_connect();

    $name = nullquot($this->name);
    $comments = nullquot($this->comments);
    
    if ($this->user_id)
    {
      $user_id = (int)$this->user_id; 
      $name = 'NULL';
    }
    else
    {
      $user_id = 'NULL'; 
    }
    
    $bsql = "INSERT INTO ta_ratings(revision_id, item_id, parent, name, comments, user_id";
    $msql = ") VALUES ($TAREVISION, $TAITEM, $parent, $name, $comments, $user_id";
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
    $this->parent =& $parent;
  }

  function loadRating($taRating)
  {
    $this->taRating = $taRating;
  }
  
  function saveRating(&$taRating)
  {
    $taRating = $this->taRating;
  }

  function loadState($new)
  {
    assert(isset($new));
    ParentWidget::loadState($new);
    if ($new) return;

    global $TAQUESTIONS, $TAVALUES;

    $this->taRating->comments  = $this->readValue('comments');
    $this->taRating->name = $this->readValue('name');

    $user_id = $this->readValue('user_id');
    if (isset($user_id))
      $this->taRating->user_id = strlen($user_id) ? (int)$user_id : NULL;

    if (!($this->taRating->name || $user_id))
    {
      $this->errors["name"] = "Missing TA Name"; 
    }
    
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
  }

  function displayHidden()
  {
    // never implemented
    assert(0);      
  } 
  
  function display()
  {
    global $TAQUESTIONS, $TAVALUES, $wces;

    if ($this->showErrors)
    {
      print("<p><strong>The form is not complete. This missing fields are listed below and highlighted in <font color-red>red</font>.</strong></p>");
      print("<ul>\n");
      foreach($this->errors as $error)
        print("  <li>$error</li>\n");
      print("</ul>\n");
    }

    $dusers = array();
    foreach ($this->parent->tas as $k => $r)
    {
      if ($this->index == $k) continue;
      if ($r->user_id) $dusers[] = $r->user_id;
    }

    if (count($dusers))
      $dcond = ' AND e.user_id NOT IN (' . implode($dusers, ',') . ')';
    else
      $dcond = '';

    wces_connect();
    $r = pg_go("
      SELECT e.user_id, u.firstname, u.lastname FROM enrollments AS e
      INNER JOIN users AS u USING (user_id)
      WHERE e.class_id = {$this->parent->class_id} AND e.status & 2 <> 0 $dcond
    ", $wces, __FILE__, __LINE__);
    
    
    $text = "TA Name";
    if ($this->showErrors && isset($this->errors['name'])) $text = "<font color=red>$text</font>"; 
    
    print("<p><strong>$text</strong></p>\n");
    print("<blockquote>\n");
    $n = pg_numrows($r);
    if ($n > 0)
    {
      $fname = $this->name('user_id');

      for ($i = 0; $i < $n; ++$i)
      {
        $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
        $selected = $row['user_id'] == $this->taRating->user_id ? " checked" : "";
        $name = trim("$row[firstname] $row[lastname]");
        print("<input type=radio name=\"$fname\" id=\"$fname$i\" value=\"$row[user_id]\"$selected><label for=\"$fname$i\">$name</label><br>\n");        
      }
      $selected = $this->taRating->user_id === 0 ? " checked" : "";
      print("<input type=radio name=\"$fname\" id=\"$fname$i\" value=\"0\" $selected><label for=\"$fname$i\">Other (please specify)</label>\n");        
    }
    print("<input name=\"". $this->name('name') . "\" size=25 value=\"" . htmlspecialchars($this->taRating->name) . "\">\n");
    
    print("</blockquote>\n");

    foreach($TAQUESTIONS AS $key => $text)
    {
      if ($this->showErrors && isset($this->errors[$key])) $text = "<font color=red>$text</font>";
      print("<p><b>$text</b></p>\n");
      print("<blockquote>\n");
      foreach($TAVALUES AS $vkey => $vtext)
      {
        $selected = isset($this->taRating->answers[$key]) && $vkey == $this->taRating->answers[$key] ? " checked" : "";
        print("<input type=radio name=\"" . $this->name("ta$key") . "\" id=\"" . $this->name("ta$key$vkey") . "\" value=\"$vkey\"$selected><label for=\"" . $this->name("ta$key$vkey") . "\">$vtext</label>\n");
      }
      print("</blockquote>\n");
    }
    print("<p><b>Comments</b></p>\n");
    print("<blockquote><textarea name=\"" . $this->name("comments") . "\" rows=8 cols=50 wrap=virtual>" . htmlspecialchars($this->taRating->comments) . "</textarea></blockquote>\n");
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

  function TaSurvey($class_id, $name, &$parent)
  {
    $this->StatefullWidget($name, $parent);
    $this->class_id = $class_id;
  }

  function & handleEvent($event, $param, $isNew)
  {
    if ($event == OldSurvey_taedit)
    {
      $this->taWidget =& new TaWidget('taWidget', $this);
      $this->taWidget->index = is_numeric($param) ? $param : -1;
      $this->taWidget->loadRating(isset($this->tas[$param]) ? $this->tas[$param] : new TaRating);
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
    return $null;
  }

  function loadState($new)
  {
    assert(isset($new));
    if (!$new) $this->tas = unserialize($this->readValue('tas'));    
    StatefullWidget::loadState($new);
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

        if ($value->user_id)
        {
          global $wces;
          wces_connect();
          $r = pg_go("
            SELECT btrim(firstname || ' ' || lastname) AS name 
            FROM users WHERE user_id = {$value->user_id}
          ", $wces, __FILE__, __LINE__);
          $taname = pg_result($r, 0, 0);
        }
        else
          $taname = $value->name ? $value->name : "??";

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

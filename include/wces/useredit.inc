<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
require_once("widgets/SqlBox.inc");

/*

CREATE TABLE pictures
(
  file_id INTEGER NOT NULL PRIMARY KEY DEFAULT(nextval('picture_ids')),
  name TEXT NOT NULL
);

CREATE SEQUENCE picture_ids;

ALTER TABLE professor_data ADD COLUMN picture_id INTEGER;
ALTER TABLE professor_data ADD FOREIGN KEY (picture_id) REFERENCES pictures;

CREATE FUNCTION insert_picture(TEXT) RETURNS INTEGER AS '
DECLARE
  filename ALIAS FOR $1;
  id INTEGER;
BEGIN
  SELECT INTO id file_id FROM pictures WHERE name = filename;
  IF FOUND THEN
    RETURN id;
  END IF;
  INSERT INTO pictures (name) VALUES (filename);
  RETURN currval(''picture_ids'');
END;
' LANGUAGE 'plpgsql';

UPDATE professor_data SET picname = NULL, picture_id = NULL WHERE picname = '';
UPDATE professor_data SET picture_id = insert_picture(picname) WHERE picture_id IS NULL AND picname IS NOT NULL;

*/

class ImageUpload extends Widget
{
  var $file_path;
  var $maxsize = 1048576; // 1 megabyte

  var $errors = array();
  var $picture_id = null;

  function ImageUpload($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function loadState($new)
  {
    assert(isset($new));
    if ($new) return;
    
    global $wces, $WCES_CONFIG_UPLOAD_PHYSICAL_PATH;

    $extensions = array(1 => ".gif", 2 => ".jpg", 3 => ".png");

    $file = &$_FILES[$this->name()];
    
    if (is_array($file) && is_uploaded_file($tname = $file['tmp_name']))
    {
      $fname = $file['name'];

      if (!($i = GetImageSize($tname)) || !isset($extensions[$i[2]]))
      {
        $this->errors[] = "Uploaded file is not a JPEG, GIF, or PNG image";
        return;
      }
      
      // add the extension if it doesn't already exist
      $ext = $extensions[$i[2]];
      $j = strlen($ext);

      if (strlen($fname) < $j || strcasecmp($ext, substr($fname, -$j)) != 0)
        $fname .= $ext;

      // get a unique number
      wces_connect();
      $r = pg_go("SELECT nextval('picture_ids')", $wces, __FILE__, __LINE__);
      $picture_id = (int)pg_result($r, 0, 0);

      // resulting filename
      $this->file_name = $picture_id . '-' . $fname;
      
      $whole_name = $WCES_CONFIG_UPLOAD_PHYSICAL_PATH . $this->file_name;
      
      if (!move_uploaded_file($tname, $whole_name))
      {
        $this->errors[] = "Unable to rename '$tname' to '$whole_name'";
        return;
      };
        
      $r = pg_go("INSERT INTO pictures (file_id, name) VALUES ($picture_id, '"
        . addslashes($this->file_name) . "')", $wces, __FILE__, __LINE__);
      $this->picture_id = $picture_id;
    }
    else
      $this->picture_id = $this->readValue("id");
  }

  function displayHidden()
  {
    $this->printValue("id", $this->picture_id);
  }
  
  function display()
  {
    $this->printValue("id", $this->picture_id);
    print('<input name="' . $this->name() . '" type=file>');
  }
}

define("UserEditor_save", 1);
define("UserEditor_cancel", 2);
define("UserEditor_preview", 3);

class UserEditor extends ParentWidget
{
  var $user_id;

  var $show_prof = true;
  var $show_admin = true;
  var $done = false;
  var $saved = false;

  var $message = "";
  var $errors = array();
  
  function UserEditor($user_id, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->user_id    = (int)$user_id;
    $this->event      =& new EventWidget("event", $this);
    $this->firstName  =& new TextBox(0, 30, "", "firstName", $this);
    $this->lastName   =& new TextBox(0, 30, "", "lastName", $this);
    $this->email      =& new TextBox(0, 30, "", "email", $this);
    $this->department =& new SqlBox("
      SELECT department_id AS id, (code::text || ' - ' || name::text) AS name
      FROM departments ORDER BY code
    ", false, "department", $this);
    
    if ($this->show_prof)
    {
      $this->url       =& new TextBox(0, 60, "", "url", $this);
      $this->statement =& new TextBox(6, 60, "", "statement", $this);
      $this->profile   =& new TextBox(6, 60, "", "profile", $this);
      $this->education =& new TextBox(6, 60, "", "education", $this);
      $this->picture   =& new ImageUpload("picture", $this);
    };
    
    if ($this->show_admin)
    {
      $this->is_admin =& new CheckBox("is_admin", $this);
      $this->is_dadmin =& new CheckBox("is_dadmin", $this);
      $this->is_prof =& new CheckBox("is_prof", $this);
      $this->is_student =& new CheckBox("is_student", $this);
      $this->optout =& new CheckBox("optout", $this);
    }
  }

  function loadInitialState()
  {
    global $wces;
    
    wces_connect();
    $r = pg_go("
      SELECT u.firstname, u.lastname, u.email, u.department_id, d.url, 
       d.statement, d.profile, d.education, d.picture_id, u.flags
      FROM users AS u
      LEFT JOIN professor_data AS d USING (user_id)
      WHERE u.user_id = $this->user_id", $wces, __FILE__, __LINE__);
    
    if (pg_numrows($r) != 1)
    {
      $this->errors[] = "User does not exist";
      return;
    }
    
    extract(pg_fetch_row($r, 0, PGSQL_ASSOC));
    
    $this->firstName->text = $firstname;
    $this->lastName->text = $lastname;
    $this->email->text = $email;
    $this->department->selected = $department_id;
    
    if ($this->show_prof)
    {
      $this->url->text = $url;
      $this->statement->text = $statement;
      $this->profile->text = $profile;
      $this->education->text = $education;
      $this->picture->picture_id = $picture_id;  
    };

    if ($this->show_admin)
    {
      $this->optout->checked = (bool)($flags & 0x80);
      $this->is_admin->checked = (bool)($flags & 0x1);
      $this->is_dadmin->checked = (bool)($flags & 0x2);
      $this->is_prof->checked = (bool)($flags & 0x4);
      $this->is_student->checked = (bool)($flags & 0x8);
    }
  }
  
  function save()
  {
    global $wces;
    wces_connect();

    if (strlen(trim($this->lastName->text)) == 0)
      $this->errors[] = "The last name field cannot be blank.";
    
    if (count($this->errors)) return false;

    $fn = quot($this->firstName->text);
    $ln = quot($this->lastName->text);
    $em = nullquot($this->email->text);
    $dt = $this->department->selected ? (int)$this->department->selected : "NULL";

    $result = true;
    $result = $result && pg_go("
      UPDATE users SET
        firstname = $fn, lastname = $ln, email = $em, department_id = $dt
      WHERE user_id = $this->user_id
    ", $wces, __FILE__, __LINE__);
    
    if ($this->show_prof)
    {
      $ur = nullquot($this->url->text);
      $st = nullquot($this->statement->text);
      $pr = nullquot($this->profile->text);
      $ed = nullquot($this->education->text);
      $pi = $this->picture->picture_id ? (int)$this->picture->picture_id : "NULL";

      $r= pg_go("SELECT EXISTS(SELECT * FROM professor_data WHERE user_id = $this->user_id)", $wces, __FILE__, __LINE__);
      if (pg_result($r, 0, 0) == 't')
      {
        $result = $result && pg_go("
          UPDATE professor_data SET
            url = $ur,
            statement = $st,
            profile = $pr,
            education = $ed,
            picture_id = $pi
          WHERE user_id = $this->user_id
        ", $wces, __FILE__, __LINE__);
      }
      else
      {
        $result = $result && pg_go("
          INSERT INTO professor_data (user_id, url, statement, profile, education, picture_id)
          VALUES ($this->user_id, $ur, $st, $pr, $ed, $pi)
        ", $wces, __FILE__, __LINE__);     
      };
    };

    if ($this->show_admin)
    {
      $plus = $minus = 0;
      if ($this->optout->checked) $plus |= 0x80; else $minus |= 0x80;
      if ($this->is_admin->checked) $plus |= 0x1; else $minus |= 0x1;
      if ($this->is_dadmin->checked) $plus |= 0x2; else $minus |= 0x2;
      if ($this->is_prof->checked) $plus |= 0x4; else $minus |= 0x4;
      if ($this->is_student->checked) $plus |= 0x8; else $minus |= 0x8;
      $result = $result && pg_go("UPDATE users SET flags = ((flags | $plus) & ~$minus) WHERE user_id = $this->user_id
      ", $wces, __FILE__, __LINE__);
    }

    return (bool)$result;
  }
  
  function loadState($new)
  {
    assert(isset($new));
    ParentWidget::loadState($new);
    
    if ($new) return $this->loadInitialState();

    if ($this->event->event == UserEditor_save)
    {
      if ($this->save())
      {
        $this->done = true;
        $this->saved = true;
        $this->message = "<p><font color=blue>User Editor: Changes saved successfully</font></p>";
      }
      else
        $this->errors[] = "Save failed";
    }
    else if ($this->event->event == UserEditor_cancel)
    {
      $this->done = true;
      $this->message = "<p><font color=red>User Editor: No changes were saved.</font></p>";
    }
    else if ($this->event->event == UserEditor_preview)
    {
      // nothing to do
    }
    array_splice($this->errors, count($this->errors), 0, $this->picture->errors);
  }
  
  function printVisible()
  {
    if (count($this->errors) > 0)
    {
      print("<p>Please correct the following errors:</p>\n<ul>\n");
      foreach($this->errors as $e)
        print("  <li>$e</li>\n");
      print("</ul>\n");
    }
?>
<table>

<tr><td>First Name:</td><td><? $this->firstName->display(); ?></td></tr>
<tr><td>Last Name:</td><td><? $this->lastName->display(); ?></td></tr>
<tr><td valign=top>Email:</td><td><? $this->email->display(); ?>
<? if ($this->show_admin) { ?>
<br>
<? $this->optout->display(); $this->optout->displaylabel("Don't send reminder emails"); ?>
<? } ?>
</td></tr>

<? if ($this->show_admin) { ?>
<tr><td valign=top>Status:</td><td>
<? $this->is_admin->display(); $this->is_admin->displaylabel("Administrator"); ?><br>
<? $this->is_dadmin->display(); $this->is_dadmin->displaylabel("Department Administrator"); ?><br>
<? $this->is_prof->display(); $this->is_prof->displaylabel("Professor"); ?><br>
<? $this->is_student->display(); $this->is_student->displaylabel("Student"); ?>
</td></tr>
<? } ?>

<tr><td>Department:</td><td><? $this->department->display(); ?></td></tr>

<? if ($this->show_prof) { ?>
<tr><td>URL:</td><td><? $this->url->display(); ?></td></tr>
<tr><td valign=top>Statement:</td><td><? $this->statement->display(); ?></td></tr>
<tr><td valign=top>Profile:</td><td><? $this->profile->display(); ?></td></tr>
<tr><td valign=top>Education:</td><td><? $this->education->display(); ?></td></tr>
<tr><td valign=top>Picture:</td><td><? $this->picture->display(); 
$this->event->displayButton("Preview", UserEditor_preview); ?><br>
<?
  $src = picture_src($this->picture->picture_id);
  if ($src) print("<img src=\"$src\">"); else print("<p><i>No Photo Uploaded</i></p><br>");
?>
</td></tr>
<? } ?>
<tr><td>&nbsp;</td><td><? $this->event->displayButton("Save", UserEditor_save); $this->event->displayButton("Cancel", UserEditor_cancel); ?></td></tr>
</table>
<?
  }
};

?>
<?

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
  var $picture_id = NULL;

  function ImageUpload($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function loadValues()
  {
    global $wces, $HTTP_POST_FILES, $server_upload;

    $extensions = array(1 => ".gif", 2 => ".jpg", 3 => ".png");

    $file = &$HTTP_POST_FILES[$this->name];
    
    if (is_array($file) && is_uploaded_file($tname = $file['tmp_name']))
    {
      $fname = $file['name'];

      if (!($i = GetImageSize($tname)) || !isset($extensions[$i[2]]))
      {
        $this->errors[] = "Uploaded file is not an image";
        return;
      }
      
      // add the extension if it doesn't already exist
      $ext = $extensions[$i[2]];
      $j = strlen($ext);

      if (strlen($fname) < $j || strcasecmp($ext, substr($fname, -$j)) != 0)
        $fname .= $ext;

      // get a unique number
      wces_connect();
      $r = pg_query("SELECT nextval('picture_ids')", $wces, __FILE__, __LINE__);
      $picture_id = (int)pg_result($r, 0, 0);

      // resulting filename
      $this->file_name = $picture_id . '-' . $fname;
      
      $whole_name = $server_upload . $this->file_name;
      
      if (!move_uploaded_file($tname, $whole_name))
      {
        $this->errors[] = "Unable to rename '$tname' to '$whole_name'";
        return;
      };
        
      $r = pg_query("INSERT INTO pictures (file_id, name) VALUES ($picture_id, '"
        . addslashes($this->file_name) . "')", $wces, __FILE__, __LINE__);
      $this->picture_id = $picture_id;
    }
    else
      $this->picture_id = $this->loadValue("id");
  }

  function display()
  {
    if ($this->picture_id) $this->printValue("id", $this->picture_id);
    print('<input name="' . $this->name . '" type=file>');
  } 
}

define("UserEditor_save", 1);
define("UserEditor_cancel", 2);
define("UserEditor_preview", 3);

class SqlBox extends Widget
{
  var $sql;
  var $multiple;
  var $selected;
  
  function SqlBox($sql, $multiple, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->sql = (string)$sql;
    $this->multiple = (bool)$multiple;
    $this->selected = $multiple ? array() : null;
  }

  function loadValues()
  {
    $selected = $this->loadValue();
    if (isset($selected))
    {
      if (!$this->multiple)
        $this->selected = (int)$selected;
      else if (is_array($selected))
        $this->selected = $selected;
    }
  }
  
  function displayHidden()
  {
    $name = $this->name . ($this->multiple ? '[]' : '');
    if (isset($this->selected))
    {
      if (!$this->multiple)
          print("<input type=hidden name=$name value=" . ((int)$this->selected) . "$params>");    
      else
      {
        foreach($this->selected as $s)
          print("<input type=hidden name=$name value=" . ((int)$s) . "$params>");
      }
    }
  }
  
  function display($params="")
  {
    global $wces;
    $name = $this->name . $this->multiple ? '[]' : '';
    if ($params) $params = " $params";

    wces_connect();
    $result = pg_query($this->sql, $wces, __FILE__, __LINE__);
    $n = pg_numrows($result);
    
    print("<select name=$name$params>\n");
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result, $i, PGSQL_ASSOC));
      if ((!$this->multiple && $id == $this->selected) || ($this->multiple && in_array($id, $this->selected)))
        print("  <option value=$id selected>$name\n");
      else  
        print("  <option value=$id>$name\n");
    }
    print("</select>");
  }
};

class UserEditor extends Widget
{
  var $user_id;


  var $show_prof = true;
  var $show_admin = true;
  var $done = false;

  var $message = "";
  var $errors = array();
  
  function UserEditor($user_id, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->user_id    = (int)$user_id;
    $this->action     = & new ActionButton("action", $this);
    $this->firstName  = & new TextBox(0, 30, "", "firstName", $this);
    $this->lastName   = & new TextBox(0, 30, "", "lastName", $this);
    $this->email      = & new TextBox(0, 30, "", "email", $this);
    $this->department = & new SqlBox("
      SELECT department_id AS id, (code::text || ' - ' || name::text) AS name
      FROM departments ORDER BY code
    ", false, "department", $this);
    
    if ($this->show_prof)
    {
      $this->url       = & new TextBox(0, 60, "", "url", $this);
      $this->statement = & new TextBox(6, 60, "", "statement", $this);
      $this->profile   = & new TextBox(6, 60, "", "profile", $this);
      $this->education = & new TextBox(6, 60, "", "education", $this);
      $this->picture   = & new ImageUpload("picture", $this);
    };
    
    if ($this->show_admin)
    {
      $this->optout = & new CheckBox("optout", $this);
    }
  }

  function loadDefaults()
  {
    global $wces;
    
    wces_connect();
    $r = pg_query("
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
      $this->optout->checked = (bool)($flags & 16);
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
    $result = $result && pg_query("
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

      $r= pg_query("SELECT EXISTS(SELECT * FROM professor_data WHERE user_id = $this->user_id)", $wces, __FILE__, __LINE__);
      if (pg_result($r, 0, 0) == 't')
      {
        $result = $result && pg_query("
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
        $result = $result && pg_query("
          INSERT INTO professor_data (user_id, url, statement, profile, education, picture_id)
          VALUES ($this->user_id, $ur, $st, $pr, $ed, $pi)
        ", $wces, __FILE__, __LINE__);     
      };
    };

    if ($this->show_admin)
    {
      $oo = $this->optout->checked ? "flags | 16" : "flags & ~16";
      $result = $result && pg_query("UPDATE users SET flags = $oo WHERE user_id = $this->user_id
      ", $wces, __FILE__, __LINE__);
    }

    return (bool)$result;
  }
  
  function loadValues()
  {
    Widget::loadValues();

    if ($this->action->action == UserEditor_save)
    {
      if ($this->save())
      {
        $this->done = true;
        $this->message = "<p><font color=blue>User Editor: Changes saved successfully</font></p>";
      }
    }
    else if ($this->action->action == UserEditor_cancel)
    {
      $this->done = true;
      $this->message = "<p><font color=red>User Editor: No changes were saved.</font></p>";
    }
    else if ($this->action->action == UserEditor_preview)
    {
      // nothing to do
    }
    array_splice($this->errors, count($this->errors), 0, $this->picture->errors);
  }
  
  function display()
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
<tr><td valign=top>Email:</td><td><? $this->email->display(); ?><br>

<? if ($this->show_admin) { ?>
<? $this->optout->display(); $this->optout->displaylabel("Don't send reminder emails"); ?></td></tr>
<? } ?>

<tr><td>Department:</td><td><? $this->department->display(); ?></td></tr>

<? if ($this->show_prof) { ?>
<tr><td>URL:</td><td><? $this->url->display(); ?></td></tr>
<tr><td valign=top>Statement:</td><td><? $this->statement->display(); ?></td></tr>
<tr><td valign=top>Profile:</td><td><? $this->profile->display(); ?></td></tr>
<tr><td valign=top>Education:</td><td><? $this->education->display(); ?></td></tr>
<tr><td valign=top>Picture:</td><td><? $this->picture->display(); 
$this->action->display("Preview", UserEditor_preview); ?><br>
<?
  $src = picture_src($this->picture->picture_id);
  if ($src) print("<img src=\"$src\">"); else print("<p><i>No Photo Uploaded</i></p><br>");
?>
</td></tr>
<? } ?>
<tr><td>&nbsp;</td><td><? $this->action->display("Save", UserEditor_save); $this->action->display("Cancel", UserEditor_cancel); ?></td></tr>
</table>
<?
  }
};

?>
<?

define("IMAGE_PATH", "/home/httpd/apps/wces/www/wces/upload");

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

class ImageUpload extends FormWidget
{
  var $file_path;
  var $maxsize = 1048576; // 1 megabyte
  var $file_name;
  
  var $errors = array();
  var $picture_id = NULL;

  function ImageUpload($file_path, $prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->file_path = $file_path;
  }

  function loadvalues()
  {
    global $wces;
    global $HTTP_POST_FILES;

    $extensions = array(1 => ".gif", 2 => ".jpg", 3 => ".png");

    $file = &$HTTP_POST_FILES[$this->prefix];
    
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
      $r = pg_go("SELECT nextval('picture_ids')", $wces, __FILE__, __LINE__);
      $picture_id = (int)pg_result($r, 0, 0);

      // resulting filename
      $this->file_name = $picture_id . '-' . $fname;
      
      $whole_name = $this->file_path . '/' . $this->file_name;
      
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
      $this->picture_id = $this->loadAttribute("id");
  }

  function display()
  {
    if ($this->picture_id)
      $this->printattribute("id", $this->picture_id);

    print('<input name="' . $this->prefix . '" type=file>');
  } 
}

define("UserEditor_save", 1);
define("UserEditor_cancel", 2);
define("UserEditor_preview", 3);

class SqlBox extends FormWidget
{
  var $sql;
  var $multiple;

  var $selected;
  
  function SqlBox($sql,$multiple,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->sql = (string)$sql;
    $this->multiple = (bool)$multiple;
    $this->selected = $multiple ? array() : NULL;
  }
  
  function display($hidden = false, $params="")
  {
    global $wces;
    $suffix = $this->multiple ? '[]' : '';
    $nm = '"' . $this->prefix . $suffix . '"';
    if ($params) $params = " $params";
    
    if ($hidden)
    {
      if (isset($this->selected))
      {
        if (!$this->multiple)
            print("<input type=hidden name=$nm value=" . ((int)$this->selected) . "$params>");    
        else
        {
          foreach($this->selected as $s)
            print("<input type=hidden name=$nm value=" . ((int)$s) . "$params>");
        };
      }
    }
    else
    {
      wces_connect();

      $result = pg_go($this->sql, $wces, __FILE__, __LINE__);
      
      $n = pg_numrows($result);
      
      print("<select name=$nm$params>\n");
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
  }

  function loadvalues()
  {
    $selected = $this->loadattribute();
    if (isset($selected))
    {
      if (!$this->multiple)
        $this->selected = (int)$selected;
      else if (is_array($selected))
        $this->selected = $selected;
    }
  }
};

class UserEditor extends FormWidget
{
  var $done = false;
  var $message =   "";
  var $errors = array();
  
  var $show_prof = true;
  var $show_admin = true;
  
  function UserEditor($user_id, $prefix, $form, $formmethod)
  {
    global $dartSemesters;
    $this->user_id = (int)$user_id;
    $this->FormWidget($prefix, $form, $formmethod);

    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
    $this->firstName = new TextBox(0, 30, "", "{$prefix}_firstName", $form, $formmethod);
    $this->lastName  = new TextBox(0, 30, "", "{$prefix}_lastName", $form, $formmethod);
    $this->email     = new TextBox(0, 30, "", "{$prefix}_email", $form, $formmethod);
    $this->department = new SqlBox("
      SELECT department_id AS id, (code::text || ' - ' || name::text) AS name
      FROM departments ORDER BY code
    ", false, "{$prefix}_department", $form, $formmethod);
    
    if ($this->show_prof)
    {
      $this->url = new TextBox(0, 60, "", "{$prefix}_url", $form, $formmethod);
      $this->statement = new TextBox(6, 60, "", "{$prefix}_statement", $form, $formmethod);
      $this->profile = new TextBox(6, 60, "", "{$prefix}_profile", $form, $formmethod);
      $this->education = new TextBox(6, 60, "", "{$prefix}_education", $form, $formmethod);
      $this->picture = new ImageUpload(IMAGE_PATH, "{$prefix}_picture", $form, $formmethod);
    };
    
    $this->optout = new CheckBox("{$prefix}_optout", $form, $formmethod);
  }

  function loadDefaults()
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
    
    $this->optout->checked = (bool)($flags & 128);
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
    $oo = $this->optout->checked ? "flags | 128" : "flags & ~128";
    $result = true;
    
    $result = $result && pg_go("
      UPDATE users SET
        firstname = $fn, lastname = $ln, email = $em, department_id = $dt, flags = $oo
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
    
    return (bool)$result;
  }
  
  function loadValues()
  {
    $this->action->loadValues();
    $this->firstName->loadValues();
    $this->lastName->loadValues();
    $this->email->loadValues();
    $this->department->loadValues();
    
    if ($this->show_prof)
    {
      $this->url->loadValues();
      $this->statement->loadValues();
      $this->profile->loadValues();
      $this->education->loadValues();
      $this->picture->loadValues();
    };

    $this->optout->loadValues();
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
    {}
    array_splice($this->errors, count($this->errors), 0, $this->picture->errors);
  }
  
  function display()
  {
    global $dart_semesters;
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
<? $this->optout->display(); $this->optout->displaylabel("Don't send reminder emails"); ?></td></tr>
<tr><td>Department:</td><td><? $this->department->display(); ?></td></tr>

<? if ($this->show_prof)
{
  $src = picture_src($this->picture->picture_id);
?>
<tr><td>URL:</td><td><? $this->url->display(); ?></td></tr>
<tr><td valign=top>Statement:</td><td><? $this->statement->display(); ?></td></tr>
<tr><td valign=top>Profile:</td><td><? $this->profile->display(); ?></td></tr>
<tr><td valign=top>Education:</td><td><? $this->education->display(); ?></td></tr>
<tr><td valign=top>Picture:</td><td><? $this->picture->display();
$this->action->display("Preview", UserEditor_preview); ?>
<?
  if ($src) print("<br><img src=\"$src\">"); else print("<p><i>No Photo Uploaded</i></p><br>");

?>
</td></tr>
<? } ?>
<tr><td>&nbsp;</td><td><? $this->action->display("Save", UserEditor_save); $this->action->display("Cancel", UserEditor_cancel); ?></td></tr>
</table>
<?
  }
};


?>
<?

require_once("widgets/widgets.inc");
require_once("widgets/basic.inc");
require_once("wces/wces.inc");
require_once("wbes/postgres.inc");
require_once("wces/page.inc");
require_once("wces/login.inc");

$db_debug = true;

define("IMAGE_PATH", "c:/server/shares/wwwroot/image/");

function image_href($id)
{
  global $wces;
  
  if (!$id) return "badurl";
  
  
  wces_connect();
  
  $r = pg_go("SELECT name FROM pictures WHERE picture_id = $id", $wces, __FILE__, __LINE__);
  return "/image/" . pg_result($r, 0, 0);
};

/*

CREATE TABLE pictures
(
  file_id INTEGER NOT NULL PRIMARY KEY DEFAULT(nextval('picture_ids')),
  name TEXT NOT NULL
);

CREATE SEQUENCE picture_ids;

ALTER TABLE professor_data ADD FOREIGN KEY (picture_id) REFERENCES pictures;

ALTER TABLE classes ADD FOREIGN KEY (course_id) REFERENCES courses(course_id);

// syntax error her
CREATE FOREIGN KEY professor_data(picture_id) REFERENCES pictures;

fields
  first
  last
  email
  department

  url
  picture
  statement
  profile
  education
  
  department access
  email block

*/

class ImageUpload extends Widget
{
  var $file_path;
  var $maxsize = 1048576; // 1 megabyte
  var $file_name;
  
  var $errors = array();
  var $picture_id = NULL;

  function ImageUpload($file_path, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->file_path = $file_path;
  }

  function loadState()
  {
    global $wces;
    
    $tname = $this->readValue("");
    $fname = $this->readValue("name");
    $extensions = array(1 => ".gif", 2 => ".jpg", 3 => ".png");

    print("<h1>" . htmlspecialchars($_POST[$this->prefix]) . "</h1>");
    
    if (is_uploaded_file($_POST[$this->prefix]))
    {
      if (!$i = GetImageSize($tname) || !isset($extensions[$i[2]]))
      {
        $this->errors[] = "Uploaded file is not an image";
        return;
      }

      // add the extension if it doesn't already exist
      $ext = $extensions[$i[2]];
      $j = strlen($ext);
      if (strlen($fname) < $j || stricmp($ext, substr($fname, -$j)) != 0)
        $fname .= $ext;
      
      // get a unique number
      wces_connect();
      $r = pg_go("SELECT nextval('picture_ids')", $wces, __FILE__, __LINE_);
      $picture_id = (int)pg_result($r, 0, 0);

      // resulting filename
      $this->file_name = $picture_id . '-' . $fname;
      
      $whole_name = $this->file_path . '/' . $this->file_name;
      
      if (!move_uploaded_file($fname, $whole_name))
      {
        $this->errors[] = "Unable to rename '$tname' to '$whole_name'";
        return;
      };
        
      $r = pg_go("INSERT INTO pictures (picture_id, $name) VALUES ($picture_id, '"
        . addslashes($this->file_name) . "')", $wces__FILE__, __LINE__);
    }
    else
      $this->picture_id = $this->readValue("id");
  }

  function printState()
  {
    if ($this->picture_id)
      $this->printattribute("id", $this->picture_id);
  }
  
  function displayHidden()
  {
    $this->printState(); 
  }
  
  function display()
  {
    $this->printState();
    print('<input name="' . $this->prefix . '" type=file>');
  }
}

class SqlBox extends Widget
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
  
  function printHidden()
  {
    $suffix = $this->multiple ? '[]' : '';
    $n = '"' . $this->prefix . $suffix . '"';
    if ($params) $params = " $params";

  }
  
  function printVisible()
  {
    $suffix = $this->multiple ? '[]' : '';
    $n = '"' . $this->prefix . $suffix . '"';
    if ($params) $params = " $params";

  }
  
  function display($hidden = false, $params="")
  {
    global $wces;
    
    if ($hidden)
    {
      if (isset($this->selected))
      {
        if (!$this->multiple)
            print("<input type=hidden name=$n value=" . ((int)$this->selected) . "$params>");    
        else
        {
          foreach($this->selected as $s)
            print("<input type=hidden name=$n value=" . ((int)$s) . "$params>");
        };
      }
    }
    else
    {
      wces_connect();

      $result = pg_go($this->sql, $wces, __FILE__, __LINE__);
      
      $n = pg_numrows($result);
      
      print("<select name=$n$params>\n");
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

  function loadState()
  {
    Widget::loadState();
    $selected = $this->readValue();
    if (isset($selected))
    {
      if (!$this->multiple)
        $this->selected = (int)$selected;
      else if (is_array($selected))
        $this->selected = $selected;
    }
  }
};

define("UserEditor_save", 1);
define("UserEditor_cancel", 2);
define("UserEditor_preview", 3);

class UserEditor extends ParentWidget
{
  var $done = false;
  var $message = "";
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
      $this->url = new TextBox(0, 30, "", "{$prefix}_url", $form, $formmethod);
      $this->statement = new TextBox(0, 30, "", "{$prefix}_statement", $form, $formmethod);
      $this->profile = new TextBox(0, 30, "", "{$prefix}_profile", $form, $formmethod);
      $this->education = new TextBox(0, 30, "", "{$prefix}_education", $form, $formmethod);
      $this->picture = new ImageUpload(IMAGE_PATH, "{$prefix}_picture", $form, $formmethod);
    };
  }

  function loadDefaults()
  {
    global $wces;
    
    wces_connect();
    $r = pg_go("
      SELECT u.firstname, u.lastname, u.email, u.department_id, d.url, 
       d.statement, d.profile, d.education, d.picture_id
      FROM users AS u
      LEFT JOIN professor_data AS d USING (user_id)
      WHERE u.user_id = $this->user_id", $wces, __FILE__, __LINE__);
    
    assert(pg_numrows($r) == 1);
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
    $dt = (int)$this->department->selected;

    $result = true;
    
    $result = $result && pg_go("
      UPDATE users SET
        firstname = $fn, lastname = $ln, email = $em, department_id = $dt
      WHERE user_id = $this->user_id
    ", $wces, __FILE__, __LINE__);
    
    if ($this->show_prof)
    {
      $ur = nullquot($this->url);
      $st = nullquot($this->statement);
      $pr = nullquot($this->profile);
      $ed = nullquot($this->education);
      $pi = (int)$this->picture->picture_id;

      $r= pg_go("SELECT EXISTS(SELECT * FROM professor_data WHERE user_id = $this->user_id)", $wces, __FILE__, __LINE__);
      if (pg_result($r, 0, 0) == 't')
      {
        $result = $result && pg_result("
          UPDATE professor_data SET
            url = $ur,
            statement = $st,
            profile = $pr,
            education = $ed,
            picture_id = $pi,
          WHERE user_id = $this->user_id
        ", $wces, __FILE__, __LINE__);
      }
      else
      {
        $result = $result && pg_result("
          INSERT INTO professor_data (url, statement, profile, education, picture_id)
          VALUES ($ur, $st, $pr, $ed, $pi)
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

    if ($this->action->action == UserEditor_save)
    {
      if ($this->save())
      {
        $this->done = true;
        $this->message = "<p><font color=blue>Question Period Editor: Changes saved successfully</font></p>";
      }
    }
    else if ($this->action->action == UserEditor_cancel)
    {
      $this->done = true;
      $this->message = "<p><font color=red>Question Period Editor: No changes were saved.</font></p>";
    }
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
<tr><td>Email:</td><td><? $this->email->display(); ?></td></tr>
<tr><td>Department:</td><td><? $this->department->display(); ?></td></tr>

<? if ($this->show_prof) { ?>
<tr><td>URL:</td><td><? $this->url->display(); ?></td></tr>
<tr><td>Statement:</td><td><? $this->statement->display(); ?></td></tr>
<tr><td>Profile:</td><td><? $this->profile->display(); ?></td></tr>
<tr><td>Education:</td><td><? $this->education->display(); ?></td></tr>
<tr><td>Picture:</td><td><? $this->picture->display();
$this->action->display("Preview", UserEditor_preview); ?><br>
<img src="<?=image_href($this->picture->picture_id)?>">
</td></tr>
<? } ?>
<tr><td>&nbsp;</td><td><? $this->action->display("Save", UserEditor_save); $this->action->display("Cancel", UserEditor_cancel); ?></td></tr>
</table>
<?
  }
  
};

class UserList extends FormWidget
{
  var $user_id = 0;
  var $editor = NULL;
  var $modalChild = NULL;
  var $message = "";

  function UserList($prefix, $form, $formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
    $this->action = new ActionButton("{$prefix}_action", $form, $formmethod);
  }
  
  function loadValues()
  {
    $this->action->loadValues();
    $a = (int)$this->readValue("user_id");
    $editing = $a != 0;
    for(;;)
    if ($editing)
    {
      if ($a != 0) $this->user_id = $a;
      $this->editor = new UserEditor($this->user_id, "{$this->prefix}_editor", $this->form, $this->formmethod);
      if ($a != 0) $this->editor->loadValues(); else $this->editor->loadDefaults(); 
      $this->message .= $this->editor->message;
      if ($this->editor->done)
        $this->user_id = 0;
      else
      {
        $this->modalChild = &$this->editor;
        $this->editor->modal = true;
      }
      return;
    }   
    else
    {   
      switch ($this->action->action)
      {
        case UserList_edit:
          $editing = true;
          $this->user_id = $this->action->object == "new" ? -1 : (int)$this->action->object;
        break;
        case UserList_delete:
          $this->deleteq((int)$this->action->object);
          return;  
        default:
          return;
      }
    }
  }
  
  function deleteq($user_id)
  {
    global $wces;
    wces_connect();
    $ref = (int)pg_result(pg_go("SELECT references_user($user_id)", $wces, __FILE__, __LINE__),0,0);
    if ($ref != 0)
      $this->message = "<p><font color=red>Unable to delete question period $user_id because there are survey responses associated with it.</font></p>";
    else
      pg_go("DELETE FROM semester_users WHERE user_id = $user_id", $wces, __FILE__, __LINE__);
  }
  
  function display()
  {
    global $wces, $dartSemesters;
    print($this->message);
    $this->printAttribute("user_id", $this->user_id);
    if ($this->modalChild)
    {
      $this->modalChild->display();
      return;
    }

    wces_connect();
    $r = pg_go("SELECT user_id, displayname, EXTRACT(EPOCH FROM begindate) AS begindate, EXTRACT(EPOCH FROM enddate) AS enddate, semester, year FROM semester_users", $wces, __FILE__, __LINE__);
    $n = pg_numrows($r);

    print("<table border=1>\n");
    print("<tr><td><b>ID</b></td><td><b>Term</b></td><td><b>Name</b></td><td><b>Begin Time</b></td><td><b>End Time</b></td><td>&nbsp;</td></tr>\n");

    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_row($r, $i, PGSQL_ASSOC));
      $bd = format_date($begindate);
      $ed = format_date($enddate);
      print("<tr><td>$user_id</td><td>$dartSemesters[$semester] $year</td><td>$displayname</td><td>$bd</td><td>$ed<td>");
      $this->action->display("Edit...", UserList_edit, $user_id);
      $this->action->display("Delete", UserList_delete, $user_id);
      print("</td></tr>\n");
    }
    print("</table>\n");
    print("<p>");
    $this->action->display("New Question Period...", UserList_edit, "new");
    print("</p>");
  }
}

page_top("Question Periods");
print('<form enctype="multipart/form-data" method=post><input type=hidden name=MAX_FILE_SIZE value=1048576>');

print($ISID);
$q = new UserEditor(864, "ue", "f", WIDGET_POST);
$f = new Form("ue", "f", WIDGET_POST);
$f->loadValues();
$f->display();

if ($f->isstale)
  $q->loadValues();
else
  $q->loadDefaults();
  
$q->display();

print("</form>\n");
page_bottom();

?>

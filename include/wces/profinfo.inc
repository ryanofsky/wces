<%

require_once("wces/wces.inc");
require_once("wces/import.inc");
require_once("widgets/basic.inc");

// states, actions
define("ProfessorEditor_edit",1);
define("ProfessorEditor_save",2);
define("ProfessorEditor_cancel",3);
define("ProfessorInfo_print",1);
define("ProfessorInfo_back",2);

class ProfessorInfo extends FormWidget
{
  var $db;
  var $professorid;
  var $action;
  var $printable;
  
  function ProfessorInfo($db,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->db = $db;
    $this->professorid = 0;
    $this->printable = false;
    $this->action = new ActionButton($prefix . "_action",$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->action->loadvalues();
    $this->printable = $this->action->action == ProfessorInfo_print;
    
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    if ($this->formmethod == WIDGET_GET)
      $thearray = &$HTTP_GET_VARS;
    else if ($this->formmethod == WIDGET_POST)
      $thearray = &$HTTP_POST_VARS;
    else
      assert(0);
    
    $prefix = $this->prefix . "_editbutton";
    $l = strlen($prefix);
    foreach($thearray as $key => $value)
    if ($value && $prefix == substr($key,0,$l))
    {
      $p = strrpos($key,'_');
      $this->professorid = substr($key,$p+1);
      if ($this->professorid) break;
    }
    
    if ($this->professorid)
    {
      $this->editor = new ProfessorEditor($this->db, $this->professorid, true, $this->prefix . "_editor", $this->form, $this->formmethod);
      $this->editor->loadvalues();
    }
  }
  
  function display()
  {
    global $server_wcespath;

    if ($this->professorid) $this->editor->display();

    if (!$this->professorid || ($this->editor->action->action == ProfessorEditor_cancel || $this->editor->action->action == ProfessorEditor_save))
    {
      wces_Findclasses($this->db,"cc");
      db_exec("REPLACE INTO cc(classid) SELECT classid FROM answersets GROUP BY classid",$this->db,__FILE__,__LINE__);
      $result = db_exec("
        SELECT p.professorid,
        SUBSTRING_INDEX(p.name,' ',-1) AS last,
        SUBSTRING(p.name,1,LENGTH(p.name)-LOCATE(' ',REVERSE(p.name))) AS first,
        IFNULL(u.cunix,'') AS cunix,
        IFNULL(p.email,'') AS email, 
        IFNULL(p.url,'') AS url,
        IFNULL(d.code,'?') AS department,
        IFNULL(p.picname,'') AS picname,
        IF(LENGTH(p.statement) > 0, 'yes', 'no') AS statement,
        IF(LENGTH(p.profile) > 0, 'yes', 'no') AS profile,
        IF(LENGTH(p.education) > 0, 'yes', 'no') AS education
        FROM cc
        INNER JOIN classes AS cl USING (classid)
        INNER JOIN professors AS p USING (professorid)
        LEFT JOIN users AS u USING (userid)
        LEFT JOIN departments AS d ON d.departmentid = p.departmentid
        GROUP BY p.professorid
        ORDER BY last, first
      ",$this->db,__FILE__,__LINE__);
      
      $this->action->display(false,ProfessorInfo_print,0);
      $this->action->display(false,ProfessorInfo_back,0);
%>
<table><tr><td height=50>
<span class=off onmousedown="this.className='down'" onmouseup="this.className = 'up'"  onmouseover="this.className = 'up'" onmouseout="this.className='off'"><a class=blank href="<%= $this->action->gethref($this->printable ? ProfessorInfo_back : ProfessorInfo_print , 0) %>"><img src="<%=$server_wcespath%>media/report/<%= $this->printable ? "back" : "print"%>.gif" border=0 width=20 height=20 alt="Icon"><%=$this->printable ? "Back" : "Printable version of this page"%></a></span> 
</td></tr></table>

<form id=form1 name=form1>
<table bordercolor=black cellspacing=0 cellpadding=3 RULES="groups" FRAME=box>
<thead>
<tr>
  <td bgcolor=black><font color=white><font color=white><STRONG>Name</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>CUNIX</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Email</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Department</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Homepage</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Statement</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Profile</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Education</STRONG></font></td>
  <td bgcolor=black><font color=white><STRONG>Picture</STRONG></font></td>
  <td bgcolor=black>&nbsp;</td>
</tr>
</thead>
<tbody>
<%
      $i = 0;
      while($row = mysql_fetch_assoc($result))
      {
        $color = ++$i %2 ? '#FFFFFF' : '#EEEEEE';
        $cunix = $email = $department = $url = $statement = $profile = $education = $picname = 0;
        extract($row);
        print("<tr>");
        print("  <td bgcolor=\"$color\"><a href=\"${server_wcespath}administrators/info.php?professorid=$professorid\">$last, $first</a></td>\n");
        print("  <td bgcolor=\"$color\">" . ($cunix ? "<a href=\"http://www.columbia.edu/cgi-bin/lookup.pl?$cunix\">$cunix</a>" : "?") . "</td>\n");
        print("  <td bgcolor=\"$color\">" . ($email ? "<a href=\"mailto:$email\">$email</a>" : "?") . "</td>\n");
        print("  <td bgcolor=\"$color\">$department</td>");
        print("  <td bgcolor=\"$color\">" . ($url ? "<a href=\"$url\">yes</a>" : "no") . "</td>\n");
        print("  <td bgcolor=\"$color\">$statement</td>\n");
        print("  <td bgcolor=\"$color\">$profile</td>\n");
        print("  <td bgcolor=\"$color\">$education</td>\n");
        print("  <td bgcolor=\"$color\">" . ($picname ? "<a href=\"/oracle/prof_images/$picname\">yes</a>" : "no") . "</td>\n");
        print("  <td bgcolor=\"$color\">" . '<input type=submit name="' . $this->prefix . '_editbutton_' . $professorid . '" value="Edit..." class="tinybutton" style="width:100%; BACKGROUND:' . $color . '; COLOR: #000090;"></td>' . "\n");
        print("</tr>\n");
      }
      print("</tbody>\n</table>\n</form>");
    }
    else
      $this->printattribute("editbutton_" . $this->professorid,1);
  }  
}

class ProfessorEditor extends FormWidget
{
  var $db;
  var $professorid;
  var $isadmin; // boolean
  
  var $form,$action,$name,$email,$url,$statement,$profile,$education;
  
  function ProfessorEditor($db,$professorid,$isadmin,$prefix,$form,$formmethod)
  {
    $this->db = $db;
    $this->professorid = $professorid;
    $this->isadmin = $isadmin;
    $this->FormWidget($prefix,$form,$formmethod);

    $this->form      = new Form($prefix . "_form", $form, $formmethod);
    $this->action    = new ActionButton($prefix . "_action",$form,$formmethod);
    $this->name      = new TextBox(0,50,"",$prefix . "_name",$form,$formmethod);
    $this->email     = new TextBox(0,50,"",$prefix . "_email",$form,$formmethod);
    $this->url       = new TextBox(0,50,"",$prefix . "_url",$form,$formmethod);
    $this->statement = new TextBox(6,50,"",$prefix . "_statement",$form,$formmethod);
    $this->profile   = new TextBox(6,50,"",$prefix . "_profile",$form,$formmethod);
    $this->education = new TextBox(6,50,"",$prefix . "_education",$form,$formmethod);
    if ($isadmin)
      $this->cunix = new TextBox(0,50,"",$prefix . "_cunix",$form,$formmethod);
  }
  
  function loadvalues()
  {
    $this->form->loadvalues();
    $this->action->loadvalues();

    if($this->form->isstale)
    {
      $this->name->loadvalues();
      $this->email->loadvalues();
      $this->url->loadvalues();
      $this->statement->loadvalues();
      $this->profile->loadvalues();
      $this->education->loadvalues();
      if ($this->isadmin)
        $this->cunix->loadvalues();
    }
    else
    {
      $r = db_getrow($this->db,"professors",Array("professorid" => $this->professorid),Array("name","url","email","education","statement","profile","userid"));
      $this->name->text = $r["name"];
      $this->email->text = $r["email"];
      $this->url->text = $r["url"];
      $this->statement->text = $r["statement"];
      $this->profile->text = $r["profile"];
      $this->education->text = $r["education"];
      if ($this->isadmin && $r["userid"])
      {
        $cunix = db_getvalue($this->db,"users",Array("userid" => $r["userid"]),"cunix");
        if ($cunix) $this->cunix->text = $cunix;
      }    
    }  
  }
  
  function display()
  {
    if ($this->action->action == ProfessorEditor_save)
    {
      $save = array
      (
        "name" => $this->name->text,
        "email" => $this->email->text,
        "url" => $this->url->text,
        "statement" => $this->statement->text,
        "profile" => $this->profile->text,
        "education" => $this->education->text
      );  

      if ($this->isadmin)
      {
        if ($this->cunix->text)
        {
          $save["userid"] = db_replace($this->db,"users",array("cunix" => $this->cunix->text),array("isprofessor" => "true"),"userid");
          db_updatevalues($this->db,"professors", array("userid" => $save["userid"]),array("userid" => false));
        }  
        else
          $save["userid"] = false;
      }

      db_updatevalues($this->db, "professors", array("professorid" => $this->professorid), $save);
      
      print("<p><strong>This data was saved:</strong></p>");
      print("<p><pre>");
      foreach($save as $key => $value)
        print("<b>$key: </b>$value\n");
      print("</pre></p>");
    }
    else if ($this->action->action != ProfessorEditor_cancel)
    {
      $this->form->display();
  %>
  <table>
    <tr><td valign=top><strong>Display Name:</strong></td><td><% $this->name->display(); %></td></tr>
  <% if ($this->isadmin) { %>  
    <tr><td valign=top><strong>CUNIX ID:</strong></td><td><% $this->cunix->display(); %></td></tr>
  <% }; %>  
    <tr><td valign=top><strong>Email:</strong></td><td><% $this->email->display(); %></td></tr>
    <tr><td valign=top><strong>Homepage:</strong></td><td><% $this->url->display(); %></td></tr>
    <tr><td valign=top><strong>Statement:</strong></td><td><% $this->statement->display(); %></td></tr>
    <tr><td valign=top><strong>Profile:</strong></td><td><% $this->profile->display(); %></td></tr>
    <tr><td valign=top><strong>Education:</strong></td><td><% $this->education->display(); %></td></tr>
    <tr><td>&nbsp;</td><td><% $this->action->display("Save",ProfessorEditor_save); %> <% if ($this->isadmin) $this->action->display("Cancel",ProfessorEditor_cancel); %></td></tr>
  </table>    
  <% 
    } 
  }
}

%>







<%

require_once("widgets/widgets.inc");

$ABETQUESTIONS = array(
  "Design experiments",
  "Analyze and interpret data",
  "Conduct experiments",
  "Analyze and interpret data",
  "Design a system, component, or process to meet desired needs",
  "Function on multidisciplinary teams",
  "Identify or formulate engineering problems",
  "Solve engineering problems",
  "Understand ethical responsibilities",
  "Understand the impact of engineering solutions in a global/societal context",
  "Use modern engineering tools",
  "Communicate using oral presentations",
  "Communicate using written reports",
  "Pilot test a component prior to implementation",
  "Use text materials to support project design",
  "Integrate knowledge of mathematics, science, and engineering in engineering solutions",
  "Apply knowledge of contemporary issues to engineering solutions",
  "Recognize need to engage in lifelong learning"
);

class OldQuestionWidget extends FormWidget
{
  var $errors; // string array containing error messages if question is not filled out properly, indexed by question name;
  var $questionsetid, $classid, $questionperiodid; // database identifiers

  var $db; //database handle
  var $action;
  
  function OldQuestionWidget($db,$questionsetid, $classid, $questionperiodid, $prefix,$form,$formmethod)
  {
    $this->db = $db;
    $this->questionsetid = $questionsetid;
    $this->classid = $classid;
    $this->questionperiodid = $questionperiodid;
    $this->FormWidget($prefix, $form, $formmethod);
  }
  
  function loadvalues()
  {
  }
  
  function display()
  {
    global $ABETQUESTIONS;
    
    $db = $this->db;
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $this->questionsetid), 0);
    extract($questionset);
    $first = true;
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = $this->prefix . "_MC$i";
      $index = "MC" . $i;
      $text = $questionset[$index];
      if ($text)
      {
        if ($first) 
        {
          $first = false;
          print("<h4>$displayname</h4>\n");
        }
        
        if ($this->errors[$fieldname]) $text = "<font color=red>$text</font>";
        $val = $this->loadattribute("MC$i");
%>
<p><b><%=$text%></b></p>
<blockquote>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>a" value="a" <%=$val == "a" ? "checked" : ""%>><label for="<%=$fieldname%>a">excellent</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>b" value="b" <%=$val == "b" ? "checked" : ""%>><label for="<%=$fieldname%>b">very good</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>c" value="c" <%=$val == "c" ? "checked" : ""%>><label for="<%=$fieldname%>c">satisfactory</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>d" value="d" <%=$val == "d" ? "checked" : ""%>><label for="<%=$fieldname%>d">poor</label>
  <input type=radio name="<%=$fieldname%>" id="<%=$fieldname%>e" value="e" <%=$val == "e" ? "checked" : ""%>><label for="<%=$fieldname%>e">disastrous</label>
</blockquote>
<%
      }  
    };  
  
    for($i = 1; $i <= 2; ++$i)
    {
      $fieldname = $this->prefix . "_FR$i";
      $index = "FR" . $i;
      $text = $questionset[$index];
      if ($text)
      {
        if ($first) 
        {
          $first = false;
          print("<h4>$displayname</h4>\n");
        }       
      
%>
<p><b><%=$text%></b></p>
<blockquote><textarea name="<%=$fieldname%>" rows=8 cols=50 wrap=virtual><%=$this->loadattribute("FR$i");%></textarea></blockquote>
<%
      }  
    };
  
    $choices = array(0 => "a", 1 => "b", 2 => "c", 3 => "d", 4 => "e", 5 => "f");
    $first = true;
    $row = true;
    $abet = explode(",",$questionset["ABET"]);
    foreach ($abet as $v)
    {
      $fieldname = $this->prefix . "_ABET$v";
      $text = $ABETQUESTIONS[$v];
      if ($text)
      {
        if ($first)
        {
          $first = false;
%>
<h4>ABET Questions</h4>
<table bordercolor=black cellspacing=0 cellpadding=3 RULES="groups" FRAME=box STYLE="border: none">
<thead>
<tr>
<td colspan=2 align=left STYLE="border: none"><b>To what degree did this course enhance your ability to ...</b></td>
<td colspan=9 align=right STYLE="border: none"><b>(0 = <i>not at all</i>, 5 = <i>a great deal</i>)</b></td>
</tr>
</thead>
<tbody>
<tr>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>0</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>1</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>2</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>3</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>4</STRONG></font></td>
  <td bgcolor=black background="<%=$server_wcespath%>media/0x000000.gif" align=center><font color=white><STRONG>5</STRONG></font></td>
</tr>
<%
        }
        
        if ($this->errors[$fieldname]) $text = "<font color=red>$text</font>";
        $val = $this->loadattribute("ABET$v");
        $row = !$row;
        $color = $row ? 'bgcolor="#FFFFFF" background="' . $server_wcespath . 'media/0xFFFFFF.gif"' : 'bgcolor="#EEEEEE" background="' . $server_wcespath . 'media/0xEEEEEE.gif"';
        print("<tr><td colspan=5 $color>$text</td>");
        foreach($choices as $choice)
        {
          $selected = ($val == $choice) ? "" : " selected";
          print("<td align=center $color><input name=\"$fieldname\" value=\"$choice\" type=radio$selected></td>");
        }  
        print("</tr>\n");
      }
    }
    if (!$first) print("\n</tbody>\n</table>\n");
  }

  function validate()
  {
    $db = $this->db;
    
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid), 0);
    extract($questionset);
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = $this->prefix . "_MC$i";
      if ($questionset["MC" . $i])
      {
        $result = $this->loadattribute("MC$i");
        if (!($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e"))
          $this->errors[$fieldname] = "Missing response for ratings question #$i under '" . $displayname . "'";
      };
    };
    
    $abet = explode(",",$questionset["ABET"]);
    foreach ($abet as $v)
    {
      $cnt = 0;
      $fieldname = $this->prefix . "_ABET$v";
      $text = $ABETQUESTIONS[$v];
      if ($text)
      {
        ++$cnt;
        $result = $this->loadttribute($fieldname);
        if (!($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e" || $result == "f"))
          $this->errors[$fieldname] = "Missing response for ABET Question #$cnt";
        } 
      return count($this->errors) > 0 ? false : true;
    }
  }

  function save($userid)
  {
    $questionset = db_getrow($db,"questionsets",Array("questionsetid" => $questionsetid), 0);

    $sql = "UPDATE answersets SET responses = responses + 1";
    for($i = 1; $i <= 10; ++$i)
    {
      $fieldname = $this->prefix . "MC$i";
      if ($questionset["MC$i"])
      {
        $result = $this->loadattribute("MC$i");
        if ($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e")
        {
          $n = "MC" . $i . $result;
          $sql .= ", $n = $n + 1";
        }; 
      };
    };

    $abet = explode(",",$questionset["ABET"]);
    foreach ($abet as $k => $v)
    {
      $fieldname = $this->prefix . "_ABET$v";
      $text = $ABETQUESTIONS[$v];
      if ($text)
      {
        $result = $this->loadattribute("ABET$v");
        if ($result == "a" || $result == "b" || $result == "c" || $result == "d" || $result == "e" || $result == "f")
        {
          $n = "ABET" . $k . $result;
          $sql .= ", $n = $n + 1";
        }
      }
    }

    for($i = 1; $i <= 2; ++$i)
    {
      $fieldname = $this->prefix . "_FR$i";
      if ($questionset["FR" . $i])
      {
        $result = addslashes(addcslashes(trim($HTTP_POST_VARS[$fieldname]),"\t\\"));
        if ($result)
        {
          $n = "FR" . $i;
          $sql .= ", $n = concat($n,' $result')";
        }; 
      };
    };
    
    if (!($questionset && $this->questionsetid && $this->classid && $this->questionperiodid))
      return false;
          
    $keys = Array("questionsetid" => $this->questionsetid, "questionperiodid" => $this->questionperiodid, "classid" => $this->classid);
    $answersetid = db_replace($db, "answersets", $keys, false, "answersetid");

    $sql .= " WHERE answersetid = $answersetid";

    if (!db_exec($sql,$db,__FILE__,__LINE__))
      return false;

    if (!db_exec("INSERT INTO completesurveys(userid,answersetid) VALUES ($userid,$answersetid)",$db,__FILE__,__LINE__))
      return false;    
    
    return true;
  }
}

%>
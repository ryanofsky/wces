<%

require_once("widgets/basic.inc");

define("QuestionSetEditor_main",1);
define("QuestionSetEditor_insert",2);
define("QuestionSetEditor_modify",3);

define("QuestionSetEditor_main_delete",1);
define("QuestionSetEditor_main_moveup",2);
define("QuestionSetEditor_main_movedown",3);
define("QuestionSetEditor_main_save",4);
define("QuestionSetEditor_main_cancel",5);

define("QuestionSetEditor_insert_create",1);
define("QuestionSetEditor_insert_cancel",2);

define("QuestionSetEditor_modify_save",1);
define("QuestionSetEditor_modify_cancel",2);

class QuestionSetEditor extends FormWidget
{

  var $action;      // constructed
  var $questionset; // persistent
  var $questionsetid; // persistent

  function QuestionSetEditor($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->action = new ActionButton($prefix . "_action",$form,$formmethod);
  }
 
  function loadvalues($db = 0, $questionsetid = 0)
  {
    $this->questionsetid = $questionsetid;
    if ($questionsetid)
      $this->questionset = unserialize(db_getvalue($db,"Newquestionsets", Array("questionsetid" => $questionsetid), "data"));
    if (get_class($this->questionset) != "questionset")
    {
      $this->questionset = unserialize($this->loadattribute("questionset"));
      if (get_class($this->questionset) != "questionset")
      {
        $this->questionset = new QuestionSet();
      }  
    }  
    $this->action->loadvalues();
    $this->handleaction();
  }
  
  function handleaction()
  {
    switch ($this->action->action)
    {
      case QuestionSetEditor_insert:
        $this->insert_action = new ActionButton($this->prefix . "_list_action", $this->form, $this->formmethod);
        $this->insert_action->loadvalues();
        $this->insert_type = new RadioButton(array("Ratings Block","Text Response"), $this->prefix . "_insert_type",$this->form, $this->formmethod);
        $this->insert_type->loadvalues();

        switch($this->insert_action->action)
        {
          case QuestionSetEditor_insert_create:
            switch ($this->insert_type->checked)
            {
              case 0:
                $q = new RatingsBlock();
              break;
              case 1:
                $q = new FreeResponse();
              break;
              default:
                $q = false;
              break;
            };

            if ($q === false)
              $this->insert_badinput = true;
            else
            {  
              $q->neversaved = true;
              $this->action->object = $this->questionset->insert($q, $this->action->object);
              $this->action->action = QuestionSetEditor_modify;
              $this->handleaction();
            }  
          break;
          case QuestionSetEditor_insert_cancel:
            $this->action->action = QuestionSetEditor_main;
            $this->handleaction();
          break;
          default:
          break;
        }
        break;
      case QuestionSetEditor_modify:
      $this->modify_action = new ActionButton($this->prefix . "_modify_action", $this->form, $this->formmethod);
      $this->modify_action->loadvalues();
      $this->modify_editor = $this->questionset->questions[$this->action->object]->geteditor($this->prefix . "_modify_editor", $this->form, $this->formmethod);
      $this->modify_editor->loadvalues();
      switch($this->modify_action->action)
      {
        case QuestionSetEditor_modify_save:
          if ($this->modify_editor->checkvalues())
          {
            unset($this->modify_editor->question->neversaved);
            $this->modify_editor->savevalues();
            $this->action->action = QuestionSetEditor_main;
            $this->handleaction();
          }
        break;
        case QuestionSetEditor_modify_cancel:
          $i = $this->action->object;
          if ($this->questionset->questions[$i]->neversaved)
            $this->questionset->delete($i);
          $this->action->action = QuestionSetEditor_main;
          $this->handleaction();
        break;
      }
      break;
      case QuestionSetEditor_main:
      default:
        $this->main_action = new ActionButton($this->prefix . "_main_action", $this->form, $this->formmethod);
        $this->main_action->loadvalues();

        switch($this->main_action->action)
        {
          case QuestionSetEditor_main_delete:
            $this->questionset->delete($this->main_action->object);
          break;
          case QuestionSetEditor_main_moveup:
            $this->questionset->moveup($this->main_action->object);
          break;
          case QuestionSetEditor_main_movedown:
            $this->questionset->movedown($this->main_action->object);
          break;
          case QuestionSetEditor_main_save:
          break;
          case QuestionSetEditor_main_cancel:
          break;
          default:
          break;
        }
      break;
    }  
  }
  

  function display($hidden = false)
  {
    $this->printattribute("questionset", serialize($this->questionset));
    $q = unserialize(serialize($this->questionset));
    $this->printattribute("questionsetid", $this->questionsetid);

    if (!$hidden)
    {
      switch ($this->action->action)
      {
        case QuestionSetEditor_insert:
          $this->action->display(false);
          if ($this->insert_badinput) print("<p><font color=red>You forgot to choose an item from the list</font></p>");
          $first = true;
          foreach($this->insert_type->items as $key => $value)
          {
            if ($first) $first = false; else print("<br>\n");
            $this->insert_type->display($key);
            $this->insert_type->displaylabel($key);
          }
          print("\n");
          print("<p>");
          $this->insert_action->display("Create", QuestionSetEditor_insert_create, "", 'class="tinybutton"');
          $this->insert_action->display("Cancel", QuestionSetEditor_insert_cancel, "", 'class="tinybutton"');
          print("</p>");
        break;
        case QuestionSetEditor_modify:
          $this->action->display(false);
          $this->modify_editor->display();
          $this->modify_action->display("Save", QuestionSetEditor_modify_save, "", 'class="tinybutton"');
          $this->modify_action->display("Cancel",QuestionSetEditor_modify_cancel, "", 'class="tinybutton"');
        break;
        case QuestionSetEditor_main:
        default:
%>

<p>Welcome to the question set editor</p>

<table border=2 bordercolor=black bordercolorlight=black bordercolordark=black cellspacing=0>
<% foreach($this->questionset->questions as $key => $question)
   {
     $w = $question->getwidget($this->prefix . "_main_qwidget_" . $key, $this->form, $this->formmethod);
%>
  <tr>
    <td bgcolor="#DDDDDD">&nbsp;</td>
    <td align=center valign=middle><% $this->action->display("Insert a component here", QuestionSetEditor_insert,$key,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"'); %></td>
  </tr>
  <tr>
    <td bgcolor="#DDDDDD">
      <table>
        <tr>
          <td><% $this->action->display(" Modify ",QuestionSetEditor_modify,$key,'class="tinybutton" style="width:100%; BACKGROUND: #0030E7; COLOR: white;"'); %></td>
          <td><% $this->main_action->display(" Move Up ", QuestionSetEditor_main_moveup,$key,'class="tinybutton" style="width:100%; BACKGROUND: #EFDF18; COLOR: black;"'); %></td>
        </tr>
        <tr>
          <td><% $this->main_action->display(" Delete ", QuestionSetEditor_main_delete,$key,'class="tinybutton" style="width:100%; BACKGROUND: red; COLOR: white;"'); %></td>
          <td><% $this->main_action->display(" Move Down ", QuestionSetEditor_main_movedown,$key,'class="tinybutton" style="width:100%; BACKGROUND: #EFDF18; COLOR: black;"'); %></td>
        </tr>
      </table>
    </td>
    <td>
      <% $w->display(); %>
    </td>
  </tr>
<% } %>
  <tr>
    <td bgcolor="#DDDDDD">&nbsp;</td>
    <td align=center valign=middle><% $this->action->display("Insert a component here",QuestionSetEditor_insert,-1,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"'); %></td>
  </tr>
</table>
<%
        break;
      }
    }
  }        
}
%>



<?

require_once("widgets/basic.inc");

//states
define("QuestionSetEditor_main",1);
define("QuestionSetEditor_insert",2);
define("QuestionSetEditor_modify",3);

//main actions

define("QuestionSetEditor_main_insert",1);
define("QuestionSetEditor_main_modify",2);
define("QuestionSetEditor_main_delete",3);
define("QuestionSetEditor_main_moveup",4);
define("QuestionSetEditor_main_movedown",5);
define("QuestionSetEditor_main_save",6);
define("QuestionSetEditor_main_cancel",7);

//insert actions
define("QuestionSetEditor_insert_create",1);
define("QuestionSetEditor_insert_cancel",2);

//modify actions
define("QuestionSetEditor_modify_save",1);
define("QuestionSetEditor_modify_cancel",2);

class QuestionSetEditor extends FormWidget
{
  var $questionsetid; // constructed
  var $db;
  
  var $questionset; // persistent
  var $insert_badinput;
  var $state;
  var $currentquestion;

  function QuestionSetEditor($db, $questionsetid, $prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->db = $db;
    $this->questionsetid = $questionsetid;
    $this->questionset = false;
    $this->insert_badinput = false;
  }
 
  function loadvalues()
  {
    if ($this->questionsetid)
      $this->questionset = unserialize(db_getvalue($this->db,"Newquestionsets", Array("questionsetid" => $this->questionsetid), "data"));
    
    if (get_class($this->questionset) != "questionset")
    {
      $this->questionset = unserialize($this->loadattribute("questionset"));
      if (get_class($this->questionset) != "questionset")
      {
        $this->questionset = new QuestionSet();
      }  
    }  
    $this->state = $this->loadattribute("state");
    $this->currentquestion = $this->loadattribute("currentquestion");
    $this->handleaction();
  }

  function handleaction()
  {
    switch ($this->state)
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
              $this->state = QuestionSetEditor_modify;
              $this->currentquestion = $this->questionset->insert($q, $this->currentquestion);
              $this->handleaction();
            }  
          break;
          case QuestionSetEditor_insert_cancel:
            $this->state = QuestionSetEditor_main;
            $this->handleaction();
          break;
          default:
          break;
        }
        break;
      case QuestionSetEditor_modify:
        $this->modify_action = new ActionButton($this->prefix . "_modify_action", $this->form, $this->formmethod);
        $this->modify_action->loadvalues();
        $this->modify_editor = $this->questionset->questions[$this->currentquestion]->geteditor($this->prefix . "_modify_editor", $this->form, $this->formmethod);
        $this->modify_editor->loadvalues();
        switch($this->modify_action->action)
        {
          case QuestionSetEditor_modify_save:
            if ($this->modify_editor->checkvalues())
            {
              unset($this->modify_editor->question->neversaved);
              $this->modify_editor->savevalues();
              $this->state = QuestionSetEditor_main;
              $this->handleaction();
            }
          break;
          case QuestionSetEditor_modify_cancel:
            $i = $this->currentquestion;
            if (isset($this->questionset->questions[$i]->neversaved))
              $this->questionset->delete($i);
            $this->state = QuestionSetEditor_main;
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
          case QuestionSetEditor_main_insert:
            $this->state = QuestionSetEditor_insert;
            $this->currentquestion = $this->main_action->object;
            $this->handleaction();
          break;
          case QuestionSetEditor_main_modify:
            $this->state = QuestionSetEditor_modify;
            $this->currentquestion = $this->main_action->object;
            $this->handleaction();
          break;
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
    global $testroot;

    $this->printattribute("questionset", serialize($this->questionset));
    $q = unserialize(serialize($this->questionset));
    $this->printattribute("questionsetid", $this->questionsetid);
    $this->printattribute("currentquestion",$this->currentquestion);
    if (!$hidden)
    {
      switch ($this->state)
      {
        case QuestionSetEditor_insert:
          $this->printattribute("state",QuestionSetEditor_insert);
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
          $this->printattribute("state",QuestionSetEditor_modify);
          $this->modify_editor->display();
          $this->modify_action->display("Save", QuestionSetEditor_modify_save, "", 'class="tinybutton"');
          $this->modify_action->display("Cancel",QuestionSetEditor_modify_cancel, "", 'class="tinybutton"');
        break;
        case QuestionSetEditor_main:
        default:
          $this->printattribute("state",QuestionSetEditor_main);
          $this->main_action->display(false,"","");
          $nquestions = count($this->questionset->questions);
          if ($nquestions == 0)
          {
            $this->main_action->display("Insert a component",QuestionSetEditor_main_insert,0,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"');
          }
          else // $nquestions != 1
          {
            print("<table border=0 cellpadding=0 cellspacing=0>\n");
            for($i=0;;++$i)
            {
?>
<tr><td colspan=1><img src="<?=$testroot?>/media/nada.gif" width=1 height=10 alt="[ spacer ]"></td><td colspan=3 id="<?=$this->prefix?>_insert<?=$i?>" class=insertmenu><img src="<?=$testroot?>/media/nada.gif" width=1 height=10 alt="[ spacer ]"></td></tr>
<?
              if ($i >= $nquestions)
                break;
              else
                $question = &$this->questionset->questions[$i];
              $w = $question->getwidget($this->prefix . "_main_qwidget_" . $i, $this->form, $this->formmethod);
              $w->loadvalues();
?>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td rowspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=2><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr>
  <td width=40 valign=center><img src="<?=$testroot?>/media/editor/node.gif" width=40 height=18 border=0 align=absmiddle alt=node id=<?=$this->prefix?>_node<?=$i?>></td>
  <td width=1><img src="<?=$testroot?>/media/nada.gif" width=3 height=1 alt="[ spacer ]"></td>
  <td><? $w->display(); ?></td>
</tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td colspan=2><img src="<?=$testroot?>/media/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$testroot?>/media/0x000000.gif"><img src="<?=$testroot?>/media/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<?
        } // for i
?>
</table>
<script>
<!--
  if (document.all)
  {
    var nodes = new Array(); var inserts = new Array();
    for(var i=0;;++i)
    {
      inserts[i] = document.all("<?=$this->prefix?>_insert" + i);
      if (i >= <?=count($this->questionset->questions)?>) break;
      nodes[i] = document.all("<?=$this->prefix?>_node" + i);
    }  
    var menu = new QuestionSetEditor(nodes, inserts, "<?=$this->form?>", "<?=$this->main_action->prefix?>");
  }
// -->
</script>
<?
  } // if $nquestions == 0      
        break;
      }
    }
  }
  
  function dumpscript()
  {
    global $QuestionSetEditor_dumped,$testroot;
    if (!isset($QuestionSetEditor_dumped))
    {
      ActionButton::dumpscript();
      $QuestionSetEditor_dumped = true;
?>
<!-- Begin QuestionSetEditor Script -->

<table id=QuestionSetEditor_cmdmenu border=1 bordercolor=black cellspacing=0 cellpadding=3 style="position:absolute; visibility: hidden">
<tr><td id=QuestionSetEditor_editcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle> Edit Question</td></tr>
<tr><td id=QuestionSetEditor_deletecmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle> Delete Question</td></tr>
<tr><td id=QuestionSetEditor_moveupcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle> Move Up</td></tr>
<tr><td id=QuestionSetEditor_movedowncmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle> Move Down</td></tr>
</table>
<img id=QuestionSetEditor_insert style="position:absolute; visibility:hidden" src="<?=$testroot?>/media/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component">
<script>
<!--
  function QuestionSetEditor_Table() // extender
  {
    var a = document.all("QuestionSetEditor_cmdmenu");
    a.TDinit = QuestionSetEditor_Table_TDinit;
    a.TDinit("<?=QuestionSetEditor_main_modify?>", "QuestionSetEditor_editcmd");
    a.TDinit("<?=QuestionSetEditor_main_delete?>", "QuestionSetEditor_deletecmd");
    a.TDinit("<?=QuestionSetEditor_main_moveup?>", "QuestionSetEditor_moveupcmd");
    a.TDinit("<?=QuestionSetEditor_main_movedown?>", "QuestionSetEditor_movedowncmd");
    
    a.onmouseout = QuestionSetEditor_Table_onmouseout;
    a.onmouseover = QuestionSetEditor_Table_onmouseover;
    a.timeout = QuestionSetEditor_Table_timeout;
    a.show = QuestionSetEditor_Table_show;
    a.hide = QuestionSetEditor_Table_hide;
    a.hover = a.activenode = a.actionobject = false; //state variables
    return a;
  }

  function QuestionSetEditor_Table_TDinit(action, tdname)
  {
    var t = document.all(tdname);
    t.table = this;
    t.onmouseover = QuestionSetEditor_Table_TD_onmouseover;
    t.onmouseout = QuestionSetEditor_Table_TD_onmouseout;
    t.onclick = QuestionSetEditor_Table_TD_onclick;
    t.action = action;
  }

  function QuestionSetEditor_Table_TD_onclick()
  {
    ActionButton_go(this.action, this.table.activenode.key, this.table.actionobject);
  }

  function QuestionSetEditor_Table_TD_onmouseover()
  {
    this.className = "editmenuhover";
  }

  function QuestionSetEditor_Table_TD_onmouseout()
  {
    this.className = "editmenu";
  }

  function QuestionSetEditor_Table_onmouseover()
  {
    this.hover = true;
  }

  function QuestionSetEditor_Table_onmouseout()
  {
    this.timeout();
    this.hover = false;
  }

  function QuestionSetEditor_Table_timeout()
  {
    window.setTimeout("window.QuestionSetEditor_table.hide()",100);
  }

  function QuestionSetEditor_Table_show(activenode,actionobject)
  {
    this.activenode = activenode;
    this.actionobject = actionobject;
    var offset = getoffset(activenode);
    var s = this.style;
    s.pixelLeft = offset.x + 9;
    s.pixelTop = offset.y + 9;
    s.visibility = "visible";
  }

  function QuestionSetEditor_Table_hide()
  {
    if (!this.hover && !this.activenode.hover)
      this.style.visibility = "hidden";
  }

  window.QuestionSetEditor_table = QuestionSetEditor_Table();

///////////////////////////////////////////////////////////  
  
  function QuestionSetEditor(nodes,inserts,form,actionelement)
  {
    this.actionobject = document.forms[form][actionelement];
    this.nodes = nodes;
    this.inserts = inserts;
    for(i in this.nodes)
    {
      var node = this.nodes[i];
      node.onmouseover = QuestionSetEditor_node_onmouseover;
      node.onmouseout = QuestionSetEditor_node_onmouseout;
      node.key = i;
      node.editor = this;
    }
    
    for(i in this.inserts)
    {
      var insert = this.inserts[i];
      insert.onclick = QuestionSetEditor_insert_onclick;
      insert.onmouseover = QuestionSetEditor_insert_onmouseover;
      insert.onmouseout = QuestionSetEditor_insert_onmouseout;
      insert.key = i;
      insert.editor = this;
    }
  }

  function QuestionSetEditor_node_onmouseover()
  {
    this.hover = true;
    window.QuestionSetEditor_table.show(this,this.editor.actionobject);
  }

  function QuestionSetEditor_node_onmouseout()
  {
    this.hover = false;
    window.QuestionSetEditor_table.timeout();
  }
  
  function QuestionSetEditor_insert_onmouseover()
  {
    this.className = "insertmenuhover";
    var offset = getoffset(this);
    var s = document.all("QuestionSetEditor_insert").style;
    s.visibility = "visible";
    s.pixelLeft = offset.x + this.clientWidth + 4;
    s.pixelTop = offset.y + -3;
  }

  function QuestionSetEditor_insert_onmouseout()
  {
    this.className = "insertmenu";
    var s = document.all("QuestionSetEditor_insert").style;
    s.visibility = "hidden";
  }

  function QuestionSetEditor_insert_onclick()
  {
    ActionButton_go("<?=QuestionSetEditor_main_insert?>",this.key,this.editor.actionobject);
  }

  function getoffset(element)
  {
    var offset = new Object(); offset.x = 0; offset.y = 0;
    for(;;element = element.offsetParent)
    {
      offset.x += element.offsetLeft;
      offset.y += element.offsetTop;
      if (!element.offsetParent) return offset;
    }
  }
  
// -->
</script>

<!-- End QuestionSetEditor Script -->
<?
    }
  }
}
?>

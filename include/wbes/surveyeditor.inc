<?

require_once("widgets/basic.inc");

//states
define("SurveyEditor_main",1);
define("SurveyEditor_insert",2);
define("SurveyEditor_modify",3);

//main actions

define("SurveyEditor_main_insert",1);
define("SurveyEditor_main_modify",2);
define("SurveyEditor_main_delete",3);
define("SurveyEditor_main_moveup",4);
define("SurveyEditor_main_movedown",5);
define("SurveyEditor_main_save",6);
define("SurveyEditor_main_cancel",7);

//insert actions
define("SurveyEditor_insert_create",1);
define("SurveyEditor_insert_cancel",2);

//modify actions
define("SurveyEditor_modify_save",1);
define("SurveyEditor_modify_cancel",2);

class SurveyEditor extends FormWidget
{
  var $surveyid; // constructed
  var $db;
  
  var $survey; // persistent
  var $insert_badinput;
  var $state;
  var $currentquestion;

  function SurveyEditor($db, $surveyid, $prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->db = $db;
    $this->surveyid = $surveyid;
    $this->survey = false;
    $this->insert_badinput = false;
  }
 
  function loadvalues()
  {
    if ($this->surveyid)
      $this->survey = unserialize(db_getvalue($this->db,"Newsurveys", Array("surveyid" => $this->surveyid), "data"));
    
    if (get_class($this->survey) != "survey")
    {
      $this->survey = unserialize($this->loadattribute("survey"));
      if (get_class($this->survey) != "survey")
      {
        $this->survey = new Survey();
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
      case SurveyEditor_insert:
        $this->insert_action = new ActionButton($this->prefix . "_list_action", $this->form, $this->formmethod);
        $this->insert_action->loadvalues();
        $this->insert_type = new RadioButton(array("Ratings Block","Text Response"), $this->prefix . "_insert_type",$this->form, $this->formmethod);
        $this->insert_type->loadvalues();

        switch($this->insert_action->action)
        {
          case SurveyEditor_insert_create:
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
              $this->state = SurveyEditor_modify;
              $this->currentquestion = $this->survey->insert($q, $this->currentquestion);
              $this->handleaction();
            }  
          break;
          case SurveyEditor_insert_cancel:
            $this->state = SurveyEditor_main;
            $this->handleaction();
          break;
          default:
          break;
        }
        break;
      case SurveyEditor_modify:
        $this->modify_action = new ActionButton($this->prefix . "_modify_action", $this->form, $this->formmethod);
        $this->modify_action->loadvalues();
        $this->modify_editor = $this->survey->questions[$this->currentquestion]->geteditor($this->prefix . "_modify_editor", $this->form, $this->formmethod);
        $this->modify_editor->loadvalues();
        switch($this->modify_action->action)
        {
          case SurveyEditor_modify_save:
            if ($this->modify_editor->checkvalues())
            {
              unset($this->modify_editor->question->neversaved);
              $this->modify_editor->savevalues();
              $this->state = SurveyEditor_main;
              $this->handleaction();
            }
          break;
          case SurveyEditor_modify_cancel:
            $i = $this->currentquestion;
            if (isset($this->survey->questions[$i]->neversaved))
              $this->survey->delete($i);
            $this->state = SurveyEditor_main;
            $this->handleaction();
          break;
        }
        break;
      case SurveyEditor_main:
      default:
        $this->main_action = new ActionButton($this->prefix . "_main_action", $this->form, $this->formmethod);
        $this->main_action->loadvalues();

        switch($this->main_action->action)
        {
          case SurveyEditor_main_insert:
            $this->state = SurveyEditor_insert;
            $this->currentquestion = $this->main_action->object;
            $this->handleaction();
          break;
          case SurveyEditor_main_modify:
            $this->state = SurveyEditor_modify;
            $this->currentquestion = $this->main_action->object;
            $this->handleaction();
          break;
          case SurveyEditor_main_delete:
            $this->survey->delete($this->main_action->object);
          break;
          case SurveyEditor_main_moveup:
            $this->survey->moveup($this->main_action->object);
          break;
          case SurveyEditor_main_movedown:
            $this->survey->movedown($this->main_action->object);
          break;
          case SurveyEditor_main_save:
          break;
          case SurveyEditor_main_cancel:
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

    $this->printattribute("survey", serialize($this->survey));
    $q = unserialize(serialize($this->survey));
    $this->printattribute("surveyid", $this->surveyid);
    $this->printattribute("currentquestion",$this->currentquestion);
    if (!$hidden)
    {
      switch ($this->state)
      {
        case SurveyEditor_insert:
          $this->printattribute("state",SurveyEditor_insert);
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
          $this->insert_action->display("Create", SurveyEditor_insert_create, "", 'class="tinybutton"');
          $this->insert_action->display("Cancel", SurveyEditor_insert_cancel, "", 'class="tinybutton"');
          print("</p>");
        break;
        case SurveyEditor_modify:
          $this->printattribute("state",SurveyEditor_modify);
          $this->modify_editor->display();
          $this->modify_action->display("Save", SurveyEditor_modify_save, "", 'class="tinybutton"');
          $this->modify_action->display("Cancel",SurveyEditor_modify_cancel, "", 'class="tinybutton"');
        break;
        case SurveyEditor_main:
        default:
          $this->printattribute("state",SurveyEditor_main);
          $this->main_action->display(false,"","");
          $nquestions = count($this->survey->questions);
          if ($nquestions == 0)
          {
            $this->main_action->display("Insert a component",SurveyEditor_main_insert,0,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"');
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
                $question = &$this->survey->questions[$i];
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
      if (i >= <?=count($this->survey->questions)?>) break;
      nodes[i] = document.all("<?=$this->prefix?>_node" + i);
    }  
    var menu = new SurveyEditor(nodes, inserts, "<?=$this->form?>", "<?=$this->main_action->prefix?>");
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
    global $SurveyEditor_dumped,$testroot;
    if (!isset($SurveyEditor_dumped))
    {
      ActionButton::dumpscript();
      $SurveyEditor_dumped = true;
?>
<!-- Begin SurveyEditor Script -->

<table id=SurveyEditor_cmdmenu border=1 bordercolor=black cellspacing=0 cellpadding=3 style="position:absolute; visibility: hidden">
<tr><td id=SurveyEditor_editcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle> Edit Question</td></tr>
<tr><td id=SurveyEditor_deletecmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle> Delete Question</td></tr>
<tr><td id=SurveyEditor_moveupcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle> Move Up</td></tr>
<tr><td id=SurveyEditor_movedowncmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$testroot?>/media/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle> Move Down</td></tr>
</table>
<img id=SurveyEditor_insert style="position:absolute; visibility:hidden" src="<?=$testroot?>/media/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component">
<script>
<!--
  function SurveyEditor_Table() // extender
  {
    var a = document.all("SurveyEditor_cmdmenu");
    a.TDinit = SurveyEditor_Table_TDinit;
    a.TDinit("<?=SurveyEditor_main_modify?>", "SurveyEditor_editcmd");
    a.TDinit("<?=SurveyEditor_main_delete?>", "SurveyEditor_deletecmd");
    a.TDinit("<?=SurveyEditor_main_moveup?>", "SurveyEditor_moveupcmd");
    a.TDinit("<?=SurveyEditor_main_movedown?>", "SurveyEditor_movedowncmd");
    
    a.onmouseout = SurveyEditor_Table_onmouseout;
    a.onmouseover = SurveyEditor_Table_onmouseover;
    a.timeout = SurveyEditor_Table_timeout;
    a.show = SurveyEditor_Table_show;
    a.hide = SurveyEditor_Table_hide;
    a.hover = a.activenode = a.actionobject = false; //state variables
    return a;
  }

  function SurveyEditor_Table_TDinit(action, tdname)
  {
    var t = document.all(tdname);
    t.table = this;
    t.onmouseover = SurveyEditor_Table_TD_onmouseover;
    t.onmouseout = SurveyEditor_Table_TD_onmouseout;
    t.onclick = SurveyEditor_Table_TD_onclick;
    t.action = action;
  }

  function SurveyEditor_Table_TD_onclick()
  {
    ActionButton_go(this.action, this.table.activenode.key, this.table.actionobject);
  }

  function SurveyEditor_Table_TD_onmouseover()
  {
    this.className = "editmenuhover";
  }

  function SurveyEditor_Table_TD_onmouseout()
  {
    this.className = "editmenu";
  }

  function SurveyEditor_Table_onmouseover()
  {
    this.hover = true;
  }

  function SurveyEditor_Table_onmouseout()
  {
    this.timeout();
    this.hover = false;
  }

  function SurveyEditor_Table_timeout()
  {
    window.setTimeout("window.SurveyEditor_table.hide()",100);
  }

  function SurveyEditor_Table_show(activenode,actionobject)
  {
    this.activenode = activenode;
    this.actionobject = actionobject;
    var offset = getoffset(activenode);
    var s = this.style;
    s.pixelLeft = offset.x + 9;
    s.pixelTop = offset.y + 9;
    s.visibility = "visible";
  }

  function SurveyEditor_Table_hide()
  {
    if (!this.hover && !this.activenode.hover)
      this.style.visibility = "hidden";
  }

  window.SurveyEditor_table = SurveyEditor_Table();

///////////////////////////////////////////////////////////  
  
  function SurveyEditor(nodes,inserts,form,actionelement)
  {
    this.actionobject = document.forms[form][actionelement];
    this.nodes = nodes;
    this.inserts = inserts;
    for(i in this.nodes)
    {
      var node = this.nodes[i];
      node.onmouseover = SurveyEditor_node_onmouseover;
      node.onmouseout = SurveyEditor_node_onmouseout;
      node.key = i;
      node.editor = this;
    }
    
    for(i in this.inserts)
    {
      var insert = this.inserts[i];
      insert.onclick = SurveyEditor_insert_onclick;
      insert.onmouseover = SurveyEditor_insert_onmouseover;
      insert.onmouseout = SurveyEditor_insert_onmouseout;
      insert.key = i;
      insert.editor = this;
    }
  }

  function SurveyEditor_node_onmouseover()
  {
    this.hover = true;
    window.SurveyEditor_table.show(this,this.editor.actionobject);
  }

  function SurveyEditor_node_onmouseout()
  {
    this.hover = false;
    window.SurveyEditor_table.timeout();
  }
  
  function SurveyEditor_insert_onmouseover()
  {
    this.className = "insertmenuhover";
    var offset = getoffset(this);
    var s = document.all("SurveyEditor_insert").style;
    s.visibility = "visible";
    s.pixelLeft = offset.x + this.clientWidth + 4;
    s.pixelTop = offset.y + -3;
  }

  function SurveyEditor_insert_onmouseout()
  {
    this.className = "insertmenu";
    var s = document.all("SurveyEditor_insert").style;
    s.visibility = "hidden";
  }

  function SurveyEditor_insert_onclick()
  {
    ActionButton_go("<?=SurveyEditor_main_insert?>",this.key,this.editor.actionobject);
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

<!-- End SurveyEditor Script -->
<?
    }
  }
}
?>

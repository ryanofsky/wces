<?

require_once("widgets/basic.inc");
require_once("wbes/surveywidget.inc");
require_once("wbes/server.inc");
require_once("wbes/survey.inc");
require_once("wbes/postgres.inc");

//states
define("SurveyEditor_main",1);
define("SurveyEditor_insert",2);
define("SurveyEditor_modify",3);
define("SurveyEditor_done",4);
define("SurveyEditor_preview", 5);

//main actions
define("SurveyEditor_main_insert",1);
define("SurveyEditor_main_modify",2);
define("SurveyEditor_main_delete",3);
define("SurveyEditor_main_moveup",4);
define("SurveyEditor_main_movedown",5);
define("SurveyEditor_main_save",6);
define("SurveyEditor_main_cancel",7);
define("SurveyEditor_main_preview",8);

//insert actions
define("SurveyEditor_insert_create",1);
define("SurveyEditor_insert_cancel",2);

//modify actions
define("SurveyEditor_modify_save",1);
define("SurveyEditor_modify_cancel",2);
define("SurveyEditor_modify_preview",3);

$SurveyEditor_ns4 = $server_url->xpath == "/ns4";

class SurveyEditor extends FormWidget
{
  var $survey = false; // persistent
  var $insert_badinput = false;
  var $state;
  var $currentcomponent;
  var $barepage = false;
  var $message = "";
  var $factories;
  var $topic_id;
  var $base_branch_id;
  
  function SurveyEditor($topic_id, $base_branch_id, $user_id, $factories, $prefix,$form,$formmethod)
  {
    global $server_url;
    $this->FormWidget($prefix,$form,$formmethod);
    $this->topic_id = (int)$topic_id;
    $this->base_branch_id = (int)$base_branch_id;
    $this->user_id = (int)$user_id;
    $this->factories = $factories;
    $GLOBALS['topic_id'] = $topic_id;
  }
 
  function loadvalues($empty = false)
  {
    if(!$empty)
    {
      $this->survey = unserialize((string)urldecode((string)$this->loadattribute("survey")));

      if (get_class($this->survey) != "survey")
        $this->load();
    }

    $this->state = $this->loadattribute("state");
    $this->currentcomponent = $this->loadattribute("currentcomponent");
    //print($this->prefix . " values loaded, now handling action.\n<br>\n");
    $this->handleaction();
  }

  function load()
  {
    $this->survey = new Survey();
    $this->survey->load($this->topic_id,$this->base_branch_id,$this->factories);
  }
  
  function save()
  {
    if ($this->survey->save($this->topic_id,$this->user_id,0))
      $this->message = "<font color=red><i>Survey Builder: Saved changes</i></font>";
    else
      $this->message = "<h1>Save Error</h1>";
    $this->state = SurveyEditor_done;
  }

  function handleaction()
  {
    $GLOBALS['topic_id'] = $this->topic_id;

    switch ($this->state)
    {
      case SurveyEditor_insert:
        $this->insert_action = new ActionButton("{$this->prefix}_list_action", $this->form, $this->formmethod);
        $this->insert_action->loadvalues();
        
        $this->insert_type = new RadioButton(true, "{$this->prefix}_insert_type",$this->form, $this->formmethod);
        $this->insert_type->loadvalues();

        switch($this->insert_action->action)
        {
          case SurveyEditor_insert_create:
            $factory = @$this->factories[$this->insert_type->checked];
            $q = is_object($factory) ? $factory->create() : false;

            if (!$q)
              $this->insert_badinput = true;
            else
            {  
              $q->neversaved = true;
              $this->state = SurveyEditor_modify;
              $this->currentcomponent = $this->survey->insert($q, $this->currentcomponent);
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
        $this->modify_action = new ActionButton("{$this->prefix}_modify_action", $this->form, $this->formmethod);
        $this->modify_action->loadvalues();
        $this->modify_editor = $this->survey->components[$this->currentcomponent]->geteditor("{$this->prefix}_modify_editor", $this->form, $this->formmethod);
        $this->modify_editor->loadvalues();
        switch($this->modify_action->action)
        {
          case SurveyEditor_modify_save:
            if ($this->modify_editor->checkvalues())
            {
              unset($this->modify_editor->component->neversaved);
              $this->modify_editor->savevalues();
              $this->state = SurveyEditor_main;
              $this->handleaction();
            }
          break;
          case SurveyEditor_modify_cancel:
            $i = $this->currentcomponent;
            if (isset($this->survey->components[$i]->neversaved))
              $this->survey->delete($i);
            $this->state = SurveyEditor_main;
            $this->handleaction();
          break;
          case SurveyEditor_modify_preview:
            if($this->modify_editor->checkvalues())
              $this->modify_editor->savevalues();
            $this->barepage = true;  
            break;  
        }
        break;
      case SurveyEditor_main:
      default:
        //print($this->prefix . " Doing default. \n<br>\n");
        $this->main_action = new ActionButton("{$this->prefix}_main_action", $this->form, $this->formmethod);
        $this->main_action->loadvalues();

        switch($this->main_action->action)
        {
          case SurveyEditor_main_insert:
            $this->state = SurveyEditor_insert;
            $this->currentcomponent = $this->main_action->object;
            $this->handleaction();
          break;
          case SurveyEditor_main_modify:
            $this->state = SurveyEditor_modify;
            $this->currentcomponent = $this->main_action->object;
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
            $this->save();
          break;
          case SurveyEditor_main_cancel:
            $this->state = SurveyEditor_done;
            $this->message = "<font color=red><i>Survey Builder: No changes were saved</i></font>";
          break;
          case SurveyEditor_main_preview:
            $this->state = SurveyEditor_preview;
            $this->handleaction();
          default:
          break;
        }
      break;
      case SurveyEditor_preview:
        $this->barepage = true;
      break;
    }  
  }

  function display($hidden = false)
  {
    global $server_media, $wbes;
    
    $this->printattribute("survey", urlencode(serialize($this->survey)));
    $q = unserialize(serialize($this->survey));
    $this->printattribute("currentcomponent",$this->currentcomponent);
    if (!$hidden)
    {
    //print("Now displaying: " . $this->state);
      switch ($this->state)
      {
        case SurveyEditor_insert:
          $this->printattribute("state",SurveyEditor_insert);
          if ($this->insert_badinput) print("<p><font color=red>You forgot to choose an item from the list</font></p>");
          $first = true;
          print("<h5>Which component type would you like to insert? <a href=\"javascript:showhelp('inserttypes')\"><img src=\"$server_media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a></h5>\n");

          foreach($this->factories as $key => $f)
          {
            if ($first) $first = false; else print("<br>\n");
            $this->insert_type->display($key);
            $this->insert_type->displaylabel($key, $f->name());
          }
          print("\n");
          print("<p>");
          $this->insert_action->display("Create", SurveyEditor_insert_create, "", 'class="tinybutton"');
          $this->insert_action->display("Cancel", SurveyEditor_insert_cancel, "", 'class="tinybutton"');
          print("</p>");
        break;
        case SurveyEditor_modify:
          if ($this->modify_action->action != SurveyEditor_modify_preview)
          {
            $this->modify_action->display(false,"","");
            $this->printattribute("state",SurveyEditor_modify);
            $this->modify_editor->display();
            $this->modify_action->display("Save", SurveyEditor_modify_save, "", 'class="tinybutton"');
            print("<input type=button class=tinybutton value=Preview onclick=\"return SurveyEditor_preview(this.form,'{$this->modify_action->prefix}'," . SurveyEditor_modify_preview . ")\">");
            $this->modify_action->display("Cancel",SurveyEditor_modify_cancel, "", 'class="tinybutton"');
          }
          else
          {
            print('<center><input type=button onclick="window.close()" value="Close this window"></center>');
            $this->modify_editor->preview();
          }
        break;
        case SurveyEditor_preview:
          $this->printattribute("state",SurveyEditor_preview);
          print('<p align=center><input type=button onclick="window.close()" value="Close this window"></p>');
          $w = new SurveyWidget(0,0,0,0,0,"{$this->prefix}_preview", $this->form, $this->formmethod);
          $w->survey = $this->survey;
          $w->loadvalues();
          $w->display();
        break;
        case SurveyEditor_main:
        default:
          $this->printattribute("state",SurveyEditor_main);
          $this->main_action->display(false,"","");
          print("<h5>Survey Builder</h5>");
          $ncomponents = count($this->survey->components);
          if ($ncomponents == 0)
          {
            print("<p>This survey does not have any components. Click below to either insert a component or cancel without saving any changes.</p>");
            $this->main_action->display("Insert a component",SurveyEditor_main_insert,0,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"');
            print(" ");
            $this->main_action->display("Cancel",SurveyEditor_main_cancel,"", 'class="tinybutton" style="BACKGROUND: #CE3031; COLOR: white;"');
          }
          else // $ncomponents != 1
          {
?>
<p>This page shows a WYSIWYG view of your survey. You can modify, delete, and
rearrange survey components by moving your mouse over the blue orbs at the left
and making a selection from the popup menu. You can insert new survey components
by clicking above and below existing components.</p>
<?            
            print("<table border=0 cellpadding=0 cellspacing=0>\n");
            print("<tr><td colspan=6 align=center>");
            $this->main_action->display("Add new component",SurveyEditor_main_insert,$ncomponents,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"');
            print("</td></tr>");
            print("<tr><td colspan=5></td><td rowspan=$ncomponents><img src=\"$server_media/nada.gif\" width=1 height=1 id={$this->prefix}_insertmarker name={$this->prefix}_insertmarker>\n");
            for($i=0;;++$i)
            {
?>
<tr><td colspan=1><img src="<?=$server_media?>/nada.gif" width=1 height=10 alt="[ spacer ]"></td><td colspan=3 id="<?=$this->prefix?>_insert<?=$i?>" class=insertmenu><img src="<?=$server_media?>/nada.gif" width=1 height=10 alt="[ spacer ]" name=<?=$this->prefix?>_ns4marker<?=$i?>></td></tr>
<?
              if ($i >= $ncomponents) break;
              $component = &$this->survey->components[$i];
              $w = $component->getwidget("{$this->prefix}_main_qwidget_$i", $this->form, $this->formmethod);
              $w->loadvalues();
?>
<tr><td><img src="<?=$server_media?>/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$server_media?>/0x000000.gif"><img src="<?=$server_media?>/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$server_media?>/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td rowspan=3 bgcolor="#000000" background="<?=$server_media?>/0x000000.gif"><img src="<?=$server_media?>/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=2><img src="<?=$server_media?>/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr>
<?

$needNode = true;

if($component->locked && $component->revision_id)
{
  wbes_connect();
  $sql = "SELECT topic_id FROM branches AS a, revisions AS b WHERE a.branch_id = b.branch_id AND b.revision_id = '$component->revision_id'";
  $r = pg_query($sql, $wbes, __FILE__, __LINE__);
  $wr = new pg_wrapper($r);
    
  if($wr->row['topic_id'] != $this->topic_id)
  {
    $needNode = false;
?>
    <td width=40 valign=center><img src="<?=$server_media?>/editor/grey_node.gif" width=40 height=18 border=0 align=absmiddle alt=locked_node></td>
<?
  }
}

if($needNode)
    {
?>
  <td width=40 valign=center><img src="<?=$server_media?>/editor/node.gif" width=40 height=18 border=0 align=absmiddle alt=node id=<?=$this->prefix?>_node<?=$i?> name=<?=$this->prefix?>_node<?=$i?>></td>
<?
    }?>

  

  <td width=1><img src="<?=$server_media?>/nada.gif" width=3 height=1 alt="[ spacer ]"></td>
  <td><? $w->displaypreview(); ?></td>
</tr>
<tr><td><img src="<?=$server_media?>/nada.gif" width=1 height=3 alt="[ spacer ]"></td><td colspan=2><img src="<?=$server_media?>/nada.gif" width=1 height=3 alt="[ spacer ]"></td></tr>
<tr><td><img src="<?=$server_media?>/nada.gif" width=1 height=1 alt="[ spacer ]"></td><td colspan=3 bgcolor="#000000" background="<?=$server_media?>/0x000000.gif"><img src="<?=$server_media?>/nada.gif" width=1 height=1 alt="[ spacer ]"></td></tr>
<?
            } // for i
?>
<tr>
  <td>&nbsp;</td>
  <td colspan=3 align=center>
    <? $this->main_action->display("Save Survey",SurveyEditor_main_save,"", 'class="tinybutton"'); ?>
    <? $this->main_action->display("Cancel",SurveyEditor_main_cancel,"", 'class="tinybutton"'); ?>
    <input type=button class=tinybutton value=Preview onclick="return SurveyEditor_preview(this.form,'<?=$this->main_action->prefix?>',<?=SurveyEditor_main_preview?>)">
    
    
  </td>
</tr>
</table>

<script>SurveyEditor(<?=count($this->survey->components)?>, "<?=$this->prefix?>", "<?=$this->form?>");</script>
<?
          } // if $ncomponents == 0      
        break;
        case SurveyEditor_done:
        break;
      } // switch
    } // if hidden
  }
  
  function dumpscript()
  {
    global $server_media, $SurveyEditor_ns4;
    static $dumped = false;
    if ($dumped) return; else $dumped = true;

    ActionButton::dumpscript();
    includeScript("$server_media/editor/menus.js");

    $mod = SurveyEditor_main_modify; $del = SurveyEditor_main_delete;
    $up = SurveyEditor_main_moveup; $down = SurveyEditor_main_movedown;
    $ins = SurveyEditor_main_insert;

    if (!$SurveyEditor_ns4)
    {
      $str = <<<EOD
<div id=SurveyEditor_cmdmenu style="position:absolute; visibility: hidden; z-index: 10"><table border=1 cellspacing=0 cellpadding=3>
<tr><td id=SurveyEditor_editcmd class=editmenu bgcolor="#EEEEEE"><img src="$server_media/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle> Edit Component</td></tr>
<tr><td id=SurveyEditor_deletecmd class=editmenu bgcolor="#EEEEEE"><img src="$server_media/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle> Delete Component</td></tr>
<tr><td id=SurveyEditor_moveupcmd class=editmenu bgcolor="#EEEEEE"><img src="$server_media/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle> Move Up</td></tr>
<tr><td id=SurveyEditor_movedowncmd class=editmenu bgcolor="#EEEEEE"><img src="$server_media/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle> Move Down</td></tr>
</table></div>
<div id=SurveyEditor_insert style="position:absolute; visibility:hidden"><img src="$server_media/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component"></div>
EOD;
    }
    else
    {
      includeScript("$server_media/editor/menus_ns4.js");      

      $str = <<<EOD
<layer id=SurveyEditor_cmdmenu visibility=hide z-index=10><table border=1 cellspacing=0 cellpadding=3>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose($mod)"><img src="$server_media/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle border=0> Edit Component</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose($del)"><img src="$server_media/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle border=0> Delete Component</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose($up)"><img src="$server_media/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle border=0> Move Up</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose($down)"><img src="$server_media/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle border=0> Move Down</td></tr>
</table></layer>
<layer id=SurveyEditor_mryellow visibility=hide width=2000 height=10 bgcolor="#FFE051" z-index=5></layer>
<layer id=SurveyEditor_insert visibility=hide z-index=5><img src="$server_media/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component"></layer>
EOD;
    }
    absolutePrint($str);

    absolutePrint("<script language=javascript>
SurveyEditor_main_insert = $ins;
SurveyEditor_main_modify = $mod;
SurveyEditor_main_delete = $del;
SurveyEditor_main_moveup = $up;
SurveyEditor_main_movedown = $down;
SurveyEditor_menu = SurveyEditor_Menu();
");

    if ($SurveyEditor_ns4)
    {
      absolutePrint("
window.SurveyEditor_loadpackage = new Array();
window.SurveyEditor_spots = new Array();  
window.SurveyEditor_onload_old = window.onload;
window.SurveyEditor_hotspot = 0;
window.onload = SurveyEditor_onload;
");  
    }
    absolutePrint("</script>");
    
  }
}
?>
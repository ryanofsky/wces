<?

require_once("config/survey.inc");
require_once("widgets/basic.inc");
require_once("wbes/postgres.inc");
require_once("wbes/survey.inc");
require_once("wbes/surveywidget.inc");
require_once("wbes/server.inc");

define("SurveyEditor_inserte",1);
define("SurveyEditor_modify",2);
define("SurveyEditor_delete",3);
define("SurveyEditor_moveup",4);
define("SurveyEditor_movedown",5);
define("SurveyEditor_save",6);
define("SurveyEditor_cancel",7);
define("SurveyEditor_preview",8);

function cell($width = false, $height = false, $color = false, $tdattr = '', $imgattr = '')
{
  global $SurveyEditor_ns4, $server_media;
  $cell = '<td';
  if ($width !== false) $cell .= " width=$width";
  if ($height !== false) $cell .= " height=$height";
  if ($color !== false) $cell .= " bgcolor=\"#$color\"";
  if ($SurveyEditor_ns4) $cell .= " background=\"$server_media/$color.gif\"";
  $cell .= "$tdattr>";
  if ($SurveyEditor_ns4 || true)
  {
    if ($width === false) $width = 1;
    if ($height === false) $height = 1;
    $cell .= "<img src=\"$server_media/$color.gif\" width=$width height=$height alt=\"\">";
  }
  $cell .= '</td>';
  return $cell;
}

$SurveyEditor_ns4 = $server_url->xpath == "/ns4";
$survey_nolock = false;

class SurveyEditor extends StatefullWidget
{
  // parameters
  var $specialization_id, $base_branch_id, $user_id;

  // output
  var $done = false;
  var $message = "";
  var $barePage = false;

  // persistent
  var $survey = null;

  // widgets
  var $insert;
  var $modify;
  var $preview;

  function SurveyEditor($item_id, $specialization_id, $user_id, $name, &$parent)
  {
    global $server_url;
    $this->StatefullWidget($name, $parent);
    $this->specialization_id = (int)$specialization_id;
    $this->item_id = (int)$item_id;
    $this->user_id = (int)$user_id;
    $this->event =& new EventWidget("event", $this);
  }

  function & handleEvent($event, $param, $new)
  {
    switch($event)
    {
      case SurveyEditor_inserte:
        $this->insert =& new SurveyEditorInsert("insert", $this);
        $this->loadChild($this->insert, $new);
        if (isset($this->insert->choose))
        {
          global $survey_factories;
          $factory = $survey_factories[$this->insert->choose];
          $q = &$factory->create();
          $q->neverSaved = true;
          $this->addEvent(SurveyEditor_modify, $this->survey->insert($q, $param));
        }
      return $this->insert;

      case SurveyEditor_modify:
        $this->modify =& new SurveyEditorModify($this->survey->components[$param], "modify", $this);
        $this->loadChild($this->modify, $new);
        if ($this->modify->preview)
          $this->addEvent(SurveyEditor_preview, $param);
        else if ($this->modify->previewError)
          $this->barePage = true;
        else if ($this->modify->done)
        {
          if (isset($this->survey->components[$param]->neverSaved))
            $this->survey->delete($param);
        }
      return $this->modify;

      case SurveyEditor_preview:
        $this->barePage = true;
        $this->preview =& new SurveyEditorPreview("preview", $this);
        if (isset($param))
        {
          $c =& $this->survey->components[$param];
          $inner =& $c->getWidget("inner", $this->preview);
        }
        else
        {
          $inner =& new SurveyWidget(0,0,0,0, "inner", $this->preview);
          $inner->survey =& $this->survey;
        }
        $this->preview->inner =& $inner;
        $this->loadChild($this->preview, $new);
      return $this->preview;

      case SurveyEditor_delete:
        $this->survey->delete($param);
      break;

      case SurveyEditor_moveup:
        $this->survey->moveup($param);
      break;

      case SurveyEditor_movedown:
        $this->survey->movedown($param);
      break;

      case SurveyEditor_save:
        if (SaveSurvey($this->survey, $this->specialization_id, $this->user_id))
        {
          $this->message = "<font color=red><i>Survey Builder: Saved changes</i></font>";
          $this->done = true;
        }
        else
          $this->message = "<h1>Save Error</h1>";
      break;

      case SurveyEditor_cancel:
        $this->done = true;
        $this->message = "<font color=red><i>Survey Builder: No changes were saved</i></font>";
      break;
    }
  }

  function loadInitialState()
  {
    global $survey_factories;
    if (!isset($this->survey))
      $this->survey =& LoadSurvey($this->item_id, $this->specialization_id, $survey_factories);
  }

  function loadState()
  {
    $this->survey = unserialize($this->readValue("survey"));
    StatefullWidget::loadState();
  }

  function printState()
  {
    StatefullWidget::printState();
    $this->printValue("survey", serialize($this->survey));
  }

  function printVisible()
  {
    global $server_media, $wbes;

    $this->event->displayField();

    print("<h5>Survey Builder</h5>");
    $ncomponents = count($this->survey->components);
    if ($ncomponents == 0)
    {
      print("<p>This survey does not have any components. Click below to either insert a component or cancel without saving any changes.</p>");
      $this->event->displayButton("Insert a component",SurveyEditor_inserte,0,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"');
      print(" ");
      $this->event->displayButton("Cancel",SurveyEditor_cancel,"", 'class="tinybutton" style="BACKGROUND: #CE3031; COLOR: white;"');
    }
    else // $ncomponents != 1
    { 
      $insertMarker = $this->fullname("insertMarker");
      $horizontalBorder = '<tr>' . cell(false, 1) . cell(false, 1, '000000', ' colspan=2') . "</tr>\n";
?>
<p>This page shows a WYSIWYG view of your survey. You can modify, delete, and
rearrange survey components by moving your mouse over the blue orbs at the left
and making a selection from the popup menu. You can insert new survey components
by clicking above and below existing components.</p>

<table border=0 cellpadding=0 cellspacing=0>
<tr>
  <td colspan=2></td>
  <td align=center>
    <? $this->event->displayButton("Add new component",SurveyEditor_inserte,$ncomponents,'class="tinybutton" style="BACKGROUND: #00BE21; COLOR: white;"'); ?>
  </td>
</tr>
<tr><td colspan=3></td><td width=1><img src="<?=$server_media?>/nada.gif" width=1 height=1 id=<?=$insertMarker?> name=<?=$insertMarker?> alt=""></td></tr>
<?
      for($i=0;;++$i)
      {
        $id = $this->fullName("insert$i");
        $ns4name = $this->fullName("ns4marker$i");
        print('<tr>');
        print(cell(false, 10));
        print(cell(false, 10, false, " colspan=2 id=$id class=insertmenu", "name=$ns4name"));
        print("</tr>\n");

        if ($i >= $ncomponents) break;
        $component =& $this->survey->components[$i];
        $w = $component->getWidget("main_qwidget_$i", $this);
        $this->loadChild($w, true);

        $readOnly = false;
        global $survey_nolock;
        if(!$survey_nolock && $component->locked && $component->item_id)
        {
          wbes_connect();
          $r = pg_go("
            SELECT EXISTS (SELECT * FROM item_specializations WHERE
            item_id = $component->item_id AND specialization_id = $this->specialization_id)
          ", $wbes, __FILE__, __LINE__);
          
          $readOnly = pg_result($r, 0, 0) != 't';
        }
        $name = $this->fullName($readOnly ? "lockedNode$i" : "node$i");
        $image = $readOnly ? "grey_node.gif" : "node.gif";
        
        print($horizontalBorder);
        print("<tr>\n");
        print("  <td width=40><img src=\"$server_media/editor/$image\" width=40 height=18 border=0 id=$name name=$name align=absmiddle alt=Node></td>\n");
        print("  " . cell(1, false, '000000') . "\n");
        print("  <td style=\"padding: 10px\">"); $w->preview = true; $w->display(); print("</td>\n");
        print("</tr>\n");
        print($horizontalBorder);
      }
?>
<tr>
  <td colspan=2></td>
  <td align=center>
    <? $this->event->displayButton("Save Survey", SurveyEditor_save, null, 'class="tinybutton"'); ?>
    <? $this->event->displayButton("Cancel", SurveyEditor_cancel, null, 'class="tinybutton"'); ?>
    <input type=button class=tinybutton value=Preview onclick="return SurveyEditor_preview(this.form,'<?=$this->event->name?>',<?=SurveyEditor_preview?>)">
  </td>
</tr>
</table>

<script>SurveyEditor(<?=count($this->survey->components)?>, "<?=$this->name?>", "<?=$this->form->formName?>");</script>
<?
    } // if $ncomponents == 0
  }

  function DumpScript()
  {
    global $server_media, $SurveyEditor_ns4;
    static $dumped = false;
    if ($dumped) return; else $dumped = true;

    EventWidget::DumpScript();
    includeScript("$server_media/editor/menus.js");

    $mod = SurveyEditor_modify; $del = SurveyEditor_delete;
    $up = SurveyEditor_moveup; $down = SurveyEditor_movedown;
    $ins = SurveyEditor_inserte;

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
    html_body_append($str);

    html_body_append("<script language=javascript>
SurveyEditor_inserte = $ins;
SurveyEditor_modify = $mod;
SurveyEditor_delete = $del;
SurveyEditor_moveup = $up;
SurveyEditor_movedown = $down;
SurveyEditor_menu = SurveyEditor_Menu();
");

    if ($SurveyEditor_ns4)
    {
      html_body_append("
window.SurveyEditor_loadpackage = new Array();
window.SurveyEditor_spots = new Array();
window.SurveyEditor_onload_old = window.onload;
window.SurveyEditor_hotspot = 0;
window.onload = SurveyEditor_onload;
");
    }
    html_body_append("</script>");

  }
}

define("SurveyEditorInsert_create",1);
define("SurveyEditorInsert_cancel",2);

class SurveyEditorInsert extends ParentWidget
{
  // output
  var $done = false;
  var $choose = null;

  // widgets
  var $event;
  var $choices;

  // private
  var $badInput = false;

  function SurveyEditorInsert($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->event =& new EventWidget("event", $this);
    $this->choices =& new RadioButton(true, "choices", $this);
  }

  function loadState()
  {
    ParentWidget::loadState();
    switch($this->event->event)
    {
      case SurveyEditorInsert_create:
        global $survey_factories;
        if (!isset($survey_factories[$this->choices->checked]))
          $this->badInput = true;
        else
        {
          $this->choose = $this->choices->checked;
          $this->done = true;
        }
      break;

      case SurveyEditorInsert_cancel:
        $this->done = true;
      break;
    }
  }

  function printVisible()
  {
    global $server_media;

    if ($this->badInput)
      print("<p><font color=red>You forgot to choose an item from the list</font></p>");

    print("<h5>Which component type would you like to insert? <a href=\"javascript:showhelp('inserttypes')\"><img src=\"$server_media/help/question.gif\" width=16 height=16 border=0 align=absmiddle alt=help></a></h5>\n");

    $first = true;
    global $survey_factories;
    foreach ($survey_factories as $key => $f)
    {
      $name = $f->name();
      if (!isset($name)) continue;
      if ($first) $first = false; else print("<br>\n");
      $this->choices->display($key);
      $this->choices->displayLabel($key, $name);
    }

    print("\n<p>");
    $this->event->displayButton("Create", SurveyEditorInsert_create, null, 'class="tinybutton"');
    $this->event->displayButton("Cancel", SurveyEditorInsert_cancel, null, 'class="tinybutton"');
    print("</p>");
  }
};

define("SurveyEditorModify_save",1);
define("SurveyEditorModify_cancel",2);
define("SurveyEditorModify_preview",3);

class SurveyEditorModify extends ParentWidget
{
  // parameters
  var $component;

  // output
  var $done = false;
  var $preview = false;
  var $previewError = false;

  // widgets
  var $editor;
  var $event;

  function SurveyEditorModify(&$component, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->component =& $component;
    $this->event =& new EventWidget("event", $this);
    $this->editor =& $component->getEditor("editor", $this);
  }
  
  function loadState()
  {
    ParentWidget::loadState();
    switch($this->event->event)
    {
      case SurveyEditorModify_save:
        if ($this->editor->validate())
        {
          unset($this->editor->component->neverSaved);
          $this->editor->save();
          $this->done = true;
        }
      break;

      case SurveyEditorModify_cancel:
        $this->done = true;
      break;

      case SurveyEditorModify_preview:
        if($this->editor->validate())
        {
          $this->editor->save();
          $this->preview = true;
          $this->done = true;
        }
        else
          $this->previewError = true;
      break;

      default:
      return;
    }
  }

  function printVisible()
  {
    if ($this->previewError)
      $this->editor->printErrors();
    else
    {
      $this->editor->display();
      $this->event->displayButton("Save", SurveyEditorModify_save, null, 'class="tinybutton"');
      $this->event->displayField();
      print('<input type=button class=tinybutton value=Preview onclick="'
        . "return SurveyEditor_preview(this.form,'{$this->event->name}', "
        . SurveyEditorModify_preview . ")\">");
      $this->event->displayButton("Cancel", SurveyEditorModify_cancel, null, 'class="tinybutton"');
    }
  }
};

class SurveyEditorPreview extends ParentWidget
{
  // output
  var $done = false;

  // widgets
  var $inner = null;

  function SurveyEditorPreview($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function printVisible()
  {
    print('<p align=center><input type=button onclick="window.close()" value="Close this window"></p>');
    if (isset($this->inner)) $this->inner->display();
  }
};

?>

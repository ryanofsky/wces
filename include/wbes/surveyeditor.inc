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
  
  function SurveyEditor($topic_id, $base_branch_id, $factories, $prefix,$form,$formmethod)
  {
    global $server_url;
    $this->FormWidget($prefix,$form,$formmethod);
    $this->topic_id = (int)$topic_id;
    $this->base_branch_id = (int)$base_branch_id;
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
    if ($this->survey->save($this->topic_id,0))
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
    global $SurveyEditor_dumped,$server_media, $SurveyEditor_ns4;
    if (!isset($SurveyEditor_dumped))
    {
      ActionButton::dumpscript();
      $SurveyEditor_dumped = true;

?>

<!-- Begin SurveyEditor Script -->

<? if (!$SurveyEditor_ns4) { ?>
<div id=SurveyEditor_cmdmenu style="position:absolute; visibility: hidden; z-index: 10"><table border=1 cellspacing=0 cellpadding=3>
<tr><td id=SurveyEditor_editcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$server_media?>/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle> Edit Component</td></tr>
<tr><td id=SurveyEditor_deletecmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$server_media?>/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle> Delete Component</td></tr>
<tr><td id=SurveyEditor_moveupcmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$server_media?>/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle> Move Up</td></tr>
<tr><td id=SurveyEditor_movedowncmd class=editmenu bgcolor="#EEEEEE"><img src="<?=$server_media?>/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle> Move Down</td></tr>
</table></div>

<div id=SurveyEditor_insert style="position:absolute; visibility:hidden"><img src="<?=$server_media?>/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component"></div>
<? } else { // SurveyEditor_ns4 ?>
<layer id=SurveyEditor_cmdmenu visibility=hide z-index=10><table border=1 cellspacing=0 cellpadding=3>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose(<?=SurveyEditor_main_modify?>)"><img src="<?=$server_media?>/editor/edit.gif" width=20 height=20 alt="Edit" align=absmiddle border=0> Edit Component</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose(<?=SurveyEditor_main_delete?>)"><img src="<?=$server_media?>/editor/delete.gif" width=20 height=20 alt="Delete" align=absmiddle border=0> Delete Component</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose(<?=SurveyEditor_main_moveup?>)"><img src="<?=$server_media?>/editor/moveup.gif" width=20 height=20 alt="Move Up" align=absmiddle border=0> Move Up</td></tr>
<tr><td class=editmenu bgcolor="#EEEEEE"><a href="javascript:ns_choose(<?=SurveyEditor_main_movedown?>)"><img src="<?=$server_media?>/editor/movedown.gif" width=20 height=20 alt="Move Down" align=absmiddle border=0> Move Down</td></tr>
</table></layer>

<layer id=SurveyEditor_mryellow visibility=hide width=2000 height=10 bgcolor="#FFE051" z-index=5></layer>

<layer id=SurveyEditor_insert visibility=hide z-index=5><img src="<?=$server_media?>/editor/insertcomponent.gif" width=145 height=15 alt="Insert a component"></layer>
<? } // if SurveyEditor_ns4 ?>

<script>
<!--

  function elem(str)
  {
    if (document.all)
    {
      return document.all(str);
    }
    else if (document.getElementById)
    {
      return document.getElementById(str);
    }
  }
  
  function getoffset(element)
  {
    var offset = new Object();
    if((element.x || element.x == 0) && (element.y || element.y == 0))
    {
      offset.x = element.x;
      offset.y = element.y;
      return offset;
    }
    else
    {
      offset.x = 0;
      offset.y = 0;
      for(;;element = element.offsetParent)
      {
        offset.x += element.offsetLeft;
        offset.y += element.offsetTop;
        if (!element.offsetParent) return offset;
      }
    }
  }

  function SurveyEditor(nodes,prefix,form) // DOM
  {
<? if (!$SurveyEditor_ns4) { ?>
    var actionobject = document.forms[form][prefix + "_main_action"];
    var marker = elem(prefix + "_insertmarker");
   
    for(var i=0;;++i)
    {
      var insert = elem(prefix + "_insert" + i);
      insert.onclick = SurveyEditor_insert_onclick;
      insert.onmouseover = SurveyEditor_insert_onmouseover;
      insert.onmouseout = SurveyEditor_insert_onmouseout;
      insert.key = i;
      insert.actionobject = actionobject;
      insert.marker = marker;
      
      if (i >= nodes) break;

      var node = elem(prefix + "_node" + i);

      if(node == null)
        {continue;}
      
      node.onmouseover = SurveyEditor_node_onmouseover;
      node.onmouseout = SurveyEditor_node_onmouseout;
      node.key = i;
      node.actionobject = actionobject;
    }  
<? } else { // SurveyEditor_ns4 ?>
    var o = new Object();
    o.nodes = nodes;
    o.prefix = prefix;
    o.form = form;
    window.SurveyEditor_loadpackage.push(o);
<? } // if SurveyEditor_ns4 ?>
  }

<? if ($SurveyEditor_ns4) { ?>
  
  function SurveyEditor_Spot(x1,y1,x2,y2,obj)
  {
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;
    this.obj = obj;
  }
  
  function SurveyEditor_onload(e)
  {
    if (window.SurveyEditor_onload_old) window.SurveyEditor_onload_old(e);
    
    for (p in window.SurveyEditor_loadpackage)
    {
      var o = window.SurveyEditor_loadpackage[p];    
      var r = document.images[o.prefix + '_insertmarker'].x;
      var actionobject = document.forms[o.form][o.prefix + "_main_action"];
         
      for(var i=0;;++i)
      {
        var insert = document.images[o.prefix + '_ns4marker' + i];
        insert.ns4_onclick = SurveyEditor_insert_onclick;
        insert.ns4_onmouseover = SurveyEditor_insert_onmouseover;
        insert.ns4_onmouseout = SurveyEditor_insert_onmouseout;
        insert.key = i;
        insert.actionobject = actionobject;
        insert.right_edge = r;
        window.SurveyEditor_spots.push(new SurveyEditor_Spot(insert.x, insert.y, r, insert.y + 10, insert)); 
        
        if (i >= o.nodes) break;
        
        var node = document.images[o.prefix + '_node' + i];
        
        if (!node) continue;
        
        node.ns4_onmouseover = SurveyEditor_node_onmouseover;
        node.ns4_onmouseout = SurveyEditor_node_onmouseout;
        node.key = i;
        node.actionobject = actionobject;
        window.SurveyEditor_spots.push(new SurveyEditor_Spot(node.x, node.y, node.x + node.width, node.y + node.height, node));
      }
    }
    
    window.SurveyEditor_loadpackage = false;
  }
  
  function SurveyEditor_mousemove(e)
  {
    if (window.SurveyEditor_mousemove_old) window.SurveyEditor_mousemove_old(e);
    
    //window.status = "hello from " + e.pageX + "," + e.pageY + (window.hovermenu ? " ON" : " OFF") ;
   
    var x = e.pageX;
    var y = e.pageY;
    var oldhot = window.SurveyEditor_hotspot;

    var spot = false;
    window.SurveyEditor_hotspot = 0;

    if (!window.SurveyEditor_menu.hover)
    for(i in window.SurveyEditor_spots)
    {
      var s = window.SurveyEditor_spots[i];
      if (s.x1 <= x && x <= s.x2 && s.y1 <= y && y <= s.y2)
      {
        window.SurveyEditor_hotspot = i - -1; // netscape 4.08 thinks i is a string
        spot = s.obj;
        break;
      }
    }
    
    if(oldhot != window.SurveyEditor_hotspot)
    {
      var oldspot = oldhot ? window.SurveyEditor_spots[oldhot - 1].obj : false;
      
      if (oldspot && oldspot.ns4_onmouseout)
        oldspot.ns4_onmouseout(e);
      
      if (spot && spot.ns4_onmouseover)
        spot.ns4_onmouseover(e);
    }
  }

  function SurveyEditor_mouseclick(e)
  {
    if(window.SurveyEditor_mouseclick_old) window.SurveyEditor_mouseclick_old(e);
    
    var spot = window.SurveyEditor_hotspot ? window.SurveyEditor_spots[window.SurveyEditor_hotspot - 1].obj : false;
    if (spot && spot.ns4_onclick) spot.ns4_onclick(e);
  }

  function ns_choose(action)
  {
    var m = window.SurveyEditor_menu;
    ActionButton_go(action, m.activenode.key, m.actionobject);
  }

<? } // if SurveyEditor_ns4 ?>

  function SurveyEditor_node_onmouseover()
  {
    this.hover = true;
    window.SurveyEditor_menu.show(this,this.actionobject);
  }

  function SurveyEditor_node_onmouseout()
  {
    this.hover = false;
    window.SurveyEditor_menu.timeout();
  }
  
  function SurveyEditor_insert_onmouseover()
  {
<? if (!$SurveyEditor_ns4) { ?>
    this.className = "insertmenuhover";
    var offset = getoffset(this);

    if (this.clientWidth) // Internet Explorer
      w = offset.x + this.clientWidth + 4;
    else // DOM browser workaround
      w = getoffset(this.marker).x;

    var s = elem("SurveyEditor_insert").style;
    s.left = w + "px";
    s.top = (offset.y + -3) + "px";
    s.visibility = "visible";
<? } else { // SurveyEditor_ns4 ?>

    var l = document.SurveyEditor_mryellow;
    l.pageX = this.x;
    l.pageY = this.y;
    l.clip.right = this.right_edge - this.x;
    l.visibility = 'show';
    
    var c = document.SurveyEditor_insert 
    c.pageX = this.right_edge;
    c.pageY = this.y;
    c.visibility = 'show';
<? } // if SurveyEditor_ns4 ?>
  }

  function SurveyEditor_insert_onmouseout()
  {
<? if (!$SurveyEditor_ns4) { ?>
    this.className = "insertmenu";
    var s = elem("SurveyEditor_insert").style;
    s.visibility = "hidden";
<? } else { // SurveyEditor_ns4 ?>    
    var l = document.SurveyEditor_mryellow;
    l.visibility = 'hide';
    var c = document.SurveyEditor_insert ;
    c.visibility = 'hide';
<? } // if SurveyEditor_ns4 ?>
  }

  function SurveyEditor_insert_onclick()
  {
    ActionButton_go("<?=SurveyEditor_main_insert?>",this.key,this.actionobject);
  }

  function SurveyEditor_Menu() // extender
  {
<? if (!$SurveyEditor_ns4) { ?>
    var a = elem("SurveyEditor_cmdmenu");
    a.TDinit = SurveyEditor_Menu_TDinit;
    a.TDinit("<?=SurveyEditor_main_modify?>", "SurveyEditor_editcmd");
    a.TDinit("<?=SurveyEditor_main_delete?>", "SurveyEditor_deletecmd");
    a.TDinit("<?=SurveyEditor_main_moveup?>", "SurveyEditor_moveupcmd");
    a.TDinit("<?=SurveyEditor_main_movedown?>", "SurveyEditor_movedowncmd");
<? } else { // SurveyEditor_ns4 ?>
    var a = document.SurveyEditor_cmdmenu;

    window.captureEvents(Event.MOUSEMOVE);
    window.SurveyEditor_mousemove_old = window.onmousemove;
    window.onmousemove = SurveyEditor_mousemove;
    
    window.captureEvents(Event.MOUSEUP);
    window.SurveyEditor_mouseclick_old = window.onmouseup;
    window.onmouseup = SurveyEditor_mouseclick;
   
<? } // if SurveyEditor_ns4 ?>
    a.onmouseout = SurveyEditor_Menu_onmouseout;
    a.onmouseover = SurveyEditor_Menu_onmouseover;
    a.timeout = SurveyEditor_Menu_timeout;
    a.show = SurveyEditor_Menu_show;
    a.hide = SurveyEditor_Menu_hide;
    a.hover = a.activenode = a.actionobject = false; //state variables
    return a;    
  }

  function SurveyEditor_Menu_TDinit(action, tdname)
  {
    var t = elem(tdname);
    t.table = this;
    t.onmouseover = SurveyEditor_Menu_TD_onmouseover;
    t.onmouseout = SurveyEditor_Menu_TD_onmouseout;
    t.onclick = SurveyEditor_Menu_TD_onclick;
    t.action = action;
  }

  function SurveyEditor_Menu_TD_onclick()
  {
    ActionButton_go(this.action, this.table.activenode.key, this.table.actionobject);
  }

  function SurveyEditor_Menu_TD_onmouseover()
  {
    this.className = "editmenuhover";
  }

  function SurveyEditor_Menu_TD_onmouseout()
  {
    this.className = "editmenu";
  }

  function SurveyEditor_Menu_onmouseover()
  {
    this.hover = true;
  }

  function SurveyEditor_Menu_onmouseout()
  {
    this.hover = false;
    this.timeout();
  }

  function SurveyEditor_Menu_timeout()
  {
    window.setTimeout("window.SurveyEditor_menu.hide()",100);
  }

  function SurveyEditor_Menu_show(activenode,actionobject)
  {
    this.activenode = activenode;
    this.actionobject = actionobject;
<? if (!$SurveyEditor_ns4) { ?>
    var offset = getoffset(activenode);

    if(window.event && window.event.clientX && navigator.userAgent.indexOf("mac") != -1)
	{
	xDiff = Math.abs(window.event.clientX + window.screenLeft - offset.x);
	yDiff = Math.abs(window.event.clientY + window.screenTop - offset.y);

	if(xDiff > 5)
	    {offset.x = window.event.clientX;}
    
	if(yDiff > 5)
	    {offset.y = window.event.clientY;}
	}

    this.style.left = (offset.x + 9) + "px";
    this.style.top  = (offset.y + 9) + "px";
    this.style.visibility = "visible";
<? } else { // SurveyEditor_ns4 ?>    
    this.pageX = activenode.x + 9;
    this.pageY = activenode.y + 9;
    this.visibility = 'show';
<? } // if SurveyEditor_ns4 ?>    
  }

  function SurveyEditor_Menu_hide()
  {
    if (!this.hover && !this.activenode.hover)
<? if (!$SurveyEditor_ns4) { ?>    
      this.style.visibility = "hidden";
<? } else { // SurveyEditor_ns4 ?>
      this.visibility = "hide";
<? } // if SurveyEditor_ns4 ?>      
  }

  window.SurveyEditor_menu = SurveyEditor_Menu();
<? if ($SurveyEditor_ns4) { ?>
  window.SurveyEditor_loadpackage = new Array();
  window.SurveyEditor_spots = new Array();  
  window.SurveyEditor_onload_old = window.onload;
  window.SurveyEditor_hotspot = 0;
  window.onload = SurveyEditor_onload;
<? } // if SurveyEditor_ns4 ?>

  function SurveyEditor_preview(form,actionelement,state)
  {
    var ns4bug = navigator.appName == "Netscape" && parseFloat(navigator.appVersion) < 5;
    
    if(!ns4bug) // race condition in netscape 4, most of these commands are not serialized and execute out of turn
    {
      var w = window.open("about:blank", "previewwin", "width=500,height=410,status=0,toolbar=0,menubar=0,scrollbars=1,screenX=5,screeny=5,left=0,top=0,resizable=1",true);
      form.target = "previewwin";
      ActionButton_go(state,0,form[actionelement]);
      form.target = "_self";
      form[actionelement].value = "";
      w.focus();
    }
    else // yet another cumbersome workaround
    {
      window.SurveyEditor_form = form;
      
      var w = window.open("about:blank", "previewwin", "width=500,height=410,status=0,toolbar=0,menubar=0,scrollbars=1,screenX=5,screeny=5,left=0,top=0,resizable=1",true);
      w.opener = window;

      var d = w.document;
      d.open();
      d.write("<script>\n");
      d.write("window.opener.SurveyEditor_form.target = 'previewwin';\n");
      d.write("window.opener.ActionButton_sgo(" + state + ", 0, '" + form.name + "', '" + actionelement + "');\n");
      d.write("window.opener.SurveyEditor_form.target = '_self';\n");
      d.write("window.opener.SurveyEditor_form['" + actionelement + "'].value = '';\n");
      d.write("</script>\n");
      d.close();
      w.focus();
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
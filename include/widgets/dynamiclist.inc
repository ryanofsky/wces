<?

require_once("widgets/widgets.inc");
require_once("wbes/server.inc");

class DynamicList extends Widget
{
  var $width;
  var $height;
  var $orderOnly = false;
  
  var $items = array();
  var $keys = array();
  
  function DynamicList($width, $height, $orderOnlyitems, $name, &$parent)
  {
    $this->Widget($name, $parent);
    $this->width = (int)$width;
    $this->height = (int)$height;
    if (is_array($orderOnlyitems))
    {
      $this->keys = array_keys($orderOnlyitems);
      $this->items = array_values($orderOnlyitems);
      $this->orderOnly = true;
    }
    $this->DumpScript();    
  }
  
  function loadStringArray($attr, $separator = "\t")
  {
    $data = $this->readValue($attr);
    if (!isset($data) || $data === "") return array();
    return explode($separator,$data);
  }
  
  function makeStringArray($array)
  {
    return implode("\t", $array);
  }
  
  function loadState($strongchecking = true) // public inherited
  {
    if (!$this->orderOnly)
    {
      $this->items = $this->loadStringArray("items");
      $this->keys = $this->loadStringArray("keys");
    }  
    else
    {  
      $items = $keys = array();
      foreach ($this->loadStringArray("keys") as $k)
      if (($i = array_search($k,$this->keys)) !== false)
      { 
        $items[] = $this->items[$i];
        $keys[] = $k;
        unset($this->keys[$i]);
      }
      foreach($this->keys as $i => $k)
      {
        $items[] = $this->items[$i];
        $keys[] = $k;
      }
      $this->items = $items;
      $this->keys = $keys;
    }   
  }

  function displayHidden()
  {
    $this->printValue("keys", $this->makeStringArray($this->keys));
    $this->printValue("items", $this->makeStringArray($this->items));
  }
  
  function display($hidden = false) // public inherited
  {
    global $wces_path, $server_media;
    $this->printValue("keys", "");
    $this->printValue("items", "");
    $controller = $this->scriptName("controller");
    $doWidth = $this->width > 0 && count($this->items) <= 0;
?>  
<!-- begin FormWidget <?=$this->name?> -->
<table border="0" cellpadding="2" cellspacing="0">
  <tr>
    <td align=center>
      <select size="<?=$this->height?>" name="<?=$this->fullName("box")?>" multiple>
<?
  if ($doWidth)
    print("<option>" . str_repeat('-',$this->width) . "</option>\n");

  foreach(array_keys($this->items) as $i)
  {
    $v = isset($this->keys[$i]) ? ' value="' . htmlspecialchars($this->keys[$i]) . '"' : "";
    $t = htmlspecialchars($this->items[$i]);
    print("<option$v>$t</option>\n");
  }
?>
      </select>
    </td>
<? if (!$this->orderOnly) { ?>
    <td>
      <input type="button" value="Add..." name="<?=$this->fullName("additem")?>" style="width: 75px"><br>
      <input type="button" value="Modify..." name="<?=$this->fullName("modifyitem")?>" style="width: 75px"><br>
      <input type="button" value="Remove" name="<?=$this->fullName("removeitem")?>" style="width: 75px"><br>
    </td>
<? } ?>          
  </tr>
  <tr><td align=center>
    <a onmousedown="button_push('<?=$this->fullName('moveup')?>'); <?=$controller?>.moveup();"
       onmouseup="button_unpush('<?=$this->fullName('moveup')?>')"
       onmouseout="button_unpush('<?=$this->fullName('moveup')?>')"
       href="javascript:void(<?=$controller?>.moveup())"
       onclick="return false;"
    ><img name="<?=$this->fullName('moveup')?>" src="<?=$server_media?>/dynamiclist/moveup.gif" border=0></a>
    <a onmousedown="button_push('<?=$this->fullName('movedown')?>'); <?=$controller?>.movedown();"
       onmouseup="button_unpush('<?=$this->fullName('movedown')?>')"
       onmouseout="button_unpush('<?=$this->fullName('movedown')?>')"
       href="javascript:void(<?=$controller?>.movedown())"
       onclick="return false;"
    ><img name="<?=$this->fullName('movedown')?>" src="<?=$server_media?>/dynamiclist/movedown.gif" border=0></a>
    <script>
    <!--
      AttachPushed('<?=$this->fullName('moveup')?>',  "moveup_pushed.gif");
      AttachPushed('<?=$this->fullName('movedown')?>',"movedown_pushed.gif"); 
    //-->
    </script>
  </td><? if (!$this->orderOnly) { ?><td>&nbsp;</td><? } ?></tr>  
</table>
<script language="javascript">
<!--
    <?=$controller?> = DynamicList('<?=$this->form->formName?>','<?=$this->name?>');
    <? if($doWidth) print("$controller.selectedIndex = 0;\n    $controller.mremove();\n"); ?> 
// -->
</script>
<!-- end FormWidget <?=$this->name?> -->
<?
  }
 
  function DumpScript()
  {
    global $server_media;
    static $dumped = false;
    if ($dumped) return false; else $dumped = true;

    ImageButton::DumpScript();

    $ws = WIDGET_SEPARATOR;

    $str = <<<EOD
<!-- begin DynamicList FormWidget script -->
<script language="javascript">
<!--

  function DynamicList(formname,prefix)
  {
    var form = document.forms[formname];
    var box = form[prefix + "{$ws}box"];
    box.items = form[prefix + "{$ws}items"];
    box.keys = form[prefix + "{$ws}keys"];
    // members 'add' and 'remove' are apparently reserved on ie for mac
    box.madd = DynamicList_add;
    box.mremove = DynamicList_remove;
    box.modify = DynamicList_modify;
    box.move = DynamicList_move;
    box.moveup = DynamicList_moveup;
    box.movedown = DynamicList_movedown;
    box.save = DynamicList_save;
    box.getItem = DynamicList_getItem
    
    if (!form.DynamicList_lists)
    {
      form.DynamicList_lists = new Array();
      form.DynamicList_onsubmit_old = form.onsubmit;
      form.onsubmit = DynamicList_onsubmit;
    }

    form.DynamicList_lists[form.DynamicList_lists.length] = box;
    DynamicList_GenericButton(form[prefix + "{$ws}additem"]    , box, "this.list.madd()");
    DynamicList_GenericButton(form[prefix + "{$ws}modifyitem"] , box, "this.list.modify()");
    DynamicList_GenericButton(form[prefix + "{$ws}removeitem"] , box, "this.list.mremove()");
    return box;
  };

  function DynamicList_onsubmit()
  {
    var l = this.DynamicList_lists.length;

    for(var i = 0; i < l; ++i)
      this.DynamicList_lists[i].save();

    if (this.DynamicList_onsubmit_old)
      return this.DynamicList_onsubmit_old();
  }

  function DynamicList_save()
  {
    var o = this.options;
    var sep = '';

    this.items.value = "";
    this.keys.value = "";
    
    for(var i = 0; i < o.length; ++i)
    {
      this.items.value += sep + o[i].text;
      this.keys.value  += sep + o[i].value;
      sep = '\t';
    }
  };

  function DynamicList_add()
  {
    var s = prompt("Enter the name of an item to add to the list","");
    var o = this.options;
    if (s && s!="")
    {
      var v = new Option(s);
      v.value = "";
      o[o.length] = v;
    }
  }

  function DynamicList_remove()
  {
    for (var i = this.options.length - 1; i >= 0; --i)
    {
      if (this.options[i].selected)
        this.options[i] = null;
    }
  }

  function DynamicList_modify()
  {
    if (this.selectedIndex >= 0)
    {
      var s = prompt("Change the name of this item",this.options[this.selectedIndex].text);
      if (s == "")
        this.mremove();
      else if (s)
        this.options[this.selectedIndex].text = s;
    }
  }

  function DynamicList_getItem(index, direction)
  {
    if (direction < 0)
      return this.options[this.options.length - index - 1];
    else
      return this.options[index];
  }

  // Move selected items in the list box up or down. The list box
  // is allowed to have many non-consecutive items selected.
  // (this is the reason why the function is so long and complicated).
  // 
  // Arguments:
  // 
  //   direction  a negative number for down, a positive number for up
  //   extreme    boolean. If true it moves the currently selected items
  //              to the top or bottom of the list instead of just one
  //              position up or down.

  // XXX: this needs to be tested on IE5 for mac. If it doesn't work, see
  // comment in revision 1.9 of this file on "moveDirection" for a possible
  // fix. 

  function DynamicList_move(direction, extreme)
  {
    var o = this.options;
    var move = null;
    var target = null;
    
    // find the first element that needs to be moved
    // and the position where it needs to be moved.
    
    for(var i = 0; i < o.length; ++i)
    {
      var s = this.getItem(i, direction);
      if (s.selected)
      {
        move = i;
        target = extreme ? 0 : (i == 0 ? i : i - 1);
        break;
      }
    }
    
    // return if nothing is selected
    if (move == null) return;
    
    // cache array is used to avoid spurious writes to options array. 
    var cache = new Array();
    
    // line up selected items at the target positions.
    // move the non-selected items which are overwritten
    // to the cache array
    outer:
    for(;;)
    {
      //assert(target <= move);
      if (move != target)
      {
        var t = this.getItem(target, direction);
        var m = this.getItem(move, direction);
        if (!t.selected)
          cache[cache.length] = new Option(t.text, t.value);
        t.text = m.text;
        t.value = m.value;
        t.selected = true;
      }
      ++target;
      do
      {
        ++move;
        if (move >= o.length) break outer;
      }
      while (!(this.getItem(move, direction).selected));
    }
    
    // write out entries in the cache array
    for(var i = 0; i < cache.length; ++i)
    {
      var t = this.getItem(target, direction);
      var c = cache[i];
      if (!t.selected)
        cache[cache.length] = new Option(t.text, t.value);
      t.text = c.text;
      t.value = c.value;
      t.selected = false;
      ++target;
    }    
  }

  function DynamicList_moveup()
  {
    this.move(1);
  }

  function DynamicList_movedown()
  {
    this.move(-1);
  }

  function DynamicList_GenericButton(button,list,action)
  {
    if (button)
    {
      button.onclick = new Function(action);
      button.list = list;
    }  
  };

// -->
</script>
<!-- end DynamicList FormWidget script -->
EOD;
    html_head_append($str);
  }
};

?>
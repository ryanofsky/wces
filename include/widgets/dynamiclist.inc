<?

require_once("widgets/widgets.inc");
require_once("wbes/server.inc");

class DynamicList extends FormWidget
{
  var $width;
  var $height;
  var $orderonly = false;
  
  var $items = array();
  var $keys = array();
  
  function DynamicList($width,$height,$orderonlyitems,$prefix,$form,$formmethod) // public constructor inherited
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->width = (int)$width;
    $this->height = (int)$height;
    if (is_array($orderonlyitems))
    {
      $this->keys = array_keys($orderonlyitems);
      $this->items = array_values($orderonlyitems);
      $this->orderonly = true;
    }
  }
  
  function loadstrarray($attr, $separator = "\t")
  {
    $data = $this->loadattribute($attr);
    if (!isset($data)) return array();
    $r = explode($separator,$data);
    if (is_array($r)) return $r; else return array($r);
  }
  
  function loadvalues($strongchecking = true) // public inherited
  {
    if (!$this->orderonly)
    {
      $this->items = $this->loadstrarray("items");
      $this->keys = $this->loadstrarray("keys");
    }  
    else
    {  
      $items = $keys = array();
      foreach ($this->loadstrarray("keys") as $k)
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
 
  function dumpscript() //public static inherited
  {
    global $DynamicList_dumped, $server_media;
    if (!$DynamicList_dumped)
    {
      $DynamicList_dumped = true;
      ImageButton::dumpscript();
?>
<!-- begin DynamicList FormWidget script -->
<script language="javascript">
<!--

  function DynamicList(formname,prefix)
  {
    var form = document.forms[formname];
    var box = form[prefix + "_box"];
    box.items = form[prefix + "_items"];
    box.keys = form[prefix + "_keys"];
    box.add = DynamicList_add;
    box.remove = DynamicList_remove;
    box.modify = DynamicList_modify;
    box.move = DynamicList_move;
    box.save = DynamicList_save;
    
    if (!form.DynamicList_lists)
    {
      form.DynamicList_lists = new Array();
      form.DynamicList_onsubmit_old = form.onsubmit;
      form.onsubmit = DynamicList_onsubmit;
    }

    form.DynamicList_lists[form.DynamicList_lists.length] = box;
    DynamicList_GenericButton(form[prefix + "_additem"]    , box, "this.list.add()");
    DynamicList_GenericButton(form[prefix + "_modifyitem"] , box, "this.list.modify()");
    DynamicList_GenericButton(form[prefix + "_removeitem"] , box, "this.list.remove()");
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
    if (this.selectedIndex >= 0)
      this.options[this.selectedIndex] = null;
  }

  function DynamicList_modify()
  {
    if (this.selectedIndex >= 0)
    {
      var s = prompt("Change the name of this item",this.options[this.selectedIndex].text);
      if (s && s!="")
      {
        this.options[this.selectedIndex].text = s;
        this.options[this.selectedIndex].value = s;
      }
      else
        this.remove();
    }
  }

  function DynamicList_move(direction)
  {
    var o = this.options;
    var a = this.selectedIndex;
    var b = this.selectedIndex+direction;
    if (a>=0 && b>=0 && b<o.length)
    {
      var tv = o[a].value; var tt = o[a].text;
      o[a].value = o[b].value; o[a].text =  o[b].text;
      o[b].value = tv; o[b].text = tt;
      this.selectedIndex = b;
    };
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
<? 
    }
  }
  
  function display($hidden = false) // public inherited
  {
    global $wces_path, $server_media;

    if ($hidden)
    {
      $this->preserveattribute("keys");
      $this->preserveattribute("items");
      return;
    }
    
    $this->dumpscript();
    
    $dowidth = $this->width > 0 && count($this->items) <= 0;
?>  
<!-- begin FormWidget <?=$this->prefix?> -->
<table border="0" cellpadding="2" cellspacing="0">
  <tr>
    <td align=center>
      <select size="<?=$this->height?>" name="<?=$this->prefix?>_box">
<?
  if ($dowidth)
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
<? if (!$this->orderonly) { ?>
    <td>
      <input type="button" value="Add..." name="<?=$this->prefix?>_additem" style="width: 75px"><br>
      <input type="button" value="Modify..." name="<?=$this->prefix?>_modifyitem" style="width: 75px"><br>
      <input type="button" value="Remove" name="<?=$this->prefix?>_removeitem" style="width: 75px"><br>
    </td>
<? } ?>          
  </tr>
  <tr><td align=center>
    <a onmousedown="button_push('<?=$this->prefix?>_moveup');   <?=$this->prefix?>_controller.move(-1)" onmouseup="button_unpush('<?=$this->prefix?>_moveup')"   onmouseout="button_unpush('<?=$this->prefix?>_moveup')" href="javascript:void(<?=$this->prefix?>_controller.move(-1))" onclick="return false;"><img name="<?=$this->prefix?>_moveup"   src="<?=$server_media?>/dynamiclist/moveup.gif" border=0></a>
    <a onmousedown="button_push('<?=$this->prefix?>_movedown'); <?=$this->prefix?>_controller.move( 1)" onmouseup="button_unpush('<?=$this->prefix?>_movedown')" onmouseout="button_unpush('<?=$this->prefix?>_moveup')" href="javascript:void(<?=$this->prefix?>_controller.move(-1))" onclick="return false;"><img name="<?=$this->prefix?>_movedown" src="<?=$server_media?>/dynamiclist/movedown.gif" border=0></a>
    <script>
    <!--
      AttachPushed('<?=$this->prefix?>_moveup',  "moveup_pushed.gif");
      AttachPushed('<?=$this->prefix?>_movedown',"movedown_pushed.gif"); 
    //-->
    </script>
  </td><? if (!$this->orderonly) { ?><td>&nbsp;</td><? } ?></tr>  
</table>
<input type="hidden" name="<?=$this->prefix?>_items">
<input type="hidden" name="<?=$this->prefix?>_keys">
<script language="javascript">
<!--
    <?=$this->prefix?>_controller = DynamicList('<?=$this->form?>','<?=$this->prefix?>');
    <? if($dowidth) print("{$this->prefix}_controller.selectedIndex = 0;\n    {$this->prefix}_controller.remove();\n"); ?> 
// -->
</script>
<!-- end FormWidget <?=$this->prefix?> -->
<?
  }
};

?>
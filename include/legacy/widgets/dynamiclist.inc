<?

require_once("legacy/widgets/widgets.inc");
require_once("wbes/server.inc");

class legacy_DynamicList extends legacy_FormWidget
{
  var $width;
  var $height;
  var $items;
  var $orderonlyitems;
  
  function legacy_DynamicList($width,$height,$orderonlyitems,$prefix,$form,$formmethod) // public constructor inherited
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->width = $width;
    $this->height = $height;
    $this->orderonlyitems = $orderonlyitems;
    $this->items = is_array($orderonlyitems) ? array_keys($orderonlyitems) : array();    
  }
  
  function loadvalues($strongchecking = true) // public inherited
  {
    $d = $this->loadattribute("data");
    if($d)
    {
      $this->items = explode(is_array($this->orderonlyitems) ? "." : "\t", $d);
      if (!is_array($this->items)) $this->items = array($this->items);
      
      if (is_array($this->orderonlyitems) && $strongchecking)
      {
        $keys = array_keys($this->orderonlyitems);
        $this->items = array_unique(array_intersect($this->items,$keys));
        $this->items = array_merge($this->items, array_diff($keys,$this->items));
      }
    }
    else if (is_array($this->orderonlyitems))
      $this->items = array_keys($this->orderonlyitems);
    else
      $this->items = array();
  }
 
  function dumpscript() //public static inherited
  {
    global $DynamicList_dumped, $server_media;
    if (!$DynamicList_dumped)
    {
      $DynamicList_dumped = true;
      legacy_ImageButton::dumpscript();
?>
<!-- begin legacy_DynamicList legacy_FormWidget script -->
<script language="javascript">
<!--

  function legacy_DynamicList(formname,prefix)
  {
    var form = document.forms[formname];
    var listbox = form[prefix + "_items"];
    listbox.data = form[prefix + "_data"];
    listbox.add = DynamicList_add;
    listbox.remove = DynamicList_remove;
    listbox.modify = DynamicList_modify;
    listbox.move = DynamicList_move;
    listbox.save = DynamicList_save;
    
    if (!form.DynamicList_lists)
    {
      form.DynamicList_lists = new Array();
      form.DynamicList_onsubmit_old = form.onsubmit;
      form.onsubmit = DynamicList_onsubmit;
    }

    form.DynamicList_lists[form.DynamicList_lists.length] = listbox;
    
    if (form[prefix + "_additem"])
    {
      DynamicList_GenericButton(form[prefix + "_additem"]    ,listbox, "this.list.add()");
      DynamicList_GenericButton(form[prefix + "_modifyitem"] ,listbox, "this.list.modify()");
      DynamicList_GenericButton(form[prefix + "_removeitem"] ,listbox, "this.list.remove()");
      listbox.orderonly = false;
    }
    else
      listbox.orderonly = true;
      
    return listbox;
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
    var d = this.data;
    d.value = "";
    var first = true;
    for(var i = 0; i < o.length; ++i)
    {
      if (first) first = false; else d.value += this.orderonly ? "." : String.fromCharCode(9);
      d.value += this.orderonly ? o[i].value : o[i].text;
    }
    //alert(d.value);
  };

  function DynamicList_add()
  {
    var s = prompt("Enter the name of an item to add to the list","");
    var o = this.options;
    if (s && s!="")
      o[o.length] = new Option(s,o.length,true,true);
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
<!-- end legacy_DynamicList legacy_FormWidget script -->
<? 
    }
  }
  
  function display($hidden = false) // public inherited
  {
    global $wces_path, $server_media;

	if ($hidden)
	{
	  $this->preserveattribute("data");
	  return;
	}
    
    $this->dumpscript();
    
    $dowidth = ($this->width > 0) && (!is_array($this->items) || count($this->items) <= 0);
    
?>  
<!-- begin legacy_FormWidget <?=$this->prefix?> -->
<table border="0" cellpadding="2" cellspacing="0">
  <tr>
    <td align=center>
      <select size="<?=$this->height?>" name="<?=$this->prefix?>_items">
<?
  if ($dowidth)
  {
   $this->width = abs($this->width);
   print("<option>");
   //print(str_repeat('&nbsp;',$this->width)); // nicer, but does not work on netscape in *nixes
   print(str_repeat('-',$this->width));
   print("</option>\n");
  }

  if ($this->orderonlyitems)
  {
    foreach($this->items as $item)
      print('<option value="' . $item . '">' . htmlspecialchars($this->orderonlyitems[$item]) . "</option>\n");
  }
  else
  {
    foreach($this->items as $k => $value)
      print("<option value=$k>$value</option>\n");
  }
?>
      </select>
    </td>
<? if (!is_array($this->orderonlyitems)) { ?>
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
  </td><? if (!is_array($this->orderonlyitems)) { ?><td>&nbsp;</td><? } ?></tr>  
</table>
<input type="hidden" name="<?=$this->prefix?>_data">
<script language="javascript">
<!--
    <?=$this->prefix?>_controller = legacy_DynamicList('<?=$this->form?>','<?=$this->prefix?>');
    <? if($dowidth) print($this->prefix . "_controller.selectedIndex = 0;\n    " . $this->prefix . "_controller.remove();\n"); ?>
    //<?=$this->prefix?>_controller.save();
    //alert(document.forms["<?=$this->form?>"]["<?=$this->prefix?>_data"].value);
    //window.onload = new Function('alert("100: " + document.forms["<?=$this->form?>"]["<?=$this->prefix?>_data"].value);');
// -->
</script>
<!-- end legacy_FormWidget <?=$this->prefix?> -->
<?
  }
};

?>
<?

require_once("widgets/widgets.inc");
require_once("wbes/server.inc");

class DynamicList extends FormWidget
{
  var $width;
  var $height;
  var $items;
  var $orderonlyitems;
  
  function DynamicList($width,$height,$orderonlyitems,$prefix,$form,$formmethod) // public constructor inherited
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->width = $width;
    $this->height = $height;
    $this->orderonlyitems = $orderonlyitems;
    $this->items = is_array($orderonlyitems) ? array_keys($orderonlyitems) : array();    
  }
  
  function loadvalues($strongchecking = true) // public inherited
  {
    $items = explode(is_array($this->orderonlyitems) ? "." : "\x0D\x0A", $this->loadattribute("data"));

    if (!is_array($items))
    {
      if($items)
        $items = array($items);
      else
        $items = array();
    }

    if (is_array($this->orderonlyitems) && $strongchecking)
    {
      $order = $this->orderonlyitems;
      foreach($items as $key => $item)
      if (isset($order[$item]))
        unset($order[$item]);
      else
        unset($items[$key]);
      $this->items = array_merge(array_values($items),array_keys($order));
    }
    else
      $this->items = $items;
  }
 
  function dumpscript() //public static inherited
  {
    global $DynamicList_dumped;
    if (!$DynamicList_dumped)
    {
      $DynamicList_dumped = true;
      ImageButton::dumpscript();
?>
<!-- begin DynamicList FormWidget script -->
<script language="javascript">
<!--

  function DynamicList_add()
  {
    var s = prompt("Enter the name of an item to add to the list","");
    if (s && s!="")  
      this.options[this.options.length] = new Option(s,s,true,true);
    this.save();  
  }

  function DynamicList_remove()
  {
     if (this.selectedIndex >= 0) this.options[this.selectedIndex] = null;
     this.save();  
  }

  function DynamicList_modify()
  {
    var s = prompt("Change the name of this item",this.options[this.selectedIndex].text);
    if (s && s!="")
    {
      this.options[this.selectedIndex].text = s;
	  this.options[this.selectedIndex].value = s;
    }
    this.save();
  }

  function DynamicList_move(direction)
  {
    var o = this.options;
    var a = this.selectedIndex;
    var b = this.selectedIndex+direction;
    if (a>=0 && b>=0 && b<o.length)
    {      var tv = o[a].value; var tt = o[a].text;
      o[a].value = o[b].value; o[a].text =  o[b].text;
      o[b].value = tv; o[b].text = tt;
      this.selectedIndex = b;
    };
    this.save();    
  }

  function DynamicList_GenericButton(button,list,action)
  {
    if (button)
    {
      button.onclick = new Function(action);
      button.list = list;
    }  
  };

  function DynamicList_save()
  {
    var o = this.options;
    var d = this.data;
    d.value = "";
    var first = true;
    for(var i = 0; i < o.length; ++i)
    {
      if (first) first = false; else d.value += this.orderonly ? "." : String.fromCharCode(13) + String.fromCharCode(10);
      d.value += this.orderonly ? this.options[i].value : this.options[i].text;
    }  
  };

  function DynamicList(formname,prefix)
  {
    var form = document.forms[formname];
    var listbox = form[prefix + "_items"];
    listbox.data = form[prefix + "_data"];
    listbox.add = DynamicList_add;
    listbox.remove = DynamicList_remove;
    listbox.modify = DynamicList_modify;
    listbox.move = DynamicList_move;
    listbox.save = DynamicList_save;
    
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
// -->
</script>
<!-- end DynamicList FormWidget script -->
<? 
    }
  }
  
  function display($hidden = false) // public inherited
  {
    global $wces_path;

	if ($hidden)
	{
	  $this->preserveattribute("data");
	  return;
	}
    
    $this->dumpscript();
    
    $dowidth = ($this->width < 0) || !is_array($this->items) || (count($this->items) <=0);
    
?>  
<!-- begin FormWidget <?=$this->prefix?> -->
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
    foreach($this->items as $value)
      print("<option>$value</option>\n");
  }
?>
      </select>
    </td>
<? if (!is_array($this->orderonlyitems)) { ?>    
    <td><input type="button" value="Add..." name="<?=$this->prefix?>_additem" style="width=75"><br>
      <input type="button" value="Modify..." name="<?=$this->prefix?>_modifyitem" style="width=75"><br>
      <input type="button" value="Remove" name="<?=$this->prefix?>_removeitem" style="width=75"><br>
    </td>
<? } ?>          
  </tr>
  <tr><td align=center>
    <a onmousedown="button_push('<?=$this->prefix?>_moveup'); <?=$this->prefix?>_controller.move(-1)"   onmouseup="button_unpush('<?=$this->prefix?>_moveup')"   onmouseout="button_unpush('<?=$this->prefix?>_moveup')" href="javascript:void(<?=$this->prefix?>_controller.move(-1))" onclick="return false;"><img name="<?=$this->prefix?>_moveup"   src="<?=$wces_path?>media/dynamiclist/moveup.gif" border=0></a>
    <a onmousedown="button_push('<?=$this->prefix?>_movedown'); <?=$this->prefix?>_controller.move( 1)" onmouseup="button_unpush('<?=$this->prefix?>_movedown')" onmouseout="button_unpush('<?=$this->prefix?>_moveup')" href="javascript:void(<?=$this->prefix?>_controller.move(-1))" onclick="return false;"><img name="<?=$this->prefix?>_movedown" src="<?=$wces_path?>media/dynamiclist/movedown.gif" border=0></a>
    <script>
    <!--
      AttachPushed('<?=$this->prefix?>_moveup',"moveup_pushed.gif");
      AttachPushed('<?=$this->prefix?>_movedown',"movedown_pushed.gif"); 
    //-->
    </script>
  </td><? if (!is_array($this->orderonlyitems)) { ?><td>&nbsp;</td><? } ?></tr>  
</table>
<input type="hidden" name="<?=$this->prefix?>_data"value="">
<script language="javascript">
<!--
    <?=$this->prefix?>_controller = DynamicList('<?=$this->form?>','<?=$this->prefix?>');
    <? if($dowidth) print($this->prefix . "_controller.selectedIndex = 0;\n    " . $this->prefix . "_controller.remove();\n"); ?>
    <?=$this->prefix?>_controller.save();
// -->
</script>
<!-- end FormWidget <?=$this->prefix?> -->
<?
  }
};

?>
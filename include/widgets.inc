<%

require_once("server.inc");

define("WIDGET_POST",1);
define("WIDGET_GET",2);

class Widget // abstract
{
  var $prefix;
  var $form;
  var $formmethod;
    
  function Widget($prefix,$form,$formmethod) // public constructor
  {
    $this->prefix = $prefix;
    $this->form = $form;
    $this->formmethod = $formmethod;
  }
  
  function loadvalues() // public abstract
  {
    assert(0);
  }
  
  function display($hidden) // public abstract
  {
    assert(0);
  }
  
  function dumpscript() // public static abstract
  {
    assert(0);
  }
  
  function loadattribute($attrname) // protected
  {
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    if ($this->formmethod == WIDGET_GET)
      return $HTTP_GET_VARS[$this->prefix . "_" . $attrname];
    else if ($this->formmethod == WIDGET_POST)
      return $HTTP_POST_VARS[$this->prefix . "_" . $attrname]; 
    else
      assert(0);  
  }
  
  function printattribute($attrname,$value) // protect
  {
    $attrname = htmlspecialchars($this->prefix . "_" . $attrname);
    $i = htmlspecialchars($value);
    print("<input type=hidden name=\"$attrname\" value=\"$i\">\n");
  }
  
  function printarrayattribute($attrname,$value)
  {
    $attrname = htmlspecialchars($this->prefix . "_" . $attrname);
    foreach($value as $item)
      print("<input type=hidden name=\"$attrname\" value=\"$item\">\n");
  }
  
  function preserveattribute($attrname)
  {
    $this->printattribute($attrname,$this->loadattribute($attrname));
  }
}

class ImageButton extends Widget
{
  var $raisedsrc;
  var $pushedsrc;
  var $iparams;
  var $aparams;

  function ImageButton($href,$raisedsrc,$pushedsrc,$iparams,$aparams,$prefix,$form,$formmethod) //public constructor inherited
  {
    $this->Widget($prefix,$form,$formmethod);
    $this->raisedsrc = $raisedsrc;
    $this->pushedsrc = $pushedsrc;
    $this->iparams = $iparams;
    $this->aparams = $aparams;
  }

  function display($hidden) // public inherited
  {
    if ($hidden) return false;
    $this->dumpscript();
%>
<a onmousedown="button_push('<%=$this->prefix%>')" onmouseup="button_unpush('<%=$this->prefix%>')" onmouseout="button_unpush('<%=$this->prefix%>')" <%=$this->aparams%>"><img name="<%=$this->prefix%>" src="<%=$this->raisedsrc%>" border=0 <%=$this->iparams%>"></a><script>AttachPushed('<%=$this->prefix%>','<%=$this->pushedsrc%>')</script>
<%  
  }
  
  function dumpscript() // public static inherited
  {
    global $ImageButton_dumped;
    if (!$ImageButton_dumped)
    {
      $ImageButton_dumped = true;
%>
<script>
<!--  
  function AttachPushed(img, filename)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      i.flipimage = getImage(i,filename);
      i.pushed = false;
    };
  };
  
  function button_push(img)
  {
    if(happyrollover)
    {
      var i = document.images[img];
      if (!i.pushed)
      {  
        i.pushed = true;
        var temp = i.src;
        i.src = i.flipimage.src;
        i.flipimage.src = temp;
      }; 
    };  
  };
  
  function button_unpush(img)
  {
    if(happyrollover)
      window.setTimeout("button_realunpush('" + img + "')", 0);  // defeat ie's lazy refresh
  };
  
  function button_realunpush(img)
  {
    var i = document.images[img];
    if (i.pushed)
    {
      i.pushed = false;
      var temp = i.src;
      i.src = i.flipimage.src;
      i.flipimage.src = temp;
    };   
  };
// -->
</script>
<%
    }  
  }
}

class DynamicList extends Widget
{
  var $width;
  var $height;
  var $items;
  
  function DynamicList($width,$height,$items,$prefix,$form,$formmethod) // public constructor inherited
  {
    $this->Widget($prefix,$form,$formmethod);
    $this->width = $width;
    $this->height = $height;
    $this->items = $items;
  }
  
  function loadvalues() // public inherited
  {
    $str = $this->loadattribute("data");
    if ($str) 
    { 
      $this->items = explode("\n",$str);
      return 1;
    }
    else
      return 0;
  }
 
  function dumpscript() //public static inherited
  {
  
    global $DynamicList_dumped;
    if (!$DynamicList_dumped)
    {
      $DynamicList_dumped = true;
      ImageButton::dumpscript();
%>
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
    {
      var tv = o[a].value; var tt = o[a].text;
      o[a].value = o[b].value; o[a].text =  o[b].text;
      o[b].value = tv; o[b].text = tt;
      this.selectedIndex = b;
    };
    this.save();    
  }

  function DynamicList_GenericButton(button,list,action)
  {
    button.onclick = new Function(action);
    button.list = list;
  };

  function DynamicList_save()
  {
    var o = this.options;
    var d = this.data;
    d.value = "";
    for(var i = 0; i < o.length; ++i)
      d.value += this.options[i].text + "\n";
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
    DynamicList_GenericButton(form[prefix + "_additem"]    ,listbox, "this.list.add()");
    DynamicList_GenericButton(form[prefix + "_modifyitem"] ,listbox, "this.list.modify()");
    DynamicList_GenericButton(form[prefix + "_removeitem"] ,listbox, "this.list.remove()");
    return listbox;
  };
// -->
</script>
<% 
    }
  }
  
  function display($hidden) // public inherited
  {
    global $server_wcespath;
    
    if ($hidden) return false; // todo implement hidden mode
    
    $dowidth = ($this->width < 0) || !is_array($this->items) || (count($this->items) <=0);
    
%>  
<!-- begin widget <%=$this->prefix%> -->
<table border="0" cellpadding="2" cellspacing="0">
  <tr>
    <td><center><select size="<%=$this->height%>" name="<%=$this->prefix%>_items">
<%
  if ($dowidth)
  {
   $this->width = abs($this->width);
   print('<option>');
   //print(str_repeat('&nbsp;',$this->width)); // nicer, but does not work on netscape in *nixes
   print(str_repeat('-',$this->width));
   print('</option>');
  }
  if (is_array($this->items))
    foreach($this->items as $value)
      if ($value) print("<option>$value</option>");
%>
      </select></center>
    </td>
    <td><input type="button" value="Add..." name="<%=$this->prefix%>_additem" style="width=75"><br>
      <input type="button" value="Modify..." name="<%=$this->prefix%>_modifyitem" style="width=75"><br>
      <input type="button" value="Remove" name="<%=$this->prefix%>_removeitem" style="width=75"><br>
    </td>      
  </tr>
  <tr><td align=center>
    <a onmousedown="button_push('<%=$this->prefix%>_moveup'); <%=$this->prefix%>_controller.move(-1)"   onmouseup="button_unpush('<%=$this->prefix%>_moveup')"   onmouseout="button_unpush('<%=$this->prefix%>_moveup')" href="javascript:void(<%=$this->prefix%>_controller.move(-1))" onclick="return false;"><img name="<%=$this->prefix%>_moveup"   src="<%=$server_wcespath%>media/moveup.gif" border=0></a>
    <a onmousedown="button_push('<%=$this->prefix%>_movedown'); <%=$this->prefix%>_controller.move( 1)" onmouseup="button_unpush('<%=$this->prefix%>_movedown')" onmouseout="button_unpush('<%=$this->prefix%>_moveup')" href="javascript:void(<%=$this->prefix%>_controller.move(-1))" onclick="return false;"><img name="<%=$this->prefix%>_movedown" src="<%=$server_wcespath%>media/movedown.gif" border=0></a>
    <script>
    <!--
      AttachPushed('<%=$this->prefix%>_moveup',"moveup_pushed.gif");
      AttachPushed('<%=$this->prefix%>_movedown',"movedown_pushed.gif"); 
    //-->
    </script>
  </td><td>&nbsp;</td></tr>  
</table>
<input type="hidden" name="<%=$this->prefix%>_data"value="">
<script language="javascript">
<!--
    <%=$this->prefix%>_controller = DynamicList('<%=$this->form%>','<%=$this->prefix%>');
    <% if($dowidth) print($this->prefix . "_controller.selectedIndex = 0;\n    " . $this->prefix . "_controller.remove();\n"); %>
    <%=$this->prefix%>_controller.save();
// -->
</script>
<!-- end widget <%=$this->prefix%> -->
<%
  }
};

%>
<%

define("WIDGET_POST",1);
define("WIDGET_GET",2);

class Widget //abstract
{
  var $prefix;
  
  function Widget($prefix)
  {
    $this->prefix = $prefix;
  }
  
  function display()
  {
    assert(0);
  }
  
  function dumpscript()
  {
    assert(0);
  }
}

class FormWidget // abstract, extends Widget
{
  var $prefix;
  var $form;
  var $formmethod;
    
  function FormWidget($prefix,$form,$formmethod) // public constructor
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
  
  function dumpscript() // public static virtual
  {
  }
  
  function loadattribute($attrname = "") // protected
  {
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    $attrname = $attrname ? $this->prefix . "_$attrname" : $this->prefix;
    if ($this->formmethod == WIDGET_GET)
      return isset($HTTP_GET_VARS[$attrname]) ? $HTTP_GET_VARS[$attrname] : "";
    else if ($this->formmethod == WIDGET_POST)
      return isset($HTTP_POST_VARS[$attrname]) ? $HTTP_POST_VARS[$attrname] : "";
    else
      assert(0);
  }
  
  function printattribute($attrname,$value)
  {
    if ($attrname) $attrname = "_$attrname";
    $attrname = htmlspecialchars($this->prefix . $attrname);
    $i = htmlspecialchars($value);
    print("<input type=hidden name=\"$attrname\" value=\"$i\">\n");
  }
  
  function printarrayattribute($attrname,&$value)
  {
    if ($attrname) $attrname = "_$attrname";
    $attrname = htmlspecialchars($this->prefix . $attrname . "[]");
    foreach($value as $item)
      print("<input type=hidden name=\"$attrname\" value=\"$item\">\n");
  }
  
  function preserveattribute($attrname = "")
  {
    $this->printattribute($attrname,$this->loadattribute($attrname));
  }
  
  function preservearrayattribute($attrname)
  {
    if (is_array($a = $this->loadattribute($attrname))) $this->printarrayattribute($attrname, $a);
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

  function display($hidden = false) // public inherited
  {
    if ($hidden) return;
    $this->dumpscript();
    %><a onmousedown="button_push('<%=$this->prefix%>')" onmouseup="button_unpush('<%=$this->prefix%>')" onmouseout="button_unpush('<%=$this->prefix%>')" <%=$this->aparams%>"><img name="<%=$this->prefix%>" src="<%=$this->raisedsrc%>" border=0 <%=$this->iparams%>"></a><script>AttachPushed('<%=$this->prefix%>','<%=$this->pushedsrc%>')</script><%  
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

%>
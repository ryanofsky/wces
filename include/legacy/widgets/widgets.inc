<?

@define("WIDGET_POST",1);
@define("WIDGET_GET",2);

class legacy_Widget //abstract
{
  var $prefix;
  
  function legacy_Widget($prefix)
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

class legacy_FormWidget
{
  var $prefix;
  var $form;
  var $formmethod;
    
  function legacy_FormWidget($prefix,$form,$formmethod)
  {
    $this->prefix = $prefix;
    $this->form = $form;
    $this->formmethod = $formmethod;
  }
  
  function loadvalues()
  {
    assert(0);
  }
  
  function display($hidden)
  {
    assert(0);
  }
  
  function dumpscript()
  {
  }
  
  function loadattribute($attrname = "")
  {
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    $attrname = $attrname ? $this->prefix . "_$attrname" : $this->prefix;
    if ($this->formmethod == WIDGET_GET)
      return isset($HTTP_GET_VARS[$attrname]) ? $HTTP_GET_VARS[$attrname] : NULL;
    else if ($this->formmethod == WIDGET_POST)
      return isset($HTTP_POST_VARS[$attrname]) ? $HTTP_POST_VARS[$attrname] : NULL;
    else
      assert(0);
  }
  
  function printattribute($attrname,$value)
  {
    if (!isset($value)) return;
    if ($attrname) $attrname = "_$attrname";
    $attrname = htmlspecialchars($this->prefix . $attrname);
    $i = htmlspecialchars($value);
    print("<input type=hidden name=\"$attrname\" value=\"$i\">\n");
  }
  
  function printarrayattribute($attrname,&$value)
  {
    if (!isset($value)) return;
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

class legacy_ImageButton extends legacy_Widget
{
  var $raisedsrc;
  var $pushedsrc;
  var $iparams;
  var $aparams;

  function legacy_ImageButton($href,$raisedsrc,$pushedsrc,$iparams,$aparams,$prefix,$form,$formmethod) //public constructor inherited
  {
    $this->legacy_Widget($prefix,$form,$formmethod);
    $this->raisedsrc = $raisedsrc;
    $this->pushedsrc = $pushedsrc;
    $this->iparams = $iparams;
    $this->aparams = $aparams;
    $this->dumpscript();
  }

  function display($hidden = false)
  {
    if ($hidden) return;
    ?><a onmousedown="button_push('<?=$this->prefix?>')" onmouseup="button_unpush('<?=$this->prefix?>')" onmouseout="button_unpush('<?=$this->prefix?>')" <?=$this->aparams?>"><img name="<?=$this->prefix?>" src="<?=$this->raisedsrc?>" border=0 <?=$this->iparams?>"></a><script>AttachPushed('<?=$this->prefix?>','<?=$this->pushedsrc?>')</script><?  
  }
  
  function dumpscript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;

    $str = <<<EOS
<!-- Begin legacy_ImageButton Script -->
<script>
<!--

  var happyrollover = document.images && window.Image ? true : false;
  var happytree = happyrollover && RegExp && (document.all || document.getElementById) ? true : false;
  var detect = navigator.userAgent.toLowerCase();
  var refreshbug =  (detect.indexOf('compatible') < 0 && detect.charAt(8) == '5') ? true : false; // check for mozilla

  // fix for Netscape 3 string object
  if (happyrollover && !String.prototype.slice) String.prototype.slice = new Function("start","len","if(len<0 || len>this.length) len = this.length; var ret = new String(); for(var i = start; i < len; i++) ret += this.charAt(i); return ret");

  function getImage(oldimage,filename)
  {
    var p = oldimage.src.lastIndexOf("/");
    var q = oldimage.src.lastIndexOf("\\\\");
    var i = new Image();
    i.src = oldimage.src.slice(0,(p>q && p>0) ? p+1 : (q>0)?q+1 : 0) + filename;;
    return i;
  }; 

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
<!-- End legacy_ImageButton Script -->
EOS;
    html_head_append($str);
  }
}

?>

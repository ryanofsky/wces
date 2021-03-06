<?

require_once("legacy/widgets/widgets.inc");

class legacy_Textbox extends legacy_FormWidget
{
  var $rows,$cols,$params,$text;
  
  function legacy_Textbox($rows, $cols, $params, $prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->rows = $rows;
    $this->cols = $cols;
    $this->params = $params;
    $this->text = "";
  }
  
  function display($hidden = false)
  {
    $n = '"' . $this->prefix . '"';
    $t = htmlspecialchars($this->text);
    $s = $this->params ? " " . $this->params : "";
    $r = $this->rows;
    $c = $this->cols;
    if ($hidden)
      print("<input type=hidden name=$n value=\"$t\"$s>");
    else if ($this->rows == 0)
      print("<input type=text name=$n value=\"$t\" size=$c$s>");
    else
      print("<textarea name=$n rows=$r cols=$c$s>$t</textarea>");
  }
  
  function loadvalues()
  {
    $this->text = $this->loadattribute("");
  }
};

class legacy_CheckBox extends legacy_FormWidget
{
  var $checked;
  
  function legacy_CheckBox($prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->checked = false;
  }
  
  function display($hidden = false, $params = "")
  {
    $n = '"' . $this->prefix . '"';
    if ($params) $params = " $params";
    if($hidden)
      print("<input type=hidden name=$n value=" . ($this->checked ? 1 : 0) . ">");
    else
      print("<input type=checkbox name=$n id=$n value=1" . ($this->checked ? " checked" : "") . "$params>");
  }
  
  function displaylabel($label, $params = "")
  {
    $n = '"' . $this->prefix . '"';
    print("<label for=$n $params>$label</label>");
  }
  
  function loadvalues()
  {
    $this->checked = $this->loadattribute("") ? true : false;
  } 
}

class legacy_RadioButton extends legacy_FormWidget
{ 
  var $checked;
  var $exclusive;
  
  function legacy_RadioButton($exclusive, $prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->exclusive = $exclusive;
  }
  
  function display($index, $params="")
  {
    if (strlen($params)) $params = " $params";
    $n = $this->exclusive ? "\"{$this->prefix}\"" : "\"{$this->prefix}[]\"";
    
    if ($index === true)
    {
      if ($this->exclusive && isset($this->checked))
        print("<input type=hidden name=$n value=\"{$this->checked}\"$params>");
      else if (!$this->exclusive && is_array($this->checked))
        foreach($this->checked as $v)
          print("<input type=hidden name=$n value=$v$params>");    
    }      
    else
    {
      $i = '"' . $this->prefix . '_' . $index . '"';
      $v = '"' . $index . '"';
      if ($this->exclusive)
      {
        $t = "radio";
        $c = $this->checked == $index ? " checked" : "";
      }
      else
      {
        $t = "checkbox";
        $c = in_array($index, $this->checked) ? " checked" : "";
      }
      if ($params) $params = " $params";
      print("<input type=$t name=$n value=$v id=$i$params$c>");
    }  
  }

  function displaylabel($index, $text, $params = "")
  {
    $i = '"' . $this->prefix . '_' . $index . '"';
    print("<label for=$i $params>$text</label>");
  }
  
  function loadvalues()
  {
    $this->checked = $this->loadattribute();
  }
};

class legacy_DropBox extends legacy_FormWidget
{ 
  var $items;
  var $selected;
  
  function legacy_DropBox($items,$prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->items = $items;
    $this->selected = "";
  }
  
  function display($hidden = false, $params="")
  {
    $n = '"' . $this->prefix . '"';
    if ($params) $params = " $params";
    
    if ($hidden)
    {
      print("<input type=hidden name=$n value=\"" . htmlspecialchars($this->selected) . "\"$params>");
    }
    else
    {
      print("<select name=$n$params>\n");
      foreach($this->items as $k => $v)
      {
        if ($k == $this->selected)
          print("  <option value=$k selected>$v\n");
        else  
          print("  <option value=$k>$v\n");
      }
      print("</select>");
    }
  }

  function loadvalues()
  {
    $this->selected = $this->loadattribute();
  }
};

class legacy_ActionButton extends legacy_FormWidget
{
  var $action;
  var $object;
  
  function legacy_ActionButton($prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix, $form, $formmethod);
    $this->action = $this->object = "";
  }
  
  function loadvalues() // public abstract
  {
    $this->object = "";
    if($this->action = $this->loadattribute(""))
    {
      $p = strrpos($this->action,"-");
      if ($p !== false)
      {
        $this->object = substr($this->action,$p+1);
        $this->action = substr($this->action,0,$p);
      }
    }
    else
    {
      global $HTTP_POST_VARS, $HTTP_GET_VARS;
      if ($this->formmethod == WIDGET_GET) $vars = &$HTTP_GET_VARS; else if ($this->formmethod == WIDGET_POST) $vars = &$HTTP_POST_VARS;
      $pref = $this->prefix . '-';
      $l = strlen($pref);
      foreach($vars as $key => $value)
      if ($value && $pref == substr($key,0,$l))
      {
        $p = strrpos($key,'-');
        $this->action = substr($key,$l,$p-$l);
        $this->object = substr($key,$p+1);
        return;
      }
    }
  }

  function dumpscript()
  {
    static $dumped = false;
    if ($dumped) return; else $dumped = true;
    
    $str = <<<EOD
<!-- Begin legacy_ActionButton Script -->
<script>
<!--
  function ActionButton_go(action,object,actionobject)
  {
    //alert(actionobject.name + " set to '" + action + "'");
    actionobject.value = action + '-' + object;
    if (actionobject.form.onsubmit) actionobject.form.onsubmit();
    actionobject.form.submit();
  }

  function ActionButton_sgo(action,object,form,actionelement)
  {
    
    //alert(document.forms[form][actionelement]);
    ActionButton_go(action,object,document.forms[form][actionelement]);
  }
// -->
</script>
<!-- End legacy_ActionButton Script -->
EOD;
    html_head_append($str);
  }

  function display($label = false, $action = false, $object = false, $attribs = "")
  {
    if ($label === false) // trigger an action for the next page load that can be reset by javascript
    {
      if ($action === false)
      {
        $action = $this->action;
        $object = $this->object;
      }
      $value = $action ? "$action-$object" : "";
      print("<input type=hidden name=\"$this->prefix\" value=\"$value\">");
    }  
    else // display form button that can trigger an action
    {
      if ($attribs) $attribs = " $attribs";  
      print('<input type=submit name="' . $this->prefix . '-' . $action . '-' . $object . '" value="' . $label . '"' . $attribs . '>');
    }  
  }
  
  function gethref($action,$object)
  {
    return "javascript:ActionButton_sgo('$action','$object','$this->form','$this->prefix');";
  }
};

class legacy_Form extends legacy_FormWidget
{
  var $isstale;
  
  function legacy_Form($prefix,$form,$formmethod)
  {
    $this->legacy_FormWidget($prefix,$form,$formmethod);
    $this->isstale = false;
  }
  
  function loadvalues()
  {
    $this->isstale = $this->loadattribute("isstale") ? true : false;
  }
  
  function display()
  {
    $this->printattribute("isstale",true);
  }
};


?>

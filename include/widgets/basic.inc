<?

require_once("widgets/widgets.inc");

class Textbox extends FormWidget
{
  var $rows,$cols,$params,$text;
  
  function Textbox($rows, $cols, $params, $prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
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

class CheckBox extends FormWidget
{
  var $checked;
  
  function CheckBox($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->checked = false;
  }
  
  function display($hidden = false)
  {
    $n = '"' . $this->prefix . '"';
    if($hidden)
      print("<input type=hidden name=$n value=" . ($this->checked ? 1 : 0) . ">");
    else
      print("<input type=checkbox name=$n id=$n value=1" . ($this->checked ? " checked" : "") . ">");
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

class RadioButton extends FormWidget
{ 
  var $items;
  var $checked;
  
  function RadioButton($items,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->items = $items;
  }
  
  function display($index, $params="")
  {
    $n = '"' . $this->prefix . '"';
    if ($index === true)
    {
      $v = '"' . $this->checked . '"';
      print("<input type=hidden name=$n value=$v$params>");
    }      
    else
    {
      $i = '"' . $this->prefix . '_' . $index . '"';
      $v = '"' . $index . '"';
      if ($params) $params = " $params";
      print("<input type=radio name=$n value=$v id=$i$params" . ($this->checked == $index ? " checked" : "") . ">");
    }  
  }

  function displaylabel($index,$params = "")
  {
    $i = '"' . $this->prefix . '_' . $index . '"';
    print("<label for=$i $params>" . $this->items[$index] . "</label>");
  }
  
  function loadvalues()
  {
    $this->checked = $this->loadattribute();
  }
};

class ActionButton extends FormWidget
{
  var $action;
  var $object;
  
  function ActionButton($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix, $form, $formmethod);
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
        $this->object = substr($key,$p);
        return;
      }
    }
  }

  function dumpscript()
  {
     global $ActionButton_dumpscript;
     if (!isset($ActionButton_dumpscript))
     {
       $ActionButton_dumpscript = true;
?>
<!-- Begin ActionButton Script -->
<script>
<!--
  function ActionButton_go(action,object,actionobject)
  {
    //alert(actionobject.name + " set to '" + action + "'");
    actionobject.value = action + '-' + object;
    actionobject.form.submit();
  }

  function ActionButton_sgo(action,object,form,actionelement)
  {
    ActionButton_dyngo(action,object,document.forms[form][actionelement]);
  }
// -->
</script>
<!-- End ActionButton Script -->
<?
     }
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

class Form extends FormWidget
{
  var $isstale;
  
  function Form($prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
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

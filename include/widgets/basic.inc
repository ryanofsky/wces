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
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    
    if ($this->formmethod == WIDGET_GET)
      $thearray = &$HTTP_GET_VARS;
    else if ($this->formmethod == WIDGET_POST)
      $thearray = &$HTTP_POST_VARS;
    else
      assert(0);
    
    $l = strlen($this->prefix);
    foreach($thearray as $key => $value)
    {
      if ($value && $this->prefix == substr($key,0,$l))
      {
        $p = strrpos($key,'_');
        $this->action = substr($key,$l+1,$p-$l-1);
        $this->object = substr($key,$p+1);
        return;        
      }
    }
  }

  function display($label = false, $action = false, $object = false, $attribs = "")
  {
    if ($label === false)
    {
      $value = 0;
      if ($action === false)
      {
        $action = $this->action;
        $object = $this->object;
        if ($action) $value = 1;
      }
      print('<input type=hidden name="' . $this->prefix . '_' . $action . '_' . $object . '" value="' . $value . '">');  
    }  
    else
    {
      if ($attribs) $attribs = " $attribs";  
      print('<input type=submit name="' . $this->prefix . '_' . $action . '_' . $object . '" value="' . $label . '"' . $attribs . '>');
    }  
  }
  
  function gethref($action,$object)
  {
    return "javascript:document.forms['" . $this->form . "']['" .$this->prefix . "_" . $action . '_' . $object . "'].value = 1; void(document.forms['" . $this->form . "'].submit())";
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
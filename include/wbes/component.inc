<?

require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

class Component
{
  var $text = "";      // string, component text
  var $is_html = false;   // boolean html allowed
  var $id;
  var $type;
  var $is_shared;
  var $was_answered;

  var $componentid;
  var $replace;
  var $replace_before;
  var $replace_after;
 
  function Component()
  {
    $this->text = "";
    $this->is_html = false;
    $this->id = 0;
    $this->type = 0;
    $this->is_personal = false;
    $this->is_shared = false;
    $this->was_answered = false;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new ComponentEditor($this,$prefix,$form,$formmethod);
  }

  function getwidget($prefix,$form,$formmethod)
  {
    return new ComponentWidget($this,$prefix,$form,$formmethod);
  }
};

class ComponentEditor extends FormWidget
{
  var $text,$is_html,$loadprevious; // Widgets
  var $errors; // Mistakes in the form
  var $component; // source component
  
  function ComponentEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->text = new TextBox(4,40,"wrap=virtual",$this->prefix."_text",$this->form,$this->formmethod);
    $this->is_html = new CheckBox($this->prefix."_usehtml",$this->form,$this->formmethod);
    $this->component = &$component;
    
    $this->errors = array();
    
    $this->text->broken = false;
  }
  
  function loadvalues()
  {
    $this->loadprevious = $this->loadattribute("loadprevious");
    if ($this->loadprevious)
    {
      $this->text->loadvalues();
      $this->is_html->loadvalues();
    }
    else
    {
      $this->text->text = $this->component->text;
      $this->is_html->checked = $this->component->is_html ? true : false;
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    $this->component->text = $this->text->text;
    $this->component->is_html = $this->is_html->checked;
    return true;
  }

  function display()
  {
    $this->printattribute("loadprevious",1);
    if (count($this->errors) > 0)
    {
      print("<font color=red><ul>\n");
      foreach($this->errors as $value)
        print("  <li>$value</li>\n");
      print("</ul></font>\n");
    }
  }
  
  function fieldname($name,$isbroken)
  {
    if ($isbroken)
      print("<font color=red>$name</font>");
    else
      print($name);
  }
  
  function preview()
  {
    if (count($this->errors) > 0)
    {
      print("<p><strong>Errors:</strong></p>");
      ComponentEditor::display();
    }
    else
    {
      $w = $this->component->getwidget($this->prefix."_preview", $this->form, $this->formmethod);
      $w->display();
    }
  }
};

class ComponentWidget extends FormWidget
{
  var $component;
  
  function ComponentWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->component = &$component;
  }
  
  function display()
  {
    print("<div>");
    if ($this->component->is_html)
      print($this->component->text);
    else
      print(nl2br(htmlspecialchars($this->component->text)));
    print("</div>\n");
  }
};

?>

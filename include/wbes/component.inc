<?

require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

class Component
{
  var $text,$usehtml;  // STRING, question text, html allowed
  
  function Component()
  {
    $this->text = "";
    $this->usehtml = false;
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
  var $text,$usehtml,$loadprevious; // Widgets
  var $errors; // Mistakes in the form
  var $question; // source question
  
  function ComponentEditor(&$question,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->text = new TextBox(7,40,"wrap=virtual",$this->prefix."_text",$this->form,$this->formmethod);
    $this->usehtml = new CheckBox($this->prefix."_userhtml",$this->form,$this->formmethod);
    $this->question = &$question;
    
    $this->errors = array();
    
    $this->text->broken = false;
  }
  
  function loadvalues()
  {
    $this->loadprevious = $this->loadattribute("loadprevious");
    if ($this->loadprevious)
    {
      $this->text->loadvalues();
      $this->usehtml->loadvalues();
    }
    else
    {
      $this->text->text = $this->question->text;
      $this->usehtml->checked = $this->question->usehtml ? true : false;
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    $this->question->text = $this->text->text;
    $this->question->usehtml = $this->usehtml->checked;
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
};

class ComponentWidget extends FormWidget
{
  var $question;
  
  function ComponentWidget(&$question,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->question = &$question;
  }
  
  function display()
  {
    print("<div>");
    if ($this->question->usehtml)
      print($this->question->text);
    else
      print(nl2br(htmlspecialchars($this->question->text)));
    print("</div>\n");
  }
};

class Survey
{
  var $questions;
  
  function Survey()
  {
    $this->questions = array();
  }
  
  function insert($question,$position = 0)
  {
    $position = ((int)$position < 0) ? count($this->questions) + (int)$position + 1 : (int)$position;
    array_splice($this->questions,(int)$position,0,array($question));
    return $position;    
  }
  
  function delete($questionno)
  {
    array_splice($this->questions,$questionno,1);
  }
  
  function movedown($questionno)
  {
    if ($questionno >= count($this->questions) - 1) return;
    $q = $this->questions[$questionno];
    $this->delete($questionno);
    $this->insert($q,$questionno + 1);
  }
  
  function moveup($questionno)
  {
    if ($questionno < 1) return;
    $q = $this->questions[$questionno];
    $this->delete($questionno);
    $this->insert($q,$questionno - 1);
  }
}

?>

<?

require_once("wbes/component.inc");

class Picture extends Component
{
  var $location = "";
  var $originalname = "";
  var $format = 0;
  var $width = 0;
  var $height = 0;

  function Picture()
  {
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new PictureEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new PictureWidget($this,$prefix,$form,$formmethod);
  }
}

class PictureEditor extends ComponentEditor
{
  function PictureEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
    ComponentEditor::loadvalues();
    if ($this->loadprevious)
    {
    }  
    else
    {
    }
  }
  
  function checkvalues()
  {
    return count($this->errors) == 0;
  }
  
  function savevalues()
  {
    ComponentEditor::savevalues();
  }
  
  function display()
  {
    ComponentEditor::display();
  }  
}

class PictureWidget extends ComponentWidget
{
  function PictureWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
    $this->choices = array();
  }
  
  function loadvalues()
  {
  }
  
  function display()
  {
    ComponentWidget::display();
  }  
}

?>
<?

require_once("wbes/component.inc");

$Heading_code = 3;                
$Heading_name = "Heading Component";

function Heading_Factory(&$components, $ids, $ordinals, $revision)
{
  Component_Factory($components, $ids, $ordinals, $revision);
}

class Heading extends Component
{
  function Heading()
  {
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new HeadingEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new HeadingWidget($this,$prefix,$form,$formmethod);
  }
}

class HeadingEditor extends ComponentEditor
{
  function HeadingEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
    ComponentEditor::loadvalues();
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    ComponentEditor::savevalues();
  }
  
  function display()
  {
    ComponentEditor::display();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Text:",$this->text->broken); ?></strong></td>
  <td><? $this->text->display(false); ?><br><? $this->is_html->display(false); $this->is_html->displaylabel("Contains HTML Tags"); ?></td>
</tr>
</table>
<?
  }  
}

class HeadingWidget extends ComponentWidget
{
  function HeadingWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
    $this->choices = array();
  }
  
  function loadvalues()
  {
  }
  
  function display()
  {
    $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
    print("<h4>$text</h4>");
  } 
}

?>
<?

require_once("wbes/component.inc");

class Text extends Component
{
  function Text()
  {
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new TextEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new TextWidget($this,$prefix,$form,$formmethod);
  }
}

class TextEditor extends ComponentEditor
{
  function TextEditor(&$component,$prefix,$form,$formmethod)
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
};

class TextWidget extends ComponentWidget
{
  function TextWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
  }
  
  function display($hidden = false, $showerrors = true)
  {
    if(!$hidden)
    {
      $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
      print($text);
    }
  }  
}

?>
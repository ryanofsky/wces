<?

require_once("wbes/component.inc");

class HeadingFactory extends ComponentFactory
{
  function code() { return 4; }
  function name() { return "Heading Component"; }
  
  function create()
  {
    return new Heading();
  }
  
  function load(&$components, $ids, $ordinals, $topic_id)
  {
    return $this->textload($components, $ids, $ordinals, $topic_id, "Heading");
  }  
};

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
  
  function save($topic_id, $save_id)
  {
    return $this->textsave($topic_id, $save_id, HeadingFactory::code());
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
  <td><? $this->text->display(false); ?><br><? $this->is_html->display(false); $this->is_html->displaylabel("Contains HTML Tags"); ?><br>
  <? $this->locking->display(false); $this->locking->displaylabel("Lock this component"); ?></td>
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
  
  function display($hidden = false)
  {
    if (!$hidden)
    {
      $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
      print("<h4>$text</h4>");
    }
  } 
}

?>
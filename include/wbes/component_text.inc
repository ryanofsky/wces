<?

require_once("wbes/component.inc");

class TextFactory extends ComponentFactory
{
  function code() { return 5; }
  function name() { return "Text Component"; }

  function & create()
  {
    return new Text;
  }

  function loadComponents(&$components)
  {
    ComponentFactory::loadTextComponents($components, 'Text');
  }
};

class Text extends Component
{
  function & geteditor($name, &$parent)
  {
    return new TextEditor($this, $name, $parent);
  }

  function & getwidget($name, &$parent)
  {
    return new TextWidget($this, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    return $this->saveTextComponent($component_id, TextFactory::code());
  }
}

class TextEditor extends ComponentEditor
{
  function TextEditor(&$component, $name, &$parent)
  {
    $this->ComponentEditor($component, $name, $parent);
  }

  function printVisible()
  {
    $this->printErrors();
?>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Text:",$this->text->broken); ?></strong></td>
  <td>
    <? $this->text->display(); ?><br>
    <? $this->is_html->display(); $this->is_html->displayLabel("Contains HTML Tags"); ?><br>
    <? $this->locking->display(); $this->locking->displayLabel("Lock this component"); ?>
  </td>
</tr>
</table>
<?
  }
};

class TextWidget extends ComponentWidget
{
  function TextWidget(&$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
  }

  function printVisible()
  {
    $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
    print($text);
  }
}

?>
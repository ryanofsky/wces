<?

require_once("wbes/component.inc");

class HeadingFactory extends ComponentFactory
{
  function code() { return 4; }
  function name() { return "Heading Component"; }

  function & create()
  {
    return new Heading;
  }

  function loadComponents(&$components)
  {
    ComponentFactory::loadTextComponents($components, 4);
  }
};

class Heading extends Component
{
  function Heading()
  {}

  function & getEditor($name, &$parent)
  {
    return new HeadingEditor($this, $name, $parent);
  }

  function & getWidget($name, &$parent)
  {
    return new HeadingWidget($this, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    return $this->saveTextComponent($component_id, HeadingFactory::code());
  }

  function save($specialization_id, $save_id)
  {
    return $this->textsave($specialization_id, $save_id, HeadingFactory::code());
  }
}

class HeadingEditor extends ComponentEditor
{
  function HeadingEditor(&$component, $name, &$parent)
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
  <td><? $this->text->display(); ?><br><? $this->is_html->display(); $this->is_html->displayLabel("Contains HTML Tags"); ?><br>
  <? $this->locking->display(); $this->locking->displayLabel("Lock this component"); ?></td>
</tr>
</table>
<?
  }
}

class HeadingWidget extends ComponentWidget
{
  function HeadingWidget(&$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
  }

  function printVisible()
  {
    $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
    print("<h4>$text</h4>");
  }
}

?>
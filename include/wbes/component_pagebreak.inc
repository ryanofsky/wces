<?

require_once("wbes/component.inc");

class PageBreakFactory extends ComponentFactory
{b
  function code() { return 9; }
  function name() { return "Page Break Component"; }

  function & create()
  {
    return new PageBreak;
  }

  function loadComponents(&$components)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, renumber FROM pagebreak_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c &= new PageBreak;
      $c->renumber = $wr->row['renumber'] == 't';
      $components[$wr->row['component_id']] =& $c;
    }
  }
};

class PageBreak extends Component
{
  var $renumber = false;

  function & getEditor($name, &$parent)
  {
    return new PageBreakEditor($this, $name, $parent);
  }

  function & getWidget($name, &$parent)
  {
    return new PageBreakWidget($this, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    $renumber = $this->renumber ? "'t'" : "'f'";
    return "SELECT text_component_save($component_id, $renumber)";
  }

  function save($specialization_id, $save_id)
  {

    $revision_id = $this->revision_id ? $this->revision_id : 'NULL';
    $result = pg_go("SELECT pagebreak_component_save($specialization_id, $revision_id, $save_id, $renumber)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
};

class PageBreakEditor extends ComponentEditor
{
  // widgets
  var $renumber;

  function PageBreakEditor(&$component, $name, &$parent)
  {
    $this->ComponentEditor($component, $name, $parent);
    $this->renumber = new CheckBox("renumber", $this);
    $this->renumber->checked = $component->renumber;
  }

  function loadInitialState()
  {
    ComponentEditor::loadInitialState();
    $this->renumber->checked = $this->component->renumber;
  }

  function save()
  {
    ComponentEditor::save();
    $this->component->renumber = $this->renumber->checked;
  }

  function printVisible()
  {
    $this->printErrors();
?>
<p><strong>Page Break Component</strong></p>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Options:", false); ?></strong></td>
  <td><? $this->renumber->display(); $this->renumber->displayLabel("Restart Survey Numbering"); ?></td>
</tr>
</table>
<?
  }
};

class PageBreakWidget extends ComponentWidget
{
  function PageBreakWidget(&$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
  }

  function printVisible()
  {
    if ($this->preview) print("[ Page Break ]");
  }
};

?>
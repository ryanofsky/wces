<?

require_once("wbes/component.inc");

class PageBreakFactory extends ComponentFactory
{
  function code() { return 9; }
  function name() { return "Page Break Component"; }
  
  function & create()
  {
    return new PageBreak;
  }
  
  function load(&$components, $ids, $ordinals)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_go("SELECT revision_id, renumber FROM pagebreak_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new PageBreak();
      $c->revision_id = (int)$wr->row['revision_id'];
      $c->renumber    = $wr->row['renumber'] == 't';
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
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
  
  function save($topic_id, $save_id)
  {
    global $wbes;
    $renumber = $this->renumber ? "'t'" : "'f'";    
    $revision_id = $this->revision_id ? $this->revision_id : 'NULL';    
    $result = pg_go("SELECT pagebreak_component_save($topic_id, $revision_id, $save_id, $renumber)", $wbes, __FILE__, __LINE__);
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
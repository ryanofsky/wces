<?

require_once("wbes/component.inc");

class PageBreakFactory extends ComponentFactory
{
  function code() { return 9; }
  function name() { return "Page Break Component"; }
  
  function create()
  {
    return new PageBreak();
  }
  
  function load(&$components, $ids, $ordinals)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, renumber FROM pagebreak_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

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
  
  function PageBreak()
  {
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new PageBreakEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new PageBreakWidget($this,$prefix,$form,$formmethod);
  }
  
  function save($topic_id, $save_id)
  {
    global $wbes;
    $renumber 	= $this->renumber ? "'t'" : "'f'";    
    $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';    
    $result = pg_query("SELECT pagebreak_component_save($topic_id, $revision_id, $save_id, $renumber)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
  
}

class PageBreakEditor extends ComponentEditor
{
  var $renumber;
  
  function PageBreakEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    $this->renumber = new CheckBox("{$this->prefix}_renumber", $this->form, $this->formmethod);
    $this->renumber->checked = $component->renumber;
  }
  
  function loadvalues()
  {
    ComponentEditor::loadvalues();
    if ($this->loadprevious)
    {
      $this->renumber->loadvalues();
    }
    else
    {
    	$this->renumber->checked = $this->component->renumber;
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    ComponentEditor::savevalues();
    $this->component->renumber = $this->renumber->checked;
  }
  
  function display()
  {
    ComponentEditor::display();
?>

<p><strong>Page Break Component</strong></p>
<table>
<tr>
  <td valign=top><strong><? $this->fieldname("Options:", false); ?></strong></td>
  <td><? $this->renumber->display(false); ?><? $this->renumber->displaylabel("Restart Survey Numbering"); ?></td>
</tr>
</table>
<?    
  }
};

class PageBreakWidget extends ComponentWidget
{
  function PageBreakWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
  }

  function displaypreview()
  {
    print("[ Page Break ]");
  }

  function display($hidden = false, $showerrors = true)
  {
  }  
}

?>
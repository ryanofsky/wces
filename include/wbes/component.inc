<?

require_once("wbes/survey.inc");
require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

class ComponentFactory
{
  // in a normal language, code and name would be static constants, but
  // since php currently only supports static functions and not static
  // variables or constants, they are declared as functions instead
  function code() { return 0; }
  function name() { return 0; }
  
  function create()
  {
  }
  
  // generic load function for component types that do not define their own 
  function load(&$components, $ids, $ordinals)
  {
    global $wbes;
    $sids = implode($ids, ", ");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, data FROM generic_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);  
    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $ord = $ordinals[$rids[$wr->row['revision_id']]];
      $components[$ord] = unserialize($wr->row['data']);
      $components[$ord]->revision_id = (int)$wr->row['revision_id'];
    }
  }
  
  function textload(&$components, $ids, $ordinals, $cn)
  {
    global $wbes;
    $sids = implode($ids, ", ");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, ctext, is_html FROM text_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);  
    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $ord = $ordinals[$rids[$wr->row['revision_id']]];
      $components[$ord] = new $cn(); $c = &$components[$ord];
      $c->revision_id = (int)$wr->row['revision_id'];
      $c->is_html     = $wr->row['is_html'] == 't';
      $c->text       = $wr->row['ctext'];
    }
  }
};

class Component
{
  var $text = "";      // string, component text
  var $is_html = false;   // boolean html allowed
  var $revision_id = 0;

  function Component()
  {
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new ComponentEditor($this,$prefix,$form,$formmethod);
  }

  function getwidget($prefix,$form,$formmethod)
  {
    return new ComponentWidget($this,$prefix,$form,$formmethod);
  }
  
  // save the base fields, $text and $is_html
  function textsave($save_id, $cn)
  {
    global $wbes;

    $t = addslashes($this->text);
    $h = $this->is_html ? 't' : 'f';
    $r = $this->revision_id ? $this->revision_id : 'NULL';

    $result = pg_query("SELECT text_component_save($save_id, $cn, $r, '$t', '$h')", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
  
  // generic save function for component types that do not define their own 
  function save($save_id)
  {
    global $wbes;

    $cn = get_class($this) . "factory"; 
    // $cn = $cn::code();                doesn't work
    // $cn .= "::code"; $cn = $cn();     doesn't work either
    $cn = eval("return $cn::code();");
   
    $branch_id = 0;
    $revision_id = $this->revision_id;
    unset($this->revision_id);
    $s = addslashes(serialize($this));
    $this->revision_id = $revision_id;
    
    $changed = true;
    if ($revision_id)
    {
      $r = pg_query("SELECT data, branch_id FROM generic_components WHERE revision_id = $revision_id", $wbes, __FILE__, __LINE__);
      if (pg_numrows($r) == 1)
      {
        $changed = $s == pg_result($r,0,0);
        $branch_id = (int)pg_result($r,0,1);
      }
    }
    
    if (!$changed) return array($branch_id, $revision_id);
    
    if (!$branch_id)
    {
      $result = pg_query("
        INSERT INTO branches(branch, parent) VALUES(branch_nextval(NULL), NULL);
        SELECT currval('branch_ids');
      ", $wbes, __FILE__, __LINE__);
      
      $branch_id = pg_result($result,0,0);
    }
      
    $r = $revision_id ? $revision_id : "NULL";
    
    $result = pg_query("SELECT revision_save_start($branch_id, $r, $cn, $save_id)", $wbes, __FILE__, __LINE__);
    $revision_id = pg_result($result, 0, 0);
    
    pg_query("UPDATE generic_components SET data = '$s' WHERE revision_id = $revision_id", $wbes, __FILE__, __LINE__);
    
    $result = pg_query("SELECT revision_save_end($branch_id, $revision_id)", $wbes, __FILE__, __LINE__);
    $revision_id = pg_result($result,0,0);
    
    return $branch_id;
  }
};

class ComponentEditor extends FormWidget
{
  var $text,$is_html,$loadprevious; // Widgets
  var $errors; // Mistakes in the form
  var $component; // source component
  
  function ComponentEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->text = new TextBox(4,40,"wrap=virtual",$this->prefix."_text",$this->form,$this->formmethod);
    $this->is_html = new CheckBox($this->prefix."_usehtml",$this->form,$this->formmethod);
    $this->component = &$component;
    
    $this->errors = array();
    
    $this->text->broken = false;
  }
  
  function loadvalues()
  {
    $this->loadprevious = $this->loadattribute("loadprevious");
    if ($this->loadprevious)
    {
      $this->text->loadvalues();
      $this->is_html->loadvalues();
    }
    else
    {
      $this->text->text = $this->component->text;
      $this->is_html->checked = $this->component->is_html ? true : false;
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    $this->component->text = $this->text->text;
    $this->component->is_html = $this->is_html->checked;
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
  
  function preview()
  {
    if (count($this->errors) > 0)
    {
      print("<p><strong>Errors:</strong></p>");
      ComponentEditor::display();
    }
    else
    {
      $w = $this->component->getwidget($this->prefix."_preview", $this->form, $this->formmethod);
      $w->display();
    }
  }
};

class ComponentWidget extends FormWidget
{
  var $component;
  var $errors = array();
  
  function ComponentWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->FormWidget($prefix,$form,$formmethod);
    $this->component = &$component;
  }
  
  function validate()
  {
    return true;
  }
  
  function display()
  {
    print("<div>");
    if ($this->component->is_html)
      print($this->component->text);
    else
      print(nl2br(htmlspecialchars($this->component->text)));
    print("</div>\n");
  }
};

?>

<?

require_once("wbes/survey.inc");
require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

// keep global options that apply to all classes that inherit from Component
// on the high byte so the other three bytes can be used by individual classes

define("FLAG_LOCKED", 0x01000000);
define("FLAG_ISHTML", 0x02000000);

class ComponentFactory
{
  // in a normal language, code and name would be static constants, but
  // since php currently only supports static functions and not static
  // variables or constants, they are declared as functions instead
  function code() { return 0; }
  function name() { return 0; }
  
  function & create()
  {
  }
  
  // generic load function for component types that do not define their own 
  function load(&$components, $ids, $ordinals, $topic_id)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_go("SELECT revision_id, data FROM generic_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);  
    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = unserialize($wr->row['data']);
      $c->revision_id = (int)$wr->row['revision_id'];      
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
  
  function textLoad(&$components, $ids, $ordinals, $topic_id, $cn)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_go("SELECT revision_id, ctext, flags FROM text_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);  
    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = & new $cn;
      $c->revision_id = (int)$wr->row['revision_id'];
      $c->text        = $wr->row['ctext'];
      $flagGroup      = $wr->row['flags'];
      $c->locked      = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html     = (bool)($flagGroup & FLAG_ISHTML);
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
};

class Component
{
  var $text = "";      // string, component text
  var $is_html = false;   // boolean html allowed
  var $revision_id = 0;

  var $locked = false;

  function Component()
  {
  }
  
  function & getEditor($name, &$parent)
  {
    return new ComponentEditor($this,$name,$parent);
  }

  function & getWidget($name, &$parent)
  {
    return new ComponentWidget($this,$name,$parent);
  }
  
  // save the base fields, $text and $is_html
  function textSave($topic_id, $save_id, $cn)
  {
    global $wbes;

    $t = addslashes($this->text);
    $r = $this->revision_id ? $this->revision_id : 'NULL';

    $flagGroup = 0;
    if ($this->is_html) $flagGroup |= FLAG_ISHTML;
    if ($this->locked)  $flagGroup |= FLAG_LOCKED;

    $result = pg_go("SELECT text_component_save($topic_id, $r, $save_id, $cn, '$t', $flagGroup)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
  
  // generic save function for component types that do not define their own 
  // todo: this function is very out of date, needs to be updated
  function save($topic_id, $save_id)
  {
    die("Component::save() method is completely out of date. either rewrite it is a generic save method or write a non generic save for your component. (" . __FILE__ . ":" . __LINE__ . ")");
    
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
      $r = pg_go("SELECT data, branch_id FROM generic_components WHERE revision_id = $revision_id", $wbes, __FILE__, __LINE__);
      if (pg_numrows($r) == 1)
      {
        $changed = $s == pg_result($r,0,0);
        $branch_id = (int)pg_result($r,0,1);
      }
    }
    
    if (!$changed) return array($branch_id, $revision_id);
    
    if (!$branch_id)
    {
      $result = pg_go("
        INSERT INTO branches(branch, parent) VALUES(branch_nextval(NULL), NULL);
        SELECT currval('branch_ids');
      ", $wbes, __FILE__, __LINE__);
      
      $branch_id = pg_result($result,0,0);
    }
      
    $r = $revision_id ? $revision_id : "NULL";
    
    $result = pg_go("SELECT revision_save_start($branch_id, $r, $cn, $save_id)", $wbes, __FILE__, __LINE__);
    $revision_id = pg_result($result, 0, 0);
    
    pg_go("UPDATE generic_components SET data = '$s' WHERE revision_id = $revision_id", $wbes, __FILE__, __LINE__);
    
    $result = pg_go("SELECT revision_save_end($branch_id, $revision_id)", $wbes, __FILE__, __LINE__);
    $revision_id = pg_result($result,0,0);
    
    return $branch_id;
  }
};

class ComponentEditor extends ParentWidget
{
  // parameters
  var $component; // source component

  // widgets
  var $text, $is_html, $locking;
  
  // private
  var $errors = array();
  
  function ComponentEditor(&$component, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->component = &$component;
    $this->is_html = & new CheckBox("usehtml", $this);
    $this->locking = & new CheckBox("locked", $this);  
    $this->text = & new TextBox(4, 40, "wrap=virtual", "text", $this);    
    $this->text->broken = false;
  }
  
  function loadInitialState()
  {
    ParentWidget::loadInitialState();
    $this->text->text = $this->component->text;
    $this->is_html->checked = (bool)$this->component->is_html;
    $this->locking->checked = $this->component->locked;      
  }
  
  function validate()
  {
    return true;
  }
  
  function save()
  {
    $this->component->text = $this->text->text;
    $this->component->is_html = $this->is_html->checked;
    $this->component->locked = $this->locking->checked;
    return true;
  }

  function printErrors()
  {
    if (count($this->errors) > 0)
    {
      print("<font color=red><ul>\n");
      foreach($this->errors as $value)
        print("  <li>$value</li>\n");
      print("</ul></font>\n");
    }
  }
  
  function fieldName($name, $isBroken)
  {
    if ($isBroken)
      print("<font color=red>$name</font>");
    else
      print($name);
  }
};

class ComponentWidget extends ParentWidget
{
  // parameters
  var $component;
  var $showErrors = false;
  var $preview = false;

  // private
  var $errors = array();
  
  function ComponentWidget(&$component, $name, &$parent)
  {
    $this->ParentWidget($name, $parent);
    $this->component = &$component;
  }

  function save()
  {
    return true;
  }
  
  function validate()
  {
    return true;
  }
  
  function printVisible()
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

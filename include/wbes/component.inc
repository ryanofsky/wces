<?

require_once("wbes/survey.inc");
require_once("widgets/widgets.inc");
require_once("widgets/dynamiclist.inc");

// keep global options that apply to all classes that inherit from Component
// on the high byte so the other three bytes can be used by individual classes

define("FLAG_LOCKED", 0x01000000);
define("FLAG_ISHTML", 0x02000000);

// When copying a component from one survey to another
// the later changes to the component in the source survey
// can be made to affect the destination survey and vice versa.

// Changes made in one survey do not affect the other
define("IMPORT_INDEPENDENT", 1);

// Changes made to the source component will be mirrored to the
// destination, but not the other way around
define("IMPORT_LINKED_TO", 2);

// Changes made to the destination component will affect the
// source, but not the other way around. This will only work
// when destination specialization is on top of sources.
define("IMPORT_LINKED_FROM", 3);

// Changes made to either component will affect both. They
// will always be identical.
define("IMPORT_MIRRORED", 4);

class ComponentFactory
{
  // functions which need to be implement by child classes:
  //
  // in a normal language, code and name would be static constants, but
  // since php currently only supports static functions and not static
  // variables or constants, they are declared as functions instead
  //
  // function code();
  // function name();
  // function & create();
  // function loadComponents(&$components);

  function load(&$loads, $surveyGlobals)
  {
    $components = array();
    $loadKeys = array();
    foreach (array_keys($loads) as $key)
    {
      $load =& $loads[$key];
      if (isset($load->component)) continue;
      $components[$load->component_id] = false;
      $loadKeys[] = $key;
    }

    $this->loadComponents($components);

    foreach ($loadKeys as $key)
    {
      $load =& $loads[$key];
      $load->component =& $components[$loads->component_id];
    }

    $loadChildren = isset($loads[0]->component->getChildren());

    if ($loadChildren)
    {
      $save_id = $surveyGlobals->save_id ? $surveyGlobals->save_id : 'NULL';
      $table = "load" . md5(rand());
      $sql = '';
      foreach ($loadKeys as $key)
      {
        $load =& $loads[$key];

        if (strlen($sql) == 0)
        {
          // temporary table is used instead of a direct join with the components
          // table because postgres's concurrency model would prevent components
          // created during the execution of branch_latest() from showing up in
          // the resultset.
          $sql .= "CREATE TEMPORARY TABLE $table AS"
          $sql .= "SELECT $key AS load, ordinal, branch_latest(li.item_id, $load->specialization_id, $save_id) AS revision_id FROM list_items WHERE component = $load->component_id";
        }
        else
          $sql .= "\nUNION SELECT $key, ordinal, branch_latest(item_id, $load->specialization_id, $save_id) FROM list_items WHERE component = $load->component_id";
        ++$n;
      }

      $sql .= ";
        SELECT t.load, t.ordinal, t.revision_id, c.component_id, c.type
        FROM $table AS t
        INNER JOIN revision AS r USING (revision_id)
        INNER JOIN components AS c USING (component_id)
        ORDER t.load, t.ordinal;
      ";

      $r = pg_query($sql, $wbes, __FILE__, __LINE__);
      $result =& new pg_segment_wrapper($r, 'load');

      unset($load);
      while ($result->row)
      {
        if ($result->split)
        {
          $load =& $loads[$row['load']];
          $children &= $load->component->getChildren();
          $i = 0;
        }

        $newLoad =& $surveyGlobals->getLoad($result->row['type'], $result->row['component_id'], $load->specialization_id)
        $newLoad->addTarget($children, $i, $result->row['revision_id']);
        ++$i;

        $result->advance();

        if ($result->split)
          $children = array_pad($children, $i, false);
      }
    }
  }

  // default load function just stores unserializes component
  function loadComponents(&$components)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, data FROM generic_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
      $components[$wr->row['component_id']] = unserialize($wr->row['data']);
  }

  //
  function loadTextComponents(&$components, $ids, $ordinals, $specialization_id, $cn)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, ctext, flags FROM text_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);
    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = & new $cn;

      $c->text        = $wr->row['ctext'];
      $flagGroup      = $wr->row['flags'];
      $c->locked      = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html     = (bool)($flagGroup & FLAG_ISHTML);
      $components[$wr->row['component_id']] =& $c;
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

  function & getChildren()
  {
    return null;
  }

  function saveChildren($specialization_id, $save_id)
  {
    $list = "array_integer_cast('{'";
    foreach (array_keys($this->components) as $key)
    {
      $c =& $this->components[$key];
      $list .= "||";
      $list .= $c->save($specialization_id, $save_id);
      $list .= "::text";
    }
    $list .= "||'}')";
    return $list;
    $iid = $this->item_id ? $this->item_id : 'NULL';
    $rid = $this->revision_id ? $this->revision_id : 'NULL';
    $sql = "save(survey_save(revision_component($this->revision_id), $list), $specialization_id, $save_id, $rid, $iid)";
    return $sqlrevision;
  }

  function saveComponent($component_id, $children)
  {
    $code = isset($this->code) ? (int)$this->code : 0;
    $ser = addslashes(serialize($this));
    return "generic_save($component_id, $code, '$ser')";
  }

  function save($specialization_id, $save_id)
  {
    $cid = $this->revision_id ? "revision_component($this->revision_id)" : 'NULL';
    $component = $this->saveComponent($cid, $this->saveChildren);
    return "revision_save($component, $specialization_id, $save_id, $rid, $iid)";
  }

  function saveTextComponent($component_id, $code)
  {
    $text = addslashes($this->text);
    $flags = 0;
    if ($this->is_html) $flags |= FLAG_ISHTML;
    if ($this->locked)  $flags |= FLAG_LOCKED;
    return "SELECT text_component_save($component_id, $code, '$text', $flags)"
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

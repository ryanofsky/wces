<?

/*

The widget separator should have the following properties

1) It shouldn't be a legal character in an identifier. This eliminates
   the letters, numbers, and the underscore.

2) It can't be one of the characters that gets mangled by PHP's variable
   parsing routine. The eliminates the period symbol and the space and
   the square brackets []. (If it weren't for this condition,
   period would be perfect, since it has the right connotation and
   meets all the other requirements.)

3) To save space and make the html more transparent, the characters
   shouldn't need to be encoded in html or URLs. This eliminates
   the bizarre ascii characters, spaces, and <">#%&?+=

4) At this point what is left is: !$'()*,-/:;\^`{|}~
   But of these, the only ones that internet explorer and netscape
   won't escape when url encoding form field names are: *-
*/

define("WIDGET_SEPARATOR", "-");
define("WIDGET_GET",     0x1);
define("WIDGET_POST",    0x2);
define("WIDGET_URL",     0x4);
define("WIDGET_SESSION", 0x8);
define("WIDGET_FORM",    WIDGET_GET | WIDGET_POST);

function quote($str)
{
  return '"' . htmlspecialchars($str) . '"';
};

class Widget
{
  var $name;
  var $form;

  function Widget($name, &$parent)
  {
    $this->name = $this->joinName($parent->name, $name);
    $this->form = &$parent->form;
    $parent->addChild($this);
  }

  function joinName($str1, $str2) // static
  {
    if (strlen($str1) == 0)
      return $str2;
    else if (strlen($str2) == 0)
      return $str1;
    else
      return $str1 . WIDGET_SEPARATOR . $str2;
  }

  function shortName($name, $shortName = null)
  {
    if (!isset($shortName)) $shortName = $name;
    $this->shortNames[$name] = $shortName;
  }

  function name($str = '')
  {
    if (isset($this->shortNames[$str]))
      return $this->shortNames[$str];
    return $this->joinName($this->name, $str);
  }

  // just a convenient wrapper around Form::getUrl()
  // calls name() automatically on $vars keys or accepts a scalar $vars
  function getUrl($vars = null, $pageName = null)
  {
    $fvars = array();
    if (is_array($vars))
    {
      foreach ($vars as $k => $v)
        $fvars[$this->name($k)] = $v;
    }
    else if (isset($vars))
      $fvars[$this->name()] = $vars;

    return $this->form->getUrl($fvars, $pageName);
  }
    
  function scriptName($str)
  {
    static $i = 0;
    ++$i;
    $s = $this->joinName($this->name, $str);
    return str_replace(WIDGET_SEPARATOR, '_', $s) . '_' . $i;
  }

  function readValue($name = "", $type = WIDGET_FORM)
  {
    $n = $this->name($name);
    
    if ($type & WIDGET_POST && isset($_POST[$n]))
      return $_POST[$n];
    else if ($type & (WIDGET_GET | WIDGET_URL) && isset($_GET[$n]))
      return $_GET[$n];      
    
    return NULL;
  }

  function printValue($name, $value, $type = WIDGET_FORM)
  {
    if (!isset($value)) return;

    $n = $this->name($name);
    if ($type & WIDGET_FORM)
      print('<input type=hidden name=' . $n . ' value="'
        . htmlspecialchars($value) . '">');

    if ($type & WIDGET_URL)
    {
      $this->form->addUrlVar($n, $value);
    }

    if ($type & WIDGET_SESSION)
      die("Feature Not Implemented Yet at " . __FILE__ . ":" . __LINE__);
  }

  function printArrayValue($name, $value)
  {
    if (!is_array($value)) return;
    $n = $this->name($name) . '[]';
    foreach($value as $v)
      print('<input type=hidden name=' . $n . ' value="'
        . htmlspecialchars($v) . '">');
  }

  // $new may be true, false, or null
  //   true means the widget has been displayed in a previous load
  //     and state may be read from form variables
  //   false means that the widget is newly created and its state
  //     can't be read from form variables
  //   null means that the caller doesn't know whether the widget
  //     is new or not. Certain widgets may need non-null values
  //     to load correctly. One way to automatically provide this
  //     information to them is to make them children of an 
  //     InitializerWidget.
  function loadState($new)
  {}

  function displayHidden()
  {}

  function display()
  {}

  function DumpScript()
  {}
}

class ParentWidget extends Widget
{
  var $children = array();
  var $modalChild = null;

  function ParentWidget($name, &$parent)
  {
    $this->Widget($name, $parent);
  }

  function addChild(&$child)
  {
    $this->children[] =& $child;
  }

  function loadChild(&$child, $new = null)
  {
    $child->loadState($new);
    if (!empty($child->modal)) $this->modalChild = &$child;
  }

  function loadState($new)
  {
    foreach(array_keys($this->children) as $k)
      $this->loadChild($this->children[$k], $new);
  }

  function display()
  {
    if (isset($this->modalChild))
      $this->printModalChildren();
    else
      $this->printVisible();
    $this->printState();
  }

  function displayHidden()
  {
    foreach(array_keys($this->children) as $k)
    {
      $c = &$this->children[$k];
      if (!isset($c)) continue;
      $c->displayHidden();
    }
    $this->printHidden();
    $this->printState();
  }

  function printModalChildren()
  {
    foreach(array_keys($this->children) as $k)
    {
      $c = &$this->children[$k];
      if (!isset($c)) continue;
      if (isset($c->modal) && $c->modal) $c->display(); else $c->displayHidden();
    }
    $this->printHidden();
  }
  
  function printState()
  {}

  function printHidden()
  {}

  function printVisible()
  {}

};

class InitializerWidget extends ParentWidget
{
  function InitializerWidget($name, &$parent)
  {
    $this->ParentWidget($name, $parent);
  }
  
  function loadState($new)
  {
    if (!isset($new)) $new = !$this->readValue();
    ParentWidget::loadState($new);
    $this->modal = isset($this->modalChild);
  }
  
  function printState()
  {
    $this->printValue('', 1);
  }
}

class Form extends ParentWidget
{
  var $formName; // name of form, used for javascripting
  var $pageName; // name of current page, used for linking
  var $urlVars = array();

  function Form($formName = null, $pageName = null)
  {
    global $server_url;
    $this->form = &$this;
    $this->formName = $formName;
    $this->pageName = isset($pageName) ? $pageName : $server_url->toString(false, true, false);
  }

  function addUrlVar($name, $value)
  {
    $this->urlVars[$name] = $value;
  }
  
  function getUrl($overrides = array(), $pageName = null)
  {
    $params = '';

    foreach ($this->urlVars as $name => $value)
    {
      if (isset($overrides[$name]) || $value === false) continue;
      if (strlen($params)) $params .= '&'; else $params .= '?';
      $params .= urlencode($name);
      $params .= '=';
      $params .= urlencode($value);
    }

    foreach ($overrides as $name => $value)
    {
      if ($value === false) continue;
      if (strlen($params)) $params .= '&'; else $params .= '?';
      $params .= urlencode($name);
      $params .= '=';
      $params .= urlencode($value);
    }

    global $QSID, $ASID;
    $params .= strlen($params) ? $ASID : $QSID;

    if (!isset($pageName))
      $pageName = $this->form->pageName;

    return $pageName . $params;
  }

  function loadState()
  {
    // other loadState methods accept a parameter, but this one shouldn't
    assert(func_num_args() == 0);
    ParentWidget::loadState(null);
  }
};

?>

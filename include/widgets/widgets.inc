<?

define("WIDGET_POST",1);
define("WIDGET_GET",2);

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
define("WIDGET_FORM",    0x1);
define("WIDGET_URL",     0x2);
define("WIDGET_BIGURL",  0x4);
define("WIDGET_SESSION", 0x8);

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

  function urlShortName($name, $shortName = null)
  {
    if (!isset($shortName)) $shortName = $name;
    $this->form->formShortName($this->fullName($name), $shortName);
  }

  function joinName($str1, $str2)
  {
    if (strlen($str1) == 0)
      return $str2;
    else if (strlen($str2) == 0)
      return $str1;
    else
      return $str1 . WIDGET_SEPARATOR . $str2;
  }

  function getUrl($vars = null)
  {
    $fvars = array();
    if (is_array($vars))
    {
      foreach ($vars as $k => $v)
        $fvars[$this->fullName($k)] = $v;
    }
    else if (isset($vars))
      $fvars[$this->name] = $vars;
      
    return $this->form->pageName . $this->form->getUrlVars($fvars);
  }

  function scriptName($str)
  {
    static $i = 0;
    ++$i;
    $s = $this->fullName($str);
    return str_replace(WIDGET_SEPARATOR, '_', $s) . '_' . $i;
  }

  function fullName($str)
  {
    return $this->joinName($this->name, $str);
  }

  function readValue($name = "", $type = WIDGET_FORM)
  {
    $full = $this->fullName($name);
    if ($type & WIDGET_FORM)
    {
      if (isset($_POST[$full]))
        return $_POST[$full];
      else if (isset($_GET[$full]))
        return $_GET[$full];
    }

    if ($type & WIDGET_URL && isset($this->form->shortNames[$full]))
    {
      $sn = $this->form->shortNames[$full];
      if (isset($_GET[$sn])) return $_GET[$sn];
    }

    return NULL;
  }

  function printValue($name, $value, $type = WIDGET_FORM)
  {
    if (!isset($value)) return;

    $full = $this->fullName($name);
    if ($type & WIDGET_FORM)
      print('<input type=hidden name=' . $full . ' value="'
        . htmlspecialchars($value) . '">');

    if ($type & WIDGET_URL)
    {
      $this->form->addUrlVar($full, $value);
    }

    if ($type & WIDGET_BIGURL)
      die("Feature Not Implemented Yet at " . __FILE__ . ":" . __LINE__);

    if ($type & WIDGET_SESSION)
      die("Feature Not Implemented Yet at " . __FILE__ . ":" . __LINE__);

  }

  function printArrayValue($name, $value)
  {
    if (!is_array($value)) return;
    $full = $this->fullName($name) . '[]';
    foreach($value as $v)
      print('<input type=hidden name=' . $full . ' value="'
        . htmlspecialchars($v) . '">');
  }

  function loadInitialState()
  {}

  function loadState()
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

  function loadChild(&$child, $new)
  {
    if ($new) $child->loadInitialState(); else $child->loadState();
    if (isset($child->modal) && $child->modal) $this->modalChild = &$child;
  }

  function loadInitialState()
  {
    foreach(array_keys($this->children) as $k)
      $this->loadChild($this->children[$k], true);
  }

  function loadState()
  {
    foreach(array_keys($this->children) as $k)
      $this->loadChild($this->children[$k], false);
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

class Form extends ParentWidget
{
  var $formName; // name of form, used for javascripting
  var $pageName; // name of current page, used for linking
  var $isStale = false; // true if this page load is not the first
  var $shortNames = array();
  var $urlVars = array();

  function addUrlVar($name, $value)
  {
    $this->urlVars[$name] = $value;
  }

  function formShortName($long, $short)
  {
    $this->shortNames[$long] = $short;
  }

  function getUrlVars($overrides = array())
  {
    $url = ''; $shorted = false;

    foreach ($this->urlVars as $name => $value)
    {
      if (isset($overrides[$name]) || $value === false) continue;
      if ($url) $url .= '&'; else $url .= '?';
      if (isset($this->shortNames[$name]))
      {
        $shorted = true;
        $name = $this->shortNames[$name];
      }
      $url .= $name;
      $url .= '=';
      $url .= $value;
    }

    foreach ($overrides as $name => $value)
    {
      if ($value === false) continue;
      if ($url) $url .= '&'; else $url .= '?';
      if (isset($this->shortNames[$name]))
      {
        $shorted = true;
        $name = $this->shortNames[$name];
      }
      $url .= $name;
      $url .= '=';
      $url .= $value;
    }

    if (!$shorted && $url)
      $url .= '&isStale=1';

    return $url;
  }

  function Form($formName = null, $pageName = null)
  {
    global $server_url;
    $this->form = &$this;
    $this->name = "";
    $this->formName = $formName;
    $this->pageName = isset($pageName) ? $pageName : $server_url->toString(false, true, false);
  }

  function loadState($isStale = null)
  {
    $this->isStale = $this->readValue('isStale')
      || (count(array_intersect($this->shortNames,array_keys($_GET)))!= 0);
    if ($this->isStale)
      ParentWidget::loadState();
    else
      ParentWidget::loadInitialState();
  }

  function displayHidden()
  {
    $this->printValue('isStale', 1);
  }

  function display()
  {
    $this->printValue('isStale', 1);
  }
};

?>

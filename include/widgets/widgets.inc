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

4) At this point what is left is: !$'()*+,-/:;\^`{|}~
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

  function joinName($str1, $str2)
  {
    if (strlen($str1) == 0)
      return $str2;
    else if (strlen($str2) == 0)
      return $str1;
    else
      return $str1 . WIDGET_SEPARATOR . $str2;
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

  function readValue($name = "")
  {
    global $HTTP_POST_VARS, $HTTP_GET_VARS;
    $full = $this->fullName($name);
    if (isset($HTTP_POST_VARS[$full]))
      return $HTTP_POST_VARS[$full];
    else if (isset($HTTP_GET_VARS[$full]))
      return $HTTP_GET_VARS[$full];
    else
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
      $this->form->addUrlVar($full, $value);

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
    {
      foreach(array_keys($this->children) as $k)
      {
        $c = &$this->children[$k];
        if (!isset($c)) continue;
        if (isset($c->modal) && $c->modal) $c->display(); else $c->displayHidden();
      }
      $this->printHidden();
    }
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
  var $names = array(); // array of alternate names for isStale
  var $urlVars = "";

  function addUrlVar($name, &$value)
  {
    if ($this->urlVars) $this->urlVars .= "&";
    $this->urlVars .= "$name=" . urlencode($value);
  }

  function getUrlVars($needStale = true)
  {
    if ($needStale)
    {
      if (strlen($this->urlVars) == 0)
        return "isStale=1";
      else
        return "$this->urlVars&isStale=1";
    }
    else
      return $this->urlVars;
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
    global $HTTP_GET_VARS;

    $this->isStale = $this->readValue('isStale')
      || (count(array_intersect($this->names,array_keys($HTTP_GET_VARS)))!= 0);

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

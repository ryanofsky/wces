<?

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");

class AbetTarget
{
  function AbetTarget(&$dest)
  {
    $this->dest =& $dest;
  }
  
  function finishLoad(&$prototype)
  {
    // intentional object copy
    $this->dest = $prototype;  
  }
}

class AbetFactory extends ComponentFactory
{
  function code() { return 10; }
  function name() { return null; }

  function create()
  {
    static $prototype = NULL;
    
    if (!isset($prototype))
    {
      $component_id = $this->component_id();
      $components = array($component_id = false);
      global $wbes;
      wbes_connect();

      $factories = array(new ChoiceFactory, new ChoiceQuestionFactory);
      $surveyGlobals =& new SurveyGlobals($factories, 0);
      
      $save_id = 'NULL';
      $dest = false;
      $load =& $surveyGlobals->getLoad(ChoiceFactory::code(), $this->component_id(), $this->specialization_id());
      $load->addTarget(new AbetTarget($dest));
    
      $surveyGlobals->doLoad();
    }

    $c =& new Abet;
    $c->prototype = $dest; // intentional copy
    $c->typeCode = $this->code();
    return $c;
  }

  function loadComponents(&$components, &$surveyGlobals)
  {
    global $wbes;

    $cs = array();

    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, type, which FROM abet_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c =& new Abet;
      $c->typeCode = (int)$wr->row['type'];
      $c->which = (int)$wr->row['which'];
      $load =& $surveyGlobals->getLoad(ChoiceFactory::code(), $this->component_id(), $this->specialization_id());
      $load->addTarget(new AbetTarget($c->prototype));
      $components[$wr->row['component_id']] =& $c;
    }
  }
  
  function revision_id() { return 31; }
  function component_id() { return 31; }
  function specialization_id() { return 1; }
}

class NewAbetFactory extends AbetFactory
{
  function name() { return "ABET Question Set"; }
  function code() { return 100; }
  function revision_id() { return 1265; }
  function component_id() { return 1265; }
  function specialization_id() { return 1; }
}

class BioAbetFactory extends AbetFactory
{
  function code() { return 101; }
  function name() { return "ABET Biology Question Set"; }
  function revision_id() { return 1266; }
  function component_id() { return 1266; }
  function specialization_id() { return 1; }
}

class Abet extends Component
{
  var $which = 0;

  function Abet()
  {
    $this->Component();
  }

  function & geteditor($name, &$parent)
  {
    return new AbetEditor($this, $name, $parent);
  }

  function & getwidget($name, &$parent)
  {
    // serialize/unserialize is needed because plain old assignment
    // does not make copies of object or array member that have
    // been assigned by reference (only normal variables
    $p = unserialize(serialize($this->prototype));
    $p->revision_id = $this->revision_id;
    $p->item_id = $this->item_id;
    foreach (array_keys($p->questions) as $k)
      if (!($this->which & (1 << $k)))
        unset($p->questions[$k]);
    return new ChoiceWidget($p, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    $which = (int)$this->which;
    $typeCode = (int)$this->typeCode;
    return "abet_component_save($component_id, $typeCode, $which)";
  }
};

class AbetEditor extends ComponentEditor
{
  // widgets
  var $component;
  var $boxen;

  function AbetEditor(&$component, $name, &$parent)
  {
    $this->ComponentEditor($component, $name, $parent);
    $this->boxen = array();
    foreach(array_keys($component->prototype->questions) as $k)
      $this->boxen[$k] = & new CheckBox("abet$k", $this);
  }

  function loadState($new)
  {
    assert(isset($new));
    ComponentEditor::loadState($new);
    if ($new)
    {
      foreach(array_keys($this->boxen) as $k)
        $this->boxen[$k]->checked = (bool)($this->component->which & (1 << $k));
    }
  }

  function save()
  {
    $this->component->which = 0;
    foreach(array_keys($this->boxen) as $k)
      if ($this->boxen[$k]->checked)
        $this->component->which |= 1 << $k;
  }

  function printVisible()
  {
    global $server_media;
    $prototype =& $this->component->prototype;
?>
<h3>ABET Questions</h3>
<p>The ABET questions accept a rating from 0-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>
<table cellpadding=0 cellspacing=2>
<?
    foreach(array_keys($prototype->questions) as $k)
    {
      $v = $prototype->questions[$k]->text;
      print("  <tr><td bgcolor=\"#DDDDDD\" background=\"$server_media/0xDDDDDD.gif\" width=\"100%\">");
      print("<table><tr><td>");
      $this->boxen[$k]->display();
      print("</td><td>");
      $this->boxen[$k]->displayLabel("<b>$v</b>");
      print("</td></tr></table>");
      print("</td></tr>\n");
    }
    print("</table>");
  }
}

?>
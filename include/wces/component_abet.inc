<?

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");

class AbetTarget
{
  function AbetTarget(&$abetComponent)
  {
    $this->abetComponent =& $abetComponent;
  }
  
  function finishLoad(&$prototype)
  {
    // intentional object copy
    $this->abetComponent->prototype = $prototype;  
  }
}

class AbetFactory extends ChoiceFactory
{
  function code() { return 10; }
  function name() { return null; }

  function create()
  {
    $prototype = &$this->prototype();

    $c =& new Abet;
    $c->typeCode = $this->code();
    foreach(array_keys($prototype->questions) AS $k)
      $c->which[$k] = false;
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
      $which = (int)$wr->row['which'];
      foreach(array_keys($prototype->questions) AS $k)
        $c->which[$k] = (bool)($which & (1 << $k));
      $component_id = 'revision_component(' . $this->revision_id() . ')';
      $load =& $surveyGlobals->getLoad(ChoiceFactory::code(), $component_id, $this->specialization_id());
      $load->addTarget(new AbetLoad($c));
      $components[$wr->row['component_id']] =& $c;
    }
  }
  
  function revision_id() { return 31; }
  function specialization_id() { return 1; }
}

class NewAbetFactory extends AbetFactory
{
  function name() { return "ABET Question Set"; }
  function code() { return 100; }
  function revision_id() { return 1265; }
  function specialization_id() { return 1; }
}

class BioAbetFactory extends AbetFactory
{
  function code() { return 101; }
  function name() { return "ABET Biology Question Set"; }
  function revision_id() { return 1266; }
  function specialization_id() { return 1; }
}

class Abet extends Component
{
  // array of booleans
  var $which = array();

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
    $p = AbetFactory::prototype($this->typeCode); // p is a copy, not a reference
    $p->revision_id .= " [$this->revision_id]";
    foreach($this->which as $k => $v)
      if (!$v)
      {
        unset($p->question_ids[$k]);
        unset($p->questions[$k]);
      }
    return new ChoiceWidget($p, $name, $parent);
  }

  function save($specialization_id, $save_id)
  {
    global $wbes;

    $r = $this->revision_id ? $this->revision_id : 'NULL';

    $which = 0;
    foreach($this->which as $k => $w)
      if ($w) $which |= 1 << (int)$k;

    $result = pg_go("SELECT abet_component_save($specialization_id, $r, $save_id, $which, $this->typeCode)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
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
    foreach($component->which as $k => $v)
      $this->boxen[$k] = & new CheckBox("abet$k", $this);
  }

  function loadInitialState()
  {
    ComponentEditor::loadInitialState();
    foreach(array_keys($this->boxen) as $k)
      $this->boxen[$k]->checked = (bool)$this->component->which[$k];
  }

  function save()
  {
    $this->component->which = array();
    foreach(array_keys($this->boxen) as $k)
      $this->component->which[$k] = (bool)$this->boxen[$k]->checked;
  }

  function printVisible()
  {
    global $server_media;
    $prototype = AbetFactory::prototype($this->component->typeCode);
?>
<h3>ABET Questions</h3>
<p>The ABET questions accept a rating from 0-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>
<table cellpadding=0 cellspacing=2>
<?
    foreach($prototype->questions as $k => $v)
    {
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
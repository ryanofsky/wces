<?

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");

class AbetFactory extends ChoiceFactory
{
  var $noshow = true;

  function code() { return 10; }
  function name() { return "ABET Question Set"; }
  
  function create()
  {
    $prototype = &$this->prototype();
    
    $c =& new Abet;
    $c->typeCode = $this->code();
    foreach(array_keys($prototype->questions) AS $k)
      $c->which[$k] = false;
    return $c;
  }

  function load(&$components, $ids, $ordinals, $topic_id, $latest)
  {
    global $wbes;

    $cs = array();
    $prototype = & AbetFactory::prototype();
    
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_go("SELECT revision_id, type, which FROM abet_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = & new Abet;
      $c->typeCode = (int)$wr->row['type'];
      $c->revision_id = (int)$wr->row['revision_id'];
      $which = (int)$wr->row['which'];
      foreach(array_keys($prototype->questions) AS $k)
      {
        $c->which[$k] = (bool)($which & (1 << $k));
      }
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
  
  function & prototype($typeCode = NULL)
  {
    // putting prototypes for subclasses here is a big ugly hack
    if (!isset($typeCode)) $typeCode = $this->code();
    //$revisions = array(10 => 31, 100 => 1252, 101 => 1256);
    $revisions = array(10 => 31, 100 => 1265, 101 => 1266);
    $skips = array(10 => array(3));
    
    static $prototypes = array();
    if (!isset($prototype[$typeCode]))
    {
      
      wbes_connect();
      $cs = array();
      
      ChoiceFactory::load($cs, array($revisions[$typeCode]), array(0), 1, "NULL");
      $prototype = $cs[0];
      if (isset($skips[$typeCode]))
      foreach($skips[$typeCode] as $s)
      {
        unset($prototype->questions[$s]);
        unset($prototype->question_ids[$s]);
      }
      $prototypes[$typeCode] =& $prototype; 
    }
    return $prototypes[$typeCode];
  }
}

class NewAbetFactory extends AbetFactory
{
  var $noshow = false;
  function code() { return 100; }
}

class BioAbetFactory extends AbetFactory
{
  var $noshow = false;
  function code() { return 101; }
  function name() { return "ABET Biology Question Set"; }
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
  
  function save($topic_id, $save_id)
  {
    global $wbes;

    $r = $this->revision_id ? $this->revision_id : 'NULL';

    $which = 0;
    foreach($this->which as $k => $w)
      if ($w) $which |= 1 << (int)$k;

    $result = pg_go("SELECT abet_component_save($topic_id, $r, $save_id, $which, $this->typeCode)", $wbes, __FILE__, __LINE__);
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
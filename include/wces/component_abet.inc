<?

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");

$ABET_REVISION = 31;
$ABET_TOPIC = 1;
$ABET_SKIP = array(3);

class AbetFactory extends ChoiceFactory
{
  function code() { return 10; }
  function name() { return "ABET Question Set"; }
  
  function prototype()
  {
    global $ABET_TOPIC, $ABET_SKIP, $ABET_REVISION;
    static $prototype = NULL;
    if (!$prototype)
    {
      wbes_connect();
      $cs = array();
      ChoiceFactory::load($cs, array($ABET_REVISION), array(0), 1, "NULL");
      $prototype = $cs[0];
      foreach($ABET_SKIP as $s)
      {
        unset($prototype->questions[$s]);
        unset($prototype->question_ids[$s]);
      }
    }
    return $prototype;
  }
  
  function create()
  {
    $c = new Abet($this->prototype());
    foreach(array_keys($c->prototype->questions) AS $k)
      $c->which[$k] = false;
    return $c;
  }

  function load(&$components, $ids, $ordinals, $topic_id, $latest)
  {
    global $wbes, $ABET_SKIP;
    $cs = array();
    
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, which FROM abet_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new Abet($this->prototype());
      $c->revision_id = (int)$wr->row['revision_id'];
      $which = (int)$wr->row['which'];
      foreach(array_keys($c->prototype->questions) AS $k)
        $c->which[$k] = (bool)($which & (1 << $k));
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
}

class Abet extends Component
{
  var $prototype = NULL;
  
  // array of booleans
  var $which = array();
  
  function Abet($prototype)
  {
    $this->Component();
    $this->prototype = $prototype;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new AbetEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    $p = $this->prototype;
    foreach($this->which as $k => $v)
      if (!$v)
      {
        unset($p->question_ids[$k]);
        unset($p->questions[$k]);
      }
    return new ChoiceWidget($p,$prefix,$form,$formmethod);
  }
  
  function save($topic_id, $save_id)
  {
    global $wbes;

    $r = $this->revision_id ? $this->revision_id : 'NULL';

    $which = 0;
    foreach($this->which as $k => $w)
      if ($w) $which |= 1 << (int)$k;

    $result = pg_query("SELECT abet_component_save($topic_id, $r, $save_id, $which)", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }  
};

class AbetEditor extends ComponentEditor
{
  //widgets
  
  var $component;
  var $boxen;
 
  function AbetEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    $this->boxen = array();
    foreach($component->which as $k => $v)
      $this->boxen[$k] = new CheckBox("{$prefix}_abet$k", $form, $formmethod);
  }
  
  function loadvalues() // public abstract
  {
    if ($this->loadattribute("moldy"))
    {
      foreach(array_keys($this->boxen) as $k)
        $this->boxen[$k]->loadvalues();
    }
    else
    {
      foreach(array_keys($this->boxen) as $k)
        $this->boxen[$k]->checked = (bool) $this->component->which[$k];
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    $this->component->which = array();
    foreach(array_keys($this->boxen) as $k)
      $this->component->which[$k] = (bool)$this->boxen[$k]->checked;
  }
  
  function display()
  {
    global $server_media;
    $this->printattribute("moldy",true);
?>  
<h3>ABET Questions</h3>
<p>The ABET questions accept a rating from 0-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>
<table cellpadding=0 cellspacing=2>
<?
    foreach($this->component->prototype->questions as $k => $v)
    {
      print("  <tr><td bgcolor=\"#DDDDDD\" background=\"$server_media/0xDDDDDD.gif\" width=\"100%\">");
      print("<table><tr><td>");
      $this->boxen[$k]->display();
      print("</td><td>");
      $this->boxen[$k]->displaylabel("<b>$v</b>");
      print("</td></tr></table>");
      print("</td></tr>\n");
    }  
    print("</table>");
  }
}

?>
<?

require_once("wbes/component.inc");
require_once("wbes/component_choice.inc");

$ABETQUESTIONS = array(
   1 => "Design experiments",
   2 => "Analyze and interpret data",
   3 => "Conduct experiments",
   4 => "Analyze and interpret data",
   5 => "Design a system, component, or process to meet desired needs",
   6 => "Function on multidisciplinary teams",
   7 => "Identify or formulate engineering problems",
   8 => "Solve engineering problems",
   9 => "Understand ethical responsibilities",
  10 => "Understand the impact of engineering solutions in a global/societal context",
  11 => "Use modern engineering tools",
  12 => "Communicate using oral presentations",
  13 => "Communicate using written reports",
  14 => "Pilot test a component prior to implementation",
  15 => "Use text materials to support project design",
  16 => "Integrate knowledge of mathematics, science, and engineering in engineering solutions",
  17 => "Apply knowledge of contemporary issues to engineering solutions",
  18 => "Recognize need to engage in lifelong learning"
);

class AbetFactory extends ComponentFactory
{
  function code() { return 10; }
  function name() { return "ABET Question Set"; }

  function create()
  {
    return new Abet();
  }

  function load(&$components, $ids, $ordinals, $topic_id, $latest)
  {
    global $wbes, $ABETQUESTIONS;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, which FROM abet_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new Abet();
      $c->revision_id   = (int)$wr->row['revision_id'];
      $which = (int)$wr->row['which'];
      foreach($ABETQUESTIONS AS $k => $v)
        if ($which & (1 << $k))
          $c->which[$k] = true;
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
}

class Abet extends Component
{
  // array of booleans
  var $which = array();
  
  function Abet()
  {
    global $ABETQUESTIONS;
    $this->Component();
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new AbetEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new AbetWidget($this,$prefix,$form,$formmethod);
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
    global $ABETQUESTIONS;
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    $this->boxen = array();
    foreach($ABETQUESTIONS as $k => $v)
      $this->boxen[$k] = new CheckBox("{$prefix}_abet$k", $form, $formmethod);
  }
  
  function loadvalues() // public abstract
  {
    global $ABETQUESTIONS;
    
    if ($this->loadattribute("moldy"))
    {
      foreach($ABETQUESTIONS as $k => $v)
        $this->boxen[$k]->loadvalues();
    }
    else
    {
      foreach($ABETQUESTIONS as $k => $v)
        $this->boxen[$k]->checked = isset($this->component->which[$k]) && (bool)$this->component->which[$k];
    }
  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    global $ABETQUESTIONS;
    foreach($ABETQUESTIONS as $k => $v)
      $this->component->which[$k] = $this->boxen[$k]->checked;
  }
  
  function display()
  {
    global $ABETQUESTIONS, $server_media;

    $this->printattribute("moldy",true);
?>  

<h3>ABET Questions</h3>
<p>The ABET questions accept a rating from 0-5. You should check off any questions that are applicable to your class so they can be included in the surveys students fill out.</p>
<p>Students will be asked, "<b>To what degree did this course enhance your ability to:</b>"</p>
<table cellpadding=0 cellspacing=2>
<?
    $a = isset($questionset["ABET"]) ? explode(",",$questionset["ABET"]) : array();
    foreach($ABETQUESTIONS as $k => $v)
    {
      print("  <tr><td bgcolor=\"#DDDDDD\" background=\"$server_media/0xDDDDDD.gif\" width=\"100%\">");
      print("<p style=\"margin-left: 30px; text-indent: -30px\">");
      $this->boxen[$k]->display(false, 'style="width: 30px"');
      $this->boxen[$k]->displaylabel("<b>$v</b>");
      print("</p></td></tr>\n");
    }  
    print("</table>");
  }
}

class AbetWidget extends ChoiceWidget
{
  var $choices;
  var $legend;
  var $questions;
 
  function AbetWidget(&$c,$prefix,$form,$formmethod)
  {
    global $ABETQUESTIONS;
    $fc = new Choice();
    
    $this->ComponentWidget($fc,$prefix,$form,$formmethod);
    $this->legend = "<b>0 = <i>not at all</i>, 5 = <i>a great deal</i></b>";
    $this->choices = range(0, 5);
    $this->questions = array();
    $this->other = -1;
    foreach($ABETQUESTIONS as $k => $v)
      if (isset($c->which[$k]) && $c->which[$k])
        $this->questions[] = $v;
    
    $this->component->other_choice = false;
    $this->component->select_many = false;
    $this->component->stacked = true;
    $this->component->rows = 0;
    $this->component->intro_text = "<p>To what degree did this course enhance your ability to:</p>";
  }
}

?>
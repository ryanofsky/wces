<?

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

class ABET extends Component
{
  // array of booleans
  var $which = array();
  
  function ABET()
  {
    global $ABETQUESTIONS;
    $this->Component();
    foreach($ABETQUESTIONS as $k => $v)
      $this->which[$k] = false;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new ABETEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new ABETWidget($this,$prefix,$form,$formmethod);
  }
};

class ABETEditor extends ComponentEditor
{
  //widgets
  
  var $component;
  var $boxen;
 
  function ABETEditor(&$component,$prefix,$form,$formmethod)
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
        $this->boxen[$k]->checked = (bool)$this->component->which[$k];
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

class ABETWidget extends ComponentWidget
{
  var $choices;
  var $legend;
  var $questions;
 
  function ABETWidget(&$c,$prefix,$form,$formmethod)
  {
    global $ABETQUESTIONS;
    $this->ComponentWidget($c,$prefix,$form,$formmethod);
    
    $this->legend = "<b>0 = <i>not at all</i>, 5 = <i>a great deal</i></b>";
    $this->choices = range(0, 5);
    $this->questions = array();
    foreach($ABETQUESTIONS as $k => $v)
      if ($c->which[$k])
        $this->questions[] = $v;
    
    $this->component->select_many = false;
    $this->component->stacked = true;
    $this->component->rows = 0;
    
    $this->answers = array();
    $this->others = array();
    $this->errors = array();
  }
  
  function loadvalues() // public abstract
  {
    foreach ($this->questions as $i => $text)
    {
  	  $c = &$this->component;
  	  $name = "q$i";
  	  $a = $this->loadattribute($name);
      if ($a !== "")
        $this->answers[$i] = (int)$a;
      else  
        $this->errors[$i] = "Question '$text' is not filled in.";
    }  
  }
  
  function showchoices($questionno, $tdparams = "")
  {
  	$c = &$this->component;
  	$name = $this->prefix . "_q" . $questionno;
  	if ($c->select_many) $name .= "[]";
  	
  	if ($c->stacked) print("<td $tdparams>");

      if ($c->stacked)
        $sep = "</td>\n<td $tdparams>";
      else if ($c->vertical)
        $sep = "<br>\n";
      else
        $sep = "\n";    
      
      $first = true;
      foreach($this->choices as $i => $choice)
      {
        if ($first) $first = false; else print($sep);
        
        $id = $this->prefix . '_q' . $questionno . '_' . $i;

      	$checked = $i === $this->answers[$questionno] ? " checked" : "";
       	print("<input type=radio name=$name value=$i id=$id$checked>");

        if (!$c->stacked)
          print(" <label for=$id>$choice</label>");
        
      }  

    if ($c->stacked) print("</td>\n");
  }
  
  function display($hidden = false, $showerrors = true)
  {
    global $server_media;

    if($hidden)
    {
      foreach($this->answers as $i => $v)
      {
        $name = "q$i";
          $this->printattribute($name, $v);
      }
      return;
    }

    $c = &$this->component;
    $n = count($this->choices);
    if ($c->stacked)
    {
?>    	
<p>To what degree did this course enhance your ability to:</p>
<table bordercolor=black cellspacing=0 cellpadding=3 border=0 RULES="groups" FRAME=box STYLE="border: none">
<? if ($this->legend) { ?>
<thead>
<tr>
<td colspan=<?=$n + 1?> align=right STYLE="border: none"><b><?=$this->legend?></b></td>
</tr>
</thead>
<? } ?>
<tbody>
<tr>
  <td bgcolor=black background="<?=$server_media?>/0x000000.gif">&nbsp;</td>
<? if ($c->rows > 0) { ?>
  <td bgcolor=black background="<?=$server_media?>/0x000000.gif">&nbsp;</td>
<? } else { ?>
<? foreach($this->choices as $choice) { ?>
  <td bgcolor=black background="<?=$server_media?>/0x000000.gif" align=center><font color=white><STRONG><?=$choice?></STRONG></font></td>
<? } } ?>  
</tr>
<?
    
      $row = true;
      foreach ($this->questions as $i => $text)
      {
        $row = !$row; $color = $row ? 'bgcolor="#FFFFFF" background="' . $server_media . '/0xFFFFFF.gif"' : 'bgcolor="#EEEEEE" background="' . $server_media . '/0xEEEEEE.gif"';
        if ($this->errors[$i] && $showerrors) $text = "<b><font color=red>$text</font></b>";
        print("<tr>\n<td $color>$text</td>\n");
        $this->showchoices($i, "$color align=center");
        print("</tr>");
      }
      print("\n</tbody>\n</table>\n");
    } // if ($c->stacked)
    else
    {
      if ($this->legend) print("<p><strong>" . $this->legend . "</strong></p>\n");
      foreach ($this->questions as $i => $text)
      {
        if ($this->errors[$i] && $showerrors) $text = "<b><font color=red>$text</font></b>";        
        print("$text<br>\n");
        $this->showchoices($i);
        print("<br><br>\n");
      }
    };  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  }
}

?>
<?

class Team extends Component
{
  // array of stringed items to rate
  var $questions = array();
  
  // array of stringed choices
  var $choices = array();
  
  var $first_number = 0;
  var $last_number = 0;
  
  var $is_numeric = false;
  var $select_many = false;
  var $other_choice = "";
  var $stacked = false;
  var $vertical = false;
  var $rows = 0;
  
  function Team()
  {
    $this->Component();
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new TeamEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new TeamWidget($this,$prefix,$form,$formmethod);
  }
};

class TeamEditor extends ComponentEditor
{
  //widgets
  
  var $question;
  var $questions;
  var $ctype;
  var $choices;
  var $select_many;
  var $other;
  var $other_label;
  var $first_number;
  var $last_number;
  var $first_label;
  var $last_label;
  var $layout;
  var $controls;
  var $rows;
 
  function TeamEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    
    $this->question = new TextBox(4,40,"wrap=virtual", $this->prefix."_question", $this->form, $this->formmethod);
    $this->questions = new DynamicList(30,7,false,$this->prefix."_questions",$this->form,$this->formmethod);
    
    $this->ctype = new DropBox(array(
      ""         => "-- Select One --",
      "ntext"     => "Text Choices",
      "num"      => "Numeric Choices"
    ), $this->prefix."_ctype", $this->form, $this->formmethod);
    
    $this->choices = new DynamicList(30,7,false,$this->prefix."_choices",$this->form,$this->formmethod);
    $this->select_many = new CheckBox($this->prefix."_selectmany", $this->form, $this->formmethod);
    $this->other = new CheckBox($this->prefix."_other", $this->form, $this->formmethod);
    $this->other_label = new TextBox(0,25,"",$this->prefix."_otherlabel",$this->form,$this->formmethod);
    $this->first_number = new TextBox(0,5,"",$this->prefix."_firstnumber",$this->form,$this->formmethod);
    $this->last_number = new TextBox(0,5,"",$this->prefix."_lastnumber",$this->form,$this->formmethod);
    $this->first_label = new TextBox(0,20,"",$this->prefix."_firstlabel",$this->form,$this->formmethod);
    $this->last_label = new TextBox(0,20,"",$this->prefix."_lastlabel",$this->form,$this->formmethod);

    $this->controls = new DropBox(array(
      "buttons" => "Checkboxes",
      "select"  => "List boxes",
      "drop"    => "Drop-down lists"
    ), $this->prefix."_controls", $this->form, $this->formmethod);

    $this->rows = new DropBox(array(
      "2"  => "2",
      "3"  => "3",
      "4"  => "4",
      "5"  => "5",
      "6"  => "6",
      "7"  => "7",
      "8"  => "8",
      "9"  => "9",
      "10" => "10"     
    ), $this->prefix."_rows", $this->form, $this->formmethod);
  }
  
  function loadvalues() // public abstract
  {
    ComponentEditor::loadvalues();
    if ($this->loadprevious)
    {
      $this->question->loadvalues();    
      $this->ctype->loadvalues();       
      $this->choices->loadvalues();     
      $this->select_many->loadvalues(); 
      $this->other->loadvalues();       
      $this->other_label->loadvalues(); 
      $this->first_number->loadvalues();
      $this->last_number->loadvalues(); 
      $this->first_label->loadvalues(); 
      $this->last_label->loadvalues();  
      $this->controls->loadvalues();    
      $this->rows->loadvalues();
    }  
    else
    {
      $c = count($this->component->questions);
      
      $this->question->text = isset($this->component->questions[0]) ? $this->component->questions[0] : "";

      if ($this->component->other_choice)
      {
        $this->other->checked = true;
        $this->other_label->text = $this->component->other_choice;
      }
      else
      {
        $this->other->checked = false;
        $this->other_label->text = "Other (please specify)";
      }

      $this->first_number->text = $this->component->first_number;
      $this->last_number->text = $this->component->last_number;

      if ($this->component->is_numeric)
      {
        $this->ctype->selected = "num";
        $this->choices->items = array();
        $this->first_label->text = $this->component->choices[0];
        $this->last_label->text = $this->component->choices[1];
      }
      else if (count($this->component->choices) > 0)
      {
        $this->choices->items = $this->component->choices;
        $this->first_label->text = "";
        $this->last_label->text = "";
        if ($this->component->first_number || $this->component->first_number)
          $this->ctype->selected = "ntext";
        else
        {
          $this->ctype->selected = "text";
          $this->first_number->text = 5;
          $this->last_number->text = 1;
        }  
      }
      else
      {
        $this->ctype->selected = "";
        $this->first_number->text = 5;
        $this->last_number->text = 1;
      }
      
      $this->select_many->checked = $this->component->select_many;
      $r = $this->component->rows;
      
      if ($r == 0)
        $this->controls->selected = "buttons";
      else if ($r == 1)
        $this->controls->selected = "drop";
      else
        $this->controls->selected = "select";
      
      $this->rows->selected = $r;     
    }
  }
  
  function checkvalues()
  {
    if (strlen($this->question->text) == 0)
    {
      $this->errors[] = "You did not enter any question text.";
    }  
      
    if ($this->ctype->selected == "text" || $this->ctype->selected == "ntext")
    {
      if (count($this->choices->items) < 2)
      {
        $this->errors[] = "You need to add at least two choices.";
        $this->ctype->broken = true;
      }
    }
    else if ($this->ctype->selected != "num")
    {
      $this->errors[] = "You did not choose a choice type.";
      $this->ctype->broken = true;
    }
    
    if($this->ctype->selected == "ntext" || $this->ctype->selected == "num")
    {
      $f = $this->first_number->text;
      $l = $this->last_number->text;
      
      if (!is_numeric($f))
      {
        $this->errors[] = "You must specify a numeric first value";
        $this->first_number->broken = true;
      }

      if (!is_numeric($l))
      {
        $this->errors[] = "You must specify a numeric last value";
        $this->last_number->broken = true;
      }

      if (doubleval($f) == doubleval($l))
      {
        $this->errors[] = "First and last values cannot be equal.";
        $this->first_number->broken = true;
        $this->last_number->broken = true;
      }

      if ($this->ctype->selected == "num")
      {
        if (doubleval($f) != intval($f))
        {
          $this->errors[] = "The first value must be an integer";
          $this->first_number->broken = true;
        }
        if (doubleval($l) != intval($l))
        {
          $this->errors[] = "The last value must be an integer";
          $this->last_number->broken = true;
        }
        
        if (abs($l-$f) > 15)
        {
          $this->errors[] = "The difference between the first and last values should not exceed 15.";
          $this->first_number->broken = true;
          $this->last_number->broken = true;
        }
             
      }
    }
    
    if ($this->ctype->selected == "text" && $this->other->checked)
    {
      if (!$this->other_label->text)
      {
        $this->errors[] = 'You did not specify a label for the "other" choice.';
      }
    }
    
    return count($this->errors) == 0;
  }
  
  function savevalues()
  {
    $c = &$this->component;
    
    $c->questions = array($this->question->text);

    
    if ($this->ctype->selected == "text")
    {
      $c->choices = $this->choices->items;
    }
    else if ($this->ctype->selected == "ntext")
    {
      $c->choices = $this->choices->items;
    }
    else if ($this->ctype->selected == "num")
    {
      $c->choices = array($this->first_label->text,$this->last_label->text);
    }

    if ($this->ctype->selected == "text")
    {
      $c->select_many = $this->select_many->checked;
      $c->other_choice = $this->other->checked ? $this->other_label->text: "";
      $c->first_number = 0;
      $c->last_number = 0;
    }
    else
    {
      $c->select_many = false;
      $c->other_choice = "";
      $c->first_number = (int)$this->first_number->text;
      $c->last_number = (int)$this->last_number->text;
    }

    $c->is_numeric = $this->ctype->selected == "num";
    $c->stacked = true;
    
    if ($this->controls->selected == "buttons")
      $c->rows = 0;
    else if ($this->controls->selected == "drop")
      $c->rows = 1;
    else if ($this->controls->selected == "select")
      $c->rows = (int) $this->rows->selected;  

    $this->component->vertical = false;

    ComponentEditor::savevalues();
  }
  
  function display()
  {
    global $SurveyEditor_ns4, $server_media;
    
    ComponentEditor::display();
    $p = $this->prefix;
    
    DynamicList::dumpscript();
    
    $show_clst = $this->ctype->selected == "text" || $this->ctype->selected == "ntext";
    $show_txto = $this->ctype->selected == "text";
    $show_othr = $this->other->checked;
    $show_numr = $this->ctype->selected == "ntext" || $this->ctype->selected == "num";
    $show_nlab = $this->ctype->selected == "num";
    $show_rows = $this->controls->selected == "select";
?>    
<table>
<tr>
  <td valign=top><strong>Question:</strong></td>
  <td>
    <div id=<?=$p?>_single style="display: <?= $show_sing ? "block" : "none" ?>">
    <?$this->question->display(false);?><br>
    </div>
  </td>
</tr>
<tr>
  <td valign=top><strong>Choices:</strong></td>
  <td>
    <?= $this->ctype->display(false, "onchange=\"${p}_refresh(this.form)\""); ?>
    <a href="javascript:showhelp('choice_type')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
 
    <? if (!$SurveyEditor_ns4 || $show_clst) { ?>
    <div id=<?=$p?>_choicelist style="display: <?= $show_clst ? "block" : "none" ?>">
    <? $this->choices->display(false); ?>
    </div>
    <? } else $this->choices->display(true); ?>

    <? if (!$SurveyEditor_ns4 || $show_txto) { ?>
    <table id=<?=$p?>_textoptions style="display: <?= $show_txto ? "block" : "none" ?>">
    <tr>
      <td><? $this->select_many->display(); ?></td> 
      <td>
        <? $this->select_many->displaylabel("Allow users to select more than one choice"); ?>
        <a href="javascript:showhelp('choice_more')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    <tr>
      <td valign=top><? $this->other->display(false,"onclick=\"${p}_refresh(this.form)\""); ?></td>
      <td>
        <? $this->other->displaylabel('Include an "other" choice'); ?>
        <a href="javascript:showhelp('choice_other')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    <? if (!$SurveyEditor_ns4 || $show_othr) { ?>
    <tr>
      <td></td>
      <td><div id=<?=$p?>_olabel style="display: <?= $show_othr ? "block" : "none" ?>"><? $this->other_label->display(); ?></div></td>
    </tr>
    <? } else $this->other_label->display(true); ?>
    </table>
    <? } else { $this->select_many->display(true); $this->other->display(true); $this->other_label->display(true); } ?>
  
    <? if (!$SurveyEditor_ns4 || $show_numr) { ?>
    <table id=<?=$p?>_numchoice style="display: <?= $show_numr ? "block" : "none" ?>">
    <tr>
      <td>First Value:</td>
      <td><? $this->first_number->display(); ?></td>
    </tr>
    <tr>
      <td>Last Value:</td>
      <td><? $this->last_number->display(); ?></td>
    </tr>
    </table>
    <? } else { $this->first_number->display(true); $this->last_number->display(true); } ?>

    <? if (!$SurveyEditor_ns4 || $show_nlab) { ?>
    <table id=<?=$p?>_sparsechoice style="display: <?= $show_nlab ? "block" : "none" ?>">
    <tr>
      <td>First Label:</td>
      <td><? $this->first_label->display(); ?></td>
    </tr>
    <tr>  
      <td>Last Label:</td>
      <td><? $this->last_label->display(); ?></td>
    </tr>
    </table>
    <? } else { $this->first_label->display(true); $this->last_label->display(true); } ?>
    
  </td> 
</tr>
<tr>
  <td valign=top><strong>Appearance:</strong></td>
  <td>&nbsp;
    <table>
    <tr>
      <td>Controls:</td>
      <td>
        <? $this->controls->display(false, "onchange=\"${p}_refresh(this.form)\""); ?>
        
        <? if (!$SurveyEditor_ns4 || $show_rows) { ?>
        <span id=<?=$p?>_selrows style="display: <?= $show_rows ? "inline" : "none" ?>">
        with
        <? $this->rows->display(); ?>
        rows
        </span>
        <? } else $this->rows->display(true); ?>
        
        <a href="javascript:showhelp('choice_comp')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
        
      </td>  
    </tr>
    </table>

  <? $this->is_html->display(false); $this->is_html->displaylabel("HTML Tags in Question Text");?>
  </td>
</tr>  
</table>
<script>
<!--

function <?=$p?>_refresh(form)
{
  if (document.all || document.getElementById)
  {
    var ctype = form.<?= $this->ctype->prefix ?>.value;
    var other = form.<?= $this->other->prefix ?>.checked;
    var cntrl = form.<?= $this->controls->prefix ?>.value;
    elem("<?=$p?>_choicelist").style.display = ctype == "text" || ctype == "ntext" ? "block" : "none";
    elem("<?=$p?>_textoptions").style.display = ctype == "text" ? "block" : "none";
    elem("<?=$p?>_numchoice").style.display = ctype == "ntext" || ctype == "num" ? "block" : "none";
    elem("<?=$p?>_sparsechoice").style.display = ctype == "num" ? "block" : "none";
    elem("<?=$p?>_selrows").style.display = cntrl == "select" ? "inline" : "none";
  }
  else
  {
    if (form.onsubmit) form.onsubmit();
    form.submit();
    return;
  }
}

// -->
</script>
<?
  }
}

class TeamWidget extends ComponentWidget
{
  var $choices;
  var $other;
  var $legend;
  
  function TeamWidget(&$c,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($c,$prefix,$form,$formmethod);
    
    // abstract away the differences between different survey types 
    // for the purpose of displaying
    
    $this->heado = $c->questions[0];
    
    $this->legend = "";
    if ($c->is_numeric)
    {
    	if ($c->choices[0] || $c->choices[1])
    	  $this->legend = $c->first_number . " = " . $c->choices[0] . ", " . $c->last_number . " = " . $c->choices[1];
    	
    	if ($c->first_number < $c->last_number)
    	  $this->choices = range($c->first_number, $c->last_number);
    	else
    	  $this->choices = array_reverse(range($c->last_number, $c->first_number));	
    }
    else
      $this->choices = $c->choices;
    
    if ($c->other_choice)
      $this->other = array_push($this->choices, $c->other_choice) - 1;
    else
      $this->other = -1;
      
    global $db,$topic_id;
    if (!isset($db)) $db = wces_connect();
    
    $this->questions = array("Yourself");
    $result = db_exec("SELECT firstname FROM tusers WHERE topicid = $topic_id ORDER BY ord LIMIT 5", $db, __FILE__, __LINE__);
    while($row = mysql_fetch_assoc($result))
    {
      $this->questions[] = $row['firstname'];
    }
      
    if (!$c->is_html)
    {
      $this->legend = nl2br(htmlspecialchars($this->legend));
      foreach($this->choices as $k => $v)
      	$this->choices[$k] = nl2br(htmlspecialchars($v));
      
      foreach($this->questions as $k => $v)
      	$this->questions[$k] = nl2br(htmlspecialchars($v));
    }
  }
  
  function loadvalues() // public abstract
  {
  }
  
  function showchoices($questionno, $tdparams = "")
  {
  	$c = &$this->component;
  	$name = $this->prefix . "_q" . $questionno;
  	if ($c->select_many) $name .= "[]";
  	
  	if ($c->stacked) print("<td $tdparams>");
  	if ($c->rows > 0)
  	{
      print("<select name=$name" . ($c->rows > 1 ? " size=" . $c->rows : "") . ($c->select_many ? " multiple" : "") . ">\n");
      foreach($this->choices as $i => $choice)
        print("  <option value=$i>$choice\n");
      print("</select>\n");
  	}
  	else
  	{
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
        
        if ($c->select_many)
        	print("<input type=checkbox name=$name value=$i id=$id>");
        else
        	print("<input type=radio name=$name value=$i id=$id>");
        	
        if (!$c->stacked)
          print(" <label for=$id>$choice</label>");
        
        if ($i == $this->other)
          print(" <input type=textbox size=20 name=$id>");
      }  
 		
    }
    if ($c->stacked) print("</td>\n");
  }
  
  function display()
  {
    global $server_media;

    $c = &$this->component;
    $n = count($this->choices);
    if ($c->stacked)
    {
?>    	
<table bordercolor=black cellspacing=0 cellpadding=3 border=0 RULES="groups" FRAME=box STYLE="border: none">

<thead>
<tr>
<td colspan=<?=$n + 1?> align=left STYLE="border: none"><b><?=$this->heado?></b></td>
</tr>
<? if ($this->legend) { ?>
<tr>
<td colspan=<?=$n + 1?> align=right STYLE="border: none"><b><?=$this->legend?></b></td>
</tr>
<? } ?>
</thead>
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
        print("$text<br>\n");
        $this->showchoices($i);
        print("<br><br>\n");
      }
    }    
  }
}

?>
<?

require_once("wbes/component.inc");
require_once("Numbers/Roman.php");

define("FLAG_ISNUMERIC",  0x1);
define("FLAG_SELECTMANY", 0x2);
define("FLAG_STACKED",    0x4);
define("FLAG_VERTICAL",   0x8);
define("FLAG_NACHOICE",   0x10);

//numbering options.
define("MASK_NUMBERING",  0x60);
define("FLAG_NONUMBERS",  0x00);
define("FLAG_ROMAN",      0x20);
define("FLAG_ARABIC",     0x40);
define("FLAG_ALPHA",      0x60);

class ChoiceQuestionFactory extends ComponentFactory
{
  function code() { return 6; }
  function name() { return null; }

  function loadComponents(&$components)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, qtext FROM choice_questions WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c =& new ChoiceQuestion;
      $c->text = $wr->row['qtext'];
      $components[$wr->row['component_id']] =& $c;
    }
  }
}

class ChoiceFactory extends ComponentFactory
{
  function code() { return 2; }
  function name() { return "Multiple Choice Question Set"; }

  function & create()
  {
    return new Choice;
  }

  function loadComponents(&$components)
  {
    global $wbes;
    $cids = implode(',', array_keys($components));
    $r = pg_go("SELECT component_id, ctext, choices, other_choice, first_number, last_number, rows, flags FROM choice_components WHERE component_id IN ($cids)", $wbes, __FILE__, __LINE__);

    for($wr = & new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c =& new Choice;
      $c->text          = $wr->row['ctext'];
      $c->choices       = pg_explode($wr->row['choices']);
      $c->other_choice  = $wr->row['other_choice'];
      $c->first_number  = (int)$wr->row['first_number'];
      $c->last_number   = (int)$wr->row['last_number'];
      $c->rows          = $wr->row['rows'];
      $c->intro_text    = $wr->row['ctext'];
      $flags        = $wr->row['flags'];
      $c->locked        = (bool)($flags & FLAG_LOCKED);
      $c->is_html       = (bool)($flags & FLAG_ISHTML);
      $c->is_numeric    = (bool)($flags & FLAG_ISNUMERIC);
      $c->select_many   = (bool)($flags & FLAG_SELECTMANY);
      $c->stacked       = (bool)($flags & FLAG_STACKED);
      $c->vertical      = (bool)($flags & FLAG_VERTICAL);
      $c->na_choice     = (bool)($flags & FLAG_NACHOICE);
      $c->numbering_scheme = $flags & MASK_NUMBERING;
      $components[$wr->row['component_id']] =& $c;
    }
  }
}

class Choice extends Component
{
  // array of ChoiceQuestion objects
  var $questions = array();

  // array of stringed choices
  var $choices = array();
  var $first_number = 0;
  var $last_number = 0;

  var $is_numeric = false;
  var $select_many = false;
  var $other_choice = "";

  //added N/A choice [tyler]
  var $na_choice = false;

  //added intro text [tyler]
  var $intro_text = "";

  //added numbering options [tyler]
  var $numbering_scheme = "none";

  var $stacked = false;
  var $vertical = false;
  var $rows = 0;

  function & getChildren()
  {
    return $this->questions;
  }

  function Choice()
  {
    $this->Component();
  }

  function & geteditor($name, &$parent)
  {
    return new ChoiceEditor($this, $name, $parent);
  }

  function & getwidget($name, &$parent)
  {
    return new ChoiceWidget($this, $name, $parent);
  }

  function saveComponent($component_id, $children)
  {
    $ctext        = addslashes($this->intro_text);
    $choices      = pg_implode($this->choices);
    $other_choice = addslashes($this->other_choice);
    $first_number = $this->first_number;
    $last_number  = $this->last_number;
    $rows         = $this->rows;

    $flags = (int) $this->numbering_scheme;
    if ($this->is_numeric)  $flags |= FLAG_ISNUMERIC;
    if ($this->select_many) $flags |= FLAG_SELECTMANY;
    if ($this->stacked)     $flags |= FLAG_STACKED;
    if ($this->vertical)    $flags |= FLAG_VERTICAL;
    if ($this->na_choice)   $flags |= FLAG_NACHOICE;
    if ($this->locked)      $flags |= FLAG_LOCKED;
    if ($this->is_html)     $flags |= FLAG_ISHTML;

    return "choice_component_save($component_id,'$ctext',$children,$choices,'$other_choice',$first_number,$last_number,$flags,$rows)";
  }
};

class ChoiceEditor extends ComponentEditor
{
  // widgets
  var $qtype;
  var $question;
  var $questions;
  var $ctype;
  var $choices;
  var $select_many;
  var $other;
  var $na;
  var $intro;
  var $other_label;
  var $first_number;
  var $last_number;
  var $first_label;
  var $last_label;
  var $numbering_scheme;
  var $layout;
  var $controls;
  var $rows;
  var $orientation;

  function ChoiceEditor(&$component, $name, &$parent)
  {
    DynamicList::DumpScript();

    $this->ComponentEditor($component, $name, $parent);
    $this->qtype = & new DropBox(array(
      ""         => "-- Select One --",
      "single"   => "Single Question",
      "multiple" => "Multiple Questions"
    ), "qtype", $this);

    $this->question = & new TextBox(4, 40, "wrap=virtual", "question", $this);
    $this->questions = & new DynamicList(30, 7, false, "questions", $this);
    $this->question_id = "";

    $this->ctype = & new DropBox(array(
      ""         => "-- Select One --",
      "text"     => "Text Choices",
      "ntext"    => "Quantitative Text Choices",
      "num"      => "Numeric Choices"
    ), "ctype", $this);

    $this->choices = & new DynamicList(30, 7, false, "choices", $this);
    $this->select_many = & new CheckBox("selectmany", $this);
    $this->other = & new CheckBox("other", $this);

    //Added to support N/A option. [tyler]
    $this->na = & new CheckBox("na", $this);

    //Added to support intro text. [tyler]
    $this->intro = & new TextBox(0, 40, "", "intro", $this);

    $this->other_label = & new TextBox(0, 25, "", "otherlabel", $this);
    $this->first_number = & new TextBox(0, 5, "", "firstnumber", $this);
    $this->last_number = & new TextBox(0, 5, "", "lastnumber", $this);
    $this->first_label = & new TextBox(0, 20, "", "firstlabel", $this);
    $this->last_label = & new TextBox(0, 20, "", "lastlabel", $this);

    //added numbering support. [tyler]
    $this->numbering_scheme = & new DropBox(array(
      FLAG_NONUMBERS => "None",
      FLAG_ARABIC => "Arabic numbers",
      FLAG_ALPHA => "Letters",
      FLAG_ROMAN => "Roman Numerals"
    ), "numbering", $this);

    $this->layout = & new DropBox(array(
      "under" => "Put choices underneath the question text",
      "right" => "Put choices to the right of the question text"
    ), "layout", $this);

    $this->controls = & new DropBox(array(
      "buttons" => "Checkboxes",
      "select"  => "List boxes",
      "drop"    => "Drop-down lists"
    ), "controls", $this);

    $this->rows = & new DropBox(array(
      "2"  => "2",
      "3"  => "3",
      "4"  => "4",
      "5"  => "5",
      "6"  => "6",
      "7"  => "7",
      "8"  => "8",
      "9"  => "9",
      "10" => "10"
    ), "rows", $this);

    $this->orientation = & new DropBox(array(
      "vertical"  => "vertically",
      "horiz"  => "horizontally"
    ), "orientation", $this);
  }

  function loadInitialState()
  {
    $c = count($this->component->questions);

    if ($c >= 1)
    {
      $this->question->text = $this->component->questions[0]->text;
      $questions = array();
      foreach ($this->component->questions as $question)
        $questions[] = $question->text;
      $this->questions->items = $questions;
      $this->questions->keys = range(0, $c-1);
      $this->qtype->selected = $c == 1 ? "single" : "multiple";
    }
    else
      $this->qtype->selected = "";

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

    //added to support N/A values. [tyler]
    $this->na->checked = $this->component->na_choice;

    //added to support intro text. [tyler]
    $this->intro->text = $this->component->intro_text;

    //added to support numbering. [tyler]
    $this->numbering_scheme->selected = $this->component->numbering_scheme;

    //added to support locking. [tyler]
    $this->locking->checked = $this->component->locked;

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
      if (is_numeric($this->component->first_number) && is_numeric($this->component->last_number) && $this->component->first_number != $this->component->last_number)
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
    $this->layout->selected = $this->component->stacked ? "right" : "under";
    $r = $this->component->rows;

    if ($r == 0)
      $this->controls->selected = "buttons";
    else if ($r == 1)
      $this->controls->selected = "drop";
    else
      $this->controls->selected = "select";

    $this->rows->selected = $r;

    $this->orientation->selected = $this->component->vertical ? "vertical" : "horiz";
  }

  function loadState()
  {
    ComponentEditor::loadState();
    $this->question_id = (int)$this->readValue("question_id");
  }

  function validate()
  {
    if ($this->qtype->selected == "single")
    {
      if (strlen($this->question->text) == 0)
      {
        $this->errors[] = "You did not enter any question text.";
        $this->qtype->broken = true;
      }
    }
    else if ($this->qtype->selected == "multiple")
    {
      if (count($this->questions->items) == 0)
      {
        $this->errors[] = "You did not add any questions.";
        $this->qtype->broken = true;
      }
    }
    else
    {
      $this->errors[] = "You did not choose a question type.";
      $this->qtype->broken = true;
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
        $this->errors[] = 'You did not specify a label for the "other" choice.';
    }

    return count($this->errors) == 0;
  }

  function save()
  {
    $c = &$this->component;

    if ($this->qtype->selected == "single")
    {
      $q = isset($c->questions[0]) ? $c->questions[0] : new ChoiceQuestion;
      $q->text = $this->question->text;
      $c->questions = array($q);
      $this->numbering_scheme->selected = FLAG_NONUMBERS;
    }
    else if ($this->qtype->selected == "multiple")
    {
      $questions = array();
      foreach ($this->questions->keys as $li => $qi)
      {
        $q = isset($c->questions[$qi]) ? $c->questions[$qi] : new ChoiceQuestion;
        $q->text = $this->questions->items[$li];
        $questions[] = $q;
      }
      $c->questions = $questions;
    }


    if ($this->ctype->selected == "num")
      $c->choices = array($this->first_label->text,$this->last_label->text);
    else
      $c->choices = $this->choices->items;

    if ($this->ctype->selected == "text")
    {
      $c->select_many = $this->select_many->checked;
      $c->other_choice = $this->other->checked ? $this->other_label->text : "";
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
    $c->stacked = $this->layout->selected == "right";

    //added to support N/A option. [tyler]
    $c->na_choice = $this->na->checked;

    //added to support intro text. [tyler]
    $c->intro_text = $this->intro->text;

    //added to support numbering. [tyler]
    $c->numbering_scheme = $this->numbering_scheme->selected;

    //added locking support
    $c->locked = $this->locking->checked;

    if ($this->controls->selected == "buttons")
      $c->rows = 0;
    else if ($this->controls->selected == "drop")
      $c->rows = 1;
    else if ($this->controls->selected == "select")
      $c->rows = (int) $this->rows->selected;

    $this->component->vertical = $this->controls->selected == "buttons" && $this->orientation->selected == "vertical";

    ComponentEditor::save();
  }

  function printVisible()
  {
    global $SurveyEditor_ns4, $server_media;
    $this->printErrors();

    $show_sing = $this->qtype->selected == "single";
    $show_mult = $this->qtype->selected == "multiple";
    $show_clst = $this->ctype->selected == "text" || $this->ctype->selected == "ntext";
    $show_txto = $this->ctype->selected == "text";
    $show_othr = $this->other->checked;
    $show_numr = $this->ctype->selected == "ntext" || $this->ctype->selected == "num";
    $show_nlab = $this->ctype->selected == "num";
    $show_coun = $this->qtype->selected == "multiple";
    $show_srow = $this->controls->selected == "select";
    $show_ornt = $this->controls->selected == "buttons";

    $refreshFunction = $this->scriptName("refresh");

    if (!$SurveyEditor_ns4)
    {
?>
<script>
function <?=$refreshFunction?>(form)
{
  var qtype = form['<?=$this->qtype->name?>'].value;
  var ctype = form['<?=$this->ctype->name?>'].value;
  var other = form['<?=$this->other->name?>'].checked;
  var controls = form['<?=$this->controls->name?>'].value;
  elem('<?=$this->fullName("sing")?>').style.display = qtype == 'single' ? 'block' : 'none';
  elem('<?=$this->fullName("mult")?>').style.display = qtype == 'multiple' ? 'block' : 'none';
  elem('<?=$this->fullName("clst")?>').style.display = ctype == 'text' || ctype == 'ntext' ? 'block' : 'none';
  elem('<?=$this->fullName("txto")?>').style.display = ctype == 'text' ? 'block' : 'none';
  elem('<?=$this->fullName("othr")?>').style.display = other ? 'block' : 'none';
  elem('<?=$this->fullName("numr")?>').style.display = ctype == 'ntext' || ctype == 'num' ? 'block' : 'none';
  elem('<?=$this->fullName("nlab")?>').style.display = ctype == 'num' ? 'block' : 'none';
  elem('<?=$this->fullName("coun")?>').style.display = qtype == 'multiple' ? 'block' : 'none';
  elem('<?=$this->fullName("srow")?>').style.display = controls == 'select' ? 'inline' : 'none';
  elem('<?=$this->fullName("ornt")?>').style.display = controls == 'buttons' ? 'inline' : 'none';
}
</script>
<?
   }
   else
   {
?>
<script>
function <?=$refreshFunction?>(form)
{
  if (form.onsubmit) form.onsubmit();
  form.submit();
  return;
}
</script>
<?
   }
?>
<table>
<? /* added to support intro text. [tyler] */ ?>
<tr>
    <td colspan><strong>Introduction: </strong></td>
    <td><?= $this->intro->display(); ?> (optional)</td>
</tr>
<tr>
  <td valign=top><strong>Questions:</strong></td>
  <td>
    <? $this->qtype->display("onchange=\"$refreshFunction(this.form)\""); ?>
    <a href="javascript:showhelp('choice_mult')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>

    <? if (!$SurveyEditor_ns4 || $show_sing) { ?>
    <div id=<?=$this->fullName("sing")?> style="display: <?= $show_sing ? "block" : "none" ?>">
    <?$this->question->display(); $this->printValue("question_id", $this->question_id); ?>
    </div>
    <? } else $this->question->displayHidden(); ?>

    <? if (!$SurveyEditor_ns4 || $show_mult) { ?>
    <div id=<?=$this->fullName("mult")?> style="display: <?= $show_mult ? "block" : "none" ?>">
    <?$this->questions->display();?>
    </div>
    <? } else $this->questions->displayHidden(); ?>

  </td>
</tr>
<tr>
  <td valign=top><strong>Choices:</strong></td>
  <td>
    <?= $this->ctype->display("onchange=\"$refreshFunction(this.form)\""); ?>
    <a href="javascript:showhelp('choice_type')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
    <? if (!$SurveyEditor_ns4 || $show_clst) { ?>
    <div id=<?=$this->fullName("clst")?> style="display: <?= $show_clst ? "block" : "none" ?>">
    <? $this->choices->display(); ?>
    </div>
    <? } else $this->choices->displayHidden(); ?>

    <? if (!$SurveyEditor_ns4 || $show_txto) { ?>
    <table id=<?=$this->fullName("txto")?> style="display: <?= $show_txto ? "block" : "none" ?>">
    <tr>
      <td><? $this->select_many->display(); ?></td>
      <td>
        <? $this->select_many->displaylabel("Allow users to select more than one choice"); ?>
        <a href="javascript:showhelp('choice_more')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    <tr>
      <td valign=top><? $this->other->display("onclick=\"$refreshFunction(this.form)\""); ?></td>
      <td>
        <? $this->other->displayLabel('Include an "other" choice'); ?>
        <a href="javascript:showhelp('choice_other')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    <? if (!$SurveyEditor_ns4 || $show_othr) { ?>
    <tr>
      <td></td>
      <td><div id=<?=$this->fullName("othr")?> style="display: <?= $show_othr ? "block" : "none" ?>"><? $this->other_label->display(); ?></div></td>
    </tr>
    <? } else $this->other_label->displayHidden(); ?>
    </table>
    <? } else { $this->select_many->displayHidden(); $this->other->displayHidden(); $this->other_label->displayHidden(); } ?>

    <? if (!$SurveyEditor_ns4 || $show_numr) { ?>
    <table id=<?=$this->fullName("numr")?> style="display: <?= $show_numr ? "block" : "none" ?>">
    <tr>
      <td>First Value:</td>
      <td><? $this->first_number->display(); ?></td>
    </tr>
    <tr>
      <td>Last Value:</td>
      <td><? $this->last_number->display(); ?></td>
    </tr>
    </table>
    <? } else { $this->first_number->displayHidden(); $this->last_number->displayHidden(); } ?>

    <? if (!$SurveyEditor_ns4 || $show_nlab) { ?>
    <table id=<?=$this->fullName("nlab")?> style="display: <?= $show_nlab ? "block" : "none" ?>">
    <tr>
      <td>First Label:</td>
      <td><? $this->first_label->display(); ?></td>
    </tr>
    <tr>
      <td>Last Label:</td>
      <td><? $this->last_label->display(); ?></td>
    </tr>
    </table>
    <? } else { ?>
    <div><? $this->first_label->displayHidden(); $this->last_label->displayHidden(); ?></div>
    <? } ?>

    <? /* added to support N/A option */ ?>
    <div><? $this->na->display(); $this->na->displayLabel('Allow N/A choice')?></div>

    <? /* Locking support. [tyler]*/ ?>
    <div><? $this->locking->display(); $this->locking->displayLabel('Lock this component'); ?></div>
  </td>
</tr>
<tr>
  <td valign=top><strong>Appearance:</strong></td>
  <td>
    <? if (!$SurveyEditor_ns4 || $show_coun) { ?>
    <div id=<?=$this->fullName("coun")?> style="display: <?= $show_coun ? "block" : "none" ?>">
    Question Numbering: <? $this->numbering_scheme->display(); ?>
    </div>
    <? } else { $this->numbering_scheme->displayHidden(); } ?>
    <table>
    <tr>
      <td>Layout:</td>
      <td>
        <? $this->layout->display(); ?>
        <a href="javascript:showhelp('choice_where')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    <tr>
      <td>Controls:</td>
      <td>
        <? $this->controls->display("onchange=\"$refreshFunction(this.form)\""); ?>

        <? if (!$SurveyEditor_ns4 || $show_srow) { ?>
        <span id=<?=$this->fullName("srow")?> style="display: <?= $show_srow ? "inline" : "none" ?>">
        with
        <? $this->rows->display(); ?>
        rows
        </span>
        <? } else $this->rows->displayHidden(); ?>

        <? if (!$SurveyEditor_ns4 || $show_ornt) { ?>
        <span id=<?=$this->fullName("ornt")?> style="display: <?= $show_ornt ? "inline" : "none" ?>">
        oriented
        <? $this->orientation->display(); ?>
        </span>
        <? } else $this->orientation->displayHidden(); ?>

        <a href="javascript:showhelp('choice_comp')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
    </table>

  <? $this->is_html->display(); $this->is_html->displayLabel("HTML Tags in Question Text");?>
  </td>
</tr>
</table>
<?
  }
}

class ChoiceWidget extends ComponentWidget
{
  // persistent
  var $answers = NULL;
  var $others = NULL;
  var $errors = NULL;

  // private
  var $legend;
  var $choices;
  var $other;

  function ChoiceWidget(&$c, $name, &$parent)
  {
    $this->ComponentWidget($c, $name, $parent);

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

    if($c->na_choice)
      $this->na = array_push($this->choices, "N/A") - 1;

    if ($c->other_choice)
      $this->other = array_push($this->choices, $c->other_choice) - 1;
    else
      $this->other = -1;

    $this->questions = array();
    foreach($c->questions as $k => $v)
      $this->questions[$k] = nl2br(htmlspecialchars($v->text));
    
    if (!$c->is_html)
    {
      $this->legend = nl2br(htmlspecialchars($this->legend));
      foreach($this->choices as $k => $v)
        $this->choices[$k] = nl2br(htmlspecialchars($v));

      foreach($this->questions as $k => $v)
        $this->questions[$k] = nl2br(htmlspecialchars($v));
    }
  }

  function loadState()
  {
    ComponentWidget::loadState();
    foreach ($this->questions as $i => $text)
    {
      $c = &$this->component;
      $name = "q$i";
      $a = $this->readValue($name);

      if ($c->select_many)
        $this->answers[$i] = is_array($a) ? $a : NULL;
      else if (isset($a))
        $this->answers[$i] = (int)$a;
      else
      {
        $this->answers[$i] = NULL;
        $this->errors[$i] = "Question '$text' is not filled in.";
      }
      $this->others[$i] = (string)$this->readValue("q{$i}_{$this->other}");
    }
  }

  function save($parent, $topic_id)
  {
    global $wbes;

    $result = pg_go("
      INSERT INTO choice_responses(item_id, revision_id, parent)
      VALUES ({$this->component->item_id}, {$this->component->revision_id}, $parent);
      SELECT currval('response_ids');
    ", $wbes, __FILE__, __LINE__);

    if (!$result) return false;

    $nparent = (int)pg_result($result, 0, 0);
    $table = $this->component->select_many ? "mchoice_question_responses" : "choice_question_responses";
    $success = true;

    foreach(array_keys($this->questions) as $i)
    {
      $other = addslashes($this->others[$i]);
      $a = &$this->answers[$i];

      if ($other || isset($a))
      {
        if (!isset($a))
          $answer = "NULL";
        else if ($this->component->select_many)
          $answer = pg_implode($a);
        else
          $answer = quot($a);
          
        $q =& $this->component->questions[$i];

        if (0 && isset($a))
        {
          $cast = $this->component->select_many ? '::integer[]' : '::integer';
          $cache_update = "select cached_choice_responses_add ($topic_id, "
            . "{$this->component->item_id}, {$this->component->revision_id}, "
            . "$q->item_id, $q->revision_id, $answer$cast);";
        }
        else
          $cache_update = '';

        $success = $success && pg_go("
          INSERT INTO $table(item_id, revision_id, parent, answer, other)
          VALUES ($q->item_id, $q->revision_id, $nparent, $answer, '$other');
          $cache_update
        ", $wbes, __FILE__, __LINE__);
      }
    }
    return $success;
  }

  function printChoices($questionno, $tdparams = "")
  {
    $c = &$this->component;
    $a = &$this->answers[$questionno];
    $name = $mname = $this->fullName("q$questionno");
    if ($c->select_many) $mname .= "[]";

    if ($c->stacked && !$c->vertical) print("<td $tdparams>");
    if ($c->rows > 0)
    {
      print("<select name=$mname" . ($c->rows > 1 ? " size=" . $c->rows : "") . ($c->select_many ? " multiple" : "") . ">\n");
      foreach($this->choices as $i => $choice)
      {
        $selected = (!$c->select_many && $i === $a) || ($c->select_many && is_array($a) && in_array($i, $a)) ? " selected" : "";
        print("  <option value=$i$selected>$choice\n");
      }
      print("</select>\n");
    }
    else
    {
      if ($c->vertical)
        $sep = "\n";
      else if ($c->stacked)
        $sep = "</td>\n<td $tdparams>";
      else
        $sep = "\n";

      $first = true;
      foreach($this->choices as $i => $choice)
      {
        if ($first) $first = false; else print($sep);

        if ($c->vertical)
          print('<div>');

        $id = "{$name}_{$i}";

        if ($c->select_many)
        {
          $checked = is_array($a) && in_array($i, $a) ? " checked" : "";
          print("<input type=checkbox name=$mname value=$i id=$id$checked>");
        }
        else
        {
          $checked = $i === $a ? " checked" : "";
          print("<input type=radio name=$mname value=$i id=$id$checked>");
        }

        if (!$c->stacked || $c->vertical)
          print(" <label for=$id>$choice</label>");

        if ($i == $this->other)
          print(" <input type=textbox size=20 name=$id value=\"" . htmlspecialchars($this->others[$questionno]) . "\">");

        if ($c->vertical)
          print('</div>');
      }
    }
    if ($c->stacked && !$c->vertical) print("</td>\n");
  }

  function printNumber($i)
  {
    if($this->component->numbering_scheme == FLAG_ARABIC)
      print(($i+1) . ".  ");
    else if($this->component->numbering_scheme == FLAG_ALPHA)
    {
      $numLetters = $i / 26;
      $letter = $i % 26;

      $letters = "abcdefghijklmnopqrstuvwxyz";

      for($z = 0; $z <= $numLetters; $z++)
          {print(substr($letters, $letter, 1));}

      print(".  ");
    }
    else if($this->component->numbering_scheme == FLAG_ROMAN)
      print(Numbers_Roman::toNumeral($i+1) . ".  ");
  }

  function printHidden()
  {
    if (isset($this->answers))
    foreach($this->answers as $i => $v)
    {
      $name = "q$i";
      if (is_array($v))
        $this->printArrayValue($name, $v);
      else
        $this->printValue($name, $v);
    }

    if (isset($this->others))
    foreach($this->others as $i => $v)
      $this->printValue("q{$i}_{$this->other}", $v);
  }

  function printVisible()
  {
    global $server_media;

    $c = &$this->component;
    $n = count($this->choices);

    //added to display intro text. [tyler]
    if ($c->intro_text) print("<h3>{$c->intro_text}</h3>");

    if ($c->stacked && !$c->vertical)
    {
?>

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
        if (isset($this->errors[$i]) && $this->showErrors) $text = "<b><font color=red>$text</font></b>";
        print("<tr>\n<td $color>");

        //added support for numbering. [tyler]
        $this->printNumber($i);

        print($text . "</td>\n");
        $this->printChoices($i, "$color align=center");
        print("</tr>");
      }
      print("\n</tbody>\n</table>\n");
     } // if ($c->stacked && !$c->vertical)
    else if($c->stacked && $c->vertical) // another addition [tyler]
    {
      if ($this->legend) print("<p><strong>" . $this->legend . "</strong></p>\n");
      print("<table border=1 cellspacing=3 cellpadding=3 width=100%>");
      foreach ($this->questions as $i => $text)
      {
        if (isset($this->errors[$i]) && $this->showErrors) $text = "<b><font color=red>$text</font></b>";
        print("<tr>");
        print("<td valign=top>");

        //added support for numbering. [tyler]
        $this->printNumber($i);

        print($text . "</td>\n<td>");
        $this->printChoices($i);
        print("</td>\n");
        print("</tr>");
      }
      print("</table>");
    }
    else
    {
      if ($this->legend) print("<p><strong>" . $this->legend . "</strong></p>\n");
      foreach ($this->questions as $i => $text)
      {
        //added support for numbering. [tyler]
        $this->printNumber($i);
        if (isset($this->errors[$i]) && $this->showErrors) $text = "<b><font color=red>$text</font></b>";
        print("$text<br>\n");
        $this->printChoices($i);
        print("<br><br>\n");
      }
    }
  }
}

class ChoiceQuestion extends Component
{
  var $text;
  var $revision_id;
  var $item_id;

  function getChildren()
  {
    return null;
  }

  function saveComponent($component_id, $children)
  {
    $text = addslashes($this->text);
    return "choice_question_save($component_id, '$text')";
  }

}

?>

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

class ChoiceFactory extends ComponentFactory
{
  function code() { return 2; }
  function name() { return "Multiple Choice Question Set"; }

  function create()
  {
    return new Choice();
  }

  function load(&$components, $ids, $ordinals, $topic_id, $latest)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, ctext, choices, other_choice, first_number, last_number, rows, flags FROM choice_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new Choice();
      $c->revision_id   = (int)$wr->row['revision_id'];
      $c->text          = $wr->row['ctext'];
      $c->choices       = pg_explode($wr->row['choices']);
      $c->other_choice  = $wr->row['other_choice'];
      $c->first_number  = (int)$wr->row['first_number'];
      $c->last_number   = (int)$wr->row['last_number'];
      $c->rows          = $wr->row['rows'];

      //added to support intro text.
      $c->intro_text    = $wr->row['ctext'];

      $flagGroup        = $wr->row['flags'];
      $c->locked        = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html       = (bool)($flagGroup & FLAG_ISHTML);
      $c->is_numeric    = (bool)($flagGroup & FLAG_ISNUMERIC);
      $c->select_many   = (bool)($flagGroup & FLAG_SELECTMANY);
      $c->stacked       = (bool)($flagGroup & FLAG_STACKED);
      $c->vertical      = (bool)($flagGroup & FLAG_VERTICAL);
      $c->na_choice     = (bool)($flagGroup & FLAG_NACHOICE);
      $c->numbering_scheme = $flagGroup & MASK_NUMBERING;

      // todo: optimize. these queries can be performed once outside of the for loop

      $result = pg_query("
        CREATE TEMPORARY TABLE tmp$c->revision_id AS
        SELECT branch_latest(bt.branch_id, $latest) AS revision_id, ordinal
        FROM list_items AS li
        INNER JOIN branch_topics_cache AS bt ON bt.base_branch_id = li.item_id AND bt.topic_id = $topic_id
        WHERE li.revision_id = (SELECT revision_contents($c->revision_id));

        SELECT t.ordinal, q.revision_id, q.qtext
        FROM tmp$c->revision_id AS t INNER JOIN choice_questions AS q USING (revision_id)
        ORDER BY ordinal
      ", $wbes, __FILE__, __LINE__);

      pg_query("DROP TABLE tmp$c->revision_id", $wbes, __FILE__, __LINE__);

      $n = pg_numrows($result);

      $c->questions = $c->question_ids = array();
      for($i = 0; $i < $n; ++$i)
      {
        $a = pg_fetch_array($result,$i,PGSQL_ASSOC);
        $c->questions[] = $a['qtext'];
        $c->question_ids[] = $a['revision_id'];
      }

      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
}

class Choice extends Component
{
  // array of stringed items to rate
  var $questions = array();
  var $question_ids = array();

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

  function Choice()
  {
    $this->Component();
  }

  function geteditor($prefix,$form,$formmethod)
  {
    return new ChoiceEditor($this,$prefix,$form,$formmethod);
  }

  function getwidget($prefix,$form,$formmethod)
  {
    return new ChoiceWidget($this,$prefix,$form,$formmethod);
  }

  function save($topic_id, $save_id)
  {
    global $wbes;
    $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';
    $ctext        = addslashes($this->intro_text);
    $questions    = pg_implode($this->questions);
    $question_ids = pg_implode($this->question_ids);
    $choices      = pg_implode($this->choices);
    $other_choice = addslashes($this->other_choice);
    $first_number = $this->first_number;
    $last_number  = $this->last_number;
    $is_numeric   = $this->is_numeric ? "'t'" : "'f'";
    $select_many  = $this->select_many ? "'t'" : "'f'";
    $stacked      = $this->stacked ? "'t'" : "'f'";
    $vertical     = $this->vertical ? "'t'" : "'f'";
    $rows         = $this->rows;

    $flagGroup = (int) $this->numbering_scheme;
    if ($this->is_numeric)  $flagGroup |= FLAG_ISNUMERIC;
    if ($this->select_many) $flagGroup |= FLAG_SELECTMANY;
    if ($this->stacked)     $flagGroup |= FLAG_STACKED;
    if ($this->vertical)    $flagGroup |= FLAG_VERTICAL;
    if ($this->na_choice)   $flagGroup |= FLAG_NACHOICE;
    if ($this->locked)      $flagGroup |= FLAG_LOCKED;
    if ($this->is_html)     $flagGroup |= FLAG_ISHTML;

    $result = pg_query("
      SELECT choice_component_save($topic_id, $revision_id, $save_id, '$ctext',
        $questions::text[], $question_ids::integer[],
        $choices::text[], '$other_choice'::text, $first_number, $last_number,
        $flagGroup, $rows)
    ", $wbes, __FILE__, __LINE__);
    return pg_result($result, 0, 0);
  }
};

class ChoiceEditor extends ComponentEditor
{
  //widgets

  var $qtype;
  var $question;
  var $question_id;
  var $questions;
  var $ctype;
  var $choices;
  var $select_many;
  var $other;

  //added N/A choice. [tyler]
  var $na;

  //added intro text. [tyler]
  var $intro;

  //added support for numbering. [tyler]
  var $numbering_scheme;

  //added locking [tyler]
  var $locking;

  var $other_label;
  var $first_number;
  var $last_number;
  var $first_label;
  var $last_label;
  var $layout;
  var $controls;
  var $rows;
  var $orientation;

  function ChoiceEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentEditor($component,$prefix,$form,$formmethod);

    $this->qtype = new DropBox(array(
      ""         => "-- Select One --",
      "single"   => "Single Question",
      "multiple" => "Multiple Questions"
    ), $this->prefix."_qtype", $this->form, $this->formmethod);

    $this->question = new TextBox(4,40,"wrap=virtual", $this->prefix."_question", $this->form, $this->formmethod);
    $this->questions = new DynamicList(30,7,false,$this->prefix."_questions",$this->form,$this->formmethod);
    $this->question_id = "";

    $this->ctype = new DropBox(array(
      ""         => "-- Select One --",
      "text"     => "Text Choices",
      "ntext"    => "Quantitative Text Choices",
      "num"      => "Numeric Choices"
    ), $this->prefix."_ctype", $this->form, $this->formmethod);

    $this->choices = new DynamicList(30,7,false,$this->prefix."_choices",$this->form,$this->formmethod);
    $this->select_many = new CheckBox($this->prefix."_selectmany", $this->form, $this->formmethod);
    $this->other = new CheckBox($this->prefix."_other", $this->form, $this->formmethod);

    //Added to support N/A option. [tyler]
    $this->na = new CheckBox($this->prefix."_na", $this->form, $this->formmethod);

    //Added to support intro text. [tyler]
    $this->intro = new TextBox(0,40,"",$this->prefix."_intro",$this->form,$this->formmethod);

    $this->other_label = new TextBox(0,25,"",$this->prefix."_otherlabel",$this->form,$this->formmethod);
    $this->first_number = new TextBox(0,5,"",$this->prefix."_firstnumber",$this->form,$this->formmethod);
    $this->last_number = new TextBox(0,5,"",$this->prefix."_lastnumber",$this->form,$this->formmethod);
    $this->first_label = new TextBox(0,20,"",$this->prefix."_firstlabel",$this->form,$this->formmethod);
    $this->last_label = new TextBox(0,20,"",$this->prefix."_lastlabel",$this->form,$this->formmethod);

    //added numbering support. [tyler]
    $this->numbering_scheme = new DropBox(array(
      FLAG_NONUMBERS => "None",
      FLAG_ARABIC => "Arabic numbers",
      FLAG_ALPHA => "Letters",
      FLAG_ROMAN => "Roman Numerals"
    ),$this->prefix."_numbering", $this->form, $this->formmethod);

    $this->layout = new DropBox(array(
      "under" => "Put choices underneath the question text",
      "right" => "Put choices to the right of the question text"
    ), $this->prefix."_layout", $this->form, $this->formmethod);

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

    $this->orientation = new DropBox(array(
      "vertical"  => "vertically",
      "horiz"  => "horizontally"
    ), $this->prefix . "_orientation", $this->form, $this->formmethod);
  }

  function loadvalues()
  {
    ComponentEditor::loadvalues();

    if ($this->loadprevious)
    {
      $this->qtype->loadvalues();
      $this->question->loadvalues();
      $this->questions->loadvalues();
      $this->ctype->loadvalues();
      $this->choices->loadvalues();
      $this->select_many->loadvalues();
      $this->other->loadvalues();

      //added to support N/A values. [tyler]
      $this->na->loadvalues();

      //added to support intro text. [tyler]
      $this->intro->loadvalues();

      //added to support numbering. [tyler]
      $this->numbering_scheme->loadvalues();

      $this->other_label->loadvalues();
      $this->first_number->loadvalues();
      $this->last_number->loadvalues();
      $this->first_label->loadvalues();
      $this->last_label->loadvalues();
      $this->layout->loadvalues();
      $this->controls->loadvalues();
      $this->rows->loadvalues();
      $this->orientation->loadvalues();
      $this->question_id = (int)$this->loadattribute("question_id");
    }
    else
    {
      $c = count($this->component->questions);

      if ($c >= 1)
      {
        $this->question->text = $this->component->questions[0];
        $this->question_id = $this->component->question_ids[0];
        $this->questions->items = $this->component->questions;
        $this->questions->keys = $this->component->question_ids;
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
  }

  function checkvalues()
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

  function savevalues()
  {
    $c = &$this->component;

    if ($this->qtype->selected == "single")
    {
      $c->questions = array($this->question->text);
      $c->question_ids = array($this->question_id);
    }
    else if ($this->qtype->selected == "multiple")
    {
      $c->questions = $this->questions->items;
      $c->question_ids = $this->questions->keys;
    }

    if ($this->ctype->selected == "text" || $this->ctype->selected == "ntext")
      $c->choices = $this->choices->items;
    else if ($this->ctype->selected == "num")
      $c->choices = array($this->first_label->text,$this->last_label->text);

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

    ComponentEditor::savevalues();
  }

  function display()
  {
    global $SurveyEditor_ns4, $server_media;

    ComponentEditor::display();
    $p = $this->prefix;

    DynamicList::dumpscript();

    $show_sing = $this->qtype->selected == "single";
    $show_mult = $this->qtype->selected == "multiple";
    $show_clst = $this->ctype->selected == "text" || $this->ctype->selected == "ntext";
    $show_txto = $this->ctype->selected == "text";
    $show_othr = $this->other->checked;
    $show_numr = $this->ctype->selected == "ntext" || $this->ctype->selected == "num";
    $show_nlab = $this->ctype->selected == "num";
    $show_rows = $this->controls->selected == "select";

    //modified [tyler]
    $show_ornt = $this->controls->selected == "buttons";

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
    <? $this->qtype->display(false, "onchange=\"${p}_refresh(this.form)\""); ?>
    <a href="javascript:showhelp('choice_mult')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
    <br>

    <? if (!$SurveyEditor_ns4 || $show_sing) { ?>
    <div id=<?=$p?>_single style="display: <?= $show_sing ? "block" : "none" ?>">
    <?$this->question->display(false); $this->printattribute("question_id", $this->question_id); ?><br>
    </div>
    <? } else $this->question->display(true); ?>

    <? if (!$SurveyEditor_ns4 || $show_mult) { ?>
    <div id=<?=$p?>_multiple style="display: <?= $show_mult ? "block" : "none" ?>">
    <?$this->questions->display(false);?>
    </div>
    <? } else $this->questions->display(true); ?>

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


    <? /* added to support N/A option */ ?>
    <?= $this->na->display(); ?> Allow N/A choice <br>
    
    <? /* Locking support. [tyler]*/ ?>
    <?= $this->locking->display();?> Lock this component. <br>

  </td>
</tr>
<tr>
  <td valign=top><strong>Appearance:</strong></td>
  <td>Question Numbering: <? $this->numbering_scheme->display(); ?><br>
    <table>
    <tr>
      <td>Layout:</td>
      <td>
        <? $this->layout->display(false, "onchange=\"${p}_refresh(this.form)\""); ?>
        <a href="javascript:showhelp('choice_where')"><img src="<?=$server_media?>/help/question.gif" width=16 height=16 border=0 align=absmiddle alt=help></a>
      </td>
    </tr>
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

        <? if (!$SurveyEditor_ns4 || $show_ornt) { ?>
        <span id=<?=$p?>_borient style="display: <?= $show_ornt ? "inline" : "none" ?>">
        oriented
        <? $this->orientation->display(); ?>
        </span>
        <? } else $this->orientation->display(true); ?>

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
    var qtype = form.<?= $this->qtype->prefix ?>.value;
    var ctype = form.<?= $this->ctype->prefix ?>.value;
    var other = form.<?= $this->other->prefix ?>.checked;
    var cntrl = form.<?= $this->controls->prefix ?>.value;
    var lyout = form.<?= $this->layout->prefix ?>.value;
    elem("<?=$p?>_single").style.display = qtype == "single" ? "block" : "none";
    elem("<?=$p?>_multiple").style.display = qtype == "multiple" ? "block" : "none";
    elem("<?=$p?>_choicelist").style.display = ctype == "text" || ctype == "ntext" ? "block" : "none";
    elem("<?=$p?>_textoptions").style.display = ctype == "text" ? "block" : "none";
    elem("<?=$p?>_numchoice").style.display = ctype == "ntext" || ctype == "num" ? "block" : "none";
    elem("<?=$p?>_sparsechoice").style.display = ctype == "num" ? "block" : "none";
    elem("<?=$p?>_selrows").style.display = cntrl == "select" ? "inline" : "none";
    elem("<?=$p?>_borient").style.display = cntrl == "buttons" ? "inline" : "none";
    elem("<?=$p?>_olabel").style.display = other ? "block" : "none";
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

class ChoiceWidget extends ComponentWidget
{
  var $choices;
  var $other;
  var $legend;

  var $answers = NULL;
  var $others = NULL;
  var $errors = NULL;

  function ChoiceWidget(&$c,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($c,$prefix,$form,$formmethod);

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

    $this->questions = $c->questions;

    if (!$c->is_html)
    {
      $this->legend = nl2br(htmlspecialchars($this->legend));
      foreach($this->choices as $k => $v)
        $this->choices[$k] = nl2br(htmlspecialchars($v));

      foreach($this->questions as $k => $v)
        $this->questions[$k] = nl2br(htmlspecialchars($v));
    }
  }

  function loadvalues()
  {
    foreach ($this->questions as $i => $text)
    {
      $c = &$this->component;
      $name = "q$i";
      if ($c->select_many) $name .= "[]";
      $a = $this->loadattribute($name);
      if ($c->select_many)
        $this->answers[$i] = is_array($a) ? $a : array();
      else if (isset($a))
        $this->answers[$i] = (int)$a;
      else
      {
        $this->answers[$i] = NULL;
        $this->errors[$i] = "Question '$text' is not filled in.";
      }
      $this->others[$i] = (string)$this->loadattribute("q{$i}_{$this->other}");
    }
  }

  function save($parent)
  {
    global $wbes;

    $result = pg_query("
      INSERT INTO choice_responses(revision_id, parent) VALUES ({$this->component->revision_id}, $parent);
      SELECT currval('response_ids');
    ", $wbes, __FILE__, __LINE__);

    if (!$result) return false;

    $nparent = (int)pg_result($result, 0, 0);
    $table = $this->component->select_many ? "mchoice_question_responses" : "choice_question_responses";
    $success = true;

    foreach(array_keys($this->questions) as $i)
    {
      if ($this->component->select_many)
        $answer = pg_implode($this->answers[$i]);
      else
        $answer = quot($this->answers[$i]);
      $other = addslashes($this->others[$i]);
      $success = $success && pg_query("
        INSERT INTO $table(revision_id, parent, answer, other)
        VALUES ({$this->component->question_ids[$i]}, $nparent, $answer, '$other');
      ", $wbes, __FILE__, __LINE__);
    }
    return $success;
  }

  function showchoices($questionno, $tdparams = "")
  {
    $c = &$this->component;
    $name = $this->prefix . "_q" . $questionno;
    if ($c->select_many) $name .= "[]";

    if ($c->stacked && !$c->vertical) print("<td $tdparams>");
    if ($c->rows > 0)
    {
      print("<select name=$name" . ($c->rows > 1 ? " size=" . $c->rows : "") . ($c->select_many ? " multiple" : "") . ">\n");
      foreach($this->choices as $i => $choice)
      {
        $selected = (!$c->select_many && $i === $this->answers[$questionno]) && ($c->select_many && is_array($this->answers) && in_array($i, $this->answers[$questionno])) ? " selected" : "";
        print("  <option value=$i$selected>$choice\n");
      }
      print("</select>\n");
    }
    else
    {
      if ($c->vertical)
        $sep = "<br>\n";
      else if ($c->stacked)
        $sep = "</td>\n<td $tdparams>";
      else
        $sep = "\n";

      $first = true;
      foreach($this->choices as $i => $choice)
      {
        if ($first) $first = false; else print($sep);

        $id = $this->prefix . '_q' . $questionno . '_' . $i;

        if ($c->select_many)
        {
            $checked = isset($this->answers) && in_array($i, $this->answers[$questionno]) ? " checked" : "";
            print("<input type=checkbox name=$name value=$i id=$id$checked>");
        }
        else
        {
            $checked = isset($this->answers[$questionno]) && $i === $this->answers[$questionno] ? " checked" : "";
            print("<input type=radio name=$name value=$i id=$id$checked>");
        }

        if (!$c->stacked || $c->vertical)
          print(" <label for=$id>$choice</label>");

        if ($i == $this->other)
        {
          print(" <input type=textbox size=20 name=$id value=\"" . htmlspecialchars($this->others[$questionno]) . "\">");
        }
      }
    }
    if ($c->stacked && !$c->vertical) print("</td>\n");
  }

  function print_number($i)
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
      print(Numbers_Roman::toRoman($i+1) . ".  ");
  }

  function display($hidden = false, $showerrors = false)
  {
    global $server_media;

    if($hidden)
    {
      if (isset($this->answers))
      foreach($this->answers as $i => $v)
      {
        $name = "q$i";
        if (is_array($v))
          $this->printarrayattribute($name, $v);
        else
          $this->printattribute($name, $v);
      }

      if (isset($this->others))
      foreach($this->others as $i => $v)
        $this->printattribute("q{$i}_{$this->other}", $v);
      return;
    }

    $c = &$this->component;
    $n = count($this->choices);

    //added to display intro text. [tyler]
    ?><h3><?= $c->intro_text; ?></h3><?

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
        if (isset($this->errors[$i]) && $showerrors) $text = "<b><font color=red>$text</font></b>";
        print("<tr>\n<td $color>");

        //added support for numbering. [tyler]
        $this->print_number($i);

        print($text . "</td>\n");
        $this->showchoices($i, "$color align=center");
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
        print("<tr>");
        print("<td valign=top>");

        //added support for numbering. [tyler]
        $this->print_number($i);

        print($text . "</td>\n<td>");
        $this->showchoices($i);
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
        $this->print_number($i);
        if (isset($this->errors[$i]) && $showerrors) $text = "<b><font color=red>$text</font></b>";
        print("$text<br>\n");
        $this->showchoices($i);
        print("<br><br>\n");
      }
    }
  }
}

?>

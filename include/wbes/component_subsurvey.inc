<?

require_once("wbes/component.inc");
require_once("wces/login.inc");





class SubsurveyFactory extends ComponentFactory
{
  function code() { return 8; }
  function name() { return "Subsurvey Component"; }
  
  function create()
  {
    $sub = new Subsurvey();
    
    /* 
    this is a dirty hack, we must save the survey before
    we can really use, thems the breaks.
    */

    //print("Pre saved.");    
    $sub->base_branch = $sub->survey->save($sub->our_topic_id, null, 0, 0);
    
    return $sub;
  }
  
  function load(&$components, $ids, $ordinals, $topic_id)
  {
    global $wbes;
    $sids = implode($ids, ",");
    $rids = array_flip($ids);
    $r = pg_query("SELECT revision_id, base_branch, flags, title FROM subsurvey_components WHERE revision_id IN ($sids)", $wbes, __FILE__, __LINE__);

    for($wr = new pg_wrapper($r); $wr->row; $wr->advance())
    {
      $c = new Subsurvey();
      $c->revision_id   = (int)$wr->row['revision_id'];
      $c->base_branch   = (int)$wr->row['base_branch'];

      $c->text 		= $wr->row['title'];

    	$factories = array(
        new ChoiceFactory(),
        new TextResponseFactory(),
        new TextFactory(),
        new HeadingFactory(),
        new SubsurveyFactory()
      );
	
	//print("Loading survey.");
	$c->survey->load($topic_id, $c->base_branch, $factories);
	
	$c->our_topic_id = $topic_id;
	//print("Load 1: $topic_id<br>");


      $flagGroup        = (int)$wr->row['flags'];
      $c->locked        = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html       = (bool)($flagGroup & FLAG_ISHTML);
      $components[$ordinals[$rids[$c->revision_id]]] = $c;
    }
  }
};



class Subsurvey extends Component
{
    var $base_branch;
    var $survey;
    var $our_topic_id;
    var $mode;

  function Subsurvey()
  {
    $this->mode = 0;
    $this->our_topic_id = $GLOBALS['topic_id'];
    $this->survey = new Survey();
    $this->base_branch = 0;
  }
  
  function geteditor($prefix,$form,$formmethod)
  {
    return new SubsurveyEditor($this,$prefix,$form,$formmethod);
  }  
  
  function getwidget($prefix,$form,$formmethod)
  {
    return new SubsurveyWidget($this,$prefix,$form,$formmethod);
  }
  
  function save($topic_id, $save_id)
  {
      global $wbes;
  
      $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';
      
    $this->our_topic_id = $topic_id;
    //print("Save 1: $topic_id");
    
    $user_id = login_getuserid();
    
    if($this->base_branch == 0)
	{$this->base_branch = $this->survey->save($topic_id, null, $user_id, $save_id);}
    else
	{$this->survey->save($topic_id, $this->base_branch, $user_id, $save_id);}
  
  
    $flagGroup = 0;
    if ($this->is_html)     $flagGroup |= FLAG_ISHTML;
    if ($this->locked)      $flagGroup |= FLAG_LOCKED;
    
    $result = pg_query("SELECT subsurvey_component_save($topic_id, $revision_id, $save_id, $this->base_branch, $flagGroup, '$this->text')", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
  
};

class SubsurveyEditor extends ComponentEditor
{
    var $sub_editor;
    var $action = 0;
    var $sub_survey;

    var $c;
    var $p;
    var $f;
    var $fm;

  function SubsurveyEditor(&$component,$prefix,$form,$formmethod)
  {
    $this->c = $component;
    $this->p = $prefix;
    $this->f = $form;
    $this->fm = $formmethod;
  
  
    $this->ComponentEditor($component,$prefix,$form,$formmethod);
    $this->sub_editor = new ActionButton($prefix, $form, $formmethod);
  }
  
  function loadvalues()
  {
    //print("Load 2.<br>");
    ComponentEditor::loadvalues();
    $this->sub_editor->loadvalues();
    $this->action = $this->sub_editor->action;
    $this->c->mode = $this->loadattribute("SUBmode");
    $this->c->our_topic_id = $GLOBALS['topic_id'];
    
    if($this->action == 199 || $this->c->mode == 1)
	{
	$this->c->mode = 1;
	
	$factories = array(
      new ChoiceFactory(),
      new TextResponseFactory(),
      new TextFactory(),
      new HeadingFactory(),
      new SubsurveyFactory()
    );

	// XXX: factories array for subsurvey should be a parameter SubsurveyFactory
	// XXX: user_id should be taken from parent surveyeditor
	$this->sub_survey = new SurveyEditor($this->c->our_topic_id, $this->c->base_branch, 0, $factories, $this->p . "_sub", $this->f, $this->fm);
	
	
	$this->sub_survey->survey = $this->c->survey;
	$this->sub_survey->loadvalues();
	}

  }
  
  function checkvalues()
  {
    return true;
  }
  
  function savevalues()
  {
    //print("Save 2.");
      
    ComponentEditor::savevalues();
    
    
    if(isset($this->survey))
	{
	//print("The survey is set.");
	$user_id = login_getuserid();
    
	if($user_id == null || $user_id == "")
	    {$user_id = 0;}
    
	if($this->component->base_branch == 0)
	    {$this->component->base_branch = $this->survey->save($GLOBALS['topic_id'], null, $user_id, 0);}
	else
	    {$this->survey->save($GLOBALS['topic_id'], $this->component->base_branch, $user_id, 0);}
	}
  }
  
  function display()
  {
    $this->printattribute("SUBmode", $this->c->mode);
    $this->printattribute("TopicID", $this->c->our_topic_id);
  
    if(($this->action != 199 && $this->c->mode != 1) || $this->sub_survey->state == SurveyEditor_done)
	{
	ComponentEditor::display();
	?>
	<table>
	<tr>
	<td valign=top><strong><? $this->fieldname("Subsurvey:",$this->text->broken); ?></strong></td>
	<td><? $this->text->display(false); ?><br><? $this->is_html->display(false); 
	$this->is_html->displaylabel("Contains HTML Tags"); ?><br>
	<? $this->locking->display(false); $this->locking->displaylabel("Lock this component"); ?><br>
	<? $this->sub_editor->display("Edit subsurvey", 199); ?>
	</td>
	</tr>
	</table>
	<?
	}
    else
	{
	?>
	<h3>The Following is a nested subsurvey.</h3>
	<br><br>
	
	<? 
	if($this->sub_survey->state == SurveyEditor_done)
	    {?>Your sub-survey has been saved.<?}
	else
	    {$this->sub_survey->display();}
	?>
	
	<br><br>
	<h3>This is the end of the nested subsurvey.</h3>
	<br><?
	}
  }
};

class SubsurveyWidget extends ComponentWidget
{
  function SubsurveyWidget(&$component,$prefix,$form,$formmethod)
  {
    $this->ComponentWidget($component,$prefix,$form,$formmethod);
  }
  
  function loadvalues()
  {
  }
  
  function display($hidden = false, $showerrors = true)
  {
    //$this->printattribute("SUBmode", $this->component->mode);
    //$this->printattribute("TopicID", $this->component->our_topic_id);
  
  
    $user_id = login_getuserid();
    $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
    $url = "subshow.php?topic_id=" . $this->component->our_topic_id . "&base_branch=" . $this->component->base_branch . "&user_id=$user_id&question_period_id=9";
    ?>
    <FORM>
    <INPUT type="button" value="<?= $text ?>" onClick="window.open('<?= $url ?>','<?= $text ?>','width=600,height=400,resizable=yes,scrollbars=yes')">
    </FORM> 
    <?
  }  
}

?>
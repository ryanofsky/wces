<?

require_once("wbes/component.inc");
require_once("wces/login.inc");

class SubsurveyFactory extends ComponentFactory
{
  function code() { return 8; }
  function name() { return "Subsurvey Component"; }

  function & create()
  {
    return new Subsurvey;
  }

  function load(&$loads, &$surveyGlobals, $specialization_id)
  {
    global $wbes;

    $ids = implode(",", array_keys($loads));

    $r = pg_go("SELECT revision_id, ctext, flags, specialization_id FROM subsurvey_components WHERE revision_id IN ($ids)", $wbes, __FILE__, __LINE__);
    $n = pg_numrows($r);

    $components = array();

    for ($i = 0; $i < $n; ++$i)
    {
      $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
      $load =& $loads[$row['revision_id']];

      $c =& new Subsurvey;
      $c->survey->revision_id = (int)$row['revision_id'];
      $c->text        = $row['text'];
      $flagGroup      = $row['flags'];
      $c->locked      = (bool)($flagGroup & FLAG_LOCKED);
      $c->is_html     = (bool)($flagGroup & FLAG_ISHTML);
      $load->destArray[$load->destIndex] =& $c;
      $components[$row['revision_id']] =& $c;
    }



    $tmpname = 'tmp' . md5($ids);
    $r = pg_go("
      CREATE TEMPORARY TABLE $tmpname AS
      SELECT li.revision_id AS prevision_id, branch_latest(find_specialization_branch(li.item_id, $specialization_id), $surveyGlobals->slatest) AS revision_id, li.ordinal
      FROM list_items AS li
      WHERE li.revision_id IN ($ids);

      SELECT t.prevision_id, t.ordinal, t.revision_id, r.type
      FROM $tmpname AS t
      INNER JOIN revisions as r USING (revision_id)
      ORDER BY t.prevision_id, ordinal;
    ", $wbes, __FILE__, __LINE__);
    pg_go("DROP TABLE $tmpname", $wbes, __FILE__, __LINE__);

    $prev = null;
    $n = pg_numrows($r);
    for($i = 0; $i < $n; ++$i)
    {
      if ($prev != $row['prevision_id'])
      {
        $prev = $row['prevision_id'];
        $ordinal = 0;
      }
      $row = pg_fetch_array($result, $i, PGSQL_ASSOC);
      $c =& $components[$row['prevision_id']];
      $c->survey->components[$ordinal] = null;

      if (!isset($surveyGlobals->cfactories[$row['type']]))
      {
        print("<h4>Fatal Error: Can't load component type $type</h4>");
        print("<p>Either the type doesn't exist and the database is corrupted,"
          . " or you forgot to include this component's factory in the factories array.</p>");
        print("<p>Here is a list of factoris that were included:</p>");
        print("<table>\n");
        foreach($surveyGlobals->cfactories as $k => $v)
        {
          print("<tr><td>" . $v->code() . "</td><td>" . get_class($v) . "</td><td>" . $v->name() . "</td></tr>");
        }
        print("</table>\n");
      }
      $surveyGlobals->addLoad($row['revision_id'], $row['type'], $c->survey->components, $i);
    }
  }

  function & loadSurvey($branch_id, $specialization_id, &$surveyGlobals)
  {
    global $wbes;

    $a = array(0 => null);
    $loads = array(new LoadChild($revision_id, $a, 0));
    SubsurveyFactory::load($loads, $surveyGlobals, $specialization_id);
    return $c->survey;
  }
};

class Subsurvey extends Component
{
  var $survey;
  var $specialization_id;

  function Subsurvey()
  {
    $this->survey =& new Survey;
  }

  function geteditor($name, &$parent)
  {
    return & new SubsurveyEditor($name, $parent);
  }

  function getwidget($name, &$parent)
  {
    return & new SubsurveyWidget($name, $parent);
  }

  function save($specialization_id, $save_id)
  {
    global $wbes;

    $revision_id  = $this->revision_id ? $this->revision_id : 'NULL';

    $this->our_specialization_id = $specialization_id;
    //print("Save 1: $specialization_id");

    $user_id = login_getuserid();

    if($this->base_branch == 0)
      {$this->base_branch = $this->survey->save($specialization_id, null, $user_id, $save_id);}
    else
      {$this->survey->save($specialization_id, $this->base_branch, $user_id, $save_id);}

    $flagGroup = 0;
    if ($this->is_html)     $flagGroup |= FLAG_ISHTML;
    if ($this->locked)      $flagGroup |= FLAG_LOCKED;

    $result = pg_go("SELECT subsurvey_component_save($specialization_id, $revision_id, $save_id, $this->base_branch, $flagGroup, '$this->text')", $wbes, __FILE__, __LINE__);
    return (int) pg_result($result, 0, 0);
  }
};

class SubsurveyEditor extends ComponentEditor
{
  var $sub_editor;
  var $event = 0;
  var $sub_survey;

  var $c; //$component
  var $p; //prefix
  var $f; //form
  var $fm; //formmethod

  function SubsurveyEditor(&$component,$name, &$parent)
  {
    $this->ComponentEditor($component, $name, &$parent);
    $this->sub_editor = & new EventWidget("sub_editor", &$parent);

    $factories = array(
      &new ChoiceFactory,
      &new TextResponseFactory,
      &new TextFactory,
      &new HeadingFactory,
      &new SubsurveyFactory
    );

    // XXX: factories array for subsurvey should be a parameter SubsurveyFactory
    // XXX: user_id should be taken from parent surveyeditor
    $this->sub_survey = & new SurveyEditor($component->our_specialization_id, $component->base_branch, 0, $factories, "sub", $this);
  }

  function loadState($new)
  {
    //print("Load 2.<br>");
    assert(isset($new));
    ComponentEditor::loadState($new);

    $this->event = $this->sub_editor->event;
    $this->component->mode = $this->readValue("SUBmode");
    $this->component->our_specialization_id = $GLOBALS['specialization_id'];

    if($this->event == 199 || $this->c->mode == 1)
    {
      $this->component->mode = 1;
      $this->sub_survey->survey = $this->c->survey;
      $this->sub_survey->loadState($new);
    }
  }

  function saveValues()
  {
    //print("Save 2.");

    ComponentEditor::saveValues();

    if(isset($this->survey))
        {
        //print("The survey is set.");
        $user_id = login_getuserid();

        if($user_id == null || $user_id == "")
            {$user_id = 0;}

        if($this->component->base_branch == 0)
            {$this->component->base_branch = $this->survey->save($GLOBALS['specialization_id'], null, $user_id, 0);}
        else
            {$this->survey->save($GLOBALS['specialization_id'], $this->component->base_branch, $user_id, 0);}
        }
  }

  function printState()
      {
      $this->printValue("SUBmode", $this->c->mode);
      $this->printValue("specializationID", $this->c->our_specialization_id);
      }

  function printVisible()
      {
      if(($this->event != 199 && $this->c->mode != 1) || $this->sub_survey->state == SurveyEditor_done)
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
  function SubsurveyWidget(&$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
  }

  function printVisible()
  {
    //$this->printValue("SUBmode", $this->component->mode);
    //$this->printValue("specializationID", $this->component->our_specialization_id);
    $user_id = login_getuserid();
    $text = $this->component->is_html ? $this->component->text : nl2br(htmlspecialchars($this->component->text));
    $url = "subshow.php?specialization_id=" . $this->component->our_specialization_id . "&base_branch=" . $this->component->base_branch . "&user_id=$user_id&question_period_id=9";
    ?>
    <FORM>
    <INPUT type="button" value="<?= $text ?>" onClick="window.open('<?= $url ?>','<?= $text ?>','width=600,height=400,resizable=yes,scrollbars=yes')">
    </FORM>
    <?
  }
}

?>

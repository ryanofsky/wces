<?

require_once("wbes/component.inc");

class InstructorSurveyFactory extends ComponentFactory
{
  function code() { return 11; }
  function name() { return "Instructor Survey"; }

  function & create()
  {
    $s =& new InstructorSurvey; 
    $s->setDefault();
    return $s;
  }

  function loadComponents(&$components)
  {
    foreach(array_keys($components) as $i)
      $components[$i] =& new InstructorSurvey; 
  }
}

class InstructorSurvey extends Component
{
  var $survey;
  
  function InstructorSurvey()
  {
    $this->Component();
    $this->survey =& new Survey;
  }
  
  function setDefault()
  {
    $h =& new Heading;
    $h->text = '%PROFESSOR_NAME%';
    $this->survey->insert($h);
  }

  function & getChildren()
  {
    return $this->survey->getChildren();
  }

  function saveComponent($component_id, $children)
  {
    return "instructor_subsurvey_save($component_id, $children)";
  }
  
  function & getEditor($name, &$parent)
  {
    return new InstructorSurveyEditor($this, $name, $parent);
  }

  function & getWidget($name, &$parent, $topic_id)
  {
    return new InstructorSurveyWidget($topic_id, $this, $name, $parent);
  }
}

class InstructorSurveyEditor extends ComponentEditor
{
  var $surveyEditor;
  
  function InstructorSurveyEditor(&$component, $name, &$parent)
  {
    $this->ComponentEditor($component, $name, $parent);
    $this->surveyEditor =& new SurveyEditor(-1, -1, -1, -1, 'se', $this);
    $this->surveyEditor->survey =& $this->component->survey;
  }
  
  function loadState($new)
  {
    $this->surveyEditor->buttons =& $this->buttons;
    ComponentEditor::loadState($new);
    if ($this->surveyEditor->modalChild)
      $this->modal = true;
  }
  
  function printVisible()
  {
    print("<h3>Instructor Subsurvey</h3>\n");
    $this->surveyEditor->display(); 
  }

  function save()
  {
    return true;
  }  
}

class InstructorSurveyWidget extends ComponentWidget
{
  var $surveys = array();
  
  function InstructorSurveyWidget($topic_id, &$component, $name, &$parent)
  {
    $this->ComponentWidget($component, $name, $parent);
    $topic_id = (int)$topic_id;
    
    global $wces;
    wces_connect();

    $r = pg_go("
      SELECT e.user_id, u.firstname, u.lastname
      FROM wces_topics AS t
      INNER JOIN enrollments_p AS e USING (class_id)
      INNER JOIN users AS u USING (user_id)
      WHERE t.topic_id = $topic_id
    ", $wces, __FILE__, __LINE__);

    $n = pg_numrows($r);

    for ($i = 0;;)
    {      
      if ($i < $n)
      {
        $row = pg_fetch_row($r, $i, PGSQL_ASSOC);
        $name = trim("$row[firstname] $row[lastname]");
        $wn = "sw$row[user_id]";
      }
      else
      {
        $row = null;
        $wn = "sw"; 
      }

      $s =& new SurveyWidget(-1, -1, -1, -1, $wn, $this);
      $s->survey =& $component->survey;
      $s->instructions = false;
      if (isset($row))
      {
        $s->professor_user_id = $row['user_id'];
        $s->replacements = array('%PROFESSOR_NAME%' => $name); 
      }
      $this->surveys[] = &$s;        
      if (++$i >= $n) break;
    }    
  }
  
  function loadState($new)
  {
    ComponentWidget::loadState($new);

    foreach (array_keys($this->surveys) as $i)
    {
      $s =& $this->surveys[$i];
      foreach(array_keys($s->questionWidgets) as $k)
      {
        $w =& $s->questionWidgets[$k]; 
        array_append($this->errors, $w->errors);
      }
    }
  }
  
  function printVisible()
  {
    foreach (array_keys($this->surveys) as $i)
    {
      $s =& $this->surveys[$i];
    
      foreach(array_keys($s->questionWidgets) as $k)
      {
        $w =& $s->questionWidgets[$k];
        $w->showErrors = $this->showErrors;
      }
    
      $s->display();
    }
  }
  
  function save($parent, $topic_id)
  {
    global $wces;

    foreach (array_keys($this->surveys) as $i)
    {
      $s =& $this->surveys[$i];

      $result = pg_go("
        SELECT prof_topic_make($topic_id, $s->professor_user_id)
      ", $wces, __FILE__, __LINE__);
      
      if (!$result) return false;
     
      $ntopic_id = (int)pg_result($result, 0, 0); 

      $result = pg_go("
        INSERT INTO responses_instructor_subsurvey(item_id, revision_id, parent, topic_id)
        VALUES ({$this->component->item_id}, {$this->component->revision_id}, $parent, $ntopic_id);
        SELECT currval('response_ids');
      ", $wces, __FILE__, __LINE__);
  
      if (!$result) return false;

      $nparent = (int)pg_result($result, 0, 0);

      foreach(array_keys($s->questionWidgets) as $key)
      {
        if (!$s->questionWidgets[$key]->save($nparent, $ntopic_id))
          return false;
      }
    }
    return true;
  }  
}

?>
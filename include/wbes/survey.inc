<?

require_once("wbes/wbes.inc");

$survey_debug = false;

class SurveyGlobals
{
  var $factories;

  // references to factories indexed by component type number
  var $cfactories = array();

  // if null, will load the latest version of a survey, if not null
  var $save_id;

  // array of {arrays of loads}, index by component type number
  var $loads = array();

  // array of {arrays of indices into the loads arrays indexed by
  // component_id and specialization_id} indexed by component type
  // number
  var $loadIndices = array();

  var $loadComplete = array();

  function SurveyGlobals(&$factories, $save_id)
  {
    $this->factories =& $factories;
    $this->save_id = $save_id;
    
    foreach(array_keys($this->factories) as $i)
    {
      $f =& $this->factories[$i];
      $code = $f->code();
      $this->cfactories[$code] =& $f;
      $this->loads[$code] = array();
      $this->loadIndices[$code] = array();
      $this->loadComplete[$code] = true;
    }
  }

  function & getLoad($type, $component_id, $specialization_id)
  {
    $key = "$component_id,$specialization_id";
    if (isset($this->loadIndices[$type][$key]))
    {
      $loadIndex = $this->loadIndices[$type][$key];
      $load =& $this->loads[$type][$loadIndex];
    }
    else if (isset($this->loads[$type]))
    {
      $this->loadComplete[$type] = false;
      $loadIndex = count($this->loads[$type]);
      $load =& new SurveyLoad($component_id, $specialization_id);
      $this->loads[$type][] =& $load;
      $this->loadIndices[$type][$key] = $loadIndex;
    }
    else
    {
      print("<p><b>Warning:</b> Component of unknown type $type was not loaded.</p>"); 
    }
    return $load;
  }
  
  function doLoad()
  {
    for (;;)
    {
      foreach (array_keys($this->loads) as $type)
        if (!$this->loadComplete[$type])
        {
          $this->loadComplete[$type] = true;
          $this->cfactories[$type]->load($this->loads[$type], $this);
          continue 2;
        }
      break;
    }
  
    foreach (array_reverse(array_keys($this->loads)) as $type)
    {
      $loads =& $this->loads[$type];
      foreach (array_keys($loads) as $lk)
      {
        $load =& $loads[$lk];
        foreach (array_keys($load->targets) as $tk)
        {
          $target =& $load->targets[$tk];
          $target->finishLoad($load->component);
        }
      }
    }
  }
};

class SurveyLoadTarget
{
  var $destArray;
  var $destIndex;
  var $revision_id;
  var $item_id;

  function SurveyLoadTarget(&$destArray, $destIndex, $revision_id, $item_id)
  {
    $this->destArray =& $destArray;
    $this->destIndex = $destIndex;
    $this->revision_id = $revision_id;
    $this->item_id = $item_id;
  }
  
  function finishLoad(&$component)
  {
    $d =& $this->destArray[$this->destIndex];
    $d = $component; // intentional clone
    $d->revision_id = $this->revision_id;
    $d->item_id = $this->item_id;     
  }
};

class SurveyLoad
{
  var $component_id;
  var $specialization_id;
  var $component;
  var $targets = array();

  function SurveyLoad($component_id, $specialization_id)
  {
    $this->component_id = $component_id;
    $this->specialization_id = $specialization_id;
  }

  function addTarget(&$target)
  {
    $this->targets[] =& $target;
  }
};


function & LoadSurvey($item_id, $specialization_id, &$factories, $save_id = null)
{
  global $wbes;
  wbes_connect();
  $surveyGlobals =& new SurveyGlobals($factories, $save_id);
  
  $save_id = $save_id ? $save_id : 'NULL';
  $dest = array(false);
  $r = pg_go("SELECT branch_latest($item_id, $specialization_id, $save_id)", $wbes, __FILE__, __LINE__);
  $revision_id = (int)pg_result($r, 0, 0);
  $r = pg_go("SELECT revision_component($revision_id)", $wbes, __FILE__, __LINE__);
  $component_id = (int)pg_result($r, 0, 0);
  $load =& $surveyGlobals->getLoad(1, $component_id, $specialization_id);
  $load->addTarget(new SurveyLoadTarget($dest, 0, $revision_id, $item_id));
  $surveyGlobals->doLoad();
  $survey =& $dest[0];
  $survey->top = true;
  return $survey;
};

function SaveSurvey(&$survey, $specialization_id, $user_id)
{
  assert($survey->top);
  global $wbes;
  wbes_connect();
  $result = pg_go("
    BEGIN WORK;
    INSERT INTO saves (user_id) VALUES ($user_id);
    SELECT currval('save_ids')
  ", $wbes, __FILE__, __LINE__);
  $save_id = (int)pg_result($result,0,0);

  $sql = $survey->save($specialization_id, $save_id);

  $r = pg_go("SELECT $sql", $wbes, __FILE__, __LINE__);
  
  pg_go("COMMIT", $wbes, __FILE__, __LINE__);
  
  return pg_result($r, 0, 0);
}

class SurveyFactory extends ComponentFactory
{
  function code() { return 1; }
  function name() { return null; }
  
  function loadComponents(&$components)
  {
    foreach(array_keys($components) as $i)
      $components[$i] =& new Survey; 
  }
}

class Survey extends Component
{
  var $components = array();
  var $revision_id = null;
  
  // is top level survey
  var $top;

  function insert(&$component,$position = 0)
  {
    $position = ((int)$position < 0) ? count($this->components) + (int)$position + 1 : (int)$position;
    array_splice($this->components,(int)$position,0,array(&$component));
    return $position;
  }

  function delete($componentno)
  {
    array_splice($this->components,$componentno,1);
  }

  function moveDown($componentno)
  {
    if ($componentno >= count($this->components) - 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno + 1);
  }

  function moveUp($componentno)
  {
    if ($componentno < 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno - 1);
  }

  function & getChildren()
  {
    return $this->components;
  }

  function saveComponent($component_id, $children)
  {
    return "survey_save($component_id, $children)";
  }
};

?>

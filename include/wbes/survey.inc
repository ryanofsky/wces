<?

require_once("wbes/wbes.inc");

function get_base($specialization_id)
{
  global $wces;
  wces_connect();
  for(;;)
  {
    if ($specialization_id == 1)
      return 1;
    if ($specialization_id == 1481)
      return 609;
    else if ($specialization_id == 1482)
      return 611;
    else if ($specialization_id == 1483)
      return 612;
    else
    {
      $r = pg_go("SELECT parent FROM specializations WHERE specialization_id = $specialization_id", $wces, __FILE__, __LINE__);
      if (pg_numrows($r) == 0) return 1;
      $specialization_id = (int)pg_result($r, 0, 0);
    }
  }
}

class SurveyGlobals
{
  var $user_id;
  var $factories;
  var $question_period_id;

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

  function SurveyGlobals(&$factories, $user_id, $save_id = null)
  {
    $this->factories =& $factories;
    $this->user_id = $user_id;
    $this->question_period_id = $question_period_id;

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
    if (isset($this->loadIndices[$type][$key])
    {
      $loadIndex = $this->loadIndices[$type][$key];
      $load =& $this->loads[$type][$loadIndex];
    }
    else
    {
      $loadComplete[$code] = false;
      $loadIndex = count($this->loads[$type]);
      $load =& new SurveyLoad($component_id, $specialization_id);
      $this->loads[$type][] =& $load;
      $this->loadIndices[$type][$key] = $loadIndex;
    }
    return $load;
  }
};

class SurveyLoadTarget
{
  var $destArray;
  var $destIndex;
  var $revision_id;

  function SurveyLoadTarget(&$destArray, $destIndex, $revision_id)
  {
    $this->destArray =& $destArray;
    $this->destIndex = $destIndex;
    $this->revision_id;
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

  function addTarget(&$destArray, $destIndex, $revision_id)
  {
    $this->targets[] =& new SurveyLoadTarget($destArray, $destIndex, $revision_id);
  }
};


function & LoadSurvey($item_id, $specialization_id, &$surveyGlobals)
{
  global $wbes;
  $save_id = $surveyGlobals->save_id ? $surveyGlobals->save_id : 'NULL';
  $dest = array();
  $r = pg_go("SELECT branch_latest($item_id, $specialization_id, $save_id)", $wbes, __FILE__, __LINE__);
  $revision_id = pg_result($r, 0, 0);
  $r = pg_go("SELECT revision_component($component_id)", $wbes, __FILE__, __LINE__);
  $component_id = pg_result($r, 0, 0);
  $load =& surveyGlobals->getLoad($type, $component_id, $specialization_id);
  $load->addTarget($dest, 0, $revision_id);

  foreach (array_keys($surveyGlobals->loads) as $type)
    while (!$surveyGlobals->loadsComplete[$type])
      $cfactories[$type]->load($surveyGlobals->loads[$type], $surveyGlobals);

  foreach (array_reverse(array_keys($surveyGlobals->loads)) as $type)
  {
    $loads =& $surveyGlobals->loads[$type];
    foreach (array_keys($loads) as $lk))
    {
      $load =& $loads[$lk];
      foreach (array_keys($load->targets) as $tk)
      {
        $target =& $load->targets[$tk];
        $dest =& $target->destArray[$target->destIndex];
        $dest = $load->component; // intentional clone
        $dest->revision_id = $target->revision_id;
        $dest->item_id = $load->item_id;
      }
    }
  }
};

function SaveSurvey(&$survey, $specialization_id, $user_id)
{
  $result = pg_go("
    BEGIN WORK;
    INSERT INTO saves (user_id) VALUES ($user_id);
    SELECT currval('save_ids')
  ", $wbes, __FILE__, __LINE__);
  $save_id = (int)pg_result($result,0,0);

  $sql = $survey->save($specialization_id, $save_id);

  $r = pg_go("SELECT $sql; COMMIT", $wbes, __FILE__, __LINE__);
  return pg_result($r, 0, 0);
}

class Survey extends Component
{
  var $components = array();
  var $revision_id = null;

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

  function saveComponent($children)
  {
    return "survey_save(revision_component($this->revision_id), $children)";
  }
};

?>

<?

define("component_choice",       1);
define("component_textresponse", 2);
define("component_picture",      3);
define("component_text",         4);
define("component_heading",      5);

class Survey
{
  var $components;
  
  function Survey()
  {
    $this->components = array();
  }
  
  function insert($component,$position = 0)
  {
    $position = ((int)$position < 0) ? count($this->components) + (int)$position + 1 : (int)$position;
    array_splice($this->components,(int)$position,0,array($component));
    return $position;    
  }
  
  function delete($componentno)
  {
    array_splice($this->components,$componentno,1);
  }
  
  function movedown($componentno)
  {
    if ($componentno >= count($this->components) - 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno + 1);
  }
  
  function moveup($componentno)
  {
    if ($componentno < 1) return;
    $q = $this->components[$componentno];
    $this->delete($componentno);
    $this->insert($q,$componentno - 1);
  }
  
  // TODO:
// 
// The load and save implementations that follow are intentially monolithic
// and meant to be experimental.
// 
// A more clean OOPish way of loading and saving components might use an object
// factory and a polymorphic save method. The reason why I didn't do it this
// way (for now) is that it requires a lot of repeated SQL queries. Having all
// the relevant code in one place here makes it a little easier to look for
// optimizations and possibly minimize the number of queries by combining them
// and by moving some logic into stored procedures. In short, optimizations and
// extensibility will be added little later.

  function load($db, $id, $istransform = false)
  {
    $id = (int)$id;
    
    if ($istransform)
    {
      $transform_id = $id;
      $result = pg_query("SELECT survey_id FROM transforms WHERE transform_id = '$id'", $db, __FILE__, __LINE__);
      if (pg_num_rows($result) != 1) sendwarning("Transform $id does not exist.", __FILE__, __LINE__);
      $survey_id = (int)pg_result($result, 0, 0);
    }
    else 
      $transform_id = false;
    
    $g = pg_query("
      SELECT sc.component_id AS id, c.component_type AS text,
      c.component_text AS text, c.is_html, c.is_personal, c.is_shared
      FROM survey_components AS sc
      INNER JOIN components AS c ON c.component_id = sc.component_id
      WHERE survey_id = '$survey_id'
    ", $db, __FILE__, __LINE__);
    
    pg_show($result, "Get list of components");
    
    $qs = array();
    
    $composite = false;
    $q = NULL;
    
    for($i=0; $gr = @pg_fetch_array($result,$i); ++$i)
    {
      $id = (int)$gr["id"];
      $type = (int)$gr["type"];
      switch((int)$type)
      {
        case component_multchoice:
          $composite = false;
          $c = pg_query("
            SELECT choices, first_number, last_number, orientation, check_many, is_required 
            FROM multiple_choice
            WHERE component_id = '$id'
          ", $db, __FILE__, __LINE__);
          $cr = pg_fetcharray($c);
          $q = new MultChoice();
          $q->choices      = $cr["choices"];
          $q->first_number = (int) $cr["first_number"];
          $q->last_number  = (int) $cr["last_number"];
          $q->orientation  = (int) $cr["orientation"];
          $q->check_many   = (boolean) $cr["check_many"];
          $q->is_required  = (boolean) $cr["is_required"];
          break;
        case component_textresponse:
          $composite = false;
          $c = pg_query("
            SELECT rows, cols FROM text_responses WHERE component_id = '$id'
          ", $db, __FILE__, __LINE__);
          break;        
        case component_picture:
          $composite = false;
          $c = pg_query("
            SELECT location, originalname, format, width, height FROM pictures WHERE component_id = '$id'
          ", $db, __FILE__, __LINE__);
          $q = new Picture();
          $q->location     = $c["location"];
          $q->originalname = $c["location"];
          $q->format       = $c["location"];
          $q->width        = $c["location"];
          $q->height       = $c["location"];
          break;        
        case component_text:
          $composite = false;
          $c = pg_query("
            SELECT location, originalname, format, width, height FROM pictures WHERE component_id = '$id'
          ", $db, __FILE__, __LINE__);
          $q = new Text();
          $q->rows = $c["rows"];
          $q->cols = $c["cols"];
          break;
        case component_heading:
          $composite = false;
          break;

        case component_rating:
          $composite = true;

          $c = pg_query("
            SELECT first_number, last_number, is_required
            FROM rating_items
            WHERE component_id = '$id'
          ", $db, __FILE__, __LINE__);
          
          if (!isset($prev) || !($prev->first_number == $cr["first_number"] && $prev->last_number == $cr["last_number"]))
          {
            unset($prev);
            $prev = new Rating();
            $q->first_number = (int) $cr["first_number"];
            $q->last_number  = (int) $cr["last_number"];
            $qs[] = &$prev;
          }
          $prev->items[] = $joe;
          
          break;        
        case component_rating_head:
          $q = new Rating();
          $qs[] = &$q;
          $prev = $q;
          unset($q);
          break;        


        default:
          sendwarning("Unknown component type '$type' encountered in survey '$survey_id'", __FILE__, __LINE__);
      }
      
      if (isset($q) && !$composite)
      {
        $q->text        = $gr["text"];
        $q->is_html     = $gr["is_html"];
        $q->is_personal = $gr["is_personal"];
        $q->is_shared   = $gr["is_shared"];
        $qs[] = &$q;
        unset($q);
      }
    }
  }

  function save($db, $survey_id, $transform_id)
  {
    return array($survey_id, $transform_id);
  }
}

?>
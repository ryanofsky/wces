<?

class Survey
{
  var $components = array();
  var $orig_id = false;
 
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
  
  function load($branch_id, $component_types)
  { 
    global $wbes;

    $result = pg_query("SELECT list_update($branch_id)", $wbes, __FILE__, __LINE__);
    $this->orig_id = pg_result($result, 0, 0);

    $revision = false; // todo: sublist support    
    //todo: also, fix hazard that occurs if a new revision is saved between last
    // query and the next. it will retrieve new list contents and use the same
    // old orig_id

    $result = pg_query("
      SELECT li.ordinal, c.component_id, c.type
      FROM list_branch_cache AS lbc
      INNER JOIN list_items AS li ON li.list_id = lbc.content_id
      INNER JOIN components AS c ON c.component_id = li.item_id
      WHERE lbc.branch_id = $branch_id
      ORDER BY type, ordinal
    ", $wbes, __FILE__, __LINE__);
    
    $n = pg_numrows($result);
    $type = $lasttype = false;
    $ordinals = $components = array();
    $this->components = array_pad(array(), $n, false);
    for($i = 0; ; ++$i)
    {
      if($i < $n)
        extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      else
        $type = -1;

      if ($type !== $lasttype && $lasttype !== false)
      {
        $factory = $component_types[$lasttype] . '_Factory';
        $factory($this->components, $components, $ordinals, $revision);
      }
      
      if ($i >= $n) break;

      if ($type === $lasttype)
      {
        $ordinals[] = (int)$ordinal;
        $components[] = (int)$component_id;
      }
      else
      {
        $components = array((int)$component_id);
        $ordinals = array((int)$ordinal);
      }
      $lasttype = $type;
    }
  }
  
  function save($branch_id)
  {
    global $wbes;
    
    $this->orig_id = (int)$this->orig_id;
    
    $result = pg_query("BEGIN WORK", $wbes, __FILE__, __LINE__);
      if (!$result) return false;

    $result = pg_query("
      SELECT list_save_start($branch_id, $this->orig_id)
    ", $wbes, __FILE__, __LINE__);
    
    $save_id = (int)pg_result($result, 0, 0);
    
    $changed = false;
    $i = 0;
    foreach(array_keys($this->components) as $k)
    {
      $id = $this->components[$k]->save();
      $changed = $changed || $id != $this->components[$k]->component_id;
      pg_query("INSERT INTO list_items(list_id, ordinal, item_id) VALUES ($save_id, $i, $id)", $wbes, __FILE__, __LINE__);
      ++$i;
    }
   
   if ($changed)
   {
     $result = pg_query("
       SELECT list_save_end($branch_id, $save_id)
     ", $wbes, __FILE__, __LINE__);
     
     $status = pg_result($result,0,0);
     $result = pg_query("COMMIT", $wbes, __FILE__, __LINE__);
     return $status && $result;
   }
   else
   {
     debugout("No changes");
     return (boolean) pg_query("ROLLBACK", $wbes, __FILE__, __LINE__);
   }

  }
}

?>

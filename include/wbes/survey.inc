<?

class Survey
{
  var $components = array();
  var $survey_id = false;
  var $branch = false;
  var $revision = false;
  
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
  

  function load($survey_id, $index)
  {
    SELECT $branches, $depth FROM surveys WHERE survey_id = $survey_id;
    $revision = list_getrevision($branches);
    
    $list_id = $revision[$depth*3 + 1];
    
    $result = 
      SELECT c.component_id, c.type
      FROM list_items AS li
      INNER JOIN components AS c ON c.component_id = li.item_id
      WHERE list_id = $list_id
      ORDER BY type,ordinal;
    
    $n = pg_numrows($result);
    $lasttype = false;
    $ordinals = $components = $array();
    $this->components = array_fill(0, $n, false);
    for($i = 0; $i < $n; ++$i)
    {
      extract(pg_fetch_array($result,$i,PGSQL_ASSOC));
      
      if ($type === $lasttype)
      {
        $ordinals[] = $ordinal;
        $components[] = $component_id;
      }
      else
      {
        if ($lasttype !== false)
          $component_factory($this->components, $ids, $ordinals, $revision);
        $components = array($component_id);
        $ordinals = array($ordinal);
      }
      $lasttype = $type;
    }
    
    if ($lasttype !== false)
      $component_factory($this->components, $ids, $ordinals, $revision);
  }
  
  function save($link_id, $depth);
  {
    $revision = SELECT list_getrevision($this->list_id);
    
    SELECT $branches FROM links WHERE link_id = $link;
    foreach($branches AS $k => $v)
      if ($v != $revision[$k * 3 + 1])
        return false;                        // security breach (invalid $this->list_id)
 
    if (count($revision) + 3 == $depth * 3)  // might need to add new branch
      $b = list_newbranch($depth);           // get a new branch number
    else if (count($revision) == $depth * 3) // proper depth
      $b = false;
    else                                    
      return false;                          // security breach
    

    BEGIN TRANSACTION;
    SELECT $branches FROM links WHERE link = $link_id FOR UPDATE; // lock

    if (!$b || count($branches) == $depth) // branch already added
    {
      primary = secondary = list_getrevision($branches);
      primary[-1] += 1;
    }
    else if (count($branches) == $depth - 1)) // need to add a new branch
    {
      $branches[] = $b;
      UPDATE links SET branches = $branches;

      primary = secondary = $revision;
      primary[] = 0;
      primary[] = $b; 
      primary[] = 1;
    }
    else // this should never happen  
    { 
      ROLLBACK; return false;
    } 

    INSERT INTO lists (parent, branch, revision) VALUES (primary[-6], primary[-2], primary[-1]);
    primary[-3] := currval(list_ids);
    reset($this->components);
    while(list($k) = $each($this->components))
      $this->components->save(primary);

    IF (revision[-3] != secondary[-3]) THEN
      INSERT INTO lists (parent, branch, revision) VALUES (primary[-6], primary[-2], primary[-1] + 1);          
      merge(revision[-3],primary[-3],secondary[-3],currval(list_ids));
    END IF;
    
    COMMIT;
    return true;    
  }
}

?>
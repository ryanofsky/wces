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
  
  function load($survey_id, $branch, $component_types)
  {
    $this->survey_id = $survey_id = (int)$survey_id;
    if ($branch)
      $this->branch = $branch = (int)$branch;
    else
    {
      $this->branch = false;
      $branch = "NULL";  
    }  

    global $wbes;
    wbes_connect();

    $result = db_exec("SELECT list_latest((SELECT list_head FROM surveys WHERE survey_id = $survey_id),$branch)", $wbes, __FILE__, __LINE__);
    $list_id = pg_result($result, 0, 0);
    if (!$list_id) return;

    $result = db_exec("
      SELECT c.component_id, c.type, l.ordinal
      FROM list_items AS l, components AS c
      WHERE list_id = $list_id AND c.component_id = l.item_id
      ORDER BY type, ordinal
    ", $wbes, __FILE__, __LINE__);

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
          Component_Factory($this->components, $components, $ordinals);
        $components = array($component_id);
        $ordinals = array($ordinal);
      }
      $lasttype = $type;
    }

    if ($lasttype !== false)
      Component_Factory($this->components, $components, $ordinals);
  }

  function save($b1, $r1, $b2, $r2)
  {
    $sql = "";
    reset($this->components)
    while($list($k, $v) = each($this->components))
      $sql .= $c->save($this->branch)

    {
    }
    
    oldlist := select list_id from lists where b1 = $b1, r1 = $r1, b2 = $b2, r2 = $r2;
    
    start transaction
      if ($b2 == 0)
      {
        // lock rows to serialze list updates on this branch (is this lock too broad?)
        SELECT list_id FROM lists WHERE b1 = $b1 AND r1 >= r1 AND b2 = 0 FOR UPDATE;
 
        newr1 := SELECT MAX(r1)+1 FROM lists WHERE b1 = $b1 AND b2 = 0;
        INSERT INTO lists (b1, r1, b2, r2) VALUES ($b1, $newr1, 0, 0);
        newlist := currval(list_ids);
        
        foreach($components as $comp)
          $comp->save(newlist, ordinal, $newr1, $b2 = 0, $r2 = 0)
        
        if (newr1 > $r1 + 1)
        {
          common = select listid where (b1 = $b1, r1 = $r1, b2 = 0);
          secondary = select listid where b1 = $b1, $r1 = newr1 - 1, $b2 = 0);
          primary = newlist
          insert into lists(b1,r1,b2,r2,common,merged) values($b1, newr1 + 1, 0,0, common,secondary);
          select list_join(common,primary, secondary, curval('list_id');
        }
      }
      else // b2 != 0
      {
        if (b2 == -1)
        {
          r2 = 0;
          SELECT branch, link_id FROM links WHERE survey_id = $survey_id AND topic_id = $topic_id FOR UPDATE;
          IF branch = 0 THEN
            b2 = branch
          ELSE  
            -- this next line is potentially a bottleneck, causes branch additions to be
            -- needlessly serialized. it could be placed in a separate transaction...
            UPDATE lists SET b2 = b2 + 1 WHERE $b1 = $b1 AND r1 IS NULL;
            b2 := select b2 from lists where $b2
            UPDATE links SET branch = b2 WHERE link_id = $link_id
          END IF
        }
        
        // lock rows to serialize list updates on this branch
        SELECT list_id FROM lists WHERE b1 = $b1 AND r1 = $r1 AND b2 = $b2 and r2 >=  $r2 FOR UPDATE;


 
        newr2 := SELECT COALESCE(MAX(r2),0) + 1 FROM lists WHERE b1 = $b1 AND r1 = r1 AND b2 = b2
        INSERT INTO lists (b1, r1, b2, r2) VALUES ($b1, $newr1, 0, 0);
        newlist := currval(list_ids);
        
        foreach($components as $comp)
          $comp->save(newlist, ordinal, $newr1, $b2 = 0, $r2 = 0)
        
        if (newr2 > $r2 + 1)
        {
          common = select listid where (b1 = $b1, r1 = $r1, b2 = 0);
          secondary = select listid where b1 = $b1, $r1 = newr1 - 1, $b2 = 0);
          primary = newlist
          insert into lists(b1,r1,b2,r2,common,merged) values($b1, newr1 + 1, 0,0, common,secondary);
          select list_join(common,primary, secondary, curval('list_id');
        }      
      }
      
      
    
    end transaction;
 
    ... 

    return pg_query($sql,$db,__FILE__, __LINE__) ? true : false;
  }
}

?>
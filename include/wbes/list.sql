DROP TABLE lists;
DROP TABLE list_items;
DROP FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER);

CREATE TABLE lists
(
  list_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('list_ids'),
  parent INTEGER,
  branch INTEGER NOT NULL,
  revision INTEGER NOT NULL,
  merged INTEGER,
  UNIQUE(parent, branch, revision)
);

CREATE SEQUENCE list_ids INCREMENT 1 START 1;
CREATE SEQUENCE list_branch1s INCREMENT 1 START 1;

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  list_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);

CREATE INDEX list_idx ON list_items(list_id);

-- with integer arrays, functions here use the same resource management convention used in Microsoft COM. Arrays passed as arguments into a function must be cleaned up by that function. arrays returned by functions are cleaned up by the caller.

CREATE FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    common_id    ALIAS FOR $1;
    primary_id   ALIAS FOR $2;
    secondary_id ALIAS FOR $3;
    new_id       ALIAS FOR $4;
    rec RECORD;
    ord INTEGER;
    ck INTEGER;
    nk INTEGER;
  BEGIN
    -- Create new list based on primary but without items that were deleted in the secondary list. 
   
    ord := 0;
    FOR rec IN 
      SELECT li.item_id
      FROM list_items AS li
      WHERE
        li.list_id = primary_id
      AND NOT -- deleted
      (
        EXISTS (SELECT 1 FROM list_items AS c WHERE c.list_id = common_id AND c.item_id = li.item_id)
        AND
        NOT EXISTS (SELECT 1 FROM list_items AS h WHERE h.list_id = secondary_id AND h.item_id = li.item_id)
      )
      ORDER BY li.ordinal
    LOOP
      INSERT INTO list_items(list_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
      ord := ord + 1;      
    END LOOP;
    
    -- Insert new items from secondary into the new list

    ord := 0;  
    FOR rec IN SELECT ordinal, item_id FROM list_items WHERE list_id = secondary_id ORDER BY ordinal LOOP

      SELECT INTO ck ordinal FROM list_items WHERE list_id = common_id AND item_id = rec.item_id;
      IF NOT FOUND THEN ck := NULL; END IF;

      SELECT INTO nk ordinal FROM list_items WHERE list_id = new_id AND item_id = rec.item_id;
      IF NOT FOUND THEN nk := NULL; END IF;

      IF ck IS NULL AND nk IS NULL THEN -- needs to be inserted
        UPDATE list_items SET ordinal = ordinal + 1 WHERE list_id = new_id AND ordinal >= ord;
        INSERT INTO list_items(list_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
        nk := ord;
      END IF;

      IF (nk IS NOT NULL) THEN ord := nk + 1; END IF;
   
    END LOOP;
    RETURN 1;
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_test(INTEGER) RETURNS ia_data%ROWTYPE AS '
  SELECT * FROM ia_data WHERE ia_id = $1
' LANGUAGE 'sql';

-- helper function used by other functions, sql ugliness

CREATE FUNCTION list_max(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    parent_     ALIAS FOR $1;
    branch_     ALIAS FOR $2;
    maxrevision ALIAS FOR $3;
    lock        ALIAS FOR $4;
    rec RECORD
    r   INTEGER;
    ret INTEGER := NULL;
  BEGIN
    IF lock AND maxrevision IS NOT NULL THEN
      RAISE EXCEPTION ''list_maxrevision is not implemented for the case where a maximum revision is set and locking is requested.'';
    END IF;
    
    -- stuff between the dashes is just the same code repeated again and again
    -- in all the different variations
    
    IF parent_ IS NULL THEN
      IF maxrevision IS NULL THEN
    
        -------------------------------------------------------------------------
        SELECT INTO r MAX(revision) FROM lists WHERE parent IS NULL AND branch = branch_;
        IF r IS NOT NULL THEN
          IF lock THEN
            SELECT INTO rec list_id, revision FROM lists WHERE parent IS NULL AND branch = branch_ AND revision >= r ORDER BY revision DESC FOR UPDATE;
            ret := ia_new('{0,0}');
            ia_set(ret,1,rec.list_id);
            ia_set(ret,2,rec.revision);
          ELSE
            SELECT INTO list_id list_id FROM lists WHERE parent IS NULL AND branch = branch_ AND revision = r;
            ret := ia_new('{0,0}');
            ia_set(ret,1,list_id);
            ia_set(ret,2,r);
          END IF;
        END IF;
        -------------------------------------------------------------------------
     
      ELSE -- maxrevision IS NOT NULL
    
        -------------------------------------------------------------------------
        SELECT INTO r MAX(revision) FROM lists WHERE parent IS NULL AND branch = branch_ AND revision <= maxrevision;
        IF r IS NOT NULL THEN
          SELECT INTO list_id list_id FROM lists WHERE parent IS NULL AND branch = branch_ AND revision = r;
          ret := ia_new('{0,0}');
          ia_set(ret,1,list_id);
          ia_set(ret,2,r);
        END IF;
        -------------------------------------------------------------------------
    
      END IF;
    ELSE -- parent_ IS NOT NULL
      IF maxrevision IS NULL THEN
    
        -------------------------------------------------------------------------
        SELECT INTO r MAX(revision) FROM lists WHERE parent IS parent_ AND branch = branch_
        IF r IS NOT NULL THEN
          IF lock THEN
            SELECT INTO rec list_id, revision FROM lists WHERE parent = parent_ AND branch = branch_ AND revision >= r ORDER BY revision DESC FOR UPDATE;
            ret := ia_new('{0,0}');
            ia_set(ret,1,rec.list_id);
            ia_set(ret,2,rec.revision);
          ELSE
            SELECT INTO list_id list_id FROM lists WHERE parent = parent_ AND branch = branch_ AND revision = r;
            ret := ia_new('{0,0}');
            ia_set(ret,1,list_id);
            ia_set(ret,2,r);
          END IF;
        END IF;
        -------------------------------------------------------------------------
    
      ELSE -- maxrevision IS NOT NULL
    
        -------------------------------------------------------------------------
        SELECT INTO r MAX(revision) FROM lists WHERE parent = parent_ AND branch = branch_ AND revision <= maxrevision;
        IF r IS NOT NULL THEN
          SELECT INTO list_id list_id FROM lists WHERE parent = parent_ AND branch = branch_ AND revision = r;
          ret := ia_new('{0,0}');
          ia_set(ret,1,list_id);
          ia_set(ret,2,r);
        END IF;
        -------------------------------------------------------------------------
    
      END IF;
    END IF;
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_latest(INTEGER, INTEGER, BOOLEAN, BOOLEAN, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    base       ALIAS FOR $1; -- ia index
    d          ALIAS FOR $2;
    readonly   ALIAS FOR $3;
    beforeonly ALIAS FOR $4;
    locklast   ALIAS FOR $5;
    depth      INTEGER;
    maxr       INTEGER;
    lmax       INTEGER;
    m          INTEGER;
    last       INTEGER := 0;
    revision   INTEGER;
  BEGIN
    revision := base;
    depth := (ia_length(revision) - 1) / 3;

    -- maxr is only needed because the postgres arrays cannot hold null values.
    -- when this and other the other array problems that exist in Postgres 7.1.3
    -- are finally fixed, this function can become cleaner and more efficient.

    maxr  := ia_new('{}'); 
    i := 1;
    LOOP
      EXIT WHEN i >= d AND (NOT beforeonly OR i >= depth);
      PERFORM ia_set(maxr,i,ia_get(revision,i*3));
      i := i + 1;
    END LOOP;
    
    LOOP
      EXIT WHEN d > depth;
      tp := ia_get(revision,d*3-5);
      tb := ia_get(revision,d*3-1);
      tr := ia_get(maxr,d);
      lmax := list_max(tp, tb, tr, locklast && d = depth);
      IF lmax IS NULL THEN
        IF readonly THEN
          ia_set(maxr,d,NULL); -- truncate maxr at d
          d := d - 1;
          IF d < 1 THEN ia_delete(revision); revision := NULL; EXIT; END IF;
          UPDATE ia_data SET data[d] = data[d] - 1 WHERE ia_id = maxr; -- equivalent: maxr[d] := maxr[d] - 1
        ELSE
          i := ia_clone(revision);
          ia_set(i,d*3+1,NULL); -- truncate i at d*3+1
          i := list_latest(i, d, 1, 0, 1);
          IF i IS NULL THEN
            RAISE EXCEPTION ''Unable to find any past revisions on the current branch.'';
          END IF;
          lmax := list_max(tp, tb, tr, locklast && d = depth);
          IF lmax IS NULL THEN
            IF ia_get(i,d*3) == 0 THEN r := 0; ELSE r := 1; END IF;
            INSERT INTO lists(parent, branch, revision, merged) VALUES(tp, tb, r);
            new_id := currval(''list_ids'');
            ia_set(revision,d*3,r);
            ia_set(revision,d*3-2,new_id);
            IF r <> 0 THEN
              last := new_id;
              primary_id := ia_get(i,d*3-2);
              secondary_id = tp;
              j := d;
              LOOP
                j := j - 1;
                EXIT WHEN j <= 1 OR ia_get(i,j*3) != 0; 
              END LOOP;
              common_id := ia_get(i,j*3-2);
              array_merge(common_id, primary_id, secondary_id, new_id);
            END IF;
            d := d + 1;
          END IF;
        END IF;  
      END IF;
      IF lmax IS NOT NULL THEN 
        maxr[d] := lmax[2];
        revision[d*3-2] := lmax[1];
        revision[d*3-1] := branch[d];
        revision[d*3  ] := lmax[2];
        IF lmax[2] <> 0 THEN last := lmax[1]; END IF;
        d = d + 1;
      END IF;
    end loop
    revision[d*3-2] := last;
  END;
' LANGUAGE 'plpgsql';

























FUNCTION list_newbranch(INTEGER) RETURNS INTEGER;
DECLARE
  depth_ ALIAS FOR $1;
  branch_ INTEGER;
BEGIN
  SELECT INTO branch_ branch+1 FROM list_branches WHERE depth = depth_ FOR UPDATE;
  IF NOT FOUND THEN
    INSERT INTO list_branches (depth,branch) VALUES (depth_,1);
    RETURN 1;
  ELSE
    UPDATE branches SET branch = branch_ WHERE depth = depth_;
    RETURN branch_;
  END IF;
END;

FUNCTION list_subparent(nrev,d) -- generate 
DECLARE
  rec RECORD;
  temp INTEGER[];
BEGIN

  SELECT INTO rec l1.list_id AS common, l2.list_id AS primary, l2.revision AS primaryrev
    l2.list_id
  FROM lists AS l1
  INNER JOIN lists AS l2 ON l2.parent = l1.list_id AND l2.branch = nrev[d+1]
  WHERE l1.parent IS nrev[d-6] AND l1.branch = nrev[d-2] AND l1.revision < nrev[d-1]
  ORDER BY l1.revision DESC, l2.revision DESC;
  IF NOT FOUND
    RAISE EXCEPTION ''Sublist merge error. No preceding revision of '' || nrev || '' can be found '';
  END IF;
  
  IF rec.primaryrev = 0 THEN
    nrev[d+2] := 0;
    INSERT INTO lists (parent,branch,revision) VALUES (nrev[d-3], nrev[d+1], 0);
    nrev[d]   := currval(''list_ids'');
  ELSE
    primary = rec.primary;
    common = rec.common;
    secondary := nid;
    
    INSERT INTO lists (parent,branch,revision,merged) VALUES (nrev[d-3], nrev[d+1], nrev[d+2], 0);
    nrev[d] := currval(''list_ids'');
    
    nid := nrev[d];
    
    list_merge(common,primary,secondary,nid);
  END IF;
END    
 
 
 
function  
  ELSE                   -- merged from previous revision on the same branch
    INSERT INTO lists (parent, branch, revision, merged) VALUES (nrev[d-3], nrev[d+1], nrev[d+2], rec.merged);
    nrev[d] := currval(''list_ids'');
    nid := nrev[d];
    
    primaryr := NULL; secondaryr := NULL;
    FOR SELECT INTO rec list_id, revision FROM lists WHERE parent IS revision[d-3] AND branch = revision[d+1] AND revision < revision[d+2] ORDER BY revision DESC LIMIT 2;
      IF primaryr IS NULL THEN
        primaryr := rec.revision;
      ELSE
        secondaryr := rec.revision;
      END IF         
    END FOR;
  
    IF secondaryr IS NULL THEN
      RAISE EXCEPTION ''Unable to find ancestors for merged sublist '' || nrev;
    END;
    
    
    
    IF NOT FOUND OR rec.revision <= 1 THEN
      
    END IF;
    
    primary := rec.list_id;
    
    SELECT INTO rec list_id FROM lists WHERE parent IS
    
    SELECT 
    
    
    -- find secondary
    
  END IF;
END;


FUNCTION list_getsublist(INTEGER, INTEGER[]) RETURNS INTEGER[]
DECLARE
  branch1 ALIAS  FOR $1;
  revision ALIAS FOR $2;
  nrev INTEGER[];
  nid INTEGER;
  locked BOOLEAN;
  rec RECORD;
  d INTEGER;
  p INTEGER;
  m INTEGER;
BEGIN
  nid := 0;
  nrev[1] := 0;
  nrev[2] := branch1;
  nrev[3] := revision[3];
  LOOP
    p := nrev[d-3]; -- parent id
    locked := 0;
    LOOP
      IF p IS NULL THEN
        SELECT INTO rec list_id, revision FROM lists WHERE parent IS NULL AND branch = revision[d+1] AND revision <=(nrev[d+1] ORDER BY revision DESC;
      ELSE
        SELECT INTO rec list_id, revision FROM lists WHERE parent = p AND branch = revision[d+1] AND revision <= nrev[d+1] ORDER BY revision DESC;
      END IF;
      EXIT WHEN FOUND OR locked;
      IF p IS NULL RAISE EXCEPTION ''Sublist at branch % not found'',branch1;
      SELECT INTO locked 1 FROM lists WHERE list_id = p FOR UPDATE;
    END LOOP;
    
    m := NULL;
    SELECT INTO m merged FROM lists WHERE list_id = revision[d];
    
    IF FOUND THEN
      IF m IS NULL THEN
        nrev[d]   := rec.list_id;
        nrev[d+2] := rec.revision;
        IF rec.revision <> 0 THEN
          nid := rec.list_id;
        END IF;
      END IF;  
    END IF;  
    
    IF m IS NOT NULL THEN
      sublist_generate(rec.merged)
    END IF;
    
    d := d + 3;
    EXIT WHEN revision[d+4] IS NULL;
    nrev[d]   := 0;
    nrev[d+1] := revision[d+1];
    nrev[d+2] := revision[d+2];
  END LOOP;
  nrev[d] := 1;
  RETURN nrev;
END;  





-----------

FUNCTION list_getrevision(INTEGER[]) RETURNS INTEGER[]
DECLARE
  branches ALIAS FOR $1;
  revisions INTEGER[];
  r INTEGER;
  d INTEGER;
  rec RECORD;
  llist INTEGER;
BEGIN
  SELECT INTO r MAX(revision) FROM lists WHERE parent IS NULL AND branch = branches[1];
  IF r IS NULL THEN RETURN NULL; END IF;
  SELECT INTO rec list_id,branch,revision FROM lists WHERE parent IS NULL AND branch = branches[1] AND revision = r;
  
  revisions[1] := rec.list_id;
  revisions[2] := rec.branch;
  revisions[3] := rec.revision;
  llist        := rec.list_id;
  
  d = 2;
  LOOP
    SELECT INTO r MAX(revision) FROM lists WHERE parent = revisions[d*3-5] AND branch = branches[d];
    IF r IS NULL THEN
      SELECT * FROM lists WHERE list_id = revisions[d*3-5] FOR UPDATE;
      SELECT INTO r MAX(revision) FROM lists WHERE parent = revisions[d*3-5] AND branch = branches[d];
      IF r IS NULL THEN
        common    := 3.1;
        primary   := 3.1.2.1;
        secondary := 3.2;
        new       := 3.2.2.1;
        
        secondary := list;
        select into rec parent, revision, branch from lists WHERE list_id = list;
        
        SELECT l1.list_id AS secondary, l2.list_id AS common, l3.list_id AS primary
        FROM lists AS l1, lists AS l2, lists AS l3
        WHERE
          l1.list_id = list
          AND
          l2.parent = l1.parent
          AND
          l2.branch = l1.branch
          AND
          l3.parent = l2.list_id
          AND
          l3.branch = branches[d]
        ORDER BY l2.revision DESC, l3.revision DESC
        
        INSERT INTO lists (parent,revision,branch) VALUES (list,1,branches[d]);
        new := currval(list_ids);
      
        list_merge(common,primary,secondary,new);
        r := 1;
      END IF;
    END IF;
    SELECT INTO rec list_id,branch,revision FROM lists WHERE parent = revisions[d*3-5] AND branch = branches[d] AND revision = r;
    
    revisions[d*3-2] := rec.list_id;
    revisions[d*3-1] := rec.branch;
    revisions[d*3]   := rec.revision;
    
    d := d + 1;
    EXIT WHEN branches[d] = NULL;
  END;
  RETURN list;
END;

FUNCTION list_getrevision(INTEGER) RETURNS INTEGER[]
DECLARE
  list_id ALIAS FOR $1;
  revisions INTEGER[]
BEGIN
        revs INTEGER;
      d INTEGER;
      rec RECORD;
      
      d := 0;
      LOOP
        d = d + 2;
        IF revs[d] IS NULL THEN BREAK; END IF;
        SELECT INTO rec branch, revision, parent WHERE list_id = list;
        IF NOT FOUND THEN RAISE EXCEPTION ''Broken List''; END IF;
        revs[d-1] := revision;
        revs[d] := branch;
        list = rec.parent
      END LOOP;

      max := d - 2;
      d := 1;
      LOOP
        IF d > max/2 THEN BREAK;
        
        temp := revs[d];
        revs[d] := revs[max - (d - 1)];
        revs[max - (d - 1)] := temp;

        d := d + 1;
      END LOOP;

END;

FUNCTION list_sublist(INTEGER, INTEGER[]) RETURNS INTEGER[]
DECLARE
  branch1 AS ALIAS FOR $1;
  revisions AS ALIAS FOR $2;
BEGIN
  
END;


CREATE SEQUENCE list_item_ids INCREMENT 1 START 1;

CREATE FUNCTION list_latest(integer, integer) RETURNS INTEGER AS'
  DECLARE
    b1_ ALIAS FOR $1;
    b2_ ALIAS FOR $2;
    list_id_ INTEGER;
  BEGIN
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_newbranch(INTEGER,INTEGER,INTEGER)
  DECLARE
    topic_id_ ALIAS FOR $1;
    b1_ ALIAS FOR $2;
    r1_ ALIAS FOR $3;
    list_id_ INTEGER;
  BEGIN
    INSERT INTO lists (b1, r1, b2, r2) VALUES (b1_,r1_,(SELECT MAX(b2)+1 FROM lists WHERE b1 = b1_ AND r1 = r1_
    UPDATE topics
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_merge(common_id, 


CREATE FUNCTION list_newbranch(b1,r1)
  DECLARE
    b1_ ALIAS FOR $1;
    list_id_ INTEGER;
  BEGIN
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_latest(integer, integer) RETURNS INTEGER AS '
  DECLARE
    list_head_ ALIAS FOR $1;
    branch_    ALIAS FOR $2;
    list_id_ INTEGER;
    rev SMALLINT;
  BEGIN
    SELECT INTO rev MAX(revision) FROM lists WHERE list_head = list_head_ AND branch = 0;
    IF rev IS NULL THEN RETURN NULL; END IF;
    
    IF branch_ IS NULL THEN
      SELECT INTO list_id_ list_id FROM lists WHERE list_head = list_head_ AND branch = 0 AND revision = rev;
      RETURN list_id_;
    ELSE  
      SELECT INTO list_id_ list_id FROM lists WHERE list_head = list_head_ AND branch = branch_ AND revision = rev;
      IF FOUND THEN
        RETURN list_id_;
      ELSE  
        RETURN list_update(list_head_, branch_);
      END IF;
    END IF;  
  END;  
' LANGUAGE 'plpgsql';

drop FUNCTION array_print(INTEGER[]);
CREATE FUNCTION array_print(INTEGER[]) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
    i INTEGER;
    j INTEGER;
  BEGIN
    i := 1;
    LOOP
      j := a[i];
      RAISE NOTICE ''j = %'', j;
      EXIT WHEN j = 0 OR j IS NULL;
      i := i + 1;
    END LOOP;
    RETURN NULL;
  END;  
' LANGUAGE 'plpgsql';



select array_print('{34,35,567,56,56,34,0}');






































DROP TABLE lists;
DROP SEQUENCE list_ids;
DROP SEQUENCE list_heads;
DROP TABLE list_items;
DROP FUNCTION list_update (integer, integer);
DROP FUNCTION list_revision(integer, integer);

CREATE TABLE lists
(
  list_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('list_ids'),
  parent INTEGER
  branch INTEGER,
  revision INTEGER,
  merged INTEGER REFERENCES lists(list_id),
  modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  UNIQUE(parent, branch, revision)
);

CREATE SEQUENCE list_ids INCREMENT 1 START 1;
CREATE SEQUENCE list_b1s INCREMENT 1 START 1;

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  list_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER
);

CREATE INDEX list_idx ON list_items(list_id);




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


CREATE FUNCTION list_update (integer, integer) RETURNS INTEGER AS '
  DECLARE
    list_head_ ALIAS FOR $1;
    branch_    ALIAS FOR $2;
    rec RECORD;
    headid INTEGER;
    headrev INTEGER;
    branchid INTEGER;
    branchrev INTEGER;
    commonid INTEGER;
    newid INTEGER;
    ord INTEGER;
    ck INTEGER;
    nk INTEGER;

  BEGIN
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = list_head_ AND branch = 0 ORDER BY revision DESC LIMIT 1;
    IF NOT FOUND THEN RETURN NULL; END IF;
    headid := rec.list_id;
    headrev := rec.revision;
    
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = list_head_ AND branch = branch_ ORDER BY revision DESC LIMIT 1;
    IF NOT FOUND THEN RETURN NULL; END IF;
    IF rec.revision = headrev THEN RETURN 1; END IF;
    branchid := rec.list_id;
    branchrev := rec.revision;
    
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = list_head_ AND branch = 0 AND revision = branchrev;
    IF NOT FOUND THEN RETURN NULL; END IF;
    commonid := rec.list_id;

    INSERT INTO lists(list_head, revision, branch) VALUES(list_head_, headrev, branch_);
    newid := currval(''list_ids'');

    -- Create new list based on branch but without items that were deleted in the head list. 
    
    ord := 0;
    FOR rec IN 
      SELECT li.item_id
      FROM list_items AS li
      WHERE
        li.list_id = branchid
      AND NOT -- deleted
      (
        EXISTS (SELECT 1 FROM list_items AS c WHERE c.list_id = commonid AND c.item_id = li.item_id)
        AND
        NOT EXISTS (SELECT 1 FROM list_items AS h WHERE h.list_id = headid AND h.item_id = li.item_id)
      )
      ORDER BY li.ordinal
    LOOP
      INSERT INTO list_items(list_id, ordinal, item_id) VALUES (newid, ord, rec.item_id);
      ord := ord + 1;      
    END LOOP;
    
    -- Insert new items from HEAD into the new list

    ord := 0;  
    FOR rec IN SELECT ordinal, item_id FROM list_items WHERE list_id = headid ORDER BY ordinal LOOP

      SELECT INTO ck ordinal FROM list_items WHERE list_id = commonid AND item_id = rec.item_id;
      IF NOT FOUND THEN ck := NULL; END IF;

      SELECT INTO nk ordinal FROM list_items WHERE list_id = newid AND item_id = rec.item_id;
      IF NOT FOUND THEN nk := NULL; END IF;

      IF ck IS NULL AND nk IS NULL THEN -- needs to be inserted
        UPDATE list_items SET ordinal = ordinal + 1 WHERE list_id = newid AND ordinal >= ord;
        INSERT INTO list_items(list_id, ordinal, item_id) VALUES (newid, ord, rec.item_id);
        nk := ord;
      END IF;

      IF (nk IS NOT NULL) THEN ord := nk + 1; END IF;
   
    END LOOP;
    RETURN newid;
    
    -- TODO: come back to this code in a few weeks, and see if you can
    -- make this code less convoluted. I like the behavior, but I suspect
    -- a more direct and efficient implmentation is possible.
    
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






































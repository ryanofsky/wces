DROP FUNCTION array_length(INTEGER[]);
DROP FUNCTION array_integer_cast(TEXT);
DROP FUNCTION array_integer_starts(INTEGER[], INTEGER[]);
DROP FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_branch_update(INTEGER[], INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_branch_generate();
DROP FUNCTION list_branch_generate(INTEGER, TEXT, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_ancestor_generate();
DROP FUNCTION list_contents(INTEGER);
DROP FUNCTION list_save_start(INTEGER, INTEGER);
DROP FUNCTION list_save_end(INTEGER, INTEGER);
DROP FUNCTION list_update(INTEGER);

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

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  list_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);
CREATE TABLE list_branch_cache
(
  branch_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_branch_ids'),
  branch INTEGER[] NOT NULL,
  parent INTEGER,
  outdated BOOLEAN DEFAULT 'f',
  
  -- latest list revisions
  
  latest_id INTEGER NOT NULL,
  content_id INTEGER NOT NULL
);
CREATE SEQUENCE list_branch_ids INCREMENT 1 START 1;
CREATE SEQUENCE list_item_ids INCREMENT 1 START 1;

CREATE INDEX list_idx ON list_items(list_id);
CREATE TABLE list_ancestor_cache
(
  ancestor_id INTEGER NOT NULL,
  descendant_id INTEGER NOT NULL
);


CREATE FUNCTION array_length(INTEGER[]) RETURNS INTEGER AS '
DECLARE
  a ALIAS FOR $1;
  i INTEGER := 1;
BEGIN
  WHILE a[i] IS NOT NULL LOOP
    i := i + 1;
  END LOOP;
  RETURN i - 1;
END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION array_integer_cast(TEXT) RETURNS INTEGER[] AS '
BEGIN
  RETURN $1;
END;
' LANGUAGE 'plpgsql';

-- true if b[1:length(a)] == a && a != b
CREATE FUNCTION array_integer_starts(INTEGER[], INTEGER[]) RETURNS BOOLEAN AS '
DECLARE
  a ALIAS FOR $1;
  b ALIAS FOR $2;
  i INTEGER := 1;
BEGIN
  WHILE a[i] IS NOT NULL LOOP
    IF NOT (a[i] = b[i]) THEN RETURN ''f''; END IF;
    i := i + 1;
  END LOOP;
  RETURN b[i] IS NOT NULL;
END;
' LANGUAGE 'plpgsql';

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
    RAISE NOTICE ''list_merge(%,%,%,%)'', common_id, primary_id, secondary_id, new_id;
    
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

-- update a row in the branches table, called by list_branch_generate
CREATE FUNCTION list_branch_update(INTEGER[], INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_     ALIAS FOR $1;
    depth       ALIAS FOR $2;
    latest_id_  ALIAS FOR $3;
    content_id_ ALIAS FOR $4;
    parent_id   ALIAS FOR $5;
    rec RECORD;
  BEGIN
    
    -- look for this branch

    SELECT INTO rec branch_id FROM list_branch_cache WHERE branch = branch_;

    IF FOUND THEN

      -- outdate all of the descendants
      
      UPDATE list_branch_cache
        SET outdated = ''t''
      WHERE array_integer_starts(branch_, branch);
      
      -- update the branch information
      
      UPDATE list_branch_cache
        SET
          latest_id = latest_id_,
          content_id = content_id_,
          outdated = ''f''
      WHERE branch_id = rec.branch_id;
      
      RETURN rec.branch_id;

    ELSE
      -- insert a new row for this branch
           
      INSERT INTO list_branch_cache(branch, parent, latest_id, content_id, outdated)
      VALUES (branch_, parent_id, latest_id_, content_id_, ''f'');
      
      return currval(''list_branch_ids'');
    END IF;
  END;
' LANGUAGE 'plpgsql';


-- fill up branch cache based on entries in the lists table
CREATE FUNCTION list_branch_generate() RETURNS INTEGER AS '
  DECLARE
    rec RECORD;
    branchstring TEXT;
    pbr INTEGER;
  BEGIN
    LOCK TABLE list_branch_cache IN SHARE ROW EXCLUSIVE MODE;
    LOCK TABLE lists IN SHARE ROW EXCLUSIVE MODE;
    FOR rec IN SELECT list_id, branch FROM lists WHERE parent IS NULL ORDER BY branch, revision LOOP
      branchstring := rec.branch;
      pbr := list_branch_update(array_integer_cast(''{''||branchstring||''}''), 1, rec.list_id, rec.list_id, NULL::integer);
      PERFORM list_branch_generate(rec.list_id, branchstring, 2, rec.list_id, pbr);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';  

-- called only by list_branch_generate()
CREATE FUNCTION list_branch_generate(INTEGER, TEXT, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    parent_id    ALIAS FOR $1;
    branchstring ALIAS FOR $2;
    depth        ALIAS FOR $3;
    content_id   ALIAS FOR $4;
    parentbr     ALIAS FOR $5;
    rec          RECORD;
    content      INTEGER;
    pbr          INTEGER;
    brs          TEXT;
  BEGIN
    FOR rec IN SELECT list_id, revision, branch FROM lists WHERE parent = parent_id ORDER BY branch, revision LOOP
      brs := branchstring || '','' || rec.branch;
      content := CASE WHEN rec.revision = 0 THEN content_id ELSE rec.list_id END;
      pbr := list_branch_update(array_integer_cast(''{''||brs||''}''), depth, rec.list_id, content, parentbr);
      PERFORM list_branch_generate(rec.list_id, brs, depth+1, content, pbr);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

-- generate entries in the list_ancestor_cache table
CREATE FUNCTION list_ancestor_generate() RETURNS INTEGER AS '
  DECLARE
    rec RECORD;
  BEGIN
    LOCK TABLE list_branch_cache IN SHARE ROW EXCLUSIVE MODE;
    TRUNCATE list_ancestor_cache;
    FOR rec IN SELECT branch_id, branch FROM list_branch_cache LOOP
      INSERT INTO list_ancestor_cache (ancestor_id, descendant_id)
        SELECT rec.branch_id, branch_id
        FROM list_branch_cache
        WHERE array_integer_starts(rec.branch, branch);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';  

-- if list_id passed is a 0 revision, return the list_id that contains the list contents
CREATE FUNCTION list_contents(INTEGER) RETURNS INTEGER AS'
  DECLARE
    list_id_ ALIAS FOR $1;
    l INTEGER;
    i INTEGER;
  BEGIN
    l := list_id_;
    LOOP
      SELECT INTO i parent FROM lists WHERE list_id = l AND revision = 0;
      IF NOT FOUND THEN
        RETURN l;
      ELSE
        l := i;
      END IF;
    END LOOP;
  END;
' LANGUAGE 'plpgsql';

-- list_save_start and list_save_end are used to save edits to an already existing
-- list. To use them, perform the following steps within a *single* transaction.
--
-- 1) Call list_save_start() with the branch_id of the list and the list_id of 
--    the original revision of the list that was being edited. The function
--    will return 0 if the list_id is invalid or not on the branch, branch_id.
--    Otherwise it will create a new entry in the lists table and return
--    that list_id.
-- 
-- 2) Store the contents of the edited list in the list_items table,
--    referencing the list_id returned by the first function call.
--    
-- 3) Call list_save_end(). This call merges in any changes to the list made by
--    other processes that occured as this list was being edited.
      
-- list_save_start creates an entry for a new list revision in the lists table,
-- and returns the list_id of this entry. After this function is called, the
-- list contents should be inserted into the list_items table using the
-- list_id, and list_save_end to should be called to follow up. 

CREATE FUNCTION list_save_start(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    orig_id    ALIAS FOR $2;
    info RECORD;
    rec RECORD;
    p INTEGER;
    l INTEGER;
    i INTEGER;
    depth INTEGER;
    ret INTEGER;
  BEGIN
    -- lock the branch
    SELECT INTO info branch, latest_id, content_id
    FROM list_branch_cache
    WHERE branch_id = branch_id_ FOR UPDATE;
    
    depth := array_length(info.branch);
    
    -- check for valid orig_id, return null if its on the wrong branch
    l := orig_id;
    i := depth;
    LOOP
      SELECT INTO p parent FROM lists WHERE list_id = l AND branch = info.branch[i];
      
      IF NOT FOUND THEN RETURN NULL; END IF; -- bad branch number or list doesn''t exist
      
      IF p IS NULL OR i = 1 THEN -- time to quit
        IF p IS NULL AND i = 1 THEN
          EXIT;
        ELSE -- bad branch number
          RETURN NULL;
        END IF;
      END IF;
      
      l := p;
      i := i - 1;
    END LOOP;

    -- select the latest revision
      
    SELECT INTO rec parent, branch, revision FROM lists WHERE list_id = info.latest_id;
    
    -- insert a new revision right after
      
    INSERT INTO lists (parent, branch, revision)
      VALUES (rec.parent, rec.branch, rec.revision+1);

    ret := currval(''list_ids'');
      
    IF orig_id <> info.latest_id  THEN -- merge needed
      INSERT INTO lists (parent, branch, revision, merged)
        VALUES (rec.parent, rec.branch, rec.revision+2, orig_id);
    END IF;

    -- point to new revision
    
    UPDATE list_branch_cache SET
      latest_id  = currval(''list_ids''),
      content_id = currval(''list_ids'')
    WHERE branch_id = branch_id_;

    RETURN ret;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_save_end(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    save_id ALIAS FOR $2;
    saved RECORD;
    merged RECORD;
    latest_id INTEGER;
    merge_id INTEGER;
  BEGIN    
    
    -- get information about the just saved list revision
    SELECT INTO saved parent, branch, revision FROM lists WHERE list_id = save_id;
    IF NOT FOUND THEN
      RETURN NULL;
    END IF;
    
    -- mark sub branches for merge
    
    UPDATE list_branch_cache SET outdated = ''t'' WHERE branch_id IN 
      (SELECT descendant_id FROM list_ancestor_cache WHERE ancestor_id = branch_id_);      
    
    -- get the merge revision, if it exists
    
    SELECT INTO merged list_id, merged AS orig_id FROM lists WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch = saved.branch AND revision = saved.revision + 1;
    
    IF NOT FOUND THEN return save_id; END IF;

    -- merge needed, retrieve most recent list revision prior to save

    SELECT INTO latest_id list_id FROM lists WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch = saved.branch AND revision = saved.revision - 1;
    
    IF NOT FOUND THEN
      RAISE EXCEPTION ''Can''''t find predecessor for list revision %'', save_id;
    END IF;

    -- merge changes
          
    PERFORM list_merge
    (
      list_contents(merged.orig_id),
      save_id,
      list_contents(latest_id),
      merged.list_id
    );
      
    RETURN merged.list_id;
  END;
' LANGUAGE 'plpgsql';

-- given a branch id this function return the latest revision on the branch.
-- If the latest existing revision is out of date

CREATE FUNCTION list_update(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    info RECORD;
    rec RECORD;
    rec1 RECORD;
    depth INTEGER;
    s_bids TEXT;
    s_lids TEXT;
    s_plids TEXT;
    sep TEXT;
    i INTEGER;
    j INTEGER;
    bid INTEGER;
    par INTEGER;
    oldpar INTEGER;
  BEGIN
    
    -- get information about the branch
    
    SELECT INTO info branch, outdated, latest_id, content_id
    FROM list_branch_cache WHERE branch_id = branch_id_;
    IF NOT FOUND THEN
      RETURN NULL;
    END IF;

    IF NOT info.outdated THEN RETURN info.latest_id; END IF; 
 
    depth := array_length(info.branch);    
      
    -- current version of plpgsql (7.1.3) allows only read access to arrays,
    -- it is not possible to declare array variables or make assignments
    -- into arrays. this code gets around that restriction by putting
    -- numbers into strings, then casting those strings into array
    -- columns in a RECORD variable.
    
    s_bids := ''''; -- stringed array of branch_ids for each branch level
    s_lids := ''''; -- stringed array of list_ids for each branch level
    s_plids := ''''; -- stringed array of parent list_ids for each branch level
    sep = '''';    

    -- loop to lock and gather information about ancestor branches

    i := depth;
    bid := branch_id_;    

    LOOP
      SELECT INTO rec b.latest_id AS lid, b.parent AS pbid, l.parent AS plid
        FROM list_branch_cache AS b
        INNER JOIN lists AS l ON l.list_id = b.latest_id
        WHERE b.branch_id = bid AND b.outdated FOR UPDATE;

      IF NOT FOUND THEN EXIT; END IF;
      
      s_bids := s_bids || sep || bid::TEXT;
      s_lids := s_lids || sep || rec.lid::TEXT;
      s_plids := s_plids || sep || rec.plid::TEXT;

      i := i - 1;
      bid := rec.pbid;
      sep := '','';
    END LOOP;

    IF i < 1 THEN
      THROW EXCEPTION ''no way'';
    END IF;
 
    s_bids := ''{'' || s_bids || ''}'';
    s_lids := ''{'' || s_lids || ''}'';
    s_plids := ''{'' || s_plids || ''}'';
 
    RAISE NOTICE ''s_bids = "%"'', s_bids;
    
    SELECT INTO rec 
      array_integer_cast(s_bids) AS bids,
      array_integer_cast(s_lids) AS lids,
      array_integer_cast(s_plids) AS plids;
    --
    --  s_bids::INTEGER[] AS bids,
    --  s_lids::INTEGER[] AS lids,
    --  s_plids::INTEGER[] AS plids;
      
    -- loop to bring ancestral branches up to date
    
    SELECT INTO par latest_id FROM list_branch_cache WHERE branch_id = bid;
    FOR j IN i+1..depth LOOP
          RAISE NOTICE ''hallo3'';
      INSERT INTO lists(parent,branch,revision,merged) VALUES 
      (par, info.branch[j], 1, rec.lids[depth-j+1]);
      oldpar := par;
      par := currval(''list_ids'');
      
      PERFORM list_merge(rec.plids[depth-j+1], rec.lids[depth-j+1], oldpar, par);
      
      UPDATE list_branch_cache SET
        outdated = false, latest_id = par, content_id = par
      WHERE branch_id = bid;
            
    END LOOP;
    
    RETURN par;
  END;  
' LANGUAGE 'plpgsql';
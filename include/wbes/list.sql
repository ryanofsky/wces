DROP TABLE revisions
DROP SEQUENCE revision_ids 
DROP TABLE branches
DROP SEQUENCE branch_ids
DROP TABLE branch_ancestor_cache
DROP TABLE list_items
DROP SEQUENCE list_item_ids;
DROP FUNCTION array_integer_length(INTEGER[]);
DROP FUNCTION array_integer_cast(TEXT);
DROP FUNCTION branch_generate();
DROP FUNCTION branch_generate(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION branch_ancestor_generate();
DROP FUNCTION branch_ancestor_generate(INTEGER, INTEGER);
DROP SEQUENCE branch_top;
DROP FUNCTION branch_nextval(INTEGER);
DROP FUNCTION revision_contents(INTEGER);
DROP FUNCTION revision_save_start(INTEGER, INTEGER);
DROP FUNCTION revision_save_end(INTEGER, INTEGER);
DROP FUNCTION revision_update(INTEGER);
DROP FUNCTION revision_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION merge_integer(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION merge_text(TEXT, TEXT, TEXT, BOOLEAN);

CREATE TABLE revisions
(
  revision_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('revision_ids'),
  parent INTEGER,
  branch_id INTEGER NOT NULL,
  revision INTEGER NOT NULL,
  merged INTEGER,
  type INTEGER,
  date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  UNIQUE(parent, branch_id, revision)
);
CREATE SEQUENCE revision_ids INCREMENT 1 START 100;

CREATE TABLE branches
(
  branch_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('branch_ids'),
  branch INTEGER NOT NULL,

  -- hereafter fields are redundant but included for efficiency
  parent INTEGER,  
  outdated BOOLEAN DEFAULT 't',
  latest_id INTEGER,
  content_id INTEGER,
  nextbranch INTEGER NOT NULL DEFAULT 1,
  UNIQUE(parent, branch)
);
CREATE SEQUENCE branch_ids INCREMENT 1 START 100;

-- this entire table is redundant
CREATE TABLE branch_ancestor_cache
(
  ancestor_id INTEGER NOT NULL,
  descendant_id INTEGER NOT NULL
);

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  revision_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);

CREATE SEQUENCE list_item_ids INCREMENT 1 START 100;
CREATE INDEX revision_idx ON list_items(revision_id);

CREATE FUNCTION array_integer_length(INTEGER[]) RETURNS INTEGER AS '
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

-- update redundant fields in branches table
CREATE FUNCTION branch_generate() RETURNS INTEGER AS '
  BEGIN
    LOCK TABLE branches IN SHARE ROW EXCLUSIVE MODE;
    LOCK TABLE revisions IN SHARE ROW EXCLUSIVE MODE;
    UPDATE branches SET outdated = ''t'', parent = NULL;
    PERFORM branch_generate(NULL, NULL, NULL, ''f'');
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';  

CREATE FUNCTION branch_generate(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    parent_revision ALIAS FOR $1; 
    parent_branch   ALIAS FOR $2;
    pcontent_id     ALIAS FOR $3;
    poutdated       ALIAS FOR $4; -- parent revision is outdated
    last_branch_id  INTEGER := 0; -- d.branch_id from previous loop iteration
    last_branch     INTEGER := 0; -- d.branch from previous loop iteration
    next_branch     INTEGER;      -- result of recursive call
    new_branch      BOOLEAN;      -- d.revision_id is the newest revision
    outdated_       BOOLEAN;      -- d.revision_id is outdated
    visted          BOOLEAN;      -- already visited this branch
    content         INTEGER;      -- first nonzero ancestor of d.revision_id
    d               RECORD;
  BEGIN
    FOR d IN 
      SELECT r.revision_id, r.revision, b.branch_id, b.branch, b.parent
      FROM revisions AS r INNER JOIN branches AS b USING(branch_id)
      WHERE r.parent = parent_revision OR (r.parent IS NULL AND parent_revision IS NULL)
      ORDER BY b.branch, r.revision DESC
    LOOP
      new_branch := d.branch_id <> last_branch_id;
      outdated_ := poutdated OR NOT new_branch;
      content := CASE WHEN d.revision = 0 THEN pcontent_id ELSE d.revision_id END;
      next_branch := branch_generate(d.revision_id, d.branch_id, content, outdated_);
      IF new_branch THEN
        IF d.parent IS NULL THEN
          UPDATE branches SET
            parent = parent_branch,
            latest_id = d.revision_id, 
            content_id = content,
            outdated = outdated_,
            nextbranch = next_branch
          WHERE branch_id = d.branch_id;
        END IF;
        last_branch := d.branch;
        last_branch_id = d.branch_id;
      END IF; 
    END LOOP;  
    RETURN last_branch + 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION branch_ancestor_generate() RETURNS INTEGER AS '
  DECLARE
    rec RECORD;
  BEGIN
    LOCK TABLE branches IN SHARE ROW EXCLUSIVE MODE;
    TRUNCATE branch_ancestor_cache;
    FOR rec IN SELECT branch_id FROM BRANCHES WHERE parent IS NULL LOOP
      PERFORM branch_ancestor_generate(rec.branch_id, rec.branch_id);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

-- 3 steps:
-- 1) Install ancestor_id as an ancestor of branch_id
-- 2) Install ancestor_id as an ancestor of branch_id's descendants
-- 3) Install descendants of branch_id's descendants.
CREATE FUNCTION branch_ancestor_generate(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    ancestor_id_ ALIAS FOR $1;
    branch_id_   ALIAS FOR $2;
    rec RECORD;
    eq BOOLEAN;
  BEGIN
    eq := ancestor_id_ = branch_id_;
    
    IF NOT eq THEN -- step 1
      INSERT INTO branch_ancestor_cache(ancestor_id, descendant_id)
      VALUES(ancestor_id_, branch_id_);
    END IF;
    
    FOR rec IN SELECT branch_id FROM branches WHERE parent = branch_id_ LOOP
      PERFORM branch_ancestor_generate(ancestor_id_, rec.branch_id); -- step 2
      IF eq THEN -- step 3
        PERFORM branch_ancestor_generate(rec.branch_id, rec.branch_id); 
      END IF;
    END LOOP;
    RETURN 1;    
  END;
' LANGUAGE 'plpgsql';

CREATE SEQUENCE branch_top INCREMENT 1 START 100;

-- for efficiency, should be called outside of other transactions, otherwise
-- branch_id will be locked unneccesarily
CREATE FUNCTION branch_nextval(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    ret INTEGER;
  BEGIN
    IF branch_id_ is NULL THEN
      ret := nextval(''branch_top'');
    ELSE
      SELECT INTO ret nextbranch FROM branches WHERE branch_id = branch_id_ FOR UPDATE;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''branch_nextval(%) fails. branch_id not found'', $1;
      END IF;
      UPDATE branches SET nextbranch = nextbranch + 1 WHERE branch_id = branch_id;
    END IF;
    RETURN ret;
  END;
' LANGUAGE 'plpgsql';  

-- if revision_id passed is a 0 revision, return the revision_id that contains the revision contents
CREATE FUNCTION revision_contents(INTEGER) RETURNS INTEGER AS'
  DECLARE
    revision_id_ ALIAS FOR $1;
    r INTEGER;
    i INTEGER;
  BEGIN
    r := revision_id_;
    LOOP
      SELECT INTO i parent FROM revisions WHERE revision_id = r AND revision = 0;
      IF NOT FOUND THEN RETURN r; ELSE r := i; END IF;
    END LOOP;
  END;
' LANGUAGE 'plpgsql';

-- revision_save_start and revision_save_end are used to save edits to an already existing
-- branch. To use them, perform the following steps within a *single* transaction.
--
-- 1) Call revision_save_start() with the branch_id of the revision and the revision_id of 
--    the original revision of the revision that was being edited. The function
--    will return NULL if the revision_id is invalid or not on the branch, branch_id.
--    Otherwise it will create a new entry in the revisions table and return
--    that revision_id.
-- 
-- 2) Store the contents of the edited revision using the the revision_id returned by the
--    first function call.
--    
-- 3) Call revision_save_end(). This call merges in any changes to the revision made by
--    other processes that occured as this revision was being edited.
      
-- revision_save_start creates an entry for a new revision revision in the revisions table,
-- and returns the revision_id of this entry. After this function is called, the
-- revision contents should be inserted into the revision_items table using the
-- revision_id, and revision_save_end to should be called to follow up. 

CREATE FUNCTION revision_save_start(INTEGER, INTEGER) RETURNS INTEGER AS '
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
    FROM branches
    WHERE branch_id = branch_id_ FOR UPDATE;
    
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_start(%,%) fails. branch_id not found'', $1, $2;
    END IF;
    
    -- check for valid orig_id, return null if its on the wrong branch
    SELECT INTO i branch_id FROM revisions WHERE revision_id = orig_id;
    IF i IS NULL OR branch_id_ <> i THEN RETURN NULL; END IF;
    
    -- select the latest revision
      
    SELECT INTO rec parent, branch_id, revision FROM revisions WHERE revision_id = info.latest_id;
    
    -- insert a new revision right after
      
    INSERT INTO revisions (parent, branch_id, revision)
      VALUES (rec.parent, branch_id_, rec.revision+1);

    ret := currval(''revision_ids'');
      
    IF orig_id <> info.latest_id  THEN -- merge needed
      INSERT INTO revisions (parent, branch_id, revision, merged)
        VALUES (rec.parent, branch_id_, rec.revision+2, orig_id);
    END IF;

    -- point to new revision
    
    UPDATE branches SET
      latest_id  = currval(''revision_ids''),
      content_id = currval(''revision_ids'')
    WHERE branch_id = branch_id_;

    RETURN ret;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION revision_save_end(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    save_id ALIAS FOR $2;
    saved RECORD;
    merged RECORD;
    latest_id INTEGER;
    merge_id INTEGER;
  BEGIN    
    
    -- get information about the just saved revision
    SELECT INTO saved parent, branch_id, revision FROM revisions WHERE revision_id = save_id;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_end(%,%) fails. save_id not found'', $1, $2;
    END IF;      
    
    -- mark sub branches for merge
    
    UPDATE branches SET outdated = ''t'' WHERE branch_id IN 
      (SELECT descendant_id FROM branch_ancestor_cache WHERE ancestor_id = branch_id_); 

    -- get the merge revision, if it exists
    
    SELECT INTO merged revision_id, merged AS orig_id FROM revisions WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch_id = saved.branch_id AND revision = saved.revision + 1;
    
    IF NOT FOUND THEN return save_id; END IF;

    -- merge needed, retrieve most recent revision prior to save

    SELECT INTO latest_id revision_id FROM revisions WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch_id = saved.branch_id AND revision = saved.revision - 1;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_end(%,%) fails. Can''''t find predecessor for revision %'', $1, $2, save_id;
    END IF;

    -- merge changes
          
    PERFORM revision_merge
    (
      revision_contents(merged.orig_id),
      save_id,
      revision_contents(latest_id),
      merged.revision_id
    );
    RETURN merged.revision_id;
  END;
' LANGUAGE 'plpgsql';

-- given a branch id this function return the latest revision on the branch.
-- If the latest existing revision is out of date

CREATE FUNCTION revision_update(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    info RECORD;
    rec RECORD;
    rec1 RECORD;
    s_bids TEXT;
    s_rids TEXT;
    s_prids TEXT;
    s_revs TEXT;
    sep TEXT;
    i INTEGER;
    j INTEGER;
    bid INTEGER;
    par INTEGER;
    oldpar INTEGER;
  BEGIN
    
    -- get information about the branch
    
    SELECT INTO info branch, outdated, latest_id, content_id
    FROM branches WHERE branch_id = branch_id_;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_update(%) called with invalid branch_id'', $1;
    END IF;

    IF NOT info.outdated THEN RETURN info.latest_id; END IF; 
 
    -- current version of plpgsql (7.1.3) allows only read access to arrays,
    -- it is not possible to declare array variables or make assignments
    -- into arrays. this code gets around that restriction by putting
    -- numbers into strings, then casting those strings into array
    -- columns in a RECORD variable.
    
    s_bids  := ''''; -- stringed array of branch_ids for each branch level
    s_rids  := ''''; -- stringed array of revision_ids for each branch level
    s_prids := ''''; -- stringed array of parent revision_ids for each branch level
    s_revs  := ''''; -- stringed array of revision numbers for each branch level
    sep     := '''';    

    -- loop to lock and gather information about ancestor branches

    i := 0;
    bid := branch_id_;    

    LOOP
      SELECT INTO rec b.latest_id AS rid, b.parent AS pbid, r.parent AS prid, r.revision AS rev
        FROM branches AS b
        INNER JOIN revisions AS r ON r.revision_id = b.latest_id
        WHERE b.branch_id = bid AND b.outdated FOR UPDATE;

      IF NOT FOUND THEN EXIT; END IF;
      
      s_bids  := s_bids  || sep || bid::TEXT;
      s_rids  := s_rids  || sep || rec.rid::TEXT;
      s_prids := s_prids || sep || rec.prid::TEXT;
      s_revs  := s_revs  || sep || rec.rev::TEXT;
      
      i := i + 1;
      bid := rec.pbid;
      sep := '','';
    END LOOP;

    IF i < 1 THEN
      RAISE EXCEPTION ''revision_update(%) fails. impossible condition reached'', $1;
    END IF;
 
    s_bids := ''{'' || s_bids || ''}'';
    s_rids := ''{'' || s_rids || ''}'';
    s_prids := ''{'' || s_prids || ''}'';
    s_revs := ''{'' || s_revs || ''}'';
 
    SELECT INTO rec 
      array_integer_cast(s_bids) AS bids,
      array_integer_cast(s_rids) AS rids,
      array_integer_cast(s_prids) AS prids,
      array_integer_cast(s_revs) AS revs;
      
    -- loop to bring ancestral branches up to date
    
    SELECT INTO par latest_id FROM branches WHERE branch_id = bid;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_update(%) fails. impossible condition reached'', $1;
    END IF;
    
    FOR j IN REVERSE i..1 LOOP
      IF rec.revs[j] = 0 THEN
        INSERT INTO revisions(parent,branch_id,revision,merged) VALUES (par, rec.bids[j], 0, rec.rids[j]);
      ELSE
        INSERT INTO revisions(parent,branch_id,revision,merged) VALUES (par, rec.bids[j], 1, rec.rids[j]);
        PERFORM revision_merge(rec.prids[j], rec.rids[j], par, currval(''revision_ids''));
      END IF;
      
      oldpar := par;
      par := currval(''revision_ids'');
      UPDATE branches SET
        outdated = false, latest_id = par, content_id = par
      WHERE branch_id = rec.bids[j];
    END LOOP;
    
    RETURN par;
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION revision_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    common_id    ALIAS FOR $1;
    primary_id   ALIAS FOR $2;
    secondary_id ALIAS FOR $3;
    new_id       ALIAS FOR $4;
    c INTEGER := 0;
    t INTEGER;
    d RECORD;
  BEGIN
    -- RAISE NOTICE ''revision_merge(%,%,%,%) called'', $1, $2, $3, $4;
    
    -- make sure that revision_ids are valid and unique and of the same type
    FOR d IN SELECT type FROM revisions WHERE revision_id IN (common_id,primary_id,secondary_id,new_id) LOOP
      IF c = 0 THEN
        t = d.type;
      ELSE
        IF NOT (t = d.type) THEN t := NULL; EXIT; END IF;
      END IF;
      c := c + 1;
    END LOOP;
    
    IF c <> 4 OR t IS NULL THEN
      RAISE EXCEPTION ''revision_merge(%,%,%,%) called with one or more invalid arguments'', $1, $2, $3, $4;
    END IF;
     
     IF t = 1 THEN
       RETURN list_merge(common_id, primary_id, secondary_id, new_id);
     END IF;
     
     RAISE EXCEPTION ''revision_merge(%,%,%,%) failed. Revision type unknown.'', $1, $2, $3, $4;
  END;
' LANGUAGE 'plpgsql'; 

CREATE FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    common_id    ALIAS FOR $1;
    primary_id   ALIAS FOR $2;
    secondary_id ALIAS FOR $3;
    new_id       ALIAS FOR $4;
    rec RECORD;
    ord INTEGER := 0;
    ck INTEGER;
    nk INTEGER;
  BEGIN
    SELECT INTO ck COUNT(*) FROM list_items WHERE revision_id = new_id;
    IF NOT ck = 0 THEN
      RAISE EXCEPTION ''list_merge(%,%,%,%) failed. List % is not empty.'', $1, $2, $3, $4, $4;
    END IF;
    
    -- Create new list based on primary but without items that were deleted in the secondary list. 
   
    FOR rec IN 
      SELECT li.item_id
      FROM list_items AS li
      WHERE
        li.revision_id = primary_id
      AND NOT -- deleted
      (
        EXISTS (SELECT 1 FROM list_items AS c WHERE c.revision_id = common_id AND c.item_id = li.item_id)
        AND
        NOT EXISTS (SELECT 1 FROM list_items AS h WHERE h.revision_id = secondary_id AND h.item_id = li.item_id)
      )
      ORDER BY li.ordinal
    LOOP
      INSERT INTO list_items(revision_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
      ord := ord + 1;      
    END LOOP;
    
    -- Insert new items from secondary into the new list

    ord := 0;  
    FOR rec IN SELECT ordinal, item_id FROM list_items WHERE revision_id = secondary_id ORDER BY ordinal LOOP

      SELECT INTO ck ordinal FROM list_items WHERE revision_id = common_id AND item_id = rec.item_id;
      IF NOT FOUND THEN ck := NULL; END IF;

      SELECT INTO nk ordinal FROM list_items WHERE revision_id = new_id AND item_id = rec.item_id;
      IF NOT FOUND THEN nk := NULL; END IF;

      IF ck IS NULL AND nk IS NULL THEN -- needs to be inserted
        UPDATE list_items SET ordinal = ordinal + 1 WHERE revision_id = new_id AND ordinal >= ord;
        INSERT INTO list_items(revision_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
        nk := ord;
      END IF;

      IF (nk IS NOT NULL) THEN ord := nk + 1; END IF;
   
    END LOOP;
    RETURN 1;
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION merge_integer(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    RAISE NOTICE ''sdf'';
    IF primary_val = common_val THEN 
      IF secondary_val = common_val THEN
        RETURN common_val;
      ELSE
        RETURN secondary_val;
      END IF;
    ELSE
      IF secondary_val = common_val THEN
        RETURN primary_val;
      ELSE
        IF favor_primary THEN
          RETURN primary_val;
        ELSE
          RETURN secondary_val;
        END IF;
      END IF;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- function body is *exactly* the same for merge_integer
CREATE FUNCTION merge_text(TEXT, TEXT, TEXT, BOOLEAN) RETURNS TEXT AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    RAISE NOTICE ''sdf'';
    IF primary_val = common_val THEN 
      IF secondary_val = common_val THEN
        RETURN common_val;
      ELSE
        RETURN secondary_val;
      END IF;
    ELSE
      IF secondary_val = common_val THEN
        RETURN primary_val;
      ELSE
        IF favor_primary THEN
          RETURN primary_val;
        ELSE
          RETURN secondary_val;
        END IF;
      END IF;
    END IF;
  END;
' LANGUAGE 'plpgsql';

DROP TABLE lists;
DROP SEQUENCE list_ids;
DROP SEQUENCE list_heads;
DROP TABLE list_items;
DROP FUNCTION list_update (integer, integer);
DROP FUNCTION list_revision(integer, integer);

CREATE TABLE lists
(
  list_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('list_ids'),
  b1 INTEGER DEFAULT NEXTVAL('list_b1s'),
  r1 SMALLINT DEFAULT 0,
  b2 SMALLINT DEFAULT 0,
  r2 SMALLINT DEFAULT 0,
  merged INTEGER REFERENCES lists(list_id),
  modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  UNIQUE(b1, r1, r2)
);

CREATE SEQUENCE list_ids INCREMENT 1 START 1;
CREATE SEQUENCE list_b1s INCREMENT 1 START 1;

CREATE INDEX b1_idx ON lists(b1);


CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  list_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER
);

CREATE INDEX list_idx ON list_items(list_id);

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


CREATE TABLE lists
(
  list_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('list_ids'),
  list_head INTEGER NOT NULL DEFAULT NEXTVAL('list_heads'),
  revision INTEGER NOT NULL DEFAULT 0,
  branch INTEGER NOT NULL DEFAULT 0
);

CREATE SEQUENCE list_ids INCREMENT 1 START 1;
CREATE SEQUENCE list_heads INCREMENT 1 START 1;

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  list_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER
);

CREATE FUNCTION list_update (integer, integer, integer) RETURNS boolean AS '
  DECLARE
    _list_head ALIAS FOR $1;
    _revision ALIAS FOR $2;
    _branch ALIAS FOR $3;
    rec RECORD;
    rec1 RECORD;
    headid INTEGER;
    headrev INTEGER;
    branchid INTEGER;
    branchrev INTEGER;
    newid INTEGER;
    ord INTEGER;
    ck INTEGER;
    bk INTEGER;

  BEGIN
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = _list_head AND branch = 0 ORDER BY revision DESC LIMIT 1;
    IF NOT FOUND THEN RETURN ''f''; END IF;
    headid := rec.list_id;
    headrev := rec.revision;
    
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = _list_head AND branch = _branch ORDER BY revisions DESC LIMIT 1;
    IF NOT FOUND THEN RETURN ''f''; END IF;
    IF rec.revision = headrev THEN RETURN ''t''; END IF;
    branchid := rec.list_id;
    branchrev := rec.revision;
    
    SELECT INTO rec list_id, revision FROM lists WHERE list_head = _list_head AND branch = 0 AND revision = branchrev;
    IF NOT FOUND THEN RETURN ''f''; END IF;
    commonid := rec.list_id;

    INSERT INTO lists(list_head, revision, branch) VALUES(_list_head, headrev, _branch);
    newid := currval(''list_ids'');

    -- Create new list based on branch but without items that were deleted in the head list. 
    
    ord := 0;
    FOR rec IN 
      SELECT li.itemid AS itemid
      FROM list_items AS li
      WHERE
        list_id = branchid
      AND NOT -- deleted
      (
        EXISTS (SELECT 1 FROM list_items AS c WHERE c.list_id = commonid AND c.item_id = li.item_id)
        AND
        NOT EXISTS (SELECT 1 FROM list_items AS h WHERE h.list_id = headid AND h.item_id = li.item_id)
      )
      ORDER BY li.ordinal
    LOOP
      ord := ord + 1;
      INSERT INTO list_items(list_id, ordinal, item_id) VALUES (newid, ord, rec.itemid);
    END LOOP;
    
    -- Insert new items from HEAD into the new list

    ord := 0;  
    FOR rec IN SELECT ordinal, item_id FROM list_items WHERE list_id = headid ORDER BY ordinal LOOP

      SELECT INTO rec1 ordinal FROM list_items WHERE list_id = commonid AND item_id = rec.item_id;
      IF NOT FOUND THEN ck := 0; ELSE ck := rec1.ordinal; END IF;

      SELECT INTO rec1 ordinal FROM list_items WHERE list_id = branchid AND item_id = rec.item_id;
      IF NOT FOUND THEN bk := 0; ELSE bk := rec1.ordinal; END IF;

      IF ck = 0 AND bk = 0 THEN -- needs to be inserted
        BEGIN WORK;
          UPDATE list_items SET ordinal = ordinal + 1 WHERE list_id = newid AND ordinal >= ord;
          INSERT INTO list_items(list_id, ordinal, item_id) VALUES (newid, ord, rec.item_id);
        COMMIT WORK;
        bk := ord;
      END IF

      IF (bk > 0) THEN ord := bk; END IF;
      
    END LOOP

    RETURN ''t'';

  END;  
' LANGUAGE 'plpgsql';


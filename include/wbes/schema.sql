CREATE SEQUENCE item_ids INCREMENT 1 START 100;
CREATE SEQUENCE specialization_ids INCREMENT 1 START 200;
CREATE SEQUENCE branch_ids INCREMENT 1 START 300;
CREATE SEQUENCE revision_ids INCREMENT 1 START 400;
CREATE SEQUENCE component_ids INCREMENT 1 START 500;
CREATE SEQUENCE save_ids INCREMENT 1 START 600;
CREATE SEQUENCE question_period_ids INCREMENT 1 START 700;
CREATE SEQUENCE response_ids INCREMENT 1 START 800;

CREATE TABLE revisions
(
  revision_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('revision_ids'),
  parent INTEGER,
  branch_id INTEGER NOT NULL,
  revision INTEGER NOT NULL,
  save_id INTEGER NOT NULL,
  merged INTEGER,
  component_id INTEGER,
  UNIQUE(parent, branch_id, revision)
);

CREATE TABLE branches
(
  branch_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('branch_ids'),

  -- hereafter fields are redundant but included for efficiency
  parent INTEGER,
  outdated BOOLEAN DEFAULT 'f',
  latest_id INTEGER
);

CREATE TABLE specializations
(
  specialization_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL ('specialization_ids'),
  parent INTEGER
);

CREATE TABLE list_items
(
  component_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);

CREATE TABLE item_specializations
(
  item_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL,
  branch_id INTEGER NOT NULL,
  PRIMARY KEY(item_id, specialization_id)
);

-- meaning of type field:
-- 1 = a survey (for now, just an integer list)
-- 2 = a choice component
-- 3 = a text response component
-- 4 = a heading component
-- 5 = a text component
-- 6 = a choice question
-- 7 = a generic component (no merge defined for this type)
-- 8 = a subsurvey component
-- 9 = a page break component
-- 10 = an abet component

CREATE TABLE components
(
  component_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('component_ids'),
  type INTEGER NOT NULL
);


CREATE TABLE generic_components
(
  data TEXT
) INHERITS (components);

CREATE TABLE text_components
(
  ctext TEXT,
  flags INTEGER
) INHERITS (components);

CREATE TABLE survey_components
(
) INHERITS (text_components);

CREATE TABLE subsurvey_components
(
  specialization_id INTEGER
) INHERITS (survey_components);

CREATE TABLE choice_components
(
  choices      TEXT[],
  other_choice TEXT,
  first_number INTEGER,
  last_number  INTEGER,
  rows         INTEGER
) INHERITS (text_components);

CREATE TABLE choice_questions
(
  qtext TEXT
) INHERITS (components);

CREATE TABLE textresponse_components
(
  rows INTEGER,
  cols INTEGER
) INHERITS (text_components);

CREATE TABLE abet_components
(
  which INTEGER -- bitmask
) INHERITS (components);

CREATE TABLE pagebreak_components
(
  renumber BOOLEAN
) INHERITS (components);

CREATE TABLE saves
(
  save_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('save_ids'),
  user_id INTEGER,
  date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);



-- this entire table is redundant
CREATE TABLE branch_ancestor_cache
(
  ancestor_id INTEGER NOT NULL,
  descendant_id INTEGER NOT NULL
);

CREATE TABLE question_periods
(
  question_period_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('question_period_ids'),
  displayname VARCHAR(60),
  begindate TIMESTAMP,
  enddate TIMESTAMP
);



CREATE TABLE responses
(
  response_id INTEGER PRIMARY KEY DEFAULT NEXTVAL ('response_ids'),
  revision_id INTEGER NOT NULL,
  parent INTEGER
);

CREATE TABLE survey_responses
(
  topic_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL, -- redudant
  user_id INTEGER,
  date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) INHERITS (responses);

CREATE TABLE choice_responses
(
) INHERITS (responses);

CREATE TABLE choice_question_responses
(
  answer INTEGER,
  other TEXT
) INHERITS (responses);

CREATE TABLE mchoice_question_responses
(
  answer INTEGER[],
  other TEXT
) INHERITS (responses);

CREATE TABLE textresponse_responses
(
  rtext TEXT
) INHERITS (responses);

-- TODO: add indices and foreign key constraints

-- drastically speeds up mass mailing
CREATE INDEX survey_response_m ON survey_responses(topic_id, user_id);

CREATE INDEX response_topic_idx ON survey_responses (topic_id);
CREATE UNIQUE INDEX choice_component_idx ON choice_components(component_id);
CREATE UNIQUE INDEX choice_question_idx ON choice_questions(component_id);

CREATE INDEX text_responses_parent_idx ON textresponse_responses (parent);
CREATE INDEX choice_responses_parent_idx ON choice_responses (parent);
CREATE INDEX choiceq_responses_parent_idx ON choice_question_responses (parent);
CREATE INDEX mchoiceq_responses_parent_idx ON mchoice_question_responses (parent);

--ALTER TABLE revisions ADD FOREIGN KEY (save_id) REFERENCES saves(save_id);

-- Add these foreign keys when a future version of postgres supports them on inherited tables
--
-- ALTER TABLE revisions ADD FOREIGN KEY (parent) REFERENCES revisions(revision_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (branch_id) REFERENCES branches(branch_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (save_id) REFERENCES saves(save_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (merged) REFERENCES revisions(revision_id);
--
-- ALTER TABLE branches ADD FOREIGN KEY (specialization_id) REFERENCES specializations(specialization_id);
-- ALTER TABLE branches ADD FOREIGN KEY (parent) REFERENCES branches(branch_id);
-- ALTER TABLE branches ADD FOREIGN KEY (latest_id) REFERENCES revisions(revision_id);
--
-- ALTER TABLE branch_ancestor_cache ADD FOREIGN KEY (ancestor_id) REFERENCES branches(branch_id);
-- ALTER TABLE branch_ancestor_cache ADD FOREIGN KEY (descendant_id) REFERENCES branches(branch_id);
--
-- ALTER TABLE list_items ADD FOREIGN KEY (revision_id) REFERENCES revisions(revision_id);
-- ALTER TABLE list_items ADD FOREIGN KEY (item_id) REFERENCES branches(branch_id);
--
-- ALTER TABLE specializations ADD FOREIGN KEY (parent) REFERENCES specializations(specialization_id);
--
-- ALTER TABLE responses ADD FOREIGN KEY (revision_id) REFERENCES revisions(revision_id);
-- ALTER TABLE responses ADD FOREIGN KEY (parent) REFERENCES responses(response_id);
--
-- ALTER TABLE survey_responses ADD FOREIGN KEY (question_period_id) REFERENCES question_periods(question_period_id);
-- ALTER TABLE survey_responses ADD FOREIGN KEY (specialization_id) REFERENCES specializations(specialization_id);

CREATE OR REPLACE FUNCTION is_true(BOOLEAN) RETURNS BOOLEAN AS '
  SELECT ($1 IS NOT NULL) AND $1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION array_integer_cast(TEXT) RETURNS INTEGER[] AS '
BEGIN
  RETURN $1;
END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION eq(INTEGER, INTEGER) RETURNS BOOLEAN AS '
  SELECT ($1 IS NULL AND $2 IS NULL) OR ($1 IS NOT NULL AND $2 IS NOT NULL AND $1 = $2)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION neq(INTEGER, INTEGER) RETURNS BOOLEAN AS '
  SELECT ($1 IS NOT NULL OR $2 IS NOT NULL) AND ($1 IS NULL OR $2 IS NULL OR $1 <> $2)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION revision_branch(INTEGER) RETURNS INTEGER AS '
  SELECT branch_id FROM revisions WHERE revision_id = $1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION revision_component(INTEGER) RETURNS INTEGER AS '
  SELECT component_id FROM revisions WHERE revision_id = $1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION revision_save(INTEGER) RETURNS INTEGER AS '
  SELECT save_id FROM revisions WHERE revision_id = $1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION branch_parent(INTEGER) RETURNS INTEGER AS '
  SELECT parent FROM branches WHERE branch_id = $1;
' LANGUAGE 'sql';

-- in lieu of foreign keys, use these functions to quickly see if a branch,
-- specialization, or revision is being referenced from some table
-- XXX: actually with the rewrite, the use of inheritance is confined to
-- the components tables, foreign keys can actually be used everywhere else
CREATE OR REPLACE FUNCTION references_branch(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE branch_id = $1)                   THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE branch_id = $1)                    THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE parent = $1)                       THEN 4 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM item_specializations WHERE branch_id = $1)        THEN 8 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_ancestor_cache WHERE ancestor_id = $1)     THEN 16 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_ancestor_cache WHERE descendant_id = $1)   THEN 32 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM list_items WHERE item_id = $1)                    THEN 64 ELSE 0 END;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION references_revisions(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE parent = $1)       THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE merged = $1)       THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE latest_id = $1)     THEN 4 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM responses WHERE revision_id = $1)  THEN 8 ELSE 0 END;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION references_specialization(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM item_specializations WHERE specialization_id = $1) THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM survey_responses WHERE specialization_id = $1)     THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM specializations WHERE parent = $1)                 THEN 4 ELSE 0 END;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION references_component(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE component_id = $1)  THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM list_items WHERE component_id = $1) THEN 2 ELSE 0 END;
' LANGUAGE 'sql';

-- update redundant fields in branches table
CREATE OR REPLACE FUNCTION branch_generate() RETURNS INTEGER AS '
  BEGIN
    LOCK TABLE branches IN SHARE ROW EXCLUSIVE MODE;
    LOCK TABLE revisions IN SHARE ROW EXCLUSIVE MODE;
    UPDATE branches SET outdated = ''t'', parent = NULL;
    RETURN branch_generate(NULL, NULL, ''f'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION branch_generate(INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    parent_revision ALIAS FOR $1;
    parent_branch   ALIAS FOR $2;
    poutdated       ALIAS FOR $3; -- parent revision is outdated
    last_branch_id  INTEGER := 0; -- d.branch_id from previous loop iteration
    next_branch     INTEGER;      -- result of recursive call
    new_branch      BOOLEAN;      -- d.revision_id is the newest revision
    outdated_       BOOLEAN;      -- d.revision_id is outdated
    visted          BOOLEAN;      -- already visited this branch
    d               RECORD;
  BEGIN
    FOR d IN
      SELECT r.revision_id, r.revision, b.branch_id, b.parent
      FROM revisions AS r INNER JOIN branches AS b USING (branch_id)
      WHERE r.parent = parent_revision OR (r.parent IS NULL AND parent_revision IS NULL)
      ORDER BY b.branch_id, r.revision DESC
    LOOP
      new_branch := d.branch_id <> last_branch_id;
      outdated_ := poutdated OR NOT new_branch;

      PERFORM branch_generate(d.revision_id, d.branch_id, outdated_);
      IF new_branch THEN
        IF d.parent IS NULL THEN
          UPDATE branches SET
            parent = parent_branch,
            latest_id = d.revision_id,
            outdated = outdated_
          WHERE branch_id = d.branch_id AND latest_id IS NULL;
        END IF;
        last_branch_id := d.branch_id;
      END IF;
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION branch_ancestor_generate() RETURNS INTEGER AS '
  DECLARE
    rec RECORD;
  BEGIN
    LOCK TABLE branches IN SHARE ROW EXCLUSIVE MODE;
    TRUNCATE branch_ancestor_cache;
    FOR rec IN SELECT branch_id FROM BRANCHES WHERE parent IS NULL LOOP
      PERFORM branch_ancestor_generate(rec.branch_id);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

-- record all descendants of branch_id_
CREATE OR REPLACE FUNCTION branch_ancestor_generate(INTEGER) RETURNS INTEGER AS '
  DECLARE branch_id_ ALIAS FOR $1;
  BEGIN
    FOR rec IN SELECT branch_id FROM branches WHERE parent = branch_id_ LOOP
      PERFORM branch_ancestor_generate(ancestor_id_, rec.branch_id);
      PERFORM branch_ancestor_generate(rec.branch_id);
    END LOOP;
  END;
' LANGUAGE 'plpgsql';

-- record branch_id and all of its descendants as descendants of ancestor_id
CREATE OR REPLACE FUNCTION branch_ancestor_generate(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    ancestor_id_ ALIAS FOR $1;
    branch_id_   ALIAS FOR $2;
    rec RECORD;
    eq BOOLEAN;
  BEGIN
    INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
    VALUES(ancestor_id_, branch_id_);

    FOR rec IN SELECT branch_id FROM branches WHERE parent = branch_id_ LOOP
      PERFORM branch_ancestor_generate(ancestor_id_, rec.branch_id); -- step 2
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION branch_find(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    item_id_    ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    branch_id_ INTEGER;
    s INTEGER;
  BEGIN
    --RAISE NOTICE ''branch_find(%,%) called.'', $1, $2;
    s := specialization_id_;
    LOOP
      SELECT INTO branch_id_ branch_id FROM item_specializations WHERE item_id = item_id_ AND specialization_id = s;
      IF FOUND THEN
        RETURN branch_id_;
      ELSE
        SELECT INTO s parent FROM specializations WHERE specialization_id = s;
        IF NOT FOUND THEN RETURN NULL; END IF;
      END IF;
    END LOOP;
  END;
' LANGUAGE 'plpgsql';

-- given a branch id this function returns the latest revision on the branch.
-- If the latest existing revision is out of date, a new revision will be
-- generated and returned
CREATE OR REPLACE FUNCTION branch_latest(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    branch_info RECORD;
    arrays RECORD;
    array_length INTEGER;
    parent RECORD;
    bid INTEGER;
    type_ INTEGER;
  BEGIN
    -- get information about the branch
    SELECT INTO branch_info outdated, latest_id
    FROM branches AS b WHERE branch_id = branch_id_;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''branch_latest(%) called with invalid branch_id'', $1;
    END IF;

    IF NOT branch_info.outdated THEN RETURN branch_info.latest_id; END IF;

    -- loop to lock and gather information about ancestor branches
    array_length := 0;
    bid := branch_id_;

    DECLARE
      branch RECORD;
      latest RECORD;

      -- current version of plpgsql (7.2) allows only read access to arrays,
      -- it is not possible to declare array variables or make assignments
      -- into arrays. this code gets around that restriction by putting
      -- numbers into strings, then casting those strings into array
      -- fields in a RECORD variable.
      --
      -- alternately, this code could be rewritten to use recursion instead of loops
      -- and array storage, since the arrays are really only used as stacks. The reason
      -- I didn''t do this is that it would require the recursive functions to return
      -- two integer values and currently there isn''t a clean way of returning
      -- multiple values from postgres functions.

      s_bids  TEXT := '''';  -- stringed array of branch_ids for each branch level
      s_rids  TEXT := '''';  -- stringed array of latest revision ids for each branch
      s_prids TEXT := '''';  -- stringed array of latest revisions'' parents for each branch
      s_cids  TEXT := '''';  -- stringed array of component ids for each branch
      s_types TEXT := '''';  -- stringed array of component revision''s type for each branch level
      sep     TEXT := '''';
    BEGIN
      -- loop through branch_id_''s ancestors accumulating information
      -- about latest revisions. stop loop when an ancestor is found
      -- which is not outdated.
      LOOP
        SELECT INTO branch latest_id, parent
        FROM branches WHERE branch_id = bid AND outdated FOR UPDATE;

        IF NOT FOUND THEN EXIT; END IF;

        SELECT INTO latest r.parent, r.revision, r.component_id, c.type
        FROM revisions AS r
        INNER JOIN components AS c USING (component_id)
        WHERE revision_id = branch.latest_id;

        s_bids  := s_bids  || sep || bid::TEXT;
        s_rids  := s_rids  || sep || branch.latest_id::TEXT;
        s_prids := s_prids || sep || latest.parent::TEXT;
        s_cids  := s_cids  || sep || latest.component_id::TEXT;
        s_types := s_types || sep || latest.type::TEXT;

        array_length := array_length + 1;
        bid := branch.parent;
        sep := '','';
      END LOOP;

      -- assemble arrays
      SELECT INTO arrays
        array_integer_cast(''{'' || s_bids  || ''}'') AS bids,
        array_integer_cast(''{'' || s_rids  || ''}'') AS rids,
        array_integer_cast(''{'' || s_prids || ''}'') AS prids,
        array_integer_cast(''{'' || s_cids  || ''}'') AS cids,
        array_integer_cast(''{'' || s_types || ''}'') AS types;

      SELECT INTO parent latest_id, revision_component(latest_id) AS component_id
      FROM branches WHERE branch_id = bid;

      IF NOT FOUND THEN
        -- this could occur if a root branch is marked as outdated.
        -- this would not make any sense and indicates a corrupt database
        RAISE EXCEPTION ''branch_latest(%) fails. impossible condition reached, branch % not found'', $1, bid;
      END IF;
    END;

    -- next is a loop to bring ancestral branches up to date

    -- Example of merge:
    -- Say these revisions exist in the database:
    --
    --   1.4
    --   1.4.2.3.3.2
    --   1.4.2.17
    --   1.5
    --
    -- And branch_latest is called for branch 1.*.2.*.3.*, then the following
    -- merges need to occur:
    --
    -- merge(1.4,     1.4.2.17,    1.5,     1.5.2.1)
    -- merge(1.4.2.3, 1.4.2.3.3.2, 1.5.2.1, 1.5.2.1.3.1)
    --
    -- during this process two new revisions are created:
    --
    --   1.5.2.1 with a merge field pointing to 1.4.2.17
    --   1.5.2.1.3.1 with a merge field pointing to 1.4.2.3.3.2
    --
    -- at this point in the code, the arrays variable will hold information about
    -- these revisions:
    --
    --   1.4.2.17
    --   1.4.2.3.3.2
    --
    -- and the record variable parent will start out with information about revision 1.5

    DECLARE
      i INTEGER;
      j INTEGER;
      revision_ INTEGER;
      common_id INTEGER;
      primary_id INTEGER;
      secondary_id INTEGER;
      common_component_id INTEGER;
      primary_component_id INTEGER;
      secondary_component_id INTEGER;
    BEGIN
      FOR j IN REVERSE array_length..1 LOOP
        bid := arrays.bids[j];
        type_ := arrays.types[j];
        common_id := arrays.prids[j];
        primary_id := arrays.rids[j];
        secondary_id := parent.latest_id;
        common_component_id := revision_component(common_id);
        primary_component_id = arrays.cids[j];
        secondary_component_id := parent.component_id;

        -- If common, primary, or secondary ids are equivalent, the new revision can
        -- be a just link to a preexisting component instead of a merge.

        parent.component_id := component_merge(common_component_id,
          primary_component_id, secondary_component_id, type_);

        -- helpful revision numbering convention, not at all neccessary
        IF parent.component_id = secondary_component_id THEN
          revision_ := 0;
        ELSE
          revision_ := 1;
        END IF;

        INSERT INTO revisions (parent, branch_id, revision, save_id, merged, component_id)
        VALUES (secondary_id, bid, revision_, revision_save(secondary_id), primary_id, parent.component_id);
        parent.latest_id := currval(''revision_ids'');

        UPDATE branches SET
          outdated = false, latest_id = parent.latest_id
        WHERE branch_id = bid;
      END LOOP;
    END;

    RETURN parent.latest_id;
  END;
' LANGUAGE 'plpgsql';

-- when save_id_ is null, does the same as branch_latest(INTEGER). otherwise
-- save_id_ is used to intentionally retrieve out of date revisions with
-- save_id's lesser or equal to the argument.
CREATE OR REPLACE FUNCTION branch_latest(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    save_id_ ALIAS FOR $2;
    i INTEGER;
    j INTEGER;
  BEGIN
    --RAISE NOTICE ''branch_latest(%,%) called.'', $1, $2;
    IF save_id_ IS NULL THEN
      RETURN branch_latest(branch_id_);
    END IF;

    -- this code will search parent branches for old save_ids, but will
    -- never merge new changes from parents back onto the starting branch
    -- like branch_latest(INTEGER) does

    i := branch_id_;
    LOOP
      SELECT INTO j revision_id FROM revisions
      WHERE branch_id = i AND save_id <= save_id_
      ORDER BY save_id DESC LIMIT 1;

      IF j IS NOT NULL THEN RETURN j; END IF;

      SELECT INTO i parent FROM branches WHERE branch_id_ = i;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''branch_latest(%,%) fails. unable to find early enough revision for branch %'', $1, $2, branch_id_;
      END IF;
    END LOOP;
  END;
' LANGUAGE 'plpgsql';

-- this is the same as branch_latest(branch_id_, save_id_) except that
-- it takes a item_id and specialization_id instead of a branch_id.
-- order of the arguments are item_id and specialization_id, save_id
CREATE OR REPLACE FUNCTION branch_latest(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT branch_latest((SELECT branch_find($1,$2)),$3);
' LANGUAGE 'sql';

-- find or create a branch for the given specialization and item_id
-- returns NULL if item is not specialized for for specialization_id_ or
-- any of it's ancestors.

-- XXX: Postgres 7.2 does not allow SELECT .. FOR UPDATE on specializations
-- table because it is inherited from. This completely breaks the locking
-- done in this function and others. If this feature isn't added in 7.3, then
-- I'll have to come up with some workaround (such as doing trivial updates
-- on rows which need to be locked. For now, though, the FOR UPDATE clauses
-- are commented out and denoted by ILB (inheritance locking bug)

CREATE OR REPLACE FUNCTION branch_make_specialization(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    item_id_ ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    save_id_ ALIAS FOR $3;
    bid INTEGER;
    sid INTEGER;
    cbid INTEGER;
    sibling RECORD;
  BEGIN
    -- this preliminary check is not neccessary for correctness, but
    -- avoids unneccessary locking in most cases

    SELECT INTO bid branch_id FROM item_specializations
    WHERE specialization_id = specialization_id_ AND item_id = item_id_;

    IF FOUND THEN RETURN bid; END IF;

    sid := specialization_id_;

    --ILB
    --SELECT * FROM specializations WHERE specialization_id = sid FOR UPDATE;

    LOOP
      SELECT INTO bid branch_id FROM item_specializations
      WHERE specialization_id = sid AND item_id = item_id_;
      EXIT WHEN FOUND;

      SELECT INTO sid parent FROM specializations WHERE specialization_id = sid
      --ILB
      --FOR UPDATE
      ;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''associated_revision(%,%,%) fails. specialization % or one of its ancestors has no row in the database'', $1, $2, $3;
      END IF;

      IF sid IS NULL THEN RETURN NULL; END IF;
    END LOOP;

    IF sid = specialization_id_ THEN RETURN bid; END IF;

    cbid := branch_create_child(bid);
    PERFORM branch_add_specialization(cbid, item_id_, specialization_id_, save_id_);
    RETURN cbid;
  END;
' LANGUAGE 'plpgsql';

-- create a new branch for item_id_, specialization_id_ which will be
-- made into an ancestor of orig_branch_id, an item_id branch
-- specialized for a descendant of specialization_id
CREATE OR REPLACE FUNCTION branch_make_parent_speclzn(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    item_id_ ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    orig_branch_id ALIAS FOR $3;
    save_id_ ALIAS FOR $4;
    sid INTEGER;
    bid INTEGER;
    cbid INTEGER;
  BEGIN
    SELECT INTO sid specialization_id FROM item_specializations
    WHERE branch_id = orig_branch_id AND item_id = item_id_;

    -- sid should be a descendant of specialization_id_

    bid := orig_branch_id;

    LOOP
      IF sid IS NULL THEN RETURN NULL; END IF;

      SELECT INTO sid parent FROM specializations WHERE specialization_id = sid;

      IF NOT FOUND THEN
        RAISE EXCEPTION ''branch_make_parent_speclzn(%,%,%) failed. specialization % or one of it''''s is not present in the database'', $1, $2, $3, specialization_id_;
      END IF;

      EXIT WHEN sid = specialization_id_;

      SELECT INTO cbid branch_id FROM item_specializations WHERE specialization_id = sid AND item_id = item_id_;
      IF FOUND AND cbid IS NOT NULL THEN
        bid := cbid;
      END IF;

    END LOOP;

    pbid := branch_create_parent(bid, save_id_);
    PERFORM branch_add_specialization(pbid, item_id_, specialization_id_, save_id_);

    RETURN pbid;
  END;
' LANGUAGE 'plpgsql';

-- make a new branch which is a child of branch_id_. The latest_id and
-- outdated fields on the new branch will need to be updated after
-- this function is called.
CREATE OR REPLACE FUNCTION branch_create_child(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    bid INTEGER;
  BEGIN
    bid := nextval(''branch_ids'');

    INSERT INTO branches (branch_id, parent, outdated, latest_id)
    VALUES (bid, branch_id_, ''f'', NULL);

    IF branch_id_ IS NOT NULL THEN
      INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
      VALUES (branch_id_, bid);

      INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
      SELECT ancestor_id, bid FROM branch_ancestor_cache
      WHERE descendant_id = branch_id_;
    END IF;

    RETURN bid;
  END;
' LANGUAGE 'plpgsql';

-- create a new parent branch which branch_id_ should directly inherit from.
-- the new parent branch will be a child of branch_id's old parent
-- return value is the new branch id
CREATE OR REPLACE FUNCTION branch_create_parent(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    save_id_ ALIAS FOR $2;
    rid INTEGER;
    bid INTEGER;
    branch RECORD;
    siblings RECORD;
  BEGIN;
    SELECT INTO branch parent, outdated, latest_id FROM branches
      WHERE branch_id = branch_id_ FOR UPDATE;
    IF NOT FOUND THEN
      RAISE NOTICE ''branch_create_parent(%,%) fails. branch % does not exist.'', $1, $2, branch_id_;
    END IF;

    bid := branch_create_child(branch.parent)
    PERFORM branch_move(branch_id_, bid, save_id_);

    RETURN bid;
  END;
' LANGUAGE 'plpgsql';

-- associates an existing branch with an item's specialization
-- this function should be used with caution because it will
-- make the branch's siblings into children if they are
-- sub-specializations of the same item. This may not
-- be desireable in all cases
CREATE OR REPLACE FUNCTION branch_add_specialization(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    item_id_ ALIAS FOR $2;
    specialization_id_ ALIAS FOR $3;
    save_id_ ALIAS FOR $4;
    sibling RECORD;
    pbid INTEGER;
  BEGIN
    INSERT INTO item_specializations (item_id, specialization_id, branch_id)
    VALUES (item_id_, specialization_id_, branch_id_);

    pbid := branch_parent(branch_id_);

    FOR sibling IN
      SELECT branch_id FROM branches
      WHERE (parent = pbid OR (pbid IS NULL AND parent IS NULL))
        AND branch_id <> branch_id_
    LOOP
      IF branch_contains_child_specialzn(sibling.branch_id, specialization_id_, item_id_) THEN
        PERFORM branch_move(sibling.branch_id, branch_id_, save_id_);
      END IF;
    END LOOP;
    RETURN branch_id_;
  END;
' LANGUAGE 'plpgsql';

-- returns true if branch_id_ or one of its descendant branches
-- is associated with specialization_id_ or one of its descendant
-- specializations
CREATE OR REPLACE FUNCTION branch_contains_child_specialzn(INTEGER, INTEGER, INTEGER) RETURNS BOOLEAN AS '
  DECLARE
    branch_id_         ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    item_id_           ALIAS FOR $3;
    s RECORD;
    sid INTEGER;
  BEGIN
    FOR s IN
      SELECT i.specialization_id
      FROM
      ( SELECT branch_id_ AS branch_id
        UNION
        SELECT descendant_id
        FROM branch_ancestor_cache AS c
        WHERE c.ancestor_id = branch_id_
      ) AS b
      INNER JOIN item_specializations AS i USING (branch_id)
      WHERE i.item_id = item_id
    LOOP
      sid := s.specialization_id;
      LOOP
        IF s.specialization_id = specialization_id_ THEN RETURN ''t''; END IF;
        SELECT INTO sid parent FROM specializations WHERE specialization_id = sid;
        EXIT WHEN sid IS NULL;
      END LOOP;
    END LOOP;
    RETURN ''f'';
  END;
' LANGUAGE 'plpgsql';

-- move branch examples
-- case where new branch is empty:
--
-- branch_id_:        1.*.3.*.5.*
-- new_parent_branch: 1.*.3.*.7.*
--
-- The existing revisions are:
--
--   1.2.3.4.5.1
--   1.2.3.6.5.1
--   1.2.3.7
--
-- After branch_move, you get branch 1.*.3.*.7.*.5* with these revisions:
--
--   1.2.3.4.7.0.5.1
--   1.2.3.6.7.0.5.1 (latest)
--
-- branch 1.*.3.*.7.* is outdated with latest_id
--   1.2.3.6.7.0
--
-- case where the new parent branch is not empty:
--
-- branch_id_:        1.*.3.*.5.*
-- new_parent_branch: 1.*.3.*.7.*
--
-- The existing revisions are:
--
--   1.2.3.4.5.1
--   1.2.3.6.5.1
--   1.2.3.1.7.1
--   1.2.3.6.7.2
--
-- After branch_move, you get this branch:
--
--   1.*.3.*.7.*.5*
--
-- with these revisions:
--
--   1.2.3.4.7.0.5.1
--   1.2.3.6.7.0.5.1 (latest)
--
-- which are outdated due to the existence of
--
--   1.2.3.1.7.1
--   1.2.3.6.7.2
--
-- make branch_id_ a child of new_parent_branch_
CREATE OR REPLACE FUNCTION branch_move(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_        ALIAS FOR $1;
    new_parent_branch ALIAS FOR $2;
    save_id_          ALIAS FOR $3;
    parent_ INTEGER;
    p INTEGER;
    rid INTEGER;
    siblings RECORD;
    latest_revision INTEGER;
    new_parent_branch_info RECORD;
  BEGIN
    SELECT INTO parent_ parent FROM branches WHERE branch_id = branch_id_ FOR UPDATE;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''branch_move(%,%,%) fails. branch % not found'', $1, $2, $3, branch_id_;
    END IF;

    SELECT INTO p parent FROM branches WHERE branch_id = new_parent_branch FOR UPDATE;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''branch_move(%,%,%) fails. branch % not found'', $1, $2, $3, new_parent_branch;
    END IF;

    IF branch_id_ = new_parent_branch THEN
      RAISE EXCEPTION ''branch_move(%,%,%) fails. cannot make branch % a child of itself'', $1, $2, $3, new_parent_branch;
    END IF;

    IF (parent_ IS NOT NULL AND p IS NOT NULL) AND parent_ <> p THEN
      RAISE EXCEPTION ''branch_move(%,%,%) fails. this function is currently only implemented for the case where the new parent is a child of the old parent'', $1, $2, $3, branch_id_;
    END IF;

    IF parent_ IS NOT NULL THEN
      FOR siblings IN SELECT parent FROM revisions WHERE branch_id = branch_id_ GROUP BY parent LOOP
        IF siblings.parent IS NULL THEN
          RAISE EXCEPTION ''branch_move(%,%,%) fails. Database is corrupt. Some revisions on child branch % have NULL parents.'', $1, $2, $3,  branch_id_;
        END IF;

        rid := revision_make_parent_clone(siblings.parent, new_parent_branch, save_id);

        UPDATE revisions SET parent = rid
        WHERE branch_id = branch_id_ AND parent = siblings.parent;
      END LOOP;
    ELSE
      -- if parent_ is null, the parent value of all revisions on branch_id_ are null
      -- follow the same procedure as above using the null parent value
      rid := revision_make_parent_clone(NULL, new_parent_branch, save_id);
      UPDATE revisions SET parent = rid
      WHERE branch_id = branch_id_ AND parent IS NULL;
    END IF;

    -- revisions are all set, now update fields in the branches table

    UPDATE branches SET parent = new_parent_branch WHERE branch_id = branch_id_;

    SELECT INTO branch_info b.latest_id, b.outdated, r.parent AS latest_parent
    FROM branches AS b
    INNER JOIN revisions AS r ON r.revision_id = b.latest_id
    WHERE b.branch_id = branch_id_;

    IF NOT FOUND OR branch_info.latest_parent IS NULL THEN
      RAISE ERROR ''branch_move(%,%,%) fails. Could not find latest revision on branch % being moved'', $1, $2, $3, branch_id_;
    END IF;

    SELECT INTO new_parent_branch_info latest_id, outdated
    FROM branches WHERE branch_id = new_parent_branch;

    IF NOT FOUND THEN
      RAISE ERROR ''branch_move(%,%,%) fails. Could not find information about new parent branch %'', $1, $2, $3, new_parent_branch;
    END IF;

    IF new_parent_branch_info.latest_id IS NULL THEN
      -- parent branch is a completely new branch with no previous
      -- revisions, so update its latest and outdated fields with
      -- information about the latest revision created on it in the code
      -- above
      UPDATE branches SET
        latest_id = branch_info.latest_parent,
        outdated = branch_info.outdated
      WHERE branch_id = new_parent_branch;
    ELSIF new_parent_branch_info.latest_id <> branch_info.latest_parent THEN
      -- parent branch had previously existing revisions on it, so mark
      -- its new child as being outdated so these revisions will get merged
      -- into it.
      PERFORM branch_set_outdated(branch_id_);
    END IF;

    -- update branch_ancestor_cache

    INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
    VALUES (new_parent_branch, branch_id_)

    INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
    SELECT new_parent_branch, descendant_id
    FROM branch_ancestor_cache
    WHERE ancestor_id = branch_id_;

    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

-- create a child of parent_revision_id which precedes all of its other children
-- and which is identical to the parent, or return such a child if it already
-- exists. if the revision created by this function is the first on the branch
-- the caller of this function is responsible for updating the branch's latest_id
-- and outdated fields
--
-- find or create a child of parent_revision_id on branch_id_ which
-- points to the same component as the parent and which has a lower
-- revision number than any other child
CREATE OR REPLACE FUNCTION revision_make_parent_clone(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    parent_revision_id ALIAS FOR $1;
    branch_id_ ALIAS FOR $2;
    save_id_ ALIAS FOR $3;
    parent_component_id INTEGER;
    rec RECORD;
    revision INTEGER;
  BEGIN
    IF neq(revision_branch(parent_revision_id), branch_parent(branch_id_)) THEN
      RAISE ERROR ''make_first_cloned_child(%,%,%) fails. not a child branch'', $1, $2, $3;
    END IF;

    RETURN revision_make_parent_clone_i(parent_revision_id, branch_id_, save_id_, revision_component(parent_revision_id));
  END;
' LANGUAGE 'plpgsql';

-- actual implementation of revision_make_parent_clone,
-- takes redundant parameters and skips error checking
CREATE OR REPLACE FUNCTION revision_make_parent_clone_i(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    parent_revision_id ALIAS FOR $1;
    branch_id_ ALIAS FOR $2;
    save_id_ ALIAS FOR $3;
    parent_component_id ALIAS FOR $4;
    rec RECORD;
    revision INTEGER;
  BEGIN
    SELECT INTO rec revision_id, component_id, revision
    FROM revisions
    WHERE branch = branch_id_ AND (parent = parent_revision_id
      OR (parent IS NULL AND parent_revision_id IS NULL))
    ORDER BY revision LIMIT 1;

    IF FOUND THEN
      IF eq(rec.component_id, parent_component_id) THEN
        RETURN rec.revision_id;
      END IF;
      revision_ := rec.revision - 1;
    ELSE
      revision_ := 0;
    END IF;

    INSERT INTO revisions (parent, branch_id, revision, save_id, component_id)
    VALUES (parent_revision_id, branch_id_, revision_, save_id_, parent_component_id);

    RETURN currval(''revision_ids'');
  END;
' LANGUAGE 'plpgsql';

-- find or create a child of parent_revision_id on branch_id_ which
-- points to the same component as the parent and which has a lower
-- revision number than any other child

-- implementation of revision_make_ancestor_clone, takes redundant parameters
CREATE OR REPLACE FUNCTION revision_make_ancestor_clone_i(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    base_revision_id ALIAS FOR $1;
    branch_id_ ALIAS FOR $2;
    save_id_ ALIAS FOR $3;
    base_component_id ALIAS FOR $4;
    base_branch_id ALIAS FOR $5;
    p INTEGER;
    r INTEGER;
  BEGIN
    IF branch_id_ = base_branch_id THEN
      RETURN base_revision_id;
    ELSE
      SELECT INTO p parent FROM branches WHERE branch_id = branch_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''mak(%,%,%,%) fails. branch % doesn''''t exist'', $1, $2, $3, $4, branch_id_;
      END IF;
      r := revision_make_ancestor_clone_i(base_revision_id, p, save_id_, base_component_id, base_branch_id);
      RETURN revision_make_parent_clone_i(r, branch_id_, save_id_, base_component_id);
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- find or create a descendant of revision $1 on branch $2 which
-- points to the same component as $1 and which has a lower
-- revision number than any other descendants on the same branch
CREATE OR REPLACE FUNCTION revision_make_ancestor_clone(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT revision_make_ancestor_clone_i($1, $2, $3, revision_component($1), revision_branch($1));
' LANGUAGE 'sql';

-- save a new revision of a new item
CREATE OR REPLACE FUNCTION revision_save(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_      ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    save_id_           ALIAS FOR $3;
    iid INTEGER;
    bid INTEGER;
    rid INTEGER;
  BEGIN
    IF component_id_ IS NULL OR specialization_id_ IS NULL or save_id_ IS NULL THEN
      RAISE EXCEPTION ''revision_save(%,%,%) called with invalid arguments'', $1, $2, $3;
    END IF;

    iid := nextval(''item_ids'');
    bid := nextval(''branch_ids'');
    rid := nextval(''revision_ids'');

    INSERT INTO branches (branch_id, parent, outdated, latest_id)
    VALUES (bid, NULL, ''f'', rid);

    INSERT INTO revisions (revision_id, parent, branch_id, revision, save_id, merged, component_id)
    VALUES (rid, NULL, bid, 0, save_id_, NULL, component_id_);

    INSERT INTO item_specializations (item_id, specialization_id, branch_id)
    VALUES (iid, specialization_id_, bid);

    RETURN iid;
  END;
' LANGUAGE 'plpgsql';

-- This is the first revision on a new child branch.
--
-- Example 1:
-- Orig revision is 1.4, branch to save is 1.*.3.*.5.*.7.*
-- Existing revisions are:
--
-- 1.4
-- 1.5
-- 1.5.3.1
-- 1.6
--
-- Then this revision should be saved at 1.4.3.0.5.0.7.1
-- where 1.4.3.0 and 1.4.3.0.5.0 are identical 1.4
--
-- Example 2
-- Orig revision is 1.4, branch to save is 1.*.3.*.5.*.7.*
-- Existing revisions are:
--
-- 1.4
-- 1.4.3.1
-- 1.4.3.2
-- 1.4.3.3
--
-- If 1.4.3.1 is just a link pointing back to 1.4 then
--   this revision should be saved at 1.4.3.1.5.0.7.1
--   where 1.4.3.1.5.0 is another link back to 1.4
-- Otherwise
--   this revision should be saved at 1.4.3.0.5.0.7.1
--   where 1.4.3.0 and 1.4.3.0.5.0 are justs links to 1.4
--
-- The convention I've used in both of these examples is to
-- start with a 0 revision number for revisions that link to
-- their parents and start with a 1 revision number for revisions
-- that are different from their parents. This is an arbitrary decision
-- and no code should rely on this convention being followed. It can be
-- helpful for debugging though, because it makes it easy to spot the dummy
-- revisions created in the save process which just point back to
-- their parents (as opposed to real revisions which store real information)

-- need to find a parent revision. if parent branches are empty then special
-- dummy revisions will be created.

CREATE OR REPLACE FUNCTION revision_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_      ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    save_id_           ALIAS FOR $3;
    orig_revision_id   ALIAS FOR $4;
    orig_item_id       ALIAS FOR $5;
    bid       INTEGER;
    rid       INTEGER;
    orig      RECORD;
    orig_desc BOOLEAN := ''f'';
    latest    RECORD;
  BEGIN
    IF orig_revision_id IS NULL AND orig_item_id IS NULL THEN
      RETURN revision_save(component_id_, specialization_id_, save_id_);
    ELSIF component_id_ IS NULL OR specialization_id_ IS NULL or save_id_ IS NULL OR orig_revision_id IS NULL OR orig_item_id IS NULL THEN
      RAISE EXCEPTION ''revision_save(%,%,%,%,%) called with invalid arguments'', $1, $2, $3, $4, $5;
    END IF;

    SELECT INTO orig r.component_id, r.branch_id, c.type
    FROM revisions AS r
    INNER JOIN components AS c USING (component_id)
    WHERE r.revision_id = orig_revision_id;

    IF NOT FOUND THEN RAISE EXCEPTION ''revision_save(%,%,%,%,%) failed. orig_revision_id % not found'', $1, $2, $3, $4, $5, orig_revision_id; END IF;

    IF component_id_ = orig.component_id THEN
      bid := branch_find(orig_item_id, specialization_id_);
    ELSE
      bid := branch_make_specialization(orig_item_id, specialization_id_, save_id_);
    END IF;

    IF bid IS NULL THEN
      -- component was saved for a descendant of specialization_id_
      orig_desc := ''t'';
      bid := branch_make_parent_speclzn(orig_item_id, specialization_id_, orig.branch_id, save_id_);
      IF bid IS NULL THEN
        RAISE EXCEPTION ''revision_save(%,%,%,%,%) failed. original revision is not related to specialization %'', $1, $2, $3, $4, $5, specialization_id_;
      END IF;
    END IF;

    -- bid is the branch of item orig_item_id specialized for specialization_id_
    SELECT INTO latest b.branch_id, r.revision_id, r.component_id, r.revision, r.parent, b.parent AS bparent, b.outdated
    FROM branches AS b
    LEFT JOIN revisions AS r ON r.revision_id = b.latest_id
    WHERE b.branch_id = bid
    FOR UPDATE OF b;

    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save(%,%,%,%,%) failed. branch % not found'', $1, $2, $3, $4, $5, bid;
    END IF;

    IF orig_desc THEN
      -- setting this variable to null is just a convenient way of
      -- telling the next block of code to save the component and to
      -- not perform a merge after that
      latest.revision_id := NULL;
    ELSIF latest.revision_id IS NULL THEN
      -- branch was empty
      latest.revision := 0;
      latest.parent := revision_make_ancestor_clone_i(orig_revision_id, latest.bparent, save_id_, orig.component_id, orig.branch_id);
    END IF;

    -- don''t bother saving if the branch is not empty and new component is the same as the orignal or latest
    IF latest.revision_id IS NULL OR (component_id_ <> orig.component_id AND component_id_ <> latest.component_id) THEN
      rid := nextval(''revision_ids'');

      INSERT INTO revisions (revision_id, parent, branch_id, revision, save_id, merged, component_id)
      VALUES (rid, latest.parent, bid, latest.revision + 1, save_id_, NULL, component_id_);

      -- if the original component was out of date, merge in the new changes
      IF latest.revision_id <> orig_revision_id THEN
        DECLARE
          cid INTEGER;
        BEGIN
          cid := component_merge(orig.component_id, component_id_, latest.component_id, orig.type);
          rid := nextval(''revision_ids'');
          INSERT INTO revisions (revision_id, parent, branch_id, revision, save_id, merged, component_id)
          VALUES (rid, latest.parent, bid, latest.revision + 2, save_id_, orig_revision_id, component_id_);
        END;
      END IF;

      UPDATE branches SET latest_id = rid WHERE branch_id = bid;

      -- set descendant branches to be outdated if they are not already
      IF NOT latest.outdated THEN
        PERFORM branch_descendants_set_outdated(bid);
      END IF;
    END IF;

    RETURN orig_item_id;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION revision_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_      ALIAS FOR $1;
    specialization_id_ ALIAS FOR $2;
    save_id_           ALIAS FOR $3;
    orig_revision_id   ALIAS FOR $4;
    orig_item_id       ALIAS FOR $5;
    import_mode        ALIAS FOR $6;
    iid       INTEGER:
    bid       INTEGER;
    rid       INTEGER;
    orig      RECORD;
    i         INTEGER;
  BEGIN
    IF import_mode IS NULL THEN
      RETURN revision_save(component_id_, specialization_id_, save_id_, orig_revision_id, orig_item_id)
    ELSIF component_id_ IS NULL OR specialization_id_ IS NULL or save_id_ IS NULL OR orig_revision_id IS NULL OR orig_item_id IS NULL THEN
      RAISE EXCEPTION ''revision_save(%,%,%,%,%,%) called with invalid arguments'', $1, $2, $3, $4, $5, $6;
    END IF;

    iid := nextval(''list_item_ids'');

    SELECT INTO orig component_id, branch_id FROM revision WHERE revision_id = orig_revision_id;

    IF import_mode = 1 THEN -- IMPORT_INDEPENDENT
      bid := nextval(''branch_ids'');
      rid := nextval(''revision_ids'');

      INSERT INTO branches (branch_id, parent, outdated, latest_id)
      VALUES (bid, NULL, ''f'', rid);

      INSERT INTO revisions (revision_id, parent, branch_id, revision, save_id, merged, component_id)
      VALUES (rid, NULL, bid, 1, save_id_, NULL, orig.component_id);
    ELSIF import_mode = 2 THEN -- IMPORT_LINKED_TO
      bid := branch_create_child(orig.branch_id);
      rid := nextval(''revision_ids'');

      INSERT INTO revisions (revision_id, parent, branch_id, revision, save_id, merged, component_id)
      VALUES (rid, orig_revision_id, bid, 0, save_id_, NULL, orig.component_id);

      SELECT INTO i latest_id FROM branches WHERE branch_id = orig.branch_id;

      UPDATE branches SET
        outdated = orig.outdated OR orig_revision_id <> i,
        latest_id = rid
      WHERE branch_id = bid;
    ELSIF import_mode = 3 THEN -- IMPORT_LINKED_FROM
      RAISE EXCEPTION ''revision_save(%,%,%,%,%,%). mode IMPORT_LINKED_FROM not implemented'', $1, $2, $3, $4, $5, $6;
      -- reason why it''s not implemented: which branch should bottom_branch
      -- point to? depends on what is intended
      bottom_branch := orig.branch_id;
      branch_id_ := branch_create_parent(bottom_branch, save_id_);
      SELECT INTO rid parent FROM revisions WHERE revision_id = orig_revision_id;
      SELECT INTO branch_latest latest_id FROM branches WHERE branch_id = branch_id_;
    ELSIF import_mode = 4 THEN -- IMPORT_MIRRORED
      bid := orig.branch_id;
      rid := orig_revision_id;
    ELSE
      RAISE EXCEPTION ''revision_save(%,%,%,%,%,%) called with invalid import mode %.'',$1,$2,$3,$4,$5,$6,import_mode;
    END IF;

    INSERT INTO item_specializations (item_id, specialization_id, branch_id)
    VALUES (iid, specialization_id_, bid);

    RETURN revision_save(component_id_, specialization_id_, save_id_, rid, iid);
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION branch_descendants_set_outdated(INTEGER) RETURNS INTEGER AS '
  UPDATE branches SET outdated = ''t'' WHERE branch_id IN
    (SELECT descendant_id FROM branch_ancestor_cache WHERE ancestor_id = $1);
  SELECT 1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION branch_set_outdated(INTEGER) RETURNS INTEGER AS '
  UPDATE branches SET outdated = ''t'' WHERE branch_id = $1;
  UPDATE branches SET outdated = ''t'' WHERE branch_id IN
    (SELECT descendant_id FROM branch_ancestor_cache WHERE ancestor_id = $1);
  SELECT 1;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION specialization_modified(INTEGER, INTEGER) RETURNS BOOLEAN AS '
  SELECT EXISTS(SELECT * FROM item_specializations WHERE item_id = $1 AND specialization_id = $2)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    common_id    ALIAS FOR $1;
    primary_id   ALIAS FOR $2;
    secondary_id ALIAS FOR $3;
    type_        ALIAS FOR $4;
  BEGIN
    IF eq(common_id, primary_id) THEN
      RETURN secondary_id;
    ELSIF eq(common_id, secondary_id) THEN
      RETURN primary_id;
    ELSIF eq(primary_id, secondary_id) THEN
      RETURN primary_id;
    END IF;

    IF type_ = 1 THEN
      RETURN survey_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 2 THEN
      RETURN choice_component_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 3 THEN
      RETURN textresponse_component_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 4 OR type_ = 5 THEN
      RETURN text_component_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 6 THEN
      RETURN choice_question_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 8 THEN
      RETURN subsurvey_component_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 9 THEN
      RETURN pagebreak_merge(common_id, primary_id, secondary_id);
    ELSIF type_ = 10 THEN
      RETURN abet_component_merge(common_id, primary_id, secondary_id);
    ELSE
      RAISE EXCEPTION ''component_merge(%,%,%,%) failed. unknown revision type.'', $1, $2, $3, $4;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION list_changed(INTEGER, INTEGER[]) RETURNS BOOLEAN AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    numbers       ALIAS FOR $2;
    changed       BOOLEAN;
    rec           RECORD;
    i             INTEGER;
    j             INTEGER;
  BEGIN
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      -- see if question ordering changed
      i := 1;

      FOR rec IN SELECT item_id FROM list_items WHERE component_id = component_id_ ORDER BY ordinal LOOP
        --RAISE NOTICE ''question % changed? %'', i;
        j := numbers[i];
        IF j IS NULL OR j <> rec.item_id THEN changed := ''t''; EXIT; END IF;
        i := i + 1;
      END LOOP;
      changed := changed OR numbers[i] IS NOT NULL;
    END IF;
    RETURN changed;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION list_insert(INTEGER, INTEGER[]) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    numbers       ALIAS FOR $2;
    i             INTEGER;
    j             INTEGER;
  BEGIN
    i := 1;
    LOOP
      j := numbers[i];
      EXIT WHEN j IS NULL;
      INSERT INTO list_items (component_id, ordinal, item_id)
      VALUES (component_id_, i, j);
      i := i + 1;
    END LOOP;
    RETURN component_id_;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION survey_save(INTEGER, INTEGER[]) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    item_ids      ALIAS FOR $2;
  BEGIN
    IF list_changed(component_id_, item_ids) THEN
      INSERT INTO survey_components (type) VALUES (1);
      RETURN list_insert(currval(''component_ids'')::integer, item_ids);
    ELSE
      RETURN component_id_;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION choice_component_save(INTEGER, TEXT,
  INTEGER[], TEXT[], TEXT, INTEGER, INTEGER, INTEGER,
  INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_  ALIAS FOR $1;
    ctext_         ALIAS FOR $2;
    question_ids   ALIAS FOR $3;
    choices_       ALIAS FOR $4;
    other_choice_  ALIAS FOR $5;
    first_number_  ALIAS FOR $6;
    last_number_   ALIAS FOR $7;
    flags_         ALIAS FOR $8;
    rows_          ALIAS FOR $9;
    changed        BOOLEAN;
    r              RECORD;
    cid            INTEGER;
  BEGIN
    --RAISE NOTICE ''choice_component_save(%,%,%,%,%,%,%,%,%) called'', $1, $2, $3, $4, $5, $6, $7, $8, $9;

    changed := component_id_ IS NULL;
    IF NOT changed THEN
      changed := list_changed(component_id_, question_ids);

      IF NOT changed THEN
        SELECT INTO r ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE component_id = component_id_;
        IF NOT FOUND THEN
          RAISE EXCEPTION ''choice_component_save(%,%,%,%,%,%,%,%,%) fails. orig_id not found'', $1, $2, $3, $4, $5, $6, $7, $8, $9;
        END IF;

        changed := NOT is_true(r.ctext = ctext_ AND
          r.choices = choices_ AND r.other_choice = other_choice_ AND
          r.first_number = first_number_ AND r.last_number = last_number_ AND
          r.flags = flags_ AND r.rows = rows_);
      END IF;
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    cid := nextval(''component_ids'');

    INSERT INTO choice_components (component_id, type, ctext, flags, choices, other_choice, first_number, last_number, rows)
    VALUES (cid, 2, ctext_, flags_, choices_, other_choice_, first_number_, last_number_, rows_);

    PERFORM list_insert(cid, question_ids);

    RETURN cid;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION text_component_save(INTEGER, INTEGER, TEXT, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    type_         ALIAS FOR $2;
    ctext_        ALIAS FOR $3;
    flags_        ALIAS FOR $4;
    changed       BOOLEAN;
    rec           RECORD;
  BEGIN
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      SELECT INTO rec ctext, flags FROM text_components WHERE component_id = component_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''text_component_save(%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4;
      END IF;
      changed := NOT is_true(rec.ctext = ctext_ AND rec.flags = flags_);
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    INSERT INTO choice_components(type, ctext, flags)
    VALUES (type_, ctext_, flags_);

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION textresponse_component_save(INTEGER, TEXT, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_  ALIAS FOR $1;
    ctext_         ALIAS FOR $2;
    flags_         ALIAS FOR $3;
    rows_          ALIAS FOR $4;
    cols_          ALIAS FOR $5;
    changed        BOOLEAN;
    rec            RECORD;
  BEGIN
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      SELECT INTO rec ctext, flags, rows, cols FROM textresponse_components WHERE component_id = component_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''textresponse_component_save(%,%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4, $5;
      END IF;
      changed := NOT is_true(rec.ctext = ctext_ AND flags_ = rec.flags AND rec.rows = rows_ and rec.cols = cols_);
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    INSERT INTO textresponse_components (type, ctext, flags, rows, cols)
    VALUES (3, ctext_, flags_, rows_, cols_);

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION pagebreak_component_save(INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    renumber_     ALIAS FOR $2;
    changed       BOOLEAN;
    rec RECORD;
  BEGIN
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      SELECT INTO rec renumber FROM pagebreak_components WHERE component_id = component_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''pagebreak_component_save(%,%) fails. bad orig_id'', $1, $2;
      END IF;
      changed := NOT is_true(rec.renumber = renumber_);
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    INSERT INTO pagebreak_components(type, renumber)
    VALUES (9, renumber_);

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION choice_question_save(INTEGER, TEXT) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    qtext_        ALIAS FOR $2;
    changed       BOOLEAN;
    rec           RECORD;
  BEGIN
    -- RAISE NOTICE ''choice_question_save(%,%) called'', $1, $2;
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      SELECT INTO rec qtext FROM choice_questions WHERE component_id = component_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''choice_question_save(%,%) fails. bad orig_id'', $1, $2;
      END IF;
      changed := NOT is_true(rec.qtext = qtext_);
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    INSERT INTO choice_questions(type, qtext)
    VALUES (6, qtext_);

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION abet_component_save(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    component_id_ ALIAS FOR $1;
    type_         ALIAS FOR $2;
    which_        ALIAS FOR $3;
    changed       BOOLEAN;
    rec RECORD;
  BEGIN
    -- RAISE NOTICE ''abet_component_save(%,%) called'', $1, $2;
    changed := component_id_ IS NULL;
    IF NOT changed THEN
      SELECT INTO rec which FROM abet_components WHERE component_id = component_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''abet_component_save(%,%) fails. bad orig_id'', $1, $2;
      END IF;
      changed := NOT is_true(rec.which = which_);
    END IF;

    IF NOT changed THEN RETURN component_id_; END IF;

    INSERT INTO abet_components(type, which)
    VALUES (type_, which_);

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION choice_question_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      qtext FROM choice_questions WHERE component_id = orig_id;
    SELECT INTO primary_row   qtext FROM choice_questions WHERE component_id = primary_id;
    SELECT INTO secondary_row qtext FROM choice_questions WHERE component_id = secondary_id;
    INSERT INTO choice_questions (type, qtext) VALUES
    ( 6,
      text_merge(orig_row.qtext, primary_row.qtext, secondary_row.qtext, ''t'')
    );
    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION abet_component_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      which FROM abet_components WHERE component_id = orig_id;
    SELECT INTO primary_row   which FROM abet_components WHERE component_id = primary_id;
    SELECT INTO secondary_row which FROM abet_components WHERE component_id = secondary_id;
    INSERT INTO abet_components (type, which) VALUES
    ( 10,
      bitmask_merge(orig_row.which, primary_row.which, secondary_row.which)
    );
    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION survey_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
    new_id        INTEGER;
  BEGIN
    SELECT INTO orig_row      ctext, flags FROM survey_components WHERE component_id = orig_id;
    SELECT INTO primary_row   ctext, flags FROM survey_components WHERE component_id = primary_id;
    SELECT INTO secondary_row ctext, flags FROM survey_components WHERE component_id = secondary_id;

    INSERT INTO survey_components (type, ctext, flags) VALUES
    ( 1,
      text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags)
    );

    new_id := currval(''component_ids'');
    PERFORM list_merge(orig_id, primary_id, secondary_id, new_id);
    RETURN new_id;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION choice_component_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
    new_id        INTEGER;
  BEGIN
    SELECT INTO orig_row      ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE component_id = orig_id;
    SELECT INTO primary_row   ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE component_id = primary_id;
    SELECT INTO secondary_row ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE component_id = secondary_id;

    INSERT INTO choice_components (type, ctext, flags, choices, other_choice, first_number, last_number, rows) VALUES
    ( 2,
      text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags),
      text_array_merge(orig_row.choices, primary_row.choices, secondary_row.choices, ''t''),
      text_merge(orig_row.other_choice, primary_row.other_choice, secondary_row.other_choice, ''t''),
      integer_merge(orig_row.first_number, primary_row.first_number, secondary_row.first_number, ''t''),
      integer_merge(orig_row.last_number, primary_row.last_number, secondary_row.last_number, ''t''),
      integer_merge(orig_row.rows, primary_row.rows, secondary_row.rows, ''t'')
    );

    new_id := currval(''component_ids'');
    PERFORM list_merge(orig_id, primary_id, secondary_id, new_id);
    RETURN new_id;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION text_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    type_         ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      ctext, flags FROM text_components WHERE component_id = orig_id;
    SELECT INTO primary_row   ctext, flags FROM text_components WHERE component_id = primary_id;
    SELECT INTO secondary_row ctext, flags FROM text_components WHERE component_id = secondary_id;

    INSERT text_components (type, ctext, flags) VALUES
    ( type_
      text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags)
    );

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION textresponse_component_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      ctext, flags, rows, cols FROM textresponse_components WHERE component_id = orig_id;
    SELECT INTO primary_row   ctext, flags, rows, cols FROM textresponse_components WHERE component_id = primary_id;
    SELECT INTO secondary_row ctext, flags, rows, cols FROM textresponse_components WHERE component_id = secondary_id;

    INSERT INTO textresponse_components (type, ctext, flags, rows, cols) VALUES
    ( 3,
      text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags),
      integer_merge(orig_row.rows, primary_row.rows, secondary_row.rows, ''t''),
      integer_merge(orig_row.cols, primary_row.cols, secondary_row.cols, ''t'')
    );

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION pagebreak_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      renumber FROM text_components WHERE component_id = orig_id;
    SELECT INTO primary_row   renumber FROM text_components WHERE component_id = primary_id;
    SELECT INTO secondary_row renumber FROM text_components WHERE component_id = secondary_id;

    INSERT INTO choice_questions (type, renumber) VALUES
    ( 6,
      boolean_merge(orig_row.renumber, primary_row.renumber, secondary_row.renumber, ''t'')
    );

    RETURN currval(''component_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    common_id    ALIAS FOR $1;
    primary_id   ALIAS FOR $2;
    secondary_id ALIAS FOR $3;
    new_id       ALIAS FOR $4;
    rec RECORD;
    ord INTEGER := 1;
    ck INTEGER;
    nk INTEGER;
  BEGIN
    SELECT INTO ck COUNT(*) FROM list_items WHERE component_id = new_id;
    IF NOT ck = 0 THEN
      RAISE EXCEPTION ''list_merge(%,%,%,%) failed. List % is not empty.'', $1, $2, $3, $4, $4;
    END IF;

    -- Create new list based on primary but without items that were deleted in the secondary list.

    FOR rec IN
      SELECT li.item_id
      FROM list_items AS li
      WHERE
        li.component_id = primary_id
      AND NOT -- deleted
      (
        EXISTS (SELECT 1 FROM list_items AS c WHERE c.component_id = common_id AND c.item_id = li.item_id)
        AND
        NOT EXISTS (SELECT 1 FROM list_items AS h WHERE h.component_id = secondary_id AND h.item_id = li.item_id)
      )
      ORDER BY li.ordinal
    LOOP
      INSERT INTO list_items(component_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
      ord := ord + 1;
    END LOOP;

    -- Insert new items from secondary into the new list

    ord := 1;
    FOR rec IN SELECT ordinal, item_id FROM list_items WHERE component_id = secondary_id ORDER BY ordinal LOOP

      SELECT INTO ck ordinal FROM list_items WHERE component_id = common_id AND item_id = rec.item_id;
      IF NOT FOUND THEN ck := NULL; END IF;

      SELECT INTO nk ordinal FROM list_items WHERE component_id = new_id AND item_id = rec.item_id;
      IF NOT FOUND THEN nk := NULL; END IF;

      IF ck IS NULL AND nk IS NULL THEN -- needs to be inserted
        UPDATE list_items SET ordinal = ordinal + 1 WHERE component_id = new_id AND ordinal >= ord;
        INSERT INTO list_items(component_id, ordinal, item_id) VALUES (new_id, ord, rec.item_id);
        nk := ord;
      END IF;

      IF nk IS NOT NULL THEN ord := nk + 1; END IF;

    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION integer_merge(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    IF secondary_val = common_val OR (primary_val <> common_val AND favor_primary) THEN
      RETURN primary_val;
    ELSE
      RETURN secondary_val;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- function body is *exactly* the same for merge_integer
CREATE OR REPLACE FUNCTION text_merge(TEXT, TEXT, TEXT, BOOLEAN) RETURNS TEXT AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    IF secondary_val = common_val OR (primary_val <> common_val AND favor_primary) THEN
      RETURN primary_val;
    ELSE
      RETURN secondary_val;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- function body is *exactly* the same for merge_integer
CREATE OR REPLACE FUNCTION boolean_merge(BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN) RETURNS BOOLEAN AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    IF secondary_val = common_val OR (primary_val <> common_val AND favor_primary) THEN
      RETURN primary_val;
    ELSE
      RETURN secondary_val;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- function body is *exactly* the same for merge_integer
CREATE OR REPLACE FUNCTION array_merge(INTEGER[], INTEGER[], INTEGER[], BOOLEAN) RETURNS INTEGER[] AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    IF secondary_val = common_val OR (NOT(primary_val = common_val) AND favor_primary) THEN
      RETURN primary_val;
    ELSE
      RETURN secondary_val;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- function body is *exactly* the same for merge_integer
CREATE OR REPLACE FUNCTION text_array_merge(TEXT[], TEXT[], TEXT[], BOOLEAN) RETURNS TEXT[] AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
    IF secondary_val = common_val OR (NOT(primary_val = common_val) AND favor_primary) THEN
      RETURN primary_val;
    ELSE
      RETURN secondary_val;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION bitmask_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT (~ $1 & ($2 | $3)) | ($2 & $3)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION func_first (text[],text[]) RETURNS text[] AS '
  SELECT CASE WHEN $1 IS NOT NULL THEN $1 ELSE $2 END;
' LANGUAGE 'sql' WITH ( iscachable );

CREATE OR REPLACE FUNCTION func_last (text[],text[]) RETURNS text[] AS '
  SELECT $2;
' language 'sql' WITH ( iscachable );

--DROP AGGREGATE last text[];
--DROP AGGREGATE first text[];
CREATE AGGREGATE last ( BASETYPE = text[], SFUNC = func_last, STYPE = text[]);
CREATE AGGREGATE first ( BASETYPE = text[], SFUNC = func_first, STYPE = text[]);

CREATE TABLE cached_choice_responses
(
  topic_id INTEGER NOT NULL,
  citem_id INTEGER NOT NULL,
  crevision_id INTEGER NOT NULL,
  qitem_id INTEGER NOT NULL,
  qrevision_id INTEGER NOT NULL,
  dist INTEGER[] NOT NULL
);

CREATE OR REPLACE FUNCTION cached_choice_responses_update() RETURNS INTEGER AS '
  DELETE FROM cached_choice_responses;
  INSERT INTO cached_choice_responses (topic_id, citem_id, crevision_id, qitem_id, qrevision_id, dist)
  SELECT r.topic_id, cr.item_id, cr.revision_id, qr.item_id,
    qr.revision_id, choice_dist(qr.answer)
  FROM survey_responses AS r
  INNER JOIN choice_responses AS cr ON cr.parent = r.response_id
  INNER JOIN choice_question_responses AS qr ON qr.parent = cr.response_id
  GROUP BY r.topic_id, cr.item_id, cr.revision_id, qr.item_id, qr.revision_id;
  SELECT 1;
' LANGUAGE 'sql'; 

CREATE OR REPLACE FUNCTION cached_choice_responses_add(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_     ALIAS FOR $1;
    citem_id_     ALIAS FOR $2;
    crevision_id_ ALIAS FOR $3;
    qitem_id_     ALIAS FOR $4;
    qrevision_id_ ALIAS FOR $5;
    answer_       ALIAS FOR $6;
    t INTEGER;
  BEGIN
    SELECT INTO t topic_id FROM cached_choice_responses 
    WHERE topic_id = topic_id_ 
      AND citem_id = citem_id_ AND crevision_id = crevision_id_ 
      AND qitem_id = qitem_id_ AND qrevision_id = qrevision_id_ FOR UPDATE;
      
    IF FOUND THEN
      UPDATE cached_choice_responses SET
        dist = dist_insert(dist, answer_)
      WHERE topic_id = topic_id_ 
        AND citem_id = citem_id_ AND crevision_id = crevision_id_ 
        AND qitem_id = qitem_id_ AND qrevision_id = qrevision_id_;
    ELSE
      INSERT INTO cached_choice_responses (topic_id, citem_id, crevision_id, qitem_id, qrevision_id, dist)
      VALUES (topic_id_, citem_id_, crevision_id_, qitem_id_, qrevision_id_, dist_insert(null, answer_));
    END IF;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql'; 

-- this function body is EXACTLY the same as in the previous one
-- the only difference is that it accepts an array of integers instead of
-- an integer for the last parameter
CREATE OR REPLACE FUNCTION cached_choice_responses_add(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER[]) RETURNS INTEGER AS '
  DECLARE
    topic_id_     ALIAS FOR $1;
    citem_id_     ALIAS FOR $2;
    crevision_id_ ALIAS FOR $3;
    qitem_id_     ALIAS FOR $4;
    qrevision_id_ ALIAS FOR $5;
    answer_       ALIAS FOR $6;
    t INTEGER;
  BEGIN
    SELECT INTO t topic_id FROM cached_choice_responses 
    WHERE topic_id = topic_id_ 
      AND citem_id = citem_id_ AND crevision_id = crevision_id_ 
      AND qitem_id = qitem_id_ AND qrevision_id = qrevision_id_ FOR UPDATE;
      
    IF FOUND THEN
      UPDATE cached_choice_responses SET
        dist = dist_insert(dist, answer_)
      WHERE topic_id = topic_id_ 
        AND citem_id = citem_id_ AND crevision_id = crevision_id_ 
        AND qitem_id = qitem_id_ AND qrevision_id = qrevision_id_;
    ELSE
      INSERT INTO cached_choice_responses (topic_id, citem_id, crevision_id, qitem_id, qrevision_id, dist)
      VALUES (topic_id_, citem_id_, crevision_id_, qitem_id_, qrevision_id_, dist_insert(null, answer_));
    END IF;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql'; 

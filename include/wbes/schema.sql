DROP TABLE saves;
DROP SEQUENCE save_ids;
DROP TABLE branches;
DROP SEQUENCE branch_ids;
DROP TABLE branch_topics_cache;
DROP TABLE branch_ancestor_cache;
DROP TABLE generic_components;
DROP TABLE choice_components;
DROP TABLE choice_questions;
DROP TABLE textresponse_components;
DROP TABLE abet_components;
DROP TABLE text_components;
DROP TABLE surveys;
DROP TABLE subsurvey_components;
DROP TABLE pagebreak_components;
DROP TABLE revisions;
DROP SEQUENCE revision_ids;
DROP TABLE list_items;
DROP SEQUENCE list_item_ids;
DROP TABLE topics;
DROP SEQUENCE topic_ids;
DROP TABLE question_periods;
DROP SEQUENCE question_period_ids;
DROP TABLE survey_responses;
DROP TABLE choice_responses;
DROP TABLE choice_question_responses;
DROP TABLE mchoice_question_responses;
DROP TABLE textresponse_responses;
DROP TABLE responses;
DROP SEQUENCE response_ids;
DROP TABLE array_int_composite;
DROP FUNCTION array_int_pair(INTEGER, INTEGER);
DROP FUNCTION array_int_fill(INTEGER, INTEGER);
DROP FUNCTION boolean_cast(BOOLEAN);
DROP FUNCTION array_integer_length(INTEGER[]);
DROP FUNCTION array_integer_cast(TEXT);
DROP FUNCTION branch_generate();
DROP FUNCTION branch_generate(INTEGER, INTEGER, INTEGER, BOOLEAN, INTEGER);
DROP FUNCTION branch_ancestor_generate();
DROP FUNCTION branch_ancestor_generate(INTEGER, INTEGER);
DROP FUNCTION branch_topics_generate(INTEGER);
DROP FUNCTION branch_topics_generate();
DROP FUNCTION topic_contents(INTEGER);
DROP FUNCTION revision_contents(INTEGER);
DROP FUNCTION branch_latest(INTEGER, INTEGER);
DROP FUNCTION branch_latest(INTEGER, INTEGER, INTEGER);
DROP FUNCTION branch_topics_insert(INTEGER, INTEGER, INTEGER);
DROP FUNCTION branch_topics_add(INTEGER, INTEGER);
DROP FUNCTION branch_create(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION revision_create(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION revision_save_start(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION revision_save_end(INTEGER, INTEGER);
DROP FUNCTION branch_save(INTEGER, INTEGER);
DROP FUNCTION branch_topics_update(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_component_save(INTEGER, INTEGER, INTEGER, TEXT, TEXT[],INTEGER[], TEXT[], TEXT, INTEGER, INTEGER, INTEGER,INTEGER);
DROP FUNCTION subsurvey_component_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, TEXT);
DROP FUNCTION subsurvey_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION text_component_save(INTEGER, INTEGER, INTEGER, INTEGER, TEXT, INTEGER);
DROP FUNCTION textresponse_component_save(INTEGER, INTEGER, INTEGER, TEXT, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_question_save(INTEGER, INTEGER, INTEGER, TEXT);
DROP FUNCTION abet_component_save(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION revision_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION abet_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_question_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION text_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION textresponse_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION integer_merge(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION text_merge(TEXT, TEXT, TEXT, BOOLEAN);
DROP FUNCTION boolean_merge(BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN);
DROP FUNCTION array_merge(INTEGER[], INTEGER[], INTEGER[], BOOLEAN);
DROP FUNCTION text_array_merge(TEXT[], TEXT[], TEXT[], BOOLEAN);
DROP FUNCTION bitmask_merge(INTEGER, INTEGER, INTEGER);
DROP AGGREGATE choice_dist INTEGER;
DROP AGGREGATE choice_dist INTEGER[];
DROP FUNCTION int_distf(integer[], integer);
DROP FUNCTION int_mdistf(integer[], integer[]);
DROP FUNCTION pagebreak_component_save(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION pagebreak_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP AGGREGATE last text[];
DROP AGGREGATE first text[];
DROP FUNCTION func_first (text[],text[]);
DROP FUNCTION func_last (text[],text[]);

DROP FUNCTION references_branch(INTEGER);
DROP FUNCTION references_revisions(INTEGER);
DROP FUNCTION references_topic(INTEGER);
DROP FUNCTION references_question_period(INTEGER);


CREATE TABLE saves
(
  save_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('save_ids'),
  user_id INTEGER,
  date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE SEQUENCE save_ids INCREMENT 1 START 100;

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
-- 10 = abet component

CREATE TABLE revisions
(
  revision_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('revision_ids'),
  type INTEGER NOT NULL,
  parent INTEGER,
  branch_id INTEGER NOT NULL,
  revision INTEGER NOT NULL,
  save_id INTEGER,
  merged INTEGER,
  UNIQUE(parent, branch_id, revision)
);
CREATE SEQUENCE revision_ids INCREMENT 1 START 200;

CREATE TABLE branches
(
  branch_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('branch_ids'),
  topic_id INTEGER NOT NULL,

  -- hereafter fields are redundant but included for efficiency
  base_branch_id INTEGER NOT NULL,
  parent INTEGER,
  outdated BOOLEAN DEFAULT 'f',
  latest_id INTEGER,
  content_id INTEGER
);
CREATE SEQUENCE branch_ids INCREMENT 1 START 300;

CREATE TABLE branch_topics_cache
(
  base_branch_id INTEGER,
  topic_id INTEGER,
  branch_id INTEGER,
  PRIMARY KEY (base_branch_id,topic_id)
);

-- this entire table is redundant
CREATE TABLE branch_ancestor_cache
(
  ancestor_id INTEGER NOT NULL,
  descendant_id INTEGER NOT NULL
);

CREATE TABLE generic_components
(
  data TEXT
) INHERITS (revisions);

CREATE TABLE text_components
(
  ctext TEXT,
  flags INTEGER
) INHERITS (revisions);


---  survey_id INTEGER NOT NULL,

CREATE TABLE subsurvey_components
(
  base_branch INTEGER,
  flags INTEGER,
  title TEXT
) INHERITS (revisions);

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
) INHERITS (revisions);

CREATE TABLE textresponse_components
(
  rows INTEGER,
  cols INTEGER
) INHERITS (text_components);

CREATE TABLE abet_components
(
  which INTEGER -- bitmask
) INHERITS (revisions);

CREATE TABLE surveys
(
  flags INTEGER
) INHERITS (revisions);


CREATE TABLE pagebreak_components
(
  renumber BOOLEAN
) INHERITS (revisions);

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  revision_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);

CREATE SEQUENCE list_item_ids INCREMENT 1 START 400;
CREATE INDEX list_item_revision_idx ON list_items(revision_id);

CREATE TABLE topics
(
  topic_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL ('topic_ids'),
  parent INTEGER
);

CREATE SEQUENCE topic_ids INCREMENT 1 START 500;

CREATE TABLE question_periods
(
  question_period_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('question_period_ids'),
  displayname VARCHAR(60),
  begindate TIMESTAMP,
  enddate TIMESTAMP
);

CREATE SEQUENCE question_period_ids INCREMENT 1 START 1;

CREATE TABLE responses
(
  response_id INTEGER PRIMARY KEY DEFAULT NEXTVAL ('response_ids'),
  revision_id INTEGER NOT NULL,
  parent INTEGER
);

CREATE SEQUENCE response_ids INCREMENT 1 START 700;

CREATE TABLE survey_responses
(
  question_period_id INTEGER NOT NULL,
  topic_id INTEGER NOT NULL,
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

-- TODO: add indicies and foreign key constraints

-- drastically speeds up mass mailing
CREATE INDEX survey_response_m ON survey_responses(topic_id, question_period_id, user_id);

create index text_responses_parent_idx ON textresponse_responses (parent);
create index choice_responses_parent_idx ON choice_responses (parent);
create index choiceq_responses_parent_idx ON choice_question_responses (parent);
create index mchoiceq_responses_parent_idx ON mchoice_question_responses (parent);

ALTER TABLE revisions ADD FOREIGN KEY (save_id) REFERENCES saves(save_id);

-- Add these foreign keys when a future version of postgres supports them on inherited tables
-- 
-- ALTER TABLE revisions ADD FOREIGN KEY (parent) REFERENCES revisions(revision_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (branch_id) REFERENCES branches(branch_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (save_id) REFERENCES saves(save_id);
-- ALTER TABLE revisions ADD FOREIGN KEY (merged) REFERENCES revisions(revision_id);
-- 
-- ALTER TABLE branches ADD FOREIGN KEY (topic_id) REFERENCES topics(topic_id);
-- ALTER TABLE branches ADD FOREIGN KEY (base_branch_id) REFERENCES branches(branch_id);
-- ALTER TABLE branches ADD FOREIGN KEY (parent) REFERENCES branches(branch_id);
-- ALTER TABLE branches ADD FOREIGN KEY (latest_id) REFERENCES revisions(revision_id);
-- ALTER TABLE branches ADD FOREIGN KEY (content_id) REFERENCES revisions(revision_id);
-- 
-- ALTER TABLE branch_topics_cache ADD FOREIGN KEY (base_branch_id) REFERENCES branches(branch_id);
-- ALTER TABLE branch_topics_cache ADD FOREIGN KEY (branch_id) REFERENCES branches(branch_id);
-- ALTER TABLE branch_topics_cache ADD FOREIGN KEY (topic_id) REFERENCES topics(topic_id);
-- 
-- ALTER TABLE branch_ancestor_cache ADD FOREIGN KEY (ancestor_id) REFERENCES branches(branch_id);
-- ALTER TABLE branch_ancestor_cache ADD FOREIGN KEY (descendant_id) REFERENCES branches(branch_id);
-- 
-- ALTER TABLE list_items ADD FOREIGN KEY (revision_id) REFERENCES revisions(revision_id); 
-- ALTER TABLE list_items ADD FOREIGN KEY (item_id) REFERENCES branches(branch_id);
-- 
-- ALTER TABLE topics ADD FOREIGN KEY (parent) REFERENCES topics(topic_id);
-- 
-- ALTER TABLE responses ADD FOREIGN KEY (revision_id) REFERENCES revisions(revision_id);
-- ALTER TABLE responses ADD FOREIGN KEY (parent) REFERENCES responses(response_id);
-- 
-- ALTER TABLE survey_responses ADD FOREIGN KEY (question_period_id) REFERENCES question_periods(question_period_id);
-- ALTER TABLE survey_responses ADD FOREIGN KEY (topic_id) REFERENCES topics(topic_id);

-- in lieu of foreign keys, use these functions to quickly see if a branch,
-- topic, or revision is being referenced from some table

CREATE FUNCTION references_branch(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE branch_id = $1)                 THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE base_branch_id = $1)             THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE parent = $1)                     THEN 4 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_topics_cache WHERE base_branch_id = $1)  THEN 8 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_topics_cache WHERE branch_id = $1)       THEN 16 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_ancestor_cache WHERE ancestor_id = $1)   THEN 32 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_ancestor_cache WHERE descendant_id = $1) THEN 64 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM list_items WHERE item_id = $1)                  THEN 128 ELSE 0 END;
' LANGUAGE 'sql';

CREATE FUNCTION references_revisions(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE parent = $1)       THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM revisions WHERE merged = $1)       THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE latest_id = $1)     THEN 4 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branches WHERE content_id = $1)    THEN 8 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM list_items WHERE revision_id = $1) THEN 16 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM responses WHERE revision_id = $1)  THEN 32 ELSE 0 END;
' LANGUAGE 'sql';

CREATE FUNCTION references_topic(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM branches WHERE topic_id = $1)            THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM branch_topics_cache WHERE topic_id = $1) THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM survey_responses WHERE topic_id = $1)    THEN 4 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM topics WHERE parent = $1)                THEN 8 ELSE 0 END;
' LANGUAGE 'sql';

CREATE FUNCTION references_question_period(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM survey_responses WHERE question_period_id = $1) THEN 1 ELSE 0 END;
' LANGUAGE 'sql';

-- how to wipe out all customizations (ruins results for customizations) for a particular topic
--
-- delete from branch_topics_cache where topic_id = 999998;
-- delete from branch_ancestor_cache where ancestor_id IN (select branch_id from branches where topic_id = 999998);
-- delete from branch_ancestor_cache where descendant_id IN (select branch_id from branches where topic_id = 999998);
-- 
-- select revision_id, references_revisions(revision_id) from revisions where branch_id IN (select branch_id from branches where topic_id = 999998);
-- delete from list_items where revision_id IN (select revision_id from revisions where branch_id IN (select branch_id from branches where topic_id = 999998));
-- 
-- select revision_id, references_revision(revision_id) from revisions where branch_id IN (select branch_id from branches where topic_id = 999998);
-- delete from revisions where branch_id IN (select branch_id from branches where topic_id = 999998);
-- 
-- select branch_id, references_branch(branch_id) from branches where topic_id = 999998;
-- delete from branches where topic_id = 999998;


-- needed because there is no way currently to directly declare an array variable in plpgsql
CREATE TABLE array_int_composite(a INTEGER[]);

CREATE FUNCTION array_int_pair(INTEGER, INTEGER) RETURNS INTEGER[] AS '
  BEGIN
    RETURN ''{'' || $1 || '','' || $2 || ''}'';
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION array_int_fill(INTEGER, INTEGER) RETURNS INTEGER[] AS '
  DECLARE
    size ALIAS FOR $1;
    fill ALIAS FOR $2;
    a array_int_composite%ROWTYPE;
    i INTEGER := 1;
    s TEXT;
  BEGIN
    s := fill;
    WHILE i < size LOOP
      s := s || '','' || s;
      i := i * 2;
    END LOOP;

    s := ''{'' || s || ''}'';

    a.a := s;
    RETURN a.a[1:size];
  END;
' LANGUAGE 'plpgsql';


CREATE FUNCTION boolean_cast(BOOLEAN) RETURNS BOOLEAN AS '
  SELECT ($1 IS NOT NULL) AND $1;
' LANGUAGE 'sql';

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
    PERFORM branch_generate(NULL, NULL, NULL, ''f'', NULL);
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION branch_generate(INTEGER, INTEGER, INTEGER, BOOLEAN, INTEGER) RETURNS INTEGER AS '
  DECLARE
    parent_revision ALIAS FOR $1;
    parent_branch   ALIAS FOR $2;
    pcontent_id     ALIAS FOR $3;
    poutdated       ALIAS FOR $4; -- parent revision is outdated
    base_branch_id_ ALIAS FOR $5;
    last_branch_id  INTEGER := 0; -- d.branch_id from previous loop iteration
    next_branch     INTEGER;      -- result of recursive call
    new_branch      BOOLEAN;      -- d.revision_id is the newest revision
    outdated_       BOOLEAN;      -- d.revision_id is outdated
    visted          BOOLEAN;      -- already visited this branch
    content         INTEGER;      -- first nonzero ancestor of d.revision_id
    d               RECORD;
    bb              INTEGER;
  BEGIN
    FOR d IN
      SELECT r.revision_id, r.revision, b.branch_id, b.parent
      FROM revisions AS r INNER JOIN branches AS b USING(branch_id)
      WHERE r.parent = parent_revision OR (r.parent IS NULL AND parent_revision IS NULL)
      ORDER BY b.branch_id, r.revision DESC
    LOOP
      new_branch := d.branch_id <> last_branch_id;
      outdated_ := poutdated OR NOT new_branch;
      content := CASE WHEN d.revision = 0 THEN pcontent_id ELSE d.revision_id END;
      PERFORM branch_generate(d.revision_id, d.branch_id, content, outdated_,base_branch_id_);
      IF new_branch THEN
        IF d.parent IS NULL THEN
          bb := COALESCE(base_branch_id_, d.branch_id);
          UPDATE branches SET
            parent = parent_branch,
            latest_id = d.revision_id,
            content_id = content,
            outdated = outdated_,
            base_branch_id = bb
          WHERE branch_id = d.branch_id;
        END IF;
        last_branch_id = d.branch_id;
      END IF;
    END LOOP;
    RETURN last_branch_id;
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

CREATE FUNCTION branch_topics_generate(INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_ ALIAS FOR $1;
    t INTEGER;
    topic_info RECORD;
  BEGIN
    t := topic_id_;
    LOOP
      INSERT INTO branch_topics_cache (topic_id, base_branch_id, branch_id)
      SELECT topic_id_, base_branch_id, branch_id FROM branches
      WHERE topic_id = t;
      SELECT INTO t parent FROM topics WHERE topic_id = t;
      IF t IS NULL THEN EXIT; END IF;
    END LOOP;

    FOR topic_info IN SELECT topic_id FROM topics WHERE ((topic_id_ IS NULL AND parent IS NULL) OR topic_id_ = parent) LOOP
      IF EXISTS (SELECT * FROM branches WHERE topic_id = topic_info.topic_id) THEN
        PERFORM branch_topics_generate(topic_info.topic_id);
      END IF;
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION branch_topics_generate() RETURNS INTEGER AS '
  DELETE FROM branch_topics_cache;
  SELECT branch_topics_generate(NULL);
' LANGUAGE 'sql';

-- given topic_id that might not have any branches associated with it yet.
-- find an actual topic with saved stuff that will be used to load survey and component branches

CREATE FUNCTION topic_contents(INTEGER) RETURNS INTEGER AS '
DECLARE
  topic_id_ ALIAS FOR $1;
  e BOOLEAN;
  t INTEGER;
BEGIN
  t := topic_id_;
  LOOP
    SELECT INTO e EXISTS (SELECT branch_id FROM branch_topics_cache WHERE topic_id = t);
    IF e THEN RETURN t; END IF;
    SELECT INTO t parent FROM topics WHERE topic_id = t;
    IF NOT FOUND THEN RAISE EXCEPTION''topic_contents(%) fails. called with empty topic'', $1; END IF;
  END LOOP;
END;
' LANGUAGE 'plpgsql'
WITH (ISCACHABLE);

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

-- given a branch id this function return the latest revision on the branch.
-- If the latest existing revision is out of date, a new revision will be
-- generated and returned
-- save_id_ is used to intentionally retrieve out of date revisions with
-- save_id's lesser or equal to the argument. if null, the latest revision
-- is retrieved
DROP FUNCTION branch_latest(INTEGER, INTEGER);
CREATE FUNCTION branch_latest(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    save_id_ ALIAS FOR $2;
    info RECORD;
    rec RECORD;
    rec1 RECORD;
    rec2 RECORD;
    i INTEGER;
    j INTEGER;
    k INTEGER;
    bid INTEGER;
    par INTEGER;
    oldpar INTEGER;
    rec3 RECORD;

    -- current version of plpgsql (7.1.3) allows only read access to arrays,
    -- it is not possible to declare array variables or make assignments
    -- into arrays. this code gets around that restriction by putting
    -- numbers into strings, then casting those strings into array
    -- columns in a RECORD variable.
    s_bids  TEXT := ''''; -- stringed array of branch_ids for each branch level
    s_rids  TEXT := ''''; -- stringed array of revision_ids for each branch level
    s_prids TEXT := ''''; -- stringed array of parent revision_ids for each branch level
    s_revs  TEXT := ''''; -- stringed array of revision numbers for each branch level
    s_types TEXT := ''''; -- stringed array of types for each branch level
    sep     TEXT := '''';
  BEGIN
    -- RAISE NOTICE ''branch_latest(%,%) called'', $1, $2;

    -- get information about the branch

    SELECT INTO info outdated, latest_id, content_id
    FROM branches WHERE branch_id = branch_id_;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''branch_latest(%,%) called with invalid branch_id'', $1, $2;
    END IF;

    IF save_id_ IS NOT NULL THEN

      -- todo: this code will search parent branches for old save_ids, but will
      -- never merge new changes from parents back onto the starting branch.
      -- this functionality hasn''t been neccessary yet, so i didn''t write it,
      -- but it would be logical. haven''t put any thought into it, but it seems
      -- possible that adding the feature could make this method shorter &
      -- simpler than it is now

      i := branch_id_;
      LOOP
        
        j := NULL;
        k := NULL;

        -- this loop is needed to get the last save_id because for some reason
        -- the ORDER BY clause is getting ignored in plpgsql queries
        FOR rec3 IN SELECT revision_id, revision FROM revisions
        WHERE branch_id = branch_id_ AND save_id <= save_id_
        ORDER BY save_id_ 
        LOOP
          IF k IS NULL OR k < rec3.revision THEN
            k := rec3.revision;
            j := rec3.revision_id;  
          END IF;
        END LOOP;

        -- SELECT INTO j revision_id FROM revisions
        -- WHERE branch_id = branch_id_ AND save_id <= save_id_
        -- ORDER BY save_id_ DESC LIMIT 1;

        --RAISE NOTICE ''j = %'', j;

        IF j IS NOT NULL THEN RETURN j; END IF;

        SELECT INTO i parent FROM branches WHERE branch_id_ = i;
        IF NOT FOUND THEN
          RAISE EXCEPTION ''branch_latest(%,%) fails. unable to find early enough revision'', $1, $2;
        END IF;
      END LOOP;
    END IF;

    IF NOT info.outdated THEN RETURN info.latest_id; END IF;

    -- loop to lock and gather information about ancestor branches
    i := 0;
    bid := branch_id_;

    LOOP
      SELECT INTO rec latest_id AS rid, parent AS pbid
      FROM branches
      WHERE branch_id = bid AND outdated FOR UPDATE;

      IF NOT FOUND THEN EXIT; END IF;
      SELECT INTO rec2 parent AS prid, revision AS rev, type FROM revisions WHERE revision_id = rec.rid;

      s_bids  := s_bids  || sep || bid::TEXT;
      s_rids  := s_rids  || sep || rec.rid::TEXT;
      s_prids := s_prids || sep || rec2.prid::TEXT;
      s_revs  := s_revs  || sep || rec2.rev::TEXT;
      s_types := s_types || sep || rec2.type::TEXT;

      i := i + 1;
      bid := rec.pbid;
      sep := '','';
    END LOOP;

    IF i < 1 THEN
      RAISE EXCEPTION ''branch_latest(%,%) fails. impossible condition reached'', $1, $2;
    END IF;

    SELECT INTO rec
      array_integer_cast(''{'' || s_bids  || ''}'') AS bids,
      array_integer_cast(''{'' || s_rids  || ''}'') AS rids,
      array_integer_cast(''{'' || s_prids || ''}'') AS prids,
      array_integer_cast(''{'' || s_revs  || ''}'') AS revs,
      array_integer_cast(''{'' || s_types || ''}'') AS types;

    -- loop to bring ancestral branches up to date

    SELECT INTO par latest_id FROM branches WHERE branch_id = bid;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''branch_latest(%,%) fails. impossible condition reached'', $1, $2;
    END IF;

    FOR j IN REVERSE i..1 LOOP
      RAISE NOTICE ''branch_latest j = %'', j;
      IF rec.revs[j] = 0 THEN
        i := revision_create(rec.types[j], par, rec.bids[j], 0, NULL, rec.rids[j]);
      ELSE
        i := revision_create(rec.types[j], par, rec.bids[j], 1, NULL, rec.rids[j]);
        PERFORM revision_merge(revision_contents(rec.prids[j]), rec.rids[j], par, i);
      END IF;
      oldpar := par; par := i;
      UPDATE branches SET
        outdated = false, latest_id = par, content_id = par
      WHERE branch_id = rec.bids[j];
    END LOOP;

    RETURN par;
  END;
' LANGUAGE 'plpgsql';
SELECT branch_latest(810, 3115, 287);
SELECT branch_latest(841, 285);

-- this is the same as branch_latest(branch_id_, save_id_) except that
-- there it treats the branch_id_ like a base_branch_id and it has a
-- topic_id_ argument which it will use to conveniently find the correct
-- branch in the branch_topics_cache table. The topic *must* have entries
-- in the cache. If there is a chance that it doesn't, it should be
-- converted to a topic that does with a call to topic_contents(topic_id)

-- order of the arguments are base_branch_id, topic_id, save_id
CREATE FUNCTION branch_latest(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT branch_latest(
      (SELECT branch_id FROM branch_topics_cache WHERE topic_id = $2
        AND base_branch_id = $1), $3);
' LANGUAGE 'sql';


DROP FUNCTION revision_create(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER);
CREATE FUNCTION revision_create(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    type_        ALIAS FOR $1;
    parent_      ALIAS FOR $2;
    branch_id_   ALIAS FOR $3;
    revision_    ALIAS FOR $4;
    save_id_     ALIAS FOR $5;
    merged_      ALIAS FOR $6;
  BEGIN
    -- RAISE NOTICE ''revision_create(%,%,%,%,%,%) called'', $1, $2, $3, $4, $5, $6;
    IF type_ = 1 THEN
      INSERT INTO revisions (type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 2 THEN
      INSERT INTO choice_components (type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 3 THEN
      INSERT INTO textresponse_components (type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 4 OR type_ = 5 THEN
      INSERT INTO text_components (type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 6 THEN
      INSERT INTO choice_questions(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 7 THEN
      INSERT INTO generic_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 8 THEN
      INSERT INTO subsurvey_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 9 THEN
      INSERT INTO pagebreak_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 10 THEN
      INSERT INTO abet_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 100 THEN
      INSERT INTO abet_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE IF type_ = 101 THEN
      INSERT INTO abet_components(type, parent, branch_id, revision, save_id, merged)
      VALUES (type_, parent_, branch_id_, revision_, save_id_, merged_);
    ELSE
      RAISE EXCEPTION ''revision_create(%,%,%,%,%,%) called with unknown type number'', $1, $2, $3, $4, $5, $6;
    END IF; END IF; END IF; END IF; END IF; END IF; END IF; END IF; END IF; END IF; END IF;
    RETURN currval(''revision_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION branch_create(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_          ALIAS FOR $1;
    base_branch        ALIAS FOR $2;
    parent_branch_id   ALIAS FOR $3;
    parent_topic_id    ALIAS FOR $4;

    branch_id_        INTEGER;
    base_branch_id_   INTEGER;
    child             RECORD;
    parent            RECORD;
    nrev              INTEGER;
    t                 INTEGER;
    type_             INTEGER;
  BEGIN
    RAISE NOTICE ''branch_create(%,%,%,%) called.'', $1, $2, $3, $4;

    branch_id_ := nextval(''branch_ids'');
    base_branch_id_ := COALESCE(base_branch, branch_id_);

    INSERT INTO branches (branch_id, parent, base_branch_id, topic_id)
    VALUES (branch_id_, parent_branch_id, base_branch_id_, topic_id_);

    IF parent_branch_id IS NULL THEN
      PERFORM branch_topics_insert(topic_id_, base_branch_id_, branch_id_);
    ELSE
      -- update the branch_ancestor_cache table with information about the new branch

      INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
      SELECT ancestor_id, branch_id_
      FROM branch_ancestor_cache
      WHERE descendant_id = parent_branch_id;

      INSERT INTO branch_ancestor_cache (ancestor_id, descendant_id)
      VALUES (parent_branch_id, branch_id_);

      -- it is possible that there are child branches on parent_branch_id that pertain to topic_id, so move them underneath

      -- loop through the child branches
      FOR child IN SELECT branch_id, topic_id FROM branches WHERE parent = parent_branch_id AND branch_id <> branch_id_ LOOP
        t := child.topic_id;

        -- loop through ancestors of t
        LOOP
          SELECT INTO t parent FROM topics WHERE topic_id = t;
          IF NOT FOUND THEN RAISE EXCEPTION ''branch_id % doesn''''t belong underneath branch %.'', child.branch_id, parent_branch_id; END IF;
          IF t = topic_id_ THEN -- this branch needs to be moved
            -- loop through the parents of revisions that are on this branch.
            -- new zero revisions will be placed in between the parents and the revisions on rec1.branch_id
            FOR parent IN SELECT parent AS id FROM revisions WHERE branch_id = branch_id_ GROUP BY parent LOOP
              SELECT INTO type_ type FROM revisions WHERE revision_id_ = parent.id;
              -- revision_create not needed since this is a zero revision.
              INSERT INTO revisions (parent, branch_id, revision, save_id, type)
              VALUES (parent.id, branch_id_, 0, save_id_, type_)
              nrev := currval(''revision_ids'');
              UPDATE revisions SET parent = nrev WHERE parent = parent.id AND branch_id_ = branch_id_ AND revision_id <> nrev;
            END LOOP;
            EXIT;
          END IF;
          -- exit condition for branches that don''t need to be moved
          IF t = parent_topic_id THEN EXIT; END IF;
        END LOOP;
      END LOOP;

      -- update the branch_topics_cache table so that topic_id_ and its subtopics point to this branch

      IF NOT EXISTS (SELECT * FROM branch_topics_cache WHERE topic_id = topic_id_ AND base_branch_id = base_branch_id_) THEN
        INSERT INTO branch_topics_cache (base_branch_id, topic_id, branch_id)
          VALUES (base_branch_id_, topic_id_, branch_id_);
      END IF;
      PERFORM branch_topics_update(topic_id_, base_branch_id_, parent_branch_id, branch_id_);
    END IF;

    RETURN branch_id_;
  END;
' LANGUAGE 'plpgsql';

-- insert branch_topics for a new branch
CREATE FUNCTION branch_topics_insert(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_       ALIAS FOR $1;
    base_branch_id_ ALIAS FOR $2;
    branch_id_      ALIAS FOR $3;
    rec RECORD;
  BEGIN
    IF EXISTS (SELECT * FROM branches WHERE topic_id = topic_id_) THEN
      INSERT INTO branch_topics_cache (topic_id, base_branch_id, branch_id)
      VALUES (topic_id_, base_branch_id_, branch_id_);
    END IF;

    FOR rec IN SELECT topic_id FROM topics WHERE (topic_id_ IS NULL AND parent IS NULL) OR parent = topic_id_ LOOP
      PERFORM branch_topics_insert(rec.topic_id, base_branch_id_, branch_id_);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

-- insert a branch_topic for a (possibly) new topic
CREATE FUNCTION branch_topics_add(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_ ALIAS FOR $1;
    branch_id_ ALIAS FOR $2;
    base_branch_id_ INTEGER;
    i INTEGER;
  BEGIN
    SELECT INTO base_branch_id_ base_branch_id FROM branches WHERE branch_id = branch_id_;
    SELECT INTO i branch_id FROM branch_topics_cache WHERE topic_id = topic_id_ AND base_branch_id = base_branch_id_;
    IF NOT FOUND THEN
      INSERT INTO branch_topics_cache (base_branch_id, topic_id, branch_id)
        VALUES (base_branch_id_, topic_id_, branch_id_);
    END IF;
    RETURN base_branch_id_;
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

-- revision_save_start creates an entry for a new revision in the revisions table,
-- and returns the revision_id of this entry. After this function is called, the
-- revision contents should be inserted into the revision_items table using the
-- revision_id, and revision_save_end to should be called to follow up.

CREATE FUNCTION revision_save_start(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    orig_id    ALIAS FOR $2;
    type_      ALIAS FOR $3;
    save_id_   ALIAS FOR $4;
    info RECORD;
    rec RECORD;
    i INTEGER;
    j INTEGER;
    oparent INTEGER;
    orevision INTEGER;
    latest INTEGER;
    arr TEXT := '''';
    sep TEXT := '''';
    array RECORD;
    orig_branch INTEGER;
  BEGIN
    RAISE NOTICE ''revision_save_start(%,%,%,%) called'', $1, $2, $3, $4;

    -- lock the branch
    SELECT INTO info latest_id, content_id FROM branches
    WHERE branch_id = branch_id_ FOR UPDATE;

    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_start(%,%,%,%) fails. branch_id not found'', $1, $2, $3, $4;
    END IF;

    IF info.latest_id IS NULL AND info.content_id IS NULL THEN
      IF orig_id IS NULL THEN
        orevision := 0; oparent := NULL;
      ELSE
        -- this is the first revision on a brand new branch
        orevision := 0;

        -- need to find a parent revision. if parent branches are empty then special
        -- zero ancestor revisions will be created.

        SELECT INTO orig_branch branch_id FROM revisions WHERE revision_id = orig_id;
        i := 0;
        j := branch_id_;
        LOOP
          SELECT INTO j parent FROM branches WHERE branch_id = j;
          IF NOT FOUND THEN RAISE EXCEPTION ''beh''; END IF;
          EXIT WHEN j = orig_branch;
          arr := arr || sep || j::text;
          i   := i + 1;
          sep := '','';
        END LOOP;

        SELECT INTO array array_integer_cast(''{'' || arr || ''}'') AS a;
        oparent := orig_id;
        LOOP
          EXIT WHEN i = 0;

          SELECT INTO j revision_id FROM revisions WHERE parent = oparent AND revision = 0;
          IF FOUND THEN
            oparent := j;
          ELSE
            -- todo: insert should use the parent''s type value instead of type_
            INSERT INTO revisions (type,parent,branch_id,revision) VALUES (type_,oparent,array.a[i],0);
            SELECT INTO oparent currval(''revision_ids'');
            -- just in case branch somehow exists without revisions
            UPDATE branches SET latest_id = oparent, content_id = orig_id WHERE branch_id = array.a[i] AND latest_id IS NULL;
          END IF;
          i := i - 1;
        END LOOP;
      END IF;
    ELSE
      -- select the latest revision
      SELECT INTO rec parent, revision FROM revisions WHERE revision_id = info.latest_id;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''revision_save_start(%,%,%,%) fails. latest_id not found'', $1, $2, $3, $4;
      END IF;
      orevision := rec.revision;
      oparent := rec.parent;
    END IF;

    i := revision_create(type_, oparent, branch_id_, orevision + 1, save_id_, NULL);

    -- insert a new revision right after
    IF info.latest_id IS NOT NULL AND orig_id <> info.latest_id THEN -- merge needed
      j := revision_create(type_, oparent, branch_id_, orevision + 2, save_id_, orig_id);
    ELSE
      j := i;
    END IF;

    -- point to new revision
    UPDATE branches SET latest_id  = j, content_id = j WHERE branch_id = branch_id_;

    RETURN i;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION revision_save_end(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    saved_id ALIAS FOR $2;
    saved RECORD;
    merged RECORD;
    latest_id INTEGER;
    merge_id INTEGER;
  BEGIN
    RAISE NOTICE ''revision_save_end(%,%) called'', $1, $2;

    -- get information about the just saved revision
    SELECT INTO saved r.parent, r.branch_id, r.revision, b.base_branch_id
    FROM revisions AS r INNER JOIN branches AS b USING (branch_id)
    WHERE r.revision_id = saved_id;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_end(%,%) fails. saved_id not found'', $1, $2;
    END IF;

    -- mark sub branches for merge

    UPDATE branches SET outdated = ''t'' WHERE branch_id IN
      (SELECT descendant_id FROM branch_ancestor_cache WHERE ancestor_id = branch_id_);

    -- get the merge revision, if it exists

    SELECT INTO merged revision_id, merged AS orig_id
      FROM revisions
      WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch_id = saved.branch_id AND revision = saved.revision + 1;

    IF NOT FOUND THEN RETURN saved.base_branch_id; END IF;

    -- merge needed, retrieve most recent revision prior to save

    SELECT INTO latest_id revision_id FROM revisions WHERE
      ((parent IS NULL AND saved.parent IS NULL) OR parent = saved.parent)
      AND branch_id = saved.branch_id AND revision = saved.revision - 1;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_end(%,%) fails. Can''''t find predecessor for revision %'', $1, $2, saved_id;
    END IF;

    -- merge changes

    PERFORM revision_merge
    (
      revision_contents(merged.orig_id),
      saved_id,
      revision_contents(latest_id),
      merged.revision_id
    );
    RETURN saved.base_branch_id;
  END;
' LANGUAGE 'plpgsql';

-- todo: the revision_save functions and branch_latest functions are designed
-- when to work safely when there are concurrent calls on related revisions
-- and branches. i ignored concurrency when writing branch_save and
-- branch_create and the branch_topic functions. appropriate locking code needs
-- to be added to avoid deadlocks and exceptions and data corruption

CREATE FUNCTION branch_save(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_  ALIAS FOR $1;
    branch_id_ ALIAS FOR $2;
    orig            RECORD;
    existing_topic  INTEGER;
    existing_branch INTEGER;
  BEGIN
    RAISE NOTICE ''branch_save(%,%) called'', $1, $2;

    IF branch_id_ IS NULL THEN
      RETURN branch_create(topic_id_, NULL, NULL, NULL);
    ELSE
      SELECT INTO orig base_branch_id, topic_id FROM branches WHERE branch_id = branch_id_;
      IF topic_id_ = orig.topic_id THEN
        RETURN branch_id_;
      ELSE
        -- this means that the original component''s branch was not associated with this topic
        -- but to one of this topic''s ancestors (orig.topic_id). the following loop attempts
        -- to find a more closely related topic (and associated branch) that might have been
        -- created after the component was last loaded. The number of this branch
        -- is stored in the variable, existing_branch

        existing_topic := topic_id_;
        LOOP
          SELECT INTO existing_branch branch_id FROM branches
          WHERE base_branch_id = orig.base_branch_id AND topic_id = existing_topic;
          IF NOT FOUND THEN
            SELECT INTO existing_topic parent FROM topics WHERE topic_id = existing_topic;
            IF NOT FOUND THEN RAISE EXCEPTION ''security violation. the component that is being saved does not correspond to the survey topic, %'', topic_id_; END IF;
          ELSE
            EXIT;
          END IF;
        END LOOP;

        -- Create a branch for this topic
        RETURN branch_create(topic_id_, orig.base_branch_id, existing_branch, existing_topic);

      END IF;
    END IF;
  END;
' LANGUAGE 'plpgsql';

-- update branch_topics for a new branch

CREATE FUNCTION branch_topics_update(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_       ALIAS FOR $1;
    base_branch_id_ ALIAS FOR $2;
    oldbranch_id_   ALIAS FOR $3;
    newbranch_id_   ALIAS FOR $4;
    child RECORD;
  BEGIN
    UPDATE branch_topics_cache SET branch_id = newbranch_id_ WHERE
      base_branch_id = base_branch_id_ AND topic_id = topic_id_ AND branch_id = oldbranch_id_;
    FOR child IN SELECT topic_id FROM topics WHERE parent = topic_id_ LOOP
      PERFORM branch_topics_update(child.topic_id, base_branch_id_, oldbranch_id_, newbranch_id_);
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_component_save(INTEGER, INTEGER, INTEGER, TEXT, TEXT[],
  INTEGER[], TEXT[], TEXT, INTEGER, INTEGER, INTEGER,
  INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_      ALIAS FOR $1;
    orig_id_       ALIAS FOR $2;
    save_id_       ALIAS FOR $3;
    ctext_         ALIAS FOR $4;
    questions      ALIAS FOR $5;
    question_ids   ALIAS FOR $6;
    choices_       ALIAS FOR $7;
    other_choice_  ALIAS FOR $8;
    first_number_  ALIAS FOR $9;
    last_number_   ALIAS FOR $10;
    flags_         ALIAS FOR $11;
    rows_          ALIAS FOR $12;
    changed        BOOLEAN := ''t'';
    saveto         INTEGER;
    r              RECORD;
    s_branch_ids   TEXT := '''';
    sep            TEXT := '''';
    i              INTEGER := 1;
    j              INTEGER;
    rec            RECORD;
    rec1           RECORD;
    branch_id_     INTEGER;
  BEGIN
    --RAISE NOTICE ''choice_component_save(%,%,%,%,%,%,%,%,%,%,%,%,%) called'', $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12;

    -- save choices
    LOOP
      EXIT WHEN questions[i] IS NULL;
      --RAISE NOTICE ''saving question %'', i;
      j := CASE WHEN question_ids[i] = 0 THEN NULL ELSE question_ids[i] END;
      branch_id_ := choice_question_save(topic_id_, j, save_id_, questions[i]);
      s_branch_ids := s_branch_ids || sep || branch_id_::TEXT;
      i := i + 1;
      sep := '','';
    END LOOP;

    branch_id_ := NULL; -- reuse this variable for another purpose...
    s_branch_ids = ''{'' || s_branch_ids || ''}'';
    SELECT INTO rec array_integer_cast(s_branch_ids) AS branch_ids;

    IF orig_id_ IS NOT NULL THEN
      --RAISE NOTICE ''is this what deaner was talking about?'';
      changed := ''f'';

      -- see if question ordering changed
      i := 1;

      FOR rec1 IN SELECT item_id FROM list_items WHERE revision_id = orig_id_ ORDER BY ordinal LOOP
        --RAISE NOTICE ''question % changed? %'', i;
        j := rec.branch_ids[i];
        IF j IS NULL OR boolean_cast(j <> rec1.item_id) THEN changed := ''t''; EXIT; END IF;
        i := i + 1;
        --RAISE NOTICE ''question % not changed'', i;
      END LOOP;

      SELECT INTO r branch_id, ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''choice_component_save(%,%,%,%,%,%,%,%,%,%,%,%,%) fails. orig_id not found'', $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12;
      END IF;

      branch_id_ := r.branch_id;

      IF NOT changed THEN
        --RAISE NOTICE ''questions did not change'';
        changed := rec.branch_ids[i] IS NOT NULL OR NOT boolean_cast(
          r.ctext = ctext_ AND
          r.choices = choices_ AND r.other_choice = other_choice_ AND
          r.first_number = first_number_ AND r.last_number = last_number_ AND
          r.flags = flags_ AND r.rows = rows_);
        --RAISE NOTICE ''other parameter changed? %'', changed;
      END IF;
    END IF;

    IF changed THEN
      --RAISE NOTICE ''something changed. beginning save'', changed;
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, 2, save_id_);
      UPDATE choice_components SET
        ctext = ctext_,
        choices = choices_, other_choice = other_choice_,
        first_number = first_number_, last_number = last_number_,
       flags = flags_, rows = rows_
      WHERE revision_id = saveto;
      i := 1;
      LOOP
        j := rec.branch_ids[i];
        EXIT WHEN rec.branch_ids[i] IS NULL;
        INSERT INTO list_items (revision_id, ordinal, item_id) VALUES (saveto, i, j);
        i := i + 1;
      END LOOP;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      --RAISE NOTICE ''nothing changed'', changed;
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';






















CREATE FUNCTION subsurvey_component_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER, TEXT) RETURNS INTEGER AS '
  DECLARE
    topic_id_      ALIAS FOR $1;
    orig_id_       ALIAS FOR $2;
    save_id_       ALIAS FOR $3;
    base_branch_   ALIAS FOR $4;
    flags_         ALIAS FOR $5;
    title_ 	   ALIAS FOR $6;
    branch_id_   INTEGER := NULL;
    changed      BOOLEAN := ''t'';
    saveto       INTEGER;
    rec RECORD;
  BEGIN
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, flags, base_branch, title FROM subsurvey_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''text_component_save(%,%,%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4, $5, $6;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.base_branch = base_branch_ AND flags_ = rec.flags AND title_ = rec.title);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, 8, save_id_);
      UPDATE subsurvey_components SET base_branch = base_branch_, flags = flags_, title = title_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';





CREATE FUNCTION subsurvey_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      flags, base_branch, title FROM subsurvey_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   flags, base_branch, title FROM subsurvey_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row flags, base_branch, title FROM subsurvey_components WHERE revision_id = secondary_id;
    UPDATE choice_questions SET
      flags = integer_merge(orig_row.flags, primary_row.flags, secondary_row.flags, ''t''),
      base_branch = integer_merge(orig_row.base_branch, primary_row.base_branch, secondary_row.base_branch, ''t''),
      title = text_merge(orig_row.title, primary_row.title, secondary_row.title, ''t'')
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';


























CREATE FUNCTION text_component_save(INTEGER, INTEGER, INTEGER, INTEGER, TEXT, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_    ALIAS FOR $1;
    orig_id_     ALIAS FOR $2;
    save_id_     ALIAS FOR $3;
    type_        ALIAS FOR $4;
    ctext_       ALIAS FOR $5;
    flags_       ALIAS FOR $6;
    branch_id_   INTEGER := NULL;
    changed      BOOLEAN := ''t'';
    saveto       INTEGER;
    rec RECORD;
  BEGIN
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, ctext, flags FROM text_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''text_component_save(%,%,%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4, $5, $6;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.ctext = ctext_ AND flags_ = rec.flags);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, type_, save_id_);
      UPDATE text_components SET ctext = ctext_, flags = flags_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION textresponse_component_save(INTEGER, INTEGER, INTEGER, TEXT, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_  ALIAS FOR $1;
    orig_id_   ALIAS FOR $2;
    save_id_   ALIAS FOR $3;
    ctext_     ALIAS FOR $4;
    flags_     ALIAS FOR $5;
    rows_      ALIAS FOR $6;
    cols_      ALIAS FOR $7;
    branch_id_ INTEGER := NULL;
    changed    BOOLEAN := ''t'';
    saveto     INTEGER;
    rec RECORD;
  BEGIN
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, ctext, flags, rows, cols FROM textresponse_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''textresponse_component_save(%,%,%,%,%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4, $5, $6, $7;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.ctext = ctext_ AND flags_ = rec.flags AND rec.rows = rows_ and rec.cols = cols_);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, 3, save_id_);
      UPDATE textresponse_components SET ctext = ctext_, flags = flags_, rows = rows_, cols = cols_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION pagebreak_component_save(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    topic_id_  ALIAS FOR $1;
    orig_id_   ALIAS FOR $2;
    save_id_   ALIAS FOR $3;
    renumber_  ALIAS FOR $4;
    branch_id_ INTEGER := NULL;
    changed    BOOLEAN := ''t'';
    saveto     INTEGER;
    rec RECORD;
  BEGIN
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, renumber FROM pagebreak_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''pagebreak_component_save(%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.renumber = renumber_);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, 9, save_id_);
      UPDATE pagebreak_components SET renumber = renumber_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_question_save(INTEGER, INTEGER, INTEGER, TEXT) RETURNS INTEGER AS '
  DECLARE
    topic_id_    ALIAS FOR $1;
    orig_id_     ALIAS FOR $2;
    save_id_     ALIAS FOR $3;
    qtext_       ALIAS FOR $4;
    branch_id_   INTEGER := NULL;
    changed      BOOLEAN := ''t'';
    saveto       INTEGER;
    rec RECORD;
  BEGIN
    -- RAISE NOTICE ''choice_question_save(%,%,%,%) called'', $1, $2, $3, $4;
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, qtext FROM choice_questions WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''choice_question_save(%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.qtext = qtext_);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, 6, save_id_);
      UPDATE choice_questions SET qtext = qtext_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
  END;
' LANGUAGE 'plpgsql';

DROP FUNCTION abet_component_save(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION abet_component_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER);
CREATE FUNCTION abet_component_save(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    topic_id_    ALIAS FOR $1;
    orig_id_     ALIAS FOR $2;
    save_id_     ALIAS FOR $3;
    which_       ALIAS FOR $4;
    typecode_    ALIAS FOR $5;
    branch_id_   INTEGER := NULL;
    changed      BOOLEAN := ''t'';
    saveto       INTEGER;
    rec RECORD;
  BEGIN
    -- RAISE NOTICE ''abet_component_save(%,%,%,%) called'', $1, $2, $3, $4;
    IF orig_id_ IS NOT NULL THEN
      SELECT INTO rec branch_id, which FROM abet_components WHERE revision_id = orig_id_;
      IF NOT FOUND THEN
        RAISE EXCEPTION ''abet_component_save(%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4;
      END IF;
      branch_id_ := rec.branch_id;
      changed := NOT (rec.which = which_);
    END IF;

    IF changed THEN
      branch_id_ := branch_save(topic_id_, branch_id_);
      saveto := revision_save_start(branch_id_, orig_id_, typecode_, save_id_);
      UPDATE abet_components SET which = which_ WHERE revision_id = saveto;
      RETURN revision_save_end(branch_id_, saveto);
    ELSE
      RETURN branch_topics_add(topic_id_, branch_id_);
    END IF;
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

    IF t = 2 THEN
      RETURN choice_component_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 3 THEN
      RETURN textresponse_component_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 4 OR t = 5 THEN
      RETURN text_component_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 6 THEN
      RETURN choice_question_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 8 THEN
	RETURN subsurvey_component_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 9 THEN
      RETURN pagebreak_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    IF t = 10 THEN
      RETURN abet_component_merge(common_id, primary_id, secondary_id, new_id);
    END IF;

    RAISE EXCEPTION ''revision_merge(%,%,%,%) failed. Revision type unknown.'', $1, $2, $3, $4;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_question_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    -- RAISE NOTICE ''choice_question_merge(%,%,%,%) called'', $1, $2, $3, $4;
    SELECT INTO orig_row      qtext FROM choice_questions WHERE revision_id = orig_id;
    SELECT INTO primary_row   qtext FROM choice_questions WHERE revision_id = primary_id;
    SELECT INTO secondary_row qtext FROM choice_questions WHERE revision_id = secondary_id;
    UPDATE choice_questions SET
      qtext = text_merge(orig_row.qtext, primary_row.qtext, secondary_row.qtext, ''t'')
    WHERE revision_id = new_id;
    -- RAISE NOTICE ''choice_question_merge(%,%,%,%) returns'', $1, $2, $3, $4;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION abet_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    -- RAISE NOTICE ''abet_component_merge(%,%,%,%) called'', $1, $2, $3, $4;
    SELECT INTO orig_row      which FROM abet_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   which FROM abet_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row which FROM abet_components WHERE revision_id = secondary_id;
    UPDATE abet_components SET
      which = bitmask_merge(orig_row.which, primary_row.which, secondary_row.which)
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    -- RAISE NOTICE ''choice_component_merge(%,%,%,%) called'', $1, $2, $3, $4;
    SELECT INTO orig_row      ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, choices, other_choice, first_number, last_number, flags, rows FROM choice_components WHERE revision_id = secondary_id;
    PERFORM list_merge(orig_id, primary_id, secondary_id, new_id);
    PERFORM text_component_merge(orig_id, primary_id, secondary_id, new_id);
    UPDATE choice_components SET
      ctext        = text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      choices      = text_array_merge(orig_row.choices, primary_row.choices, secondary_row.choices, ''t''),
      other_choice = text_merge(orig_row.other_choice, primary_row.other_choice, secondary_row.other_choice, ''t''),
      first_number = integer_merge(orig_row.first_number, primary_row.first_number, secondary_row.first_number, ''t''),
      last_number  = integer_merge(orig_row.last_number, primary_row.last_number, secondary_row.last_number, ''t''),
      flags        = bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags),
      rows         = integer_merge(orig_row.rows, primary_row.rows, secondary_row.rows, ''t'')
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION text_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      ctext, flags FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, flags FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, flags FROM text_components WHERE revision_id = secondary_id;

    UPDATE text_components SET
      ctext = text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      flags = bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags)
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION textresponse_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      ctext, flags, rows, cols FROM textresponse_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, flags, rows, cols FROM textresponse_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, flags, rows, cols FROM textresponse_components WHERE revision_id = secondary_id;

    UPDATE textresponse_components SET
      ctext = text_merge(orig_row.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      flags = bitmask_merge(orig_row.flags, primary_row.flags, secondary_row.flags),
      rows  = integer_merge(orig_row.rows, primary_row.rows, secondary_row.rows, ''t''),
      cols  = integer_merge(orig_row.cols, primary_row.cols, secondary_row.cols, ''t'')
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION pagebreak_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    orig_id       ALIAS FOR $1;
    primary_id    ALIAS FOR $2;
    secondary_id  ALIAS FOR $3;
    new_id        ALIAS FOR $4;
    orig_row      RECORD;
    primary_row   RECORD;
    secondary_row RECORD;
  BEGIN
    SELECT INTO orig_row      renumber FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   renumber FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row renumber FROM text_components WHERE revision_id = secondary_id;
    UPDATE choice_questions SET
      renumber = boolean_merge(orig_row.renumber, primary_row.renumber, secondary_row.renumber, ''t'')
    WHERE revision_id = new_id;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
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

    ord := 1;
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

CREATE FUNCTION integer_merge(INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
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
CREATE FUNCTION text_merge(TEXT, TEXT, TEXT, BOOLEAN) RETURNS TEXT AS '
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
CREATE FUNCTION boolean_merge(BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN) RETURNS BOOLEAN AS '
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
CREATE FUNCTION array_merge(INTEGER[], INTEGER[], INTEGER[], BOOLEAN) RETURNS INTEGER[] AS '
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
CREATE FUNCTION text_array_merge(TEXT[], TEXT[], TEXT[], BOOLEAN) RETURNS TEXT[] AS '
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

CREATE FUNCTION bitmask_merge(INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT (~ $1 & ($2 | $3)) | ($2 & $3)
' LANGUAGE 'sql';

CREATE AGGREGATE choice_dist (
    basetype = INTEGER,
    stype = INTEGER[],
    sfunc = int_distf
);

CREATE AGGREGATE choice_dist (
    basetype = INTEGER[],
    stype = INTEGER[],
    sfunc = int_mdistf
);

CREATE FUNCTION func_first (text[],text[]) RETURNS text[] AS '
  SELECT CASE WHEN $1 IS NOT NULL THEN $1 ELSE $2 END;
' LANGUAGE 'sql' WITH ( iscachable );

CREATE FUNCTION func_last (text[],text[]) RETURNS text[] AS '
  SELECT $2;
' language 'sql' WITH ( iscachable );

CREATE AGGREGATE last ( BASETYPE = text[], SFUNC = func_last, STYPE = text[]);
CREATE AGGREGATE first ( BASETYPE = text[], SFUNC = func_first, STYPE = text[]);

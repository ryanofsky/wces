DROP TABLE saves;
DROP SEQUENCE save_ids;
DROP TABLE branches;
DROP SEQUENCE branch_ids;
DROP TABLE branch_ancestor_cache;
DROP TABLE generic_components;
DROP TABLE choice_components;
DROP TABLE choice_questions;
DROP TABLE textresponse_components;
DROP TABLE list_items;
DROP SEQUENCE list_item_ids;
DROP TABLE text_components;
DROP TABLE revisions;
DROP SEQUENCE revision_ids;
DROP FUNCTION boolean_cast(BOOLEAN);
DROP FUNCTION array_integer_length(INTEGER[]);
DROP FUNCTION array_integer_cast(TEXT);
DROP FUNCTION branch_generate();
DROP FUNCTION branch_generate(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION branch_ancestor_generate();
DROP FUNCTION branch_ancestor_generate(INTEGER, INTEGER);
DROP SEQUENCE branch_top;
DROP FUNCTION branch_nextval(INTEGER);
DROP FUNCTION revision_contents(INTEGER);
DROP FUNCTION revision_save_start(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION revision_save_end(INTEGER, INTEGER);
DROP FUNCTION branch_update(INTEGER);
DROP FUNCTION revision_create(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION text_component_save(INTEGER, INTEGER, INTEGER, TEXT, BOOLEAN);
DROP FUNCTION textresponse_component_save(INTEGER, INTEGER, TEXT, BOOLEAN, INTEGER, INTEGER);
DROP FUNCTION choice_question_save(INTEGER, INTEGER, TEXT);
DROP FUNCTION choice_component_save(INTEGER, INTEGER, TEXT, BOOLEAN, TEXT[], INTEGER[], TEXT[], TEXT, INTEGER, INTEGER, BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN, INTEGER);
DROP FUNCTION revision_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_question_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION choice_component_merge(INTEGER, INTEGER, INTEGER, INTEGER) ;
DROP FUNCTION text_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION textresponse_component_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION list_merge(INTEGER, INTEGER, INTEGER, INTEGER);
DROP FUNCTION integer_merge(INTEGER, INTEGER, INTEGER, BOOLEAN);
DROP FUNCTION text_merge(TEXT, TEXT, TEXT, BOOLEAN);
DROP FUNCTION boolean_merge(BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN);
DROP FUNCTION array_merge(INTEGER[], INTEGER[], INTEGER[], BOOLEAN);

CREATE TABLE saves
(
  save_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('save_ids'),
  user_id INTEGER,
  date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  revision_id INTEGER NOT NULL
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

CREATE TABLE revisions
(
  revision_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('revision_ids'),
  type INTEGER NOT NULL,
  parent INTEGER,
  branch_id INTEGER NOT NULL,
  revision INTEGER NOT NULL,
  save_id INTEGER,
  UNIQUE(parent, branch_id, revision)
);
CREATE SEQUENCE revision_ids INCREMENT 1 START 100;

CREATE TABLE branches
(
  branch_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('branch_ids'),
  branch INTEGER NOT NULL,

  -- hereafter fields are redundant but included for efficiency
  parent INTEGER,
  outdated BOOLEAN DEFAULT 'f',
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

CREATE TABLE generic_components
(
  data TEXT
) INHERITS (revisions);

CREATE TABLE text_components
(
  ctext TEXT, 
  is_html BOOLEAN
) INHERITS (revisions);

CREATE TABLE choice_components
(
  choices      TEXT[],
  other_choice TEXT,
  first_number INTEGER,
  last_number  INTEGER,
  is_numeric   BOOLEAN,
  select_many  BOOLEAN,
  stacked      BOOLEAN,
  vertical     BOOLEAN,
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

CREATE TABLE list_items
(
  list_item_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('list_item_ids'),
  revision_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL,
  item_id INTEGER NOT NULL
);

CREATE SEQUENCE list_item_ids INCREMENT 1 START 100;
CREATE INDEX list_item_revision_idx ON list_items(revision_id);

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
  BEGIN
    -- RAISE NOTICE ''revision_save_start(%,%,%,%) called'', $1, $2, $3, $4;
    
    -- lock the branch
    SELECT INTO info branch, latest_id, content_id FROM branches
    WHERE branch_id = branch_id_ FOR UPDATE;
    
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_start(%,%,%,%) fails. branch_id not found'', $1, $2, $3, $4;
    END IF;
    
    IF info.latest_id IS NULL AND info.content_id IS NULL AND orig_id IS NULL THEN
      orevision := 0; oparent := NULL;
    ELSE
      -- check for valid orig_id
      SELECT INTO i branch_id FROM revisions WHERE revision_id = orig_id;
      IF i IS NULL OR branch_id_ <> i THEN 
        RAISE EXCEPTION ''revision_save_start(%,%,%,%) fails. bad orig_id'', $1, $2, $3, $4;
      END IF;
    
      -- select the latest revision
      SELECT INTO rec parent, revision FROM revisions WHERE revision_id = info.latest_id;
      orevision := rec.revision; oparent := rec.parent;
    END IF;
    
    i := revision_create(type_, oparent, branch_id_, orevision + 1, save_id_);
    
    -- insert a new revision right after
    IF orig_id <> info.latest_id THEN -- merge needed
      INSERT INTO saves (revision_id) VALUES (orig_id);
      j := revision_create(type_, oparent, branch_id_, orevision + 2, currval(''save_ids''));
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
    merg e_id INTEGER;
  BEGIN
    -- RAISE NOTICE ''revision_save_end(%,%) called'', $1, $2;
    
    -- get information about the just saved revision
    SELECT INTO saved parent, branch_id, revision FROM revisions WHERE revision_id = saved_id;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_save_end(%,%) fails. saved_id not found'', $1, $2;
    END IF;

    -- mark sub branches for merge

    UPDATE branches SET outdated = ''t'' WHERE branch_id IN
      (SELECT descendant_id FROM branch_ancestor_cache WHERE ancestor_id = branch_id_);

    -- get the merge revision, if it exists

    SELECT INTO merged r.revision_id, s.revision_id AS orig_id
      FROM revisions AS r INNER JOIN saves AS s USING (save_id)
      WHERE
      ((r.parent IS NULL AND saved.parent IS NULL) OR r.parent = saved.parent)
      AND r.branch_id = saved.branch_id AND r.revision = saved.revision + 1;

    IF NOT FOUND THEN return saved_id; END IF;

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
    RETURN merged.revision_id;
  END;
' LANGUAGE 'plpgsql';

-- given a branch id this function return the latest revision on the branch.
-- If the latest existing revision is out of date
CREATE FUNCTION branch_update(INTEGER) RETURNS INTEGER AS '
  DECLARE
    branch_id_ ALIAS FOR $1;
    info RECORD;
    rec RECORD;
    rec1 RECORD;
    rec2 RECORD;
    s_bids TEXT;
    s_rids TEXT;
    s_prids TEXT;
    s_revs TEXT;
    s_types TEXT;
    sep TEXT;
    i INTEGER;
    j INTEGER;
    bid INTEGER;
    par INTEGER;
    oldpar INTEGER;
  BEGIN
    -- RAISE NOTICE ''branch_update(%) called'', $1;
    
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
    s_types := ''''; -- stringed array of types for each branch level
    sep     := '''';

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
      RAISE EXCEPTION ''revision_update(%) fails. impossible condition reached'', $1;
    END IF;

    s_bids  := ''{'' || s_bids  || ''}'';
    s_rids  := ''{'' || s_rids  || ''}'';
    s_prids := ''{'' || s_prids || ''}'';
    s_revs  := ''{'' || s_revs  || ''}'';
    s_types := ''{'' || s_types || ''}'';

    SELECT INTO rec
      array_integer_cast(s_bids) AS bids,
      array_integer_cast(s_rids) AS rids,
      array_integer_cast(s_prids) AS prids,
      array_integer_cast(s_revs) AS revs,
      array_integer_cast(s_types) AS types;

    -- loop to bring ancestral branches up to date

    SELECT INTO par latest_id FROM branches WHERE branch_id = bid;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''revision_update(%) fails. impossible condition reached'', $1;
    END IF;

    FOR j IN REVERSE i..1 LOOP
      INSERT INTO saves (revision_id, rec.rids[j]);
      IF rec.revs[j] = 0 THEN
        i := revision_create(rec.types[j], par, rec.bids[j], 0, currval(''save_ids''));
      ELSE
        i := revision_create(rec.types[j], par, rec.bids[j], 1, currval(''save_ids''));
        PERFORM revision_merge(rec.prids[j], rec.rids[j], par, i);
      END IF;
      oldpar := par; par := i;
      UPDATE branches SET
        outdated = false, latest_id = par, content_id = par
      WHERE branch_id = rec.bids[j];
    END LOOP;

    RETURN par;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION revision_create(INTEGER, INTEGER, INTEGER, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    type_        ALIAS FOR $1;
    parent_      ALIAS FOR $2;
    branch_id_   ALIAS FOR $3;
    revision_    ALIAS FOR $4;
    save_id_     ALIAS FOR $5;
  BEGIN
    -- RAISE NOTICE ''revision_create(%,%,%,%,%) called'', $1, $2, $3, $4, $5;
    IF type_ = 1 THEN
      INSERT INTO revisions (type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_);
    ELSE IF type_ = 2 THEN
      INSERT INTO choice_components (type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_);      
    ELSE IF type_ = 3 THEN
      INSERT INTO textresponse_components (type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_); 
    ELSE IF type_ = 4 OR type_ = 5 THEN
      INSERT INTO text_components (type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_);
    ELSE IF type_ = 6 THEN 
      INSERT INTO choice_questions(type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_);
    ELSE IF type_ = 7 THEN
      INSERT INTO generic_components(type, parent, branch_id, revision, save_id)
      VALUES (type_, parent_, branch_id_, revision_, save_id_);
    ELSE
      RAISE EXCEPTION ''revision_create(%,%,%,%,%) called with unknown type number'', $1, $2, $3, $4, $5;
    END IF; END IF; END IF; END IF; END IF; END IF;
    RETURN currval(''revision_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION text_component_save(INTEGER, INTEGER, INTEGER, TEXT, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    save_id      ALIAS FOR $1;
    type_        ALIAS FOR $2;
    revision_id_ ALIAS FOR $3;
    ctext_       ALIAS FOR $4;
    is_html_     ALIAS FOR $5;
    parent_ INTEGER;
    branch_id_ INTEGER;
    saveto INTEGER;
    rec RECORD;
  BEGIN
    SELECT INTO rec ctext, is_html, branch_id FROM text_components WHERE revision_id = revision_id_;
    IF FOUND THEN
      branch_id_ := rec.branch_id;
      IF rec.ctext = ctext_ AND is_html_ = rec.is_html THEN
        RETURN branch_id_;
      END IF;
    ELSE  
      parent_ := NULL; -- everything new goes on a base branch?
      INSERT INTO branches (branch, parent) VALUES (branch_nextval(parent_), parent_);
      branch_id_ := currval(''branch_ids'');
    END IF;

    saveto := revision_save_start(branch_id_, revision_id_, type_, save_id);
    UPDATE text_components SET ctext = ctext_, is_html = is_html_ WHERE revision_id = saveto;
    PERFORM revision_save_end(branch_id_, saveto);
    RETURN branch_id_;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION textresponse_component_save(INTEGER, INTEGER, TEXT, BOOLEAN, INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    save_id      ALIAS FOR $1;
    revision_id_ ALIAS FOR $2;
    ctext_       ALIAS FOR $3;
    is_html_     ALIAS FOR $4;
    rows_        ALIAS FOR $5;
    cols_        ALIAS FOR $6;
    parent_ INTEGER;
    branch_id_ INTEGER;
    saveto INTEGER;
    rec RECORD;
  BEGIN
    SELECT INTO rec ctext, is_html, branch_id, rows, cols FROM textresponse_components WHERE revision_id = revision_id_;
    IF FOUND THEN
      branch_id_ := rec.branch_id;
      IF rec.ctext = ctext_ AND is_html_ = rec.is_html AND rec.rows = rows_ and rec.cols = cols_ THEN
        RETURN branch_id_;
      END IF;
    ELSE  
      parent_ := NULL; -- everything new goes on a base branch?
      INSERT INTO branches (branch, parent) VALUES (branch_nextval(parent_), parent_);
      branch_id_ := currval(''branch_ids'');
    END IF;

    saveto := revision_save_start(branch_id_, revision_id_, 3, save_id);
    UPDATE textresponse_components SET ctext = ctext_, is_html = is_html_, rows = rows_, cols = cols_ WHERE revision_id = saveto;
    PERFORM revision_save_end(branch_id_, saveto);
    RETURN branch_id_;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_question_save(INTEGER, INTEGER, TEXT) RETURNS INTEGER AS '
  DECLARE
    save_id      ALIAS FOR $1;
    revision_id_ ALIAS FOR $2;
    qtext_       ALIAS FOR $3;
    parent_      INTEGER;
    branch_id_   INTEGER;
    saveto       INTEGER;
    rec          RECORD;
  BEGIN
    -- RAISE NOTICE ''choice_question_save(%,%,%) called'', $1, $2, $3;

    SELECT INTO rec qtext, branch_id FROM choice_questions WHERE revision_id = revision_id_;
    IF FOUND THEN
      branch_id_ := rec.branch_id;  
      IF rec.qtext = qtext_ THEN
        RETURN branch_id_;
      END IF;
    ELSE
      parent_ := NULL; -- everything new goes on a base branch?
      INSERT INTO branches (branch, parent) VALUES (branch_nextval(parent_), parent_);
      branch_id_ := currval(''branch_ids'');
    END IF;

    saveto := revision_save_start(branch_id_, revision_id_, 6, save_id);
    UPDATE choice_questions SET qtext = qtext_ WHERE revision_id = saveto;
    PERFORM revision_save_end(branch_id_, saveto);
    -- RAISE NOTICE ''choice_question_save(%,%,%) returning %'', $1, $2, $3, branch_id_;
    RETURN branch_id_;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION choice_component_save(INTEGER, INTEGER, TEXT, BOOLEAN, TEXT[],
  INTEGER[], TEXT[], TEXT, INTEGER, INTEGER, BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN,
  INTEGER) RETURNS INTEGER AS ' 
  DECLARE
    save_id        ALIAS FOR $1;
    revision_id_   ALIAS FOR $2;
    ctext_         ALIAS FOR $3;
    is_html_       ALIAS FOR $4;
    questions      ALIAS FOR $5; 
    question_ids   ALIAS FOR $6; 
    choices_       ALIAS FOR $7; 
    other_choice_  ALIAS FOR $8; 
    first_number_  ALIAS FOR $9; 
    last_number_   ALIAS FOR $10;
    is_numeric_    ALIAS FOR $11;
    select_many_   ALIAS FOR $12;
    stacked_       ALIAS FOR $13;
    vertical_      ALIAS FOR $14;
    rows_          ALIAS FOR $15;
    i              INTEGER := 1;
    j              INTEGER;
    branch_id_     INTEGER;
    parent_        INTEGER;
    saveto         INTEGER;
    s_branch_ids   TEXT := '''';
    sep            TEXT := '''';
    rec            RECORD;
    rec1           RECORD;
    r              RECORD;
    changed        BOOLEAN := ''f'';
  BEGIN
    -- save choices
    LOOP
      EXIT WHEN questions[i] IS NULL;
      j := CASE WHEN question_ids[i] = 0 THEN NULL ELSE question_ids[i] END;
      branch_id_ := choice_question_save(save_id, j, questions[i]);
      s_branch_ids := s_branch_ids || sep || branch_id_::TEXT;
      i := i + 1;
      sep := '','';
    END LOOP;
    
    s_branch_ids = ''{'' || s_branch_ids || ''}'';
    SELECT INTO rec array_integer_cast(s_branch_ids) AS branch_ids;
    
    -- see if choice ordering changed
    i := 1;
    FOR rec1 IN SELECT item_id FROM list_items WHERE revision_id = save_id ORDER BY ordinal LOOP
      IF NOT rec.branch_ids[i] = rec1.item_id THEN changed := ''t''; EXIT; END IF;
      i := i + 1;
    END LOOP;
    
    SELECT INTO r ctext, is_html, branch_id, choices, other_choice, first_number, last_number, is_numeric, select_many, stacked, vertical, rows
    FROM choice_components WHERE revision_id = revision_id_;
    IF NOT FOUND THEN
      parent_ := NULL; -- everything new goes on a base branch?
      INSERT INTO branches (branch, parent) VALUES (branch_nextval(parent_), parent_);
      branch_id_ := currval(''branch_ids'');
      changed := ''t'';
    ELSE
      branch_id_ := r.branch_id;
      IF NOT boolean_cast(r.ctext = ctext_ AND r.is_html = is_html_ AND
        r.choices = choices_ AND r.other_choice = other_choice_ AND
        r.first_number = first_number_ AND r.last_number = last_number_ AND
        r.is_numeric = is_numeric_ AND r.select_many = select_many_ AND
        r.stacked = stacked_ AND r.vertical = vertical_ AND r.rows = rows_)
      THEN
        changed := ''t'';
      END IF;
    END IF;
    
    IF changed THEN
      saveto := revision_save_start(branch_id_, revision_id_, 2, save_id);
      UPDATE choice_components SET 
        ctext = ctext_, is_html = is_html_,
        choices = choices_, other_choice = other_choice_,
        first_number = first_number_, last_number = last_number_,
        is_numeric = is_numeric_, select_many = select_many_,
        stacked = stacked_, vertical = vertical_, rows = rows_
      WHERE revision_id = saveto;
      i := 1;
      LOOP
        EXIT WHEN rec.branch_ids[i] IS NULL;
        j := rec.branch_ids[i];
        -- RAISE NOTICE ''insert into list_items(%, %, %)'', saveto, i, j;
        INSERT INTO list_items (revision_id, ordinal, item_id) VALUES (saveto, i, rec.branch_ids[i]);
        i := i + 1;
      END LOOP;      
      PERFORM revision_save_end(branch_id_, saveto);
    END IF;
    RETURN branch_id_;
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
    END;
    
    IF t = 4 OR t = 5 THEN
      RETURN text_component_merge(common_id, primary_id, secondary_id, new_id);
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
    SELECT INTO orig_row      ctext, is_html FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, is_html FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, is_html FROM text_components WHERE revision_id = secondary_id;
    save choice_questions SET
      qtext = text_merge(orig.qtext, primary_row.qtext, secondary_row.qtext, ''t'')
    WHERE revision_id = new_id;
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
    SELECT INTO orig_row      ctext, is_html, choices, other_choice, first_number, last_number, is_numeric, select_many, stacked, vertical, rows FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, is_html, choices, other_choice, first_number, last_number, is_numeric, select_many, stacked, vertical, rows FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, is_html, choices, other_choice, first_number, last_number, is_numeric, select_many, stacked, vertical, rows FROM text_components WHERE revision_id = secondary_id;
    PERFORM text_component_merge(orig_id, primary_id, secondary_id, new_id);
    save choice_components SET
      ctext        =    text_merge(orig.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      is_html      =    text_merge(orig.is_html, primary_row.is_html, secondary_row.is_html, ''t''),
      choices      =   array_merge(orig.choices, primary_row.choices, secondary_row.choices, ''t''),
      other_choice =    text_merge(orig.other_choice, primary_row.other_choice, secondary_row.other_choice, ''t''),
      first_number = integer_merge(orig.first_number, primary_row.first_number, secondary_row.first_number, ''t''),
      last_number  = integer_merge(orig.last_number, primary_row.last_number, secondary_row.last_number, ''t''),
      is_numeric   = boolean_merge(orig.is_numeric, primary_row.is_numeric, secondary_row.is_numeric, ''t''),
      select_many  = boolean_merge(orig.select_many, primary_row.select_many, secondary_row.select_many, ''t''),
      stacked      = boolean_merge(orig.stacked, primary_row.stacked, secondary_row.stacked, ''t''),
      vertical     = boolean_merge(orig.vertical, primary_row.vertical, secondary_row.vertical, ''t''),
      rows         = integer_merge(orig.rows, primary_row.rows, secondary_row.rows, ''t'')
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
    SELECT INTO orig_row      ctext, is_html FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, is_html FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, is_html FROM text_components WHERE revision_id = secondary_id;
    
    save text_components SET
      ctext   = text_merge(orig.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      is_html = boolean_merge(orig.is_html, primary_row.is_html, secondary_row.is_html, ''t'')
    WHERE revision_id = new_id;
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
    SELECT INTO orig_row      ctext, is_html, rows, cols FROM text_components WHERE revision_id = orig_id;
    SELECT INTO primary_row   ctext, is_html, rows, cols FROM text_components WHERE revision_id = primary_id;
    SELECT INTO secondary_row ctext, is_html, rows, cols FROM text_components WHERE revision_id = secondary_id;
    
    save text_components SET
      ctext   = text_merge(orig.ctext, primary_row.ctext, secondary_row.ctext, ''t''),
      is_html = boolean_merge(orig.is_html, primary_row.is_html, secondary_row.is_html, ''t'')
      rows    = integer_merge(orig.rows, primary_row.rows, secondary_row.rows, ''t''),
      cols    = integer_merge(orig.cols, primary_row.cols, secondary_row.cols, ''t'')
    WHERE revision_id = new_id;
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
        save list_items SET ordinal = ordinal + 1 WHERE revision_id = new_id AND ordinal >= ord;
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
CREATE FUNCTION text_merge(TEXT, TEXT, TEXT, BOOLEAN) RETURNS TEXT AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
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
CREATE FUNCTION boolean_merge(BOOLEAN, BOOLEAN, BOOLEAN, BOOLEAN) RETURNS BOOLEAN AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
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
CREATE FUNCTION array_merge(INTEGER[], INTEGER[], INTEGER[], BOOLEAN) RETURNS INTEGER[] AS '
  DECLARE
    common_val    ALIAS FOR $1;
    primary_val   ALIAS FOR $2;
    secondary_val ALIAS FOR $3;
    favor_primary ALIAS FOR $4;
  BEGIN
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

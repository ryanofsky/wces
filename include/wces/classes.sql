CREATE TABLE classes
(
  class_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('class_ids'),

  -- Identifiers

  course_id INTEGER NOT NULL,
  section CHAR(3) NOT NULL,
  year SMALLINT NOT NULL,
  semester SMALLINT NOT NULL,

  -- Attributes

  name VARCHAR(124),
  time VARCHAR(60),
  location VARCHAR(60),
  students SMALLINT,
  callnumber INTEGER,

  -- Foreign Attributes

  department_id INTEGER,
  division_id INTEGER,
  school_id INTEGER,

  -- Keys

  UNIQUE (course_id, section, year, semester)
);

CREATE SEQUENCE class_ids INCREMENT 1 START 1;

COMMENT ON COLUMN classes.semester IS '0 - spring, 1 - summer, 2 - fall';

COMMENT ON COLUMN classes.division_id IS 'TODO: See if this attribute is shared among all classes in a course. If so, move it to the courses table.';

CREATE TABLE courses
(
  course_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('course_ids'),

  -- Identifiers

  subject_id INTEGER NOT NULL,
  code SMALLINT NOT NULL,
  divisioncode CHAR(1) NOT NULL,

  -- Attributes

  name VARCHAR(124),
  information TEXT,
  guess_department_id INTEGER,

  -- Keys

  UNIQUE (subject_id, code, divisioncode)
);

CREATE SEQUENCE course_ids INCREMENT 1 START 1;

CREATE TABLE subjects
(
  subject_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('subject_ids'),
  code CHAR(4) NOT NULL UNIQUE DEFAULT '',
  name VARCHAR(124)
);

CREATE SEQUENCE subject_ids INCREMENT 1 START 1;

CREATE TABLE departments
(
  department_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('department_ids'),
  code CHAR(4) NOT NULL UNIQUE DEFAULT '',
  name VARCHAR(124)
);

CREATE SEQUENCE department_ids INCREMENT 1 START 1;

CREATE TABLE divisions
(
  division_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('division_ids'),
  code VARCHAR(2) NOT NULL DEFAULT '',
  shortcode CHAR(1) NOT NULL DEFAULT '',
  name VARCHAR(124) NOT NULL,
  UNIQUE (name, code, shortcode)
);

CREATE SEQUENCE division_ids INCREMENT 1 START 1;

CREATE TABLE schools
(
  school_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('school_ids'),
  name VARCHAR(252) NOT NULL UNIQUE
);

CREATE SEQUENCE school_ids INCREMENT 1 START 1;

CREATE TABLE users
(
  user_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('user_ids'),
  uni VARCHAR(12) UNIQUE,
  lastname VARCHAR(28),
  firstname VARCHAR(28),
  email VARCHAR(60),
  department_id INTEGER,
  flags INTEGER NOT NULL,
  lastlogin TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE SEQUENCE user_ids INCREMENT 1 START 1;

COMMENT ON COLUMN users.flags IS '
Bitmask    | Nonzero when the user...
0x00000001 | is an administrator
0x00000002 | is an administrator within his/her department
0x00000004 | is a professor
0x00000008 | is a student
0x00000010 | is a ta
0x00000080 | opts out of mass emails
';

CREATE TABLE enrollments
(
  user_id INTEGER NOT NULL,
  class_id INTEGER NOT NULL,
  status INTEGER NOT NULL,
  lastseen TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY(user_id, class_id)
);

-- this table only exists as an optimization.
-- it is faster to select from enrollments_p than to
-- select from enrollments where status = 4. it is not
-- a problem as long as clients that need to write into
-- the enrollments table use the enrollment_update()
-- and enrollment_update_status() functions instead of
-- updating or inserting into the enrollments table
-- directly
CREATE TABLE enrollments_p () INHERITS (enrollments);
ALTER TABLE enrollments ADD CONSTRAINT enrollments_status CHECK (status IN (0,1,2,3,4));
ALTER TABLE enrollments_p ADD CONSTRAINT enrollments_p_only CHECK (status = 4);

COMMENT ON COLUMN enrollments.status IS '1 - student, 2 - ta, 3 - ta and student, 4 - professor, 0 - dropped class';

CREATE UNIQUE INDEX enrollment_p_idx ON enrollments_p (user_id, class_id);

CREATE TABLE professor_data
(
  user_id INTEGER NOT NULL PRIMARY KEY,
  url VARCHAR(252),
  picname VARCHAR(124),
  statement TEXT,
  profile TEXT,
  education TEXT,
  picture_id INTEGER
);

CREATE TABLE pictures (
  file_id INTEGER PRIMARY KEY NOT NULL DEFAULT nextval('picture_ids'),
  name text NOT NULL
);

CREATE SEQUENCE picture_ids INCREMENT 1 START 1;

CREATE TABLE professor_hooks
(
  professor_hook_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('professor_hook_ids'),
  user_id INTEGER NOT NULL,
  source SMALLINT NOT NULL,
  name VARCHAR(60),
  firstname VARCHAR(28),
  lastname VARCHAR(28),
  middle VARCHAR(1),
  uni VARCHAR(16),
  pid varchar(10) UNIQUE
);

CREATE SEQUENCE professor_hook_ids INCREMENT 1 START 1;
CREATE INDEX name_idx ON professor_hooks (name);
CREATE INDEX first_idx ON professor_hooks (firstname);
CREATE INDEX last_idx ON professor_hooks (lastname);

COMMENT ON COLUMN professor_hooks.source IS '
1 - Regripper Dump from RegRipper (first, last, MI, not separated)
2 - Registrar PID files from cunix /wwws/data/cu/bulletin/uwb-test/include/ (first, last, MI, separated)
3 - Imported from the original WCES''s professor table (first and last separated, no MI)
4 - Imported from the original WCES''s class table OR from the registrar spreadsheets (first and last separated, no MI)
';

CREATE TABLE acis_groups
(
  acis_group_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('acis_group_ids'),
  class_id INTEGER,
  code VARCHAR(60) UNIQUE NOT NULL,
  status INTEGER
);

CREATE SEQUENCE acis_group_ids INCREMENT 1 START 1;

CREATE TABLE acis_affiliations
(
  user_id INTEGER NOT NULL,
  acis_group_id INTEGER NOT NULL,
  PRIMARY KEY(user_id, acis_group_id)
);

CREATE TABLE question_periods
(
  question_period_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('question_period_ids'),
  displayname VARCHAR(60),
  begindate TIMESTAMP,
  enddate TIMESTAMP,
  semester INTEGER,
  year INTEGER,
  profdate TIMESTAMP,
  oracledate TIMESTAMP
);

CREATE SEQUENCE question_period_ids INCREMENT 1 START 1;

CREATE TABLE sent_mails
(
  sent_mail_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('sent_mail_ids'),
  sent TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
  user_id INTEGER NOT NULL,
  mail_from TEXT,
  reply_to TEXT,
  mail_to TEXT,
  subject TEXT,
  body TEXT,
  question_period_id INTEGER,
  category_id INTEGER
);

CREATE TABLE sent_mails_topics
(
  sent_mail_id INTEGER NOT NULL,
  topic_id INTEGER NOT NULL
);

CREATE SEQUENCE sent_mail_ids INCREMENT 1 START 1;

CREATE TABLE wces_topics
(
  topic_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('topic_ids'),
  class_id INTEGER,
  question_period_id INTEGER,
  item_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL,
  category_id INTEGER,
  make_public BOOLEAN NOT NULL,
  cancelled BOOLEAN NOT NULL
);

CREATE TABLE categories
(
  category_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('survey_category_ids'),
  name TEXT
);

CREATE SEQUENCE survey_category_ids INCREMENT 1 START 100;

CREATE TABLE ta_ratings
(
    name TEXT,
    overall INTEGER,
    knowledgeability INTEGER,
    approachability INTEGER,
    availability INTEGER,
    communication INTEGER,
    comments TEXT,
    user_id INTEGER
)
INHERITS (responses);

CREATE TABLE wces_base_topics
(
  topic_id INTEGER PRIMARY KEY not null default nextval('topic_ids'),
  name TEXT,
  ordinal INTEGER NOT NULL
);

CREATE TABLE wces_prof_topics
(
  topic_id INTEGER PRIMARY KEY not null default nextval('topic_ids'),
  class_topic_id INTEGER NOT NULL,
  user_id INTEGER NOT NULL,
  UNIQUE (class_topic_id, user_id)
);

CREATE TABLE wces_course_topics
(
  topic_id INTEGER PRIMARY KEY not null default nextval('topic_ids'),
  course_id INTEGER NOT NULL,
  item_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL,
  UNIQUE (course_id)
);

CREATE INDEX class_idx ON enrollments (class_id);
CREATE INDEX student_class_idx ON enrollments (class_id) WHERE status = 1;
CREATE INDEX user_idx ON enrollments (user_id);
CREATE INDEX ta_ratings_parent_idx ON ta_ratings (parent);
CREATE INDEX enrollment_p_class ON enrollments_p (class_id);
CREATE INDEX enrollment_p_user ON enrollments_p (user_id);

CREATE INDEX enrollment_prof_idx ON enrollments (user_id) WHERE status = 3;
ALTER TABLE classes ADD CONSTRAINT course_fk FOREIGN KEY (course_id) REFERENCES courses;
ALTER TABLE classes ADD CONSTRAINT department_fk FOREIGN KEY (department_id) REFERENCES departments;
ALTER TABLE classes ADD CONSTRAINT division_fk FOREIGN KEY (division_id) REFERENCES divisions;
ALTER TABLE classes ADD CONSTRAINT school_fk FOREIGN KEY (school_id) REFERENCES schools;
ALTER TABLE courses ADD CONSTRAINT department_fk FOREIGN KEY (guess_department_id) REFERENCES departments(department_id);
ALTER TABLE courses ADD CONSTRAINT subject_fk FOREIGN KEY (subject_id) REFERENCES subjects;
ALTER TABLE enrollments ADD CONSTRAINT class_fk FOREIGN KEY (class_id) REFERENCES classes;
ALTER TABLE enrollments ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE professor_data ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE professor_data ADD CONSTRAINT picture_fk FOREIGN KEY (picture_id) REFERENCES pictures(file_id);
ALTER TABLE professor_hooks ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE acis_groups ADD CONSTRAINT class_fk FOREIGN KEY (class_id) REFERENCES classes;
ALTER TABLE acis_affiliations ADD CONSTRAINT acis_group_fk FOREIGN KEY (acis_group_id) REFERENCES acis_groups;
ALTER TABLE acis_affiliations ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE sent_mails ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE sent_mails ADD CONSTRAINT category_fk FOREIGN KEY (category_id) REFERENCES categories;
ALTER TABLE sent_mails_topics ADD CONSTRAINT sent_mail_fk FOREIGN KEY (sent_mail_id) REFERENCES sent_mails(sent_mail_id);
ALTER TABLE users ADD CONSTRAINT departmentfk FOREIGN KEY (department_id) REFERENCES departments;
ALTER TABLE wces_topics ADD CONSTRAINT specialization_fk FOREIGN KEY (specialization_id) REFERENCES specializations(specialization_id);
ALTER TABLE ta_ratings ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE sent_mails ADD CONSTRAINT question_period_fk FOREIGN KEY (question_period_id) REFERENCES question_periods;
ALTER TABLE wces_prof_topics ADD CONSTRAINT topic_fk FOREIGN KEY (class_topic_id) REFERENCES wces_topics (topic_id);
ALTER TABLE users ADD constraint uni_rule CHECK (uni IS NULL OR char_length(uni) > 0);
ALTER TABLE professor_hooks ADD constraint uni_rule CHECK (uni IS NULL OR char_length(uni) > 0);

-- can't currently create these due to errors in data
ALTER TABLE wces_topics ADD CONSTRAINT class_fk FOREIGN KEY (class_id) REFERENCES classes;
ALTER TABLE wces_topics ADD CONSTRAINT category_fk FOREIGN KEY (category_id) REFERENCES categories;
ALTER TABLE saves ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;
ALTER TABLE responses_survey ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;


-- might someday create an items table...
--ALTER TABLE wces_topics ADD CONSTRAINT item_fk FOREIGN KEY (item_id) REFERENCES ???;

-- foreign keys from temporary tables, these should be moved into whatever files
-- the tables are declared in. also, some of these tables are probably obsolete
--ALTER TABLE temp_class ADD CONSTRAINT class_fk FOREIGN KEY (newid) REFERENCES classes(class_id);
--ALTER TABLE temp_course ADD CONSTRAINT course_fk FOREIGN KEY (newid) REFERENCES courses(course_id);
--ALTER TABLE temp_dept ADD CONSTRAINT department_fk FOREIGN KEY (newid) REFERENCES departments(department_id);
--ALTER TABLE temp_div ADD CONSTRAINT division_fk FOREIGN KEY (newid) REFERENCES divisions(division_id);
--ALTER TABLE temp_sch ADD CONSTRAINT school_fk FOREIGN KEY (newid) REFERENCES schools(school_id);
--ALTER TABLE temp_subj ADD CONSTRAINT subject_fk FOREIGN KEY (newid) REFERENCES subjects(subject_id);
--ALTER TABLE temp_topic ADD CONSTRAINT category_fk FOREIGN KEY (newid) REFERENCES categories(category_id);
--ALTER TABLE temp_prof ADD CONSTRAINT prof_fk FOREIGN KEY (newid) REFERENCES users(user_id);
--ALTER TABLE presps ADD CONSTRAINT course_fk FOREIGN KEY (course_id) REFERENCES courses;
--ALTER TABLE presps ADD CONSTRAINT user_fk FOREIGN KEY (user_id) REFERENCES users;

CREATE OR REPLACE FUNCTION references_class(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM wces_topics WHERE class_id = $1)                  THEN 1 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM enrollments WHERE class_id = $1 AND status <> 4)  THEN 2 ELSE 0 END |
  CASE WHEN EXISTS (SELECT * FROM acis_groups WHERE class_id = $1)                  THEN 4 ELSE 0 END;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION references_question_period(INTEGER) RETURNS INTEGER AS '
  SELECT
  CASE WHEN EXISTS (SELECT * FROM wces_topics WHERE question_period_id = $1)        THEN 1 ELSE 0 END
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION professor_find(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, INTEGER) RETURNS INTEGER AS '
  DECLARE
    name_       ALIAS FOR $1;
    firstname_  ALIAS FOR $2;
    middle_     ALIAS FOR $3;
    lastname_   ALIAS FOR $4;
    pid_        ALIAS FOR $5;
    uni_        ALIAS FOR $6;
    source_     ALIAS FOR $7;
    i INTEGER := NULL;
    j INTEGER := NULL;
    rec RECORD;
  BEGIN
    IF source_ = 1 THEN
      SELECT INTO i user_id FROM professor_hooks WHERE source = source_ AND name = name_;
      IF FOUND THEN RETURN i; END IF;

      SELECT INTO i user_id FROM professor_hooks WHERE source = 2 AND name = name_;

      IF NOT FOUND THEN
        INSERT INTO users (firstname, lastname, flags, lastlogin) VALUES (firstname_, lastname_, 4, NULL);
        i := currval(''user_ids'');
      END IF;

      INSERT INTO professor_hooks(user_id, source, name) VALUES (i, source_, name_);
      RETURN i;
    ELSE IF source_ = 2 THEN
      SELECT INTO i user_id FROM professor_hooks WHERE source = 2
        AND firstname = firstname_ AND middle = middle_
        AND lastname = lastname_ AND pid = pid_;
      IF FOUND THEN RETURN i; END IF;

      FOR rec IN SELECT user_id FROM professor_hooks WHERE
        (source IN (3,4) AND firstname = firstname AND lastname_ = lastname_) OR
        (source = 1 AND name = name_)
        GROUP BY user_id
      LOOP
        IF i IS NULL THEN i := rec.user_id; ELSE PERFORM professor_merge(i,rec.user_id); END IF;
      END LOOP;

      IF i IS NULL THEN
        INSERT INTO users (firstname, lastname, flags, lastlogin) VALUES (firstname, lastname, 4, NULL);
        i := currval(''user_ids'');
      END IF;

      INSERT INTO professor_hooks (user_id, source, name, firstname, lastname, middle, pid)
      VALUES (i, source_, name_, firstname_, lastname_, middle_, pid_)

      RETURN i;
    ELSE IF source_ = 3 OR source_ = 4 THEN
      SELECT INTO i user_id FROM professor_hooks 
      WHERE source = source_ AND firstname = firstname_ 
        AND lastname = lastname_ AND eq(uni::text, uni_);

      IF FOUND THEN RETURN i; END IF;

      IF uni_ IS NOT NULL THEN
        SELECT INTO i user_id FROM professor_hooks WHERE uni = uni_;
        
        IF i IS NULL THEN
          SELECT INTO i user_id FROM users WHERE uni = uni_;
        END IF;
      END IF;

      IF i IS NULL THEN
        SELECT INTO i user_id FROM professor_hooks WHERE source IN (3,4) AND firstname = firstname_ AND lastname = lastname_;
      END IF;
      
      IF i IS NULL THEN
        FOR rec IN SELECT user_id FROM professor_hooks WHERE source = 2 AND firstname = firstname_ AND lastname = lastname_ GROUP BY user_id LOOP
          IF i IS NULL THEN i := rec.user_id; ELSE i := NULL; EXIT; END IF;
        END LOOP;
      END IF;

      IF i IS NULL THEN
        INSERT INTO users (uni, firstname, lastname, flags, lastlogin) VALUES (uni_, firstname_, lastname_, 4, NULL);
        i := currval(''user_ids'');
      END IF;

      INSERT INTO professor_hooks(user_id, source, firstname, lastname, uni) VALUES (i, source_, firstname_, lastname_, uni_);
      RETURN i;
    ELSE
      RAISE EXCEPTION ''profesor_replace(%,%,%,%,%) fails. unknown source'', $1, $2, $3, $4, $5;
    END IF; END IF; END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION course_find(TEXT, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    ccode ALIAS FOR $1;
    allow_insert ALIAS FOR $2;
    subj CHAR(4);
    dcode CHAR(1);
    cnum CHAR(4);
    num INTEGER;
    rec RECORD;
    subjectid INTEGER;
    courseid INTEGER;
  BEGIN
    IF ccode IS NULL OR char_length(ccode) <> 9 THEN RETURN 0; END IF;
    subj := upper(substring(ccode from 1 for 4));
    dcode := upper(substring(ccode from 5 for 1));
    cnum := substring(ccode from 6 for 4);
    IF char_length(btrim(cnum,''0123456789'')) <> 0 THEN RETURN 0; END IF;
    num := to_number(cnum,''9999'');

    SELECT INTO subjectid subject_id FROM subjects WHERE code = subj;
    IF NOT FOUND THEN
      IF NOT allow_insert THEN RETURN 0; END IF;
      INSERT INTO subjects (code) VALUES (subj);
      subjectid := currval(''subject_ids'');
    END IF;

    SELECT INTO courseid course_id FROM courses WHERE subject_id = subjectid AND code = num AND divisioncode = dcode;
    IF NOT FOUND THEN
      IF NOT allow_insert THEN RETURN 0; END IF; 
      INSERT INTO courses (subject_id, code, divisioncode) VALUES (subjectid,num,dcode);
      courseid := currval(''course_ids'');
    END IF;

    RETURN courseid;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION course_find(TEXT) RETURNS INTEGER AS '
  SELECT course_find($1, ''t'');
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION class_find(TEXT) RETURNS INTEGER AS '
  DECLARE
    ccode ALIAS FOR $1;
    sec CHAR(3);
    cyr CHAR(4);
    csem CHAR(1);
    yr SMALLINT;
    sem  SMALLINT;
    courseid INTEGER;
    classid INTEGER;
  BEGIN

    -- SUBJD1234_000_2001_3

    IF
      ccode IS NULL
      OR
      char_length(ccode) <> 20
      OR
      position(''_'' IN ccode) <> 10
      OR
      position(''_'' IN substring(ccode from 11)) <> 4
      OR
      position(''_'' IN substring(ccode from 15)) <> 5
    THEN RETURN 0; END IF;

    sec  := SUBSTRING(ccode FROM 11 FOR 3);
    cyr  := SUBSTRING(ccode FROM 15 FOR 4);
    csem := SUBSTRING(ccode FROM 20 FOR 1);

    IF char_length(btrim(cyr,''0123456789'')) <> 0 THEN RETURN 0; END IF;
    IF char_length(btrim(csem,''0123456789'')) <> 0 THEN RETURN 0; END IF;

    yr  := to_number(cyr,''9999'');
    sem := to_number(csem,''9'') - 1;

    IF sem NOT BETWEEN 0 AND 2 THEN RETURN 0; END IF;

    courseid = course_find(substring(ccode from 1 for 9));
    IF courseid = 0 THEN RETURN 0; END IF;

    SELECT INTO classid class_id FROM classes WHERE course_id = courseid AND section = sec AND year = yr AND semester = sem;
    IF NOT FOUND THEN
      INSERT INTO classes (course_id, section, year, semester) VALUES (courseid, sec, yr, sem);
      classid := currval(''class_ids'');
    END IF;

    RETURN classid;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION strpos(text,text,integer) RETURNS integer AS '
  DECLARE
    haystack ALIAS FOR $1;
    needle ALIAS FOR $2;
    offset ALIAS FOR $3;
    i INTEGER;
  BEGIN
    IF offset > char_length(haystack) THEN RETURN 0; END IF;
    i := position(needle in substring(haystack from offset));
    IF i = 0 THEN RETURN 0; ELSE RETURN offset+i-1; END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION login_parse(VARCHAR(12),VARCHAR(28), VARCHAR (28), VARCHAR(28),TEXT) RETURNS INTEGER AS '
  DECLARE
    uni_s ALIAS FOR $1;
    email_s ALIAS FOR $2;
    lastname_s ALIAS FOR $3;
    firstname_s ALIAS FOR $4;
    affiliations ALIAS FOR $5;
    affils TEXT;
    affil TEXT;
    flags_s INTEGER;
    userid INTEGER;
    classid INTEGER;
    class_status INTEGER;
    i INTEGER;
    acis_groupid INTEGER;
    rec RECORD;
    relevant INTEGER;
    last INTEGER;
    curtime TIMESTAMP WITH TIME ZONE;
  BEGIN
    curtime := CURRENT_TIMESTAMP;

    -- Update the users record

    SELECT INTO rec user_id,email,lastname,firstname,flags FROM users WHERE uni = uni_s;
    IF FOUND THEN
      flags_s := rec.flags;
      userid := rec.user_id;

      UPDATE users SET lastlogin = curtime WHERE user_id = userid;

      IF rec.email IS NULL OR char_length(rec.email) = 0 THEN
        UPDATE users SET email = email_s WHERE user_id = userid;
      END IF;

      IF (rec.lastname IS NULL OR char_length(rec.lastname) = 0) AND (rec.firstname IS NULL OR char_length(rec.firstname) = 0) THEN
        UPDATE users SET lastname = lastname_s, firstname = firstname_s WHERE user_id = userid;
      END IF;
    ELSE
      INSERT INTO users (uni,email,lastname,firstname,flags,lastlogin) VALUES (uni_s,email_s,lastname_s,firstname_s,0,curtime);
      userid := currval(''user_ids'');
      flags_s := 0;
    END IF;

    -- Update the AcIS affliations

    DELETE FROM acis_affiliations WHERE user_id = userid;

    affils := translate(affiliations, ''\\001\\002\\003\\004\\005\\006\\007\\010\\011\\012\\013\\014\\015\\016\\017\\020\\021\\022\\023\\024\\025\\026\\027\\030\\031\\032\\033\\034\\035\\036\\037'',''                               '') ;
    i := 0;
    LOOP
      i := position('' '' IN affils);
      IF i = 0 THEN
        affil := affils;
      ELSE
        affil := substring(affils FROM 0 FOR i);
      END IF;

      -- RAISE NOTICE ''(%)'',affil;

      IF char_length(affil) > 0 THEN

         -- Find the acis group id and class id

        classid := 0; class_status := 0;
        SELECT INTO rec class_id, acis_group_id, status FROM acis_groups WHERE code = affil;
        IF FOUND THEN
          acis_groupid := rec.acis_group_id;
          IF rec.class_id THEN classid := rec.class_id; END IF;
          IF rec.status THEN class_status := rec.status; END IF;
        ELSE
          IF SUBSTRING(affil FROM 1 FOR 9) = ''CUcourse_'' THEN
            classid := class_find(SUBSTRING(affil FROM 10));
            class_status := 1;
            -- RAISE NOTICE ''classid (%)'',classid;
          ELSE
            IF SUBSTRING(affil FROM 1 FOR 8) = ''CUinstr_'' THEN
              classid := class_find(SUBSTRING(affil FROM 9));
              class_status := 4;
              -- RAISE NOTICE ''classid (%)'',classid;
            END IF;
          END IF;
          IF classid = 0 THEN
            INSERT INTO acis_groups (code) VALUES (affil);
          ELSE
            INSERT INTO acis_groups (code,class_id,status) VALUES (affil,classid,class_status);
          END IF;
          acis_groupid := currval(''acis_group_ids'');
        END IF;

        -- Update the class enrollment and AcIS affiliation
        IF NOT EXISTS(SELECT * FROM acis_affiliations WHERE user_id = userid AND acis_group_id = acis_groupid) THEN
          INSERT INTO acis_affiliations(user_id, acis_group_id) VALUES (userid, acis_groupid);
        END IF;
        IF classid <> 0 THEN
          PERFORM enrollment_update(userid, classid, class_status, curtime);
        END IF;
      END IF;

      EXIT WHEN (i = 0);

      affils := substring(affils FROM i + 1);

    END LOOP;

    SELECT INTO rec MIN(cl.year * 3 + cl.semester), MAX(cl.year * 3 + cl.semester)
    FROM acis_affiliations AS aa
    INNER JOIN acis_groups AS ag ON aa.acis_group_id = ag.acis_group_id
    INNER JOIN classes AS cl ON ag.class_id = cl.class_id
    WHERE aa.user_id = userid;

    IF FOUND AND rec.max IS NOT NULL THEN
      i := rec.max - 2;
      -- RAISE NOTICE ''min %'',rec.min;
      -- RAISE NOTICE ''max %'',rec.max;
      -- RAISE NOTICE ''i %'',i;

      IF rec.min > i THEN
        relevant := rec.min;
      ELSE
        relevant := i;
      END IF;

      -- RAISE NOTICE ''relevant %'',relevant;

      i := EXTRACT(MONTH FROM curtime);
      IF i BETWEEN 1 AND 5 THEN
        i := 0;
      ELSE
        IF i BETWEEN 6 AND 8 THEN
          i := 1;
        ELSE
          i := 2;
        END IF;
      END IF;

      i := EXTRACT(YEAR FROM curtime) * 3 + i - 1;

      IF i > relevant THEN relevant := i; END IF;

      -- i := (relevant - 1) / 3;
      -- RAISE NOTICE ''relevant year %'',i;
      -- i := ((relevant - 1) % 3) + 1;
      -- RAISE NOTICE ''relevant smst %'',i;

      UPDATE enrollments SET status = 0       -- look for dropped classes
      WHERE user_id = userid AND
        class_id IN
        (
          SELECT e.class_id
          FROM enrollments AS e
          INNER JOIN classes AS cl ON cl.class_id = e.class_id
          WHERE
            e.user_id = userid
            AND
            e.status = 1
            AND
            (e.lastseen IS NULL OR e.lastseen < curtime)
            AND
            ((cl.year * 3 + cl.semester) >= relevant)
        );

    END IF;

    -- is student or professor?

    IF EXISTS (
      SELECT * FROM acis_affiliations AS aa
      INNER JOIN acis_groups AS ag ON aa.acis_group_id = ag.acis_group_id
      WHERE
        aa.user_id = userid
        AND
        (
          ag.code = ''CUinstructor'' OR
          ag.code = ''BCinstructor''
        )) OR EXISTS (
      SELECT * FROM enrollments_p WHERE user_id = userid)
    THEN
      flags_s := flags_s | 4;
    END IF;
    
    IF EXISTS (
      SELECT *
      FROM acis_affiliations AS aa
      INNER JOIN acis_groups AS ag ON aa.acis_group_id = ag.acis_group_id
      WHERE
        aa.user_id = userid
        AND
        (
          ag.code = ''CUstudent''   OR
          ag.code = ''BCstudent''   OR
          ag.code = ''BC2student''  OR
          ag.code = ''CPMCstudent'' OR
          ag.code = ''TCstudent''   OR
          ag.code = ''UTSstudent''
        )) OR EXISTS (
      SELECT * FROM enrollments WHERE user_id = userid AND status & 1 <> 0)
    THEN
      flags_s := flags_s | 8;
    END IF;
    
    IF EXISTS (SELECT * FROM enrollments WHERE user_id = userid AND status & 2 <> 0) THEN
      flags_s := flags_s | 16;
    END IF;

    UPDATE users SET flags = flags_s WHERE user_id = userid;
    RETURN userid;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION class_update(INTEGER,CHAR(3),SMALLINT,SMALLINT,VARCHAR(124),VARCHAR(60),VARCHAR(60),SMALLINT,INTEGER,INTEGER,INTEGER,INTEGER) RETURNS INTEGER AS '
  DECLARE
    courseid   ALIAS FOR $1;
    sec        ALIAS FOR $2;
    yr         ALIAS FOR $3;
    sem        ALIAS FOR $4;
    name_s     ALIAS FOR $5;
    time_s     ALIAS FOR $6;
    location_s ALIAS FOR $7;
    students_s ALIAS FOR $8;
    callnum    ALIAS FOR $9;
    departmentid ALIAS FOR $10;
    divisionid ALIAS FOR $11;
    schoolid ALIAS FOR $12;
    classid INTEGER;
  BEGIN
    SELECT INTO classid class_id FROM classes WHERE course_id = courseid AND section = sec AND year = yr AND semester = sem;
    IF NOT FOUND THEN
      INSERT INTO classes(course_id, section, year, semester, name, time, location, students, callnumber, department_id, division_id, school_id)
      VALUES(courseid, sec, yr, sem, name_s, time_s, location_s, students_s, callnum, departmentid, divisionid, schoolid);
      RETURN currval(''class_ids'');
    ELSE
      IF name_s IS NOT NULL AND char_length(name_s) > 0 THEN
        UPDATE classes SET name = name_s WHERE class_id = classid;
      END IF;
      IF time_s IS NOT NULL AND char_length(time_s) > 0 THEN
        UPDATE classes SET time = time_s WHERE class_id = classid;
      END IF;
      IF location_s IS NOT NULL AND char_length(location_s) > 0 THEN
        UPDATE classes SET location = location_s WHERE class_id = classid;
      END IF;
      IF students_s IS NOT NULL AND students_s > 0 THEN
        UPDATE classes SET students = students_s WHERE class_id = classid;
      END IF;
      IF callnum IS NOT NULL AND callnum > 0 THEN
        UPDATE classes SET callnumber = callnum WHERE class_id = classid;
      END IF;
      IF departmentid IS NOT NULL AND departmentid > 0 THEN
        UPDATE classes SET department_id = departmentid WHERE class_id = classid;
      END IF;
      IF divisionid IS NOT NULL AND divisionid > 0 THEN
        UPDATE classes SET division_id = divisionid WHERE class_id = classid;
      END IF;
      IF schoolid IS NOT NULL AND schoolid > 0 THEN
        UPDATE classes SET school_id = schoolid WHERE class_id = classid;
      END IF;
      RETURN classid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION course_update(INTEGER,SMALLINT,CHAR(1),VARCHAR(124),TEXT) RETURNS INTEGER AS '
  DECLARE
    subjectid ALIAS FOR $1;
    ccode     ALIAS FOR $2;
    dcode     ALIAS FOR $3;
    name_s    ALIAS FOR $4;
    inf       ALIAS FOR $5;
    courseid INTEGER;
  BEGIN
    SELECT INTO courseid course_id FROM courses WHERE subject_id = subjectid AND code = ccode AND divisioncode = dcode;
    IF NOT FOUND THEN
      INSERT INTO courses (subject_id, code, divisioncode, name, information)
      VALUES (subjectid, ccode, dcode, name_s, inf);
      RETURN currval(''course_ids'');
    ELSE
      IF name_s IS NOT NULL AND char_length(name_s) > 0 THEN
        UPDATE courses SET name = name_s WHERE course_id = courseid;
      END IF;
      IF inf IS NOT NULL AND char_length(inf) > 0 THEN
        UPDATE courses SET information = inf WHERE course_id = courseid;
      END IF;
      RETURN courseid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION subject_update(CHAR(4),VARCHAR(124)) RETURNS INTEGER AS '
  DECLARE
    scode ALIAS FOR $1;
    sname ALIAS FOR $2;
    subjectid INTEGER;
  BEGIN
    SELECT INTO subjectid subject_id FROM subjects WHERE code = scode;
    IF NOT FOUND THEN
      INSERT INTO subjects (code, name) VALUES (scode, sname);
      RETURN currval(''subject_ids'');
    ELSE
      IF sname IS NOT NULL AND char_length(sname) > 0 THEN
        UPDATE subjects SET name = sname WHERE subject_id = subjectid;
      END IF;
      RETURN subjectid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION department_update(CHAR(4),VARCHAR(124)) RETURNS INTEGER AS '
  DECLARE
    dcode ALIAS FOR $1;
    dname ALIAS FOR $2;
    departmentid INTEGER;
  BEGIN
    SELECT INTO departmentid department_id FROM departments WHERE code = dcode;
    IF NOT FOUND THEN
      INSERT INTO departments (code, name) VALUES (dcode, dname);
      RETURN currval(''department_ids'');
    ELSE
      IF dname IS NOT NULL AND char_length(dname) > 0 THEN
        UPDATE departments SET name = dname WHERE department_id = departmentid;
      END IF;
      RETURN departmentid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION division_update(VARCHAR(2),CHAR(1),VARCHAR(124)) RETURNS INTEGER AS '
  DECLARE
    dcode ALIAS FOR $1;
    dscode ALIAS FOR $2;
    dname ALIAS FOR $3;
    divisionid INTEGER;
  BEGIN
    SELECT INTO divisionid division_id FROM divisions WHERE code = dcode AND shortcode = dscode AND name = dname;
    IF NOT FOUND THEN
      INSERT INTO divisions (code, shortcode, name) VALUES (dcode, dscode, dname);
      RETURN currval(''division_ids'');
    ELSE
      RETURN divisionid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION school_update(VARCHAR(252)) RETURNS INTEGER AS '
  DECLARE
    sname ALIAS FOR $1;
    schoolid INTEGER;
  BEGIN
    SELECT INTO schoolid school_id FROM schools WHERE name = sname;
    IF NOT FOUND THEN
      INSERT INTO schools (name) VALUES (sname);
      RETURN currval(''school_ids'');
    ELSE
      RETURN schoolid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION enrollment_update_status(INTEGER, INTEGER, INTEGER, INTEGER, BOOLEAN) RETURNS INTEGER AS '
  DECLARE
    user_id_ ALIAS FOR $1;
    class_id_ ALIAS FOR $2;
    old_status ALIAS FOR $3;
    new_status ALIAS FOR $4;
    leave_professor ALIAS FOR $5;
    rec RECORD;
  BEGIN
    --RAISE NOTICE ''enrollment_update_status(%,%,%,%,%) called'', $1, $2, $3, $4, $5;
    IF new_status = 4 THEN
      IF old_status = 4 THEN
        RETURN 4;
      ELSE
        SELECT INTO rec lastseen FROM enrollments
          WHERE user_id = user_id_ AND class_id = class_id_;
        DELETE FROM enrollments WHERE user_id = user_id_ AND class_id = class_id_;        
        INSERT INTO enrollments_p (user_id, class_id, status, lastseen)
        VALUES (user_id_, class_id_, new_status, rec.lastseen);
        RETURN 4;
      END IF;
    ELSE
      IF old_status = 4 THEN
        IF leave_professor THEN
          RETURN old_status;
        ELSE
          SELECT INTO rec lastseen FROM enrollments_p
            WHERE user_id = user_id_ AND class_id = class_id_;
          DELETE FROM enrollments_p WHERE user_id = user_id_ AND class_id = class_id_;        
          INSERT INTO enrollments (user_id, class_id, status, lastseen)
          VALUES (user_id_, class_id_, new_status, rec.lastseen);
          RETURN new_status;
        END IF;
      ELSE
        IF new_status = 0 THEN
          UPDATE enrollments SET status = 0
          WHERE user_id = user_id_ AND class_id = class_id_;
          RETURN new_status;
        ELSE
          UPDATE enrollments SET status = status | new_status
          WHERE user_id = user_id_ AND class_id = class_id_;          
          RETURN old_status | new_status;
        END IF;
      END IF;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION enrollment_update(INTEGER,INTEGER,INTEGER,TIMESTAMP WITH TIME ZONE) RETURNS INTEGER AS '
  DECLARE
    userid ALIAS FOR $1;
    classid ALIAS FOR $2;
    status_ ALIAS FOR $3;
    tyme ALIAS FOR $4;
    rec RECORD;
  BEGIN
    SELECT INTO rec status, lastseen FROM enrollments WHERE user_id = userid AND class_id = classid;
    IF NOT FOUND THEN
      IF status_ = 4 THEN
        INSERT INTO enrollments_p (user_id, class_id, status, lastseen)
        VALUES (userid, classid, status_, tyme);        
      ELSE
        INSERT INTO enrollments (user_id, class_id, status, lastseen)
        VALUES (userid, classid, status_, tyme);
      END IF;
      RETURN status_;
    ELSE
      IF (tyme IS NULL AND rec.lastseen IS NOT NULL) OR tyme < rec.lastseen THEN
        RETURN rec.status;  
      END IF;
      IF tyme IS NOT NULL THEN
        UPDATE enrollments SET lastseen = tyme WHERE user_id = userid AND class_id = classid;
      END IF;
      RETURN enrollment_update_status(userid, classid, rec.status, status_, ''t'');
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION enrollment_drop_ta(INTEGER, INTEGER) RETURNS VOID AS '
  UPDATE enrollments SET status = status & ~2
  WHERE user_id = $1 AND class_id = $2;
  DELETE FROM enrollments
  WHERE user_id = $1 AND class_id = $2 AND status = 0 AND lastseen IS NULL;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION enrollment_add_ta(INTEGER, INTEGER) RETURNS VOID AS '
  DECLARE
    user_id_ ALIAS FOR $1;
    class_id_ ALIAS FOR $2;
    s INTEGER;
  BEGIN
    SELECT INTO s status FROM enrollments WHERE user_id = user_id_
      AND class_id = class_id_;
    IF NOT FOUND THEN
       INSERT INTO enrollments (user_id, class_id, status, lastseen)
       VALUES (user_id_, class_id_, 2, NULL);
    ELSIF s = 4 THEN
      RAISE EXCEPTION ''enrollment_add_ta(%, %) fails. User % is already a professor of class %'', $1, $2, user_id_, class_id_;
    ELSE
      UPDATE enrollments SET status = status | 2 
      WHERE user_id = user_id_ AND class_id = class_id_;
    END IF;
    RETURN;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION enrollment_count(INTEGER) RETURNS INTEGER AS '
  SELECT COUNT(DISTINCT user_id)::INTEGER FROM enrollments 
  WHERE class_id = $1 AND status & 1 <> 0;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION user_update(VARCHAR(12),VARCHAR(28),VARCHAR(28),VARCHAR(28),INTEGER,TIMESTAMP WITH TIME ZONE,INTEGER) RETURNS INTEGER AS '
  DECLARE
    uni_s   ALIAS FOR $1;
    last    ALIAS FOR $2;
    first   ALIAS FOR $3;
    email_s ALIAS FOR $4;
    flags_s ALIAS FOR $5;
    tyme    ALIAS FOR $6;
    departmentid ALIAS FOR $7;
    userid INTEGER;
  BEGIN
    SELECT INTO userid user_id FROM users WHERE uni = uni_s;
    IF NOT FOUND THEN
      INSERT INTO users(uni, lastname, firstname, email, department_id, flags, lastlogin)
      VALUES (uni_s, last, first, email_s, departmentid, flags_s, tyme);
      RETURN currval(''user_ids'');
    ELSE
      IF (last IS NOT NULL AND char_length(last) > 0) OR (first IS NOT NULL AND char_length(first) > 0) THEN
        UPDATE users SET lastname = last WHERE user_id = userid;
        UPDATE users SET firstname = first WHERE user_id = userid;
      END IF;
      IF email_s IS NOT NULL AND char_length(email_s) > 0 THEN
        UPDATE users SET email = email_s WHERE user_id = userid;
      END IF;
      IF flags_s IS NOT NULL THEN
        UPDATE users SET flags = flags | flags_s WHERE user_id = userid;
      END IF;
      IF tyme IS NOT NULL THEN
        UPDATE users SET lastlogin = tyme WHERE user_id = userid;
      END IF;
      IF departmentid <> 0 THEN
        UPDATE users SET department_id = departmentid WHERE user_id = userid;
      END IF;
      RETURN userid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION professor_data_update(INTEGER,VARCHAR(252),VARCHAR(124),TEXT,TEXT,TEXT) RETURNS INTEGER AS '
  DECLARE
    userid   ALIAS FOR $1;
    url_s     ALIAS FOR $2;
    picname_s ALIAS FOR $3;
    statement_s ALIAS FOR $4;
    profile_s ALIAS FOR $5;
    education_s ALIAS FOR $6;
    i INTEGER;
  BEGIN
    SELECT INTO i user_id FROM professor_data WHERE user_id = userid;
    IF NOT FOUND THEN
      INSERT INTO professor_data(user_id, url, picname, statement, profile, education)
      VALUES (userid, url_s, picname_s, statement_s, profile_s, education_s);
    ELSE
      IF url_s IS NOT NULL AND char_length(url_s) > 0 THEN
        UPDATE professor_data SET url = url_s WHERE user_id = userid;
      END IF;
      IF picname_s IS NOT NULL AND char_length(picname_s) > 0 THEN
        UPDATE professor_data SET picname = picname_s WHERE user_id = userid;
      END IF;
      IF statement_s IS NOT NULL AND char_length(statement_s) > 0 THEN
        UPDATE professor_data SET statement = statement_s WHERE user_id = userid;
      END IF;
      IF profile_s IS NOT NULL AND char_length(profile_s) > 0 THEN
        UPDATE professor_data SET profile = profile_s WHERE user_id = userid;
      END IF;
      IF education_s IS NOT NULL AND char_length(education_s) > 0 THEN
        UPDATE professor_data SET education = education_s WHERE user_id = userid;
      END IF;
    END IF;
    RETURN userid;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION professor_hooks_update(INTEGER,SMALLINT,TEXT,TEXT,TEXT,TEXT,TEXT) RETURNS INTEGER AS '
  DECLARE
    userid      ALIAS FOR $1;
    src         ALIAS FOR $2;
    name_s      ALIAS FOR $3;
    firstname_s ALIAS FOR $4;
    lastname_s  ALIAS FOR $5;
    middle_s    ALIAS FOR $6;
    pid_s       ALIAS FOR $7;
    i INTEGER;
  BEGIN
     IF src = 1 THEN
       SELECT INTO i professor_hook_id FROM professor_hooks WHERE user_id = userid AND source = 1 and name = name_s;
     ELSE IF src = 2 THEN
       SELECT INTO i professor_hook_id FROM professor_hooks WHERE user_id = userid AND source = 2 AND firstname = firstname_s AND lastname = lastname_s AND middle = middle_s AND pid = pid_s;
     ELSE IF src = 3 THEN
       SELECT INTO i professor_hook_id FROM professor_hooks WHERE user_id = userid AND source = 3 AND firstname = firstname_s AND lastname = lastname_s;
     ELSE IF src = 4 THEN
       SELECT INTO i professor_hook_id FROM professor_hooks WHERE user_id = userid AND source = 4 AND firstname = firstname_s AND lastname = lastname_s;
     ELSE
       RETURN 0;
     END IF; END IF; END IF; END IF;

     IF FOUND THEN RETURN i; END IF;

    INSERT INTO professor_hooks (user_id, source, name, firstname, lastname, middle, pid) VALUES (userid, src, name_s, firstname_s, lastname_s, middle_s, pid_s);
    RETURN currval(''professor_hook_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION text_join(TEXT, TEXT, TEXT) RETURNS TEXT AS '
  DECLARE
    first ALIAS FOR $1;
    second ALIAS FOR $2;
    separator ALIAS FOR $3;
  BEGIN
    IF first IS NULL OR char_length(first) = 0 THEN
      RETURN NULLIF(second,'''');
    END IF;

    IF second IS NULL OR char_length(second) = 0 THEN
      RETURN NULLIF(first,'''');
    END IF;

    IF separator IS NULL THEN
      RETURN first || second;
    ELSE
      RETURN first || separator || second;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION professor_hooked_uni(INTEGER, TEXT) RETURNS BOOLEAN AS '
  SELECT EXISTS (SELECT * FROM professor_hooks WHERE user_id = $1 AND uni = $2)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION professor_unused_uni(INTEGER) RETURNS BOOLEAN AS '
  SELECT EXISTS (SELECT * FROM users WHERE user_id = $1 AND lastlogin IS NULL)
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION professor_merge(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    primary_id ALIAS FOR $1;
    secondary_id ALIAS FOR $2;
    primary_row RECORD;
    secondary_row RECORD;
    pfound BOOLEAN;
    sfound BOOLEAN;
    pinfo RECORD;
    sinfo RECORD;
    t TEXT;
    ers RECORD;
  BEGIN
    RAISE NOTICE ''professor_merge(%,%) called'', $1, $2;

    IF primary_id = secondary_id THEN RETURN primary_id; END IF;

    SELECT INTO primary_row uni, lastname, firstname, email, department_id, flags, lastlogin FROM users WHERE user_id = primary_id;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''professor_merge(%,%) fails. invalid primary_id'', $1, $2;
    END IF;

    SELECT INTO secondary_row uni, lastname, firstname, email, department_id, flags, lastlogin FROM users WHERE user_id = secondary_id;
    IF NOT FOUND THEN
      RAISE EXCEPTION ''professor_merge(%,%) fails. invalid secondary_id'', $1, $2;
    END IF;

    IF primary_row.uni IS NOT NULL AND secondary_row.uni IS NOT NULL THEN
      pfound := professor_hooked_uni(primary_id, primary_row.uni);
      sfound := professor_hooked_uni(secondary_id, secondary_row.uni);
      IF pfound AND NOT sfound AND professor_unused_uni(primary_id) THEN
        primary_row.uni := NULL;
      ELSIF sfound AND NOT pfound AND professor_unused_uni(secondary_id)THEN
        secondary_row.uni := NULL;
      ELSIF sfound AND pfound THEN
        pfound := professor_unused_uni(primary_id);
        sfound := professor_unused_uni(secondary_id);
        IF pfound AND NOT sfound THEN
          primary_row.uni := NULL;
        ELSIF sfound AND NOT pfound THEN
          secondary_row.uni := NULL;
        END IF;
      END IF;
    END IF;

    IF primary_row.uni IS NOT NULL AND secondary_row.uni IS NOT NULL THEN
      RAISE EXCEPTION ''professor_merge(%,%) fails. cannot merge two professors with different cunix ids'', $1, $2;
    END IF;
    
    IF primary_row.uni IS NULL THEN
      primary_row.uni := secondary_row.uni;
      primary_row.lastlogin := secondary_row.lastlogin;
    END IF;

    IF primary_row.lastname IS NULL AND primary_row.firstname IS NULL THEN
      primary_row.lastname  := secondary_row.lastname;
      primary_row.firstname := secondary_row.firstname;
    END IF;

    IF primary_row.email IS NULL THEN
      primary_row.email := secondary_row.email;
    END IF;

    IF primary_row.department_id IS NULL THEN
      primary_row.department_id := secondary_row.department_id;
    END IF;

    SELECT INTO pinfo url, picname, statement, profile, education FROM professor_data
    WHERE user_id = primary_id;
    pfound := FOUND;

    SELECT INTO sinfo url, picname, statement, profile, education FROM professor_data
    WHERE user_id = secondary_id;
    sfound := FOUND;

    IF NOT (sfound AND pfound) THEN
      UPDATE professor_data SET user_id = primary_id WHERE user_id IN (primary_id, secondary_id);
    ELSE
      pinfo.url := text_join(pinfo.url, sinfo.url, ''	'');

      IF pinfo.picname IS NULL OR 0 = char_length(pinfo.picname) THEN
        pinfo.picname = sinfo.picname;
      END IF;

      pinfo.statement := text_join(pinfo.statement,sinfo.statement, ''<hr>'');
      pinfo.profile := text_join(pinfo.profile,sinfo.profile, ''<hr>'');
      pinfo.education := text_join(pinfo.education,sinfo.education, ''<hr>'');
      
      UPDATE professor_data SET
        url = pinfo.url,
        picname = pinfo.picname,
        statement = pinfo.statement,
        education = pinfo.education
      WHERE user_id = primary_id;
      
      DELETE FROM professor_data WHERE user_id = secondary_id;
    END IF;

    -- Combine enrollments
    
    FOR ers IN 
      SELECT e1.class_id, e1.lastseen AS primary_time, 
        e1.status AS primary_status, e2.lastseen AS secondary_time,
        e2.status AS secondary_status
      FROM enrollments AS e1
      INNER JOIN enrollments AS e2 USING (class_id)
      WHERE e1.user_id = primary_id AND e2.user_id = secondary_id
    LOOP
      IF (ers.primary_time IS NULL AND ers.secondary_time IS NOT NULL)
        OR (ers.primary_time < ers.secondary_time) 
      THEN
        PERFORM enrollment_update(primary_id, ers.class_id, ers.secondary_status, ers.secondary_time);
        DELETE FROM enrollments WHERE user_id = secondary_id AND class_id = ers.class_id;
      ELSE
        PERFORM enrollment_update(secondary_id, ers.class_id, ers.primary_status, ers.primary_time);
        DELETE FROM enrollments WHERE user_id = primary_id AND class_id = ers.class_id;
      END IF;
    END LOOP;

    UPDATE enrollments SET user_id = primary_id WHERE user_id = secondary_id;

    -- Transfer hooks and enrollments from secondary to primary
    UPDATE professor_hooks SET user_id = primary_id WHERE user_id = secondary_id;
    UPDATE temp_user SET newid = primary_id WHERE newid = secondary_id;
    UPDATE temp_prof SET newid = primary_id WHERE newid = secondary_id;
    UPDATE acis_affiliations SET user_id = primary_id WHERE user_id = secondary_id;
    -- Delete secondary user
    
    DELETE FROM users WHERE user_id = secondary_id;

    UPDATE users SET uni = primary_row.uni, email = primary_row.email,
      lastname = primary_row.lastname, firstname = primary_row.firstname,
      department_id = primary_row.department_id,
      lastlogin = primary_row.lastlogin,
      flags = flags | primary_row.flags
    WHERE user_id = primary_id;
    RETURN primary_id;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION MAX(INTEGER, INTEGER) RETURNS INTEGER AS '
  SELECT CASE WHEN $2 IS NULL OR $1 > $2 THEN $1 ELSE $2 END;
' LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION get_profs(INTEGER) RETURNS TEXT AS '
  DECLARE
    classid ALIAS FOR $1;
    list TEXT := '''';
    rec RECORD;
  BEGIN
    FOR rec IN SELECT
      u.user_id, u.firstname, u.lastname
      FROM enrollments_p AS e
      INNER JOIN users AS u USING (user_id)
      WHERE e.class_id = classid
    LOOP
      IF char_length(list) > 0 THEN
        list := list || ''\n'';
      END IF;
      list := list || rec.user_id || ''\\n'' || COALESCE(rec.lastname, '''') 
        || ''\\n'' || COALESCE(rec.firstname, '''');
    END LOOP;
    RETURN list;
  END;
' LANGUAGE 'plpgsql'
WITH (ISCACHABLE);

CREATE OR REPLACE FUNCTION get_class(INTEGER) RETURNS TEXT AS '
  SELECT COALESCE(s.code, '''') || ''\n'' 
    || COALESCE(c.divisioncode, '''')
    || LTRIM(to_char(COALESCE(c.code, 0)::integer, ''0000'')) || ''\n''
    || COALESCE(cl.section, '''')     || ''\n'' || COALESCE(cl.year::text, '''') || ''\n'' 
    || COALESCE(cl.semester::text, '''')    || ''\n'' || COALESCE(c.name, '''') || ''\n''
    || COALESCE(cl.name, '''') || ''\n'' || $1 || ''\n'' || c.course_id
  FROM classes AS cl
  INNER JOIN courses AS c USING (course_id)
  INNER JOIN subjects AS s USING (subject_id)
  WHERE cl.class_id = $1
' LANGUAGE 'sql' WITH (ISCACHABLE);

CREATE OR REPLACE FUNCTION get_course(INTEGER) RETURNS TEXT AS '
  SELECT COALESCE(s.code, '''') || ''\n'' 
    || COALESCE(c.divisioncode, '''')
    || LTRIM(to_char(COALESCE(c.code, 0)::integer, ''0000''))  || ''\n''
    || COALESCE(c.name, '''') || ''\n''
    ||  $1
  FROM courses AS c
  INNER JOIN subjects AS s USING (subject_id)
  WHERE c.course_id = $1
' LANGUAGE 'sql' WITH (ISCACHABLE);

CREATE OR REPLACE FUNCTION get_question_period() RETURNS INTEGER AS '
  DECLARE
    curtime TIMESTAMP;
    i INTEGER;
  BEGIN
    curtime := NOW();
    SELECT INTO i question_period_id
    FROM question_periods
    WHERE begindate < curtime
    ORDER BY begindate DESC;
    RETURN i;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION get_next_question_period() RETURNS INTEGER AS '
  DECLARE
    curtime TIMESTAMP;
    i INTEGER;
  BEGIN
    curtime := NOW();
    SELECT INTO i question_period_id
    FROM question_periods
    WHERE curtime < enddate
    ORDER BY enddate;
    RETURN i;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION get_anext_question_period () RETURNS integer AS '
  DECLARE
    curtime TIMESTAMP;
    i INTEGER;
  BEGIN
    curtime := NOW();
    SELECT INTO i question_period_id
    FROM question_periods
    WHERE curtime < enddate
    ORDER BY enddate;
   
    IF NOT FOUND THEN
      SELECT INTO i question_period_id
      FROM question_periods
      ORDER BY enddate DESC;
    END IF;
    RETURN i;
  END;
' LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION prof_topic_make(INTEGER, INTEGER) RETURNS INTEGER AS '
  DECLARE
    class_topic_id_ ALIAS FOR $1;
    user_id_ ALIAS FOR $2;
    topic_id_ INTEGER;
  BEGIN
    SELECT INTO topic_id_ topic_id
    FROM wces_prof_topics
    WHERE class_topic_id = class_topic_id_ AND user_id = user_id_;

    IF FOUND THEN RETURN topic_id_; END IF;

    INSERT INTO wces_prof_topics (class_topic_id, user_id)
    VALUES (class_topic_id_, user_id_);

    RETURN currval(''topic_ids'');
  END;
' LANGUAGE 'plpgsql';

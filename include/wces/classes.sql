DROP TABLE classes;
DROP SEQUENCE class_ids;
DROP TABLE courses ;
DROP SEQUENCE course_ids;
DROP TABLE subjects;
DROP SEQUENCE subject_ids;
DROP TABLE departments;
DROP SEQUENCE department_ids ;
DROP TABLE divisions;
DROP SEQUENCE division_ids;
DROP TABLE schools;
DROP SEQUENCE school_ids;
DROP TABLE enrollments;
DROP TABLE professor_data;
DROP INDEX name_idx;
DROP INDEX first_idx;
DROP INDEX last_idx;
DROP TABLE professor_hooks;
DROP SEQUENCE professor_hook_ids;
DROP TABLE users;
DROP SEQUENCE user_ids;
DROP TABLE acis_groups;
DROP SEQUENCE acis_group_ids;
DROP TABLE acis_affiliations;
DROP FUNCTION prof_wwwreplace(VARCHAR(60));
DROP FUNCTION prof_pidreplace(VARCHAR(28), VARCHAR(28), char(1), varchar(10));
DROP FUNCTION prof_regreplace(VARCHAR(28), VARCHAR(28));
DROP FUNCTION course_find(TEXT);
DROP FUNCTION class_find(TEXT);
DROP FUNCTION strpos(text,text,integer);
DROP FUNCTION login_parse(VARCHAR(12),VARCHAR(28), VARCHAR (28), VARCHAR(28),TEXT);
DROP FUNCTION class_update(INTEGER,CHAR(3),SMALLINT,SMALLINT,VARCHAR(124),VARCHAR(60),VARCHAR(60),SMALLINT,INTEGER,INTEGER,INTEGER,INTEGER);
DROP FUNCTION course_update(INTEGER,SMALLINT,CHAR(1),VARCHAR(124),TEXT);
DROP FUNCTION subject_update(CHAR(4),VARCHAR(124));
DROP FUNCTION department_update(CHAR(4),VARCHAR(124));
DROP FUNCTION division_update(VARCHAR(2),CHAR(1),VARCHAR(124));
DROP FUNCTION school_update(VARCHAR(252));
DROP FUNCTION enrollment_update(INTEGER,INTEGER,INTEGER,TIMESTAMP);
DROP FUNCTION user_update(VARCHAR(12),VARCHAR(28),VARCHAR(28),VARCHAR(28),INTEGER,TIMESTAMP,INTEGER);
DROP FUNCTION professor_data_update(INTEGER,VARCHAR(252),VARCHAR(124),TEXT,TEXT,TEXT);

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
  email VARCHAR(28),
  department_id INTEGER,
  flags INTEGER NOT NULL,
  lastlogin TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE SEQUENCE user_ids INCREMENT 1 START 1;

COMMENT ON COLUMN users.flags IS '
Bitmask    | Nonzero when the user...

0x00000001 | is an administrator
0x00000002 | is an administrator within his/her department
0x00000004 | is a professor
0x00000008 | is a student';    
                                   
CREATE TABLE enrollments
(
  user_id INTEGER NOT NULL,
  class_id INTEGER NOT NULL,
  status INTEGER NOT NULL,
  lastseen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY(user_id, class_id)
);

COMMENT ON COLUMN enrollments.status IS '1 - student, 2 - ta, 3 - professor, 0 - dropped class';

CREATE TABLE professor_data
(
  user_id INTEGER NOT NULL PRIMARY KEY,
  url VARCHAR(252),
  picname VARCHAR(124),
  statement TEXT,
  profile TEXT,
  education TEXT
);

CREATE TABLE professor_hooks
(
  professor_hook_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('professor_hook_ids'),
  user_id INTEGER NOT NULL,
  source SMALLINT NOT NULL,
  name VARCHAR(60),
  firstname VARCHAR(28),
  lastname VARCHAR(28),
  middle char(1),
  pid varchar(10) UNIQUE
);

CREATE SEQUENCE professor_hook_ids INCREMENT 1 START 1;
CREATE INDEX name_idx ON professor_hooks (name);
CREATE INDEX first_idx ON professor_hooks (firstname);
CREATE INDEX last_idx ON professor_hooks (lastname);

COMMENT ON COLUMN professor_hooks.source IS '
1 - Regripper Dump ( http://www.columbia.edu/cu/bulletin/uwb/ )
2 - Registrar PID files ( cunix.columbia.edu/wwws/data/cu/bulletin/uwb-test/include/ )
3 - Imported from old oracle database
4 - Imported from registrar text files';

CREATE TABLE acis_groups
(
  acis_group_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('acis_group_ids'),
  class_id INTEGER,
  code VARCHAR(60) UNIQUE NOT NULL
);

CREATE SEQUENCE acis_group_ids INCREMENT 1 START 1;

CREATE TABLE acis_affiliations
(
  user_id INTEGER NOT NULL,
  acis_group_id INTEGER NOT NULL,
  PRIMARY KEY(user_id, acis_group_id)
);

ALTER TABLE classes ADD FOREIGN KEY (course_id) REFERENCES courses(course_id);
ALTER TABLE classes ADD FOREIGN KEY (department_id) REFERENCES departments(department_id);
ALTER TABLE classes ADD FOREIGN KEY (division_id) REFERENCES divisions(division_id);
ALTER TABLE classes ADD FOREIGN KEY (school_id) REFERENCES schools(school_id);
ALTER TABLE courses ADD FOREIGN KEY (subject_id) REFERENCES subjects(subject_id);
ALTER TABLE users ADD FOREIGN KEY (department_id) REFERENCES departments(department_id);
ALTER TABLE professor_data ADD FOREIGN KEY (user_id) REFERENCES users(user_id);
ALTER TABLE professor_hooks ADD FOREIGN KEY (user_id) REFERENCES users(user_id);
ALTER TABLE enrollments ADD FOREIGN KEY (user_id) REFERENCES users(user_id);
ALTER TABLE enrollments ADD FOREIGN KEY (class_id) REFERENCES classes(class_id);
ALTER TABLE acis_groups ADD FOREIGN KEY (class_id) REFERENCES classes(class_id);
ALTER TABLE acis_affiliations ADD FOREIGN KEY (user_id) REFERENCES users(user_id);
ALTER TABLE acis_affiliations ADD FOREIGN KEY (acis_group_id) REFERENCES acis_groups(acis_group_id);

CREATE FUNCTION prof_wwwreplace(VARCHAR(60)) RETURNS INTEGER AS '
  DECLARE
    name_s ALIAS FOR $1;
    profid INTEGER;
    i INTEGER;
    j INTEGER;
    last VARCHAR (28);
    first VARCHAR (28);
    rec RECORD;
  BEGIN
    IF name_s IS NULL OR char_length(name_s) = 0 OR lower(name_s) = ''faculty'' OR lower(name_s) = ''. faculty'' THEN
      RETURN 0;
    END IF;  
    
    SELECT INTO rec user_id FROM professor_hooks WHERE source = 1 and name = name_s;
    IF FOUND THEN RETURN rec.user_id; END IF;
    
    profid := 0;
    
    SELECT INTO rec user_id FROM professor_hooks WHERE source = 2 and name = name_s;
    IF FOUND THEN profid := rec.user_id; END IF;
    
    IF profid = 0 THEN
      SELECT INTO rec user_id FROM professor_hooks WHERE source IN (3,4) and name = name_s;
      IF FOUND THEN profid := rec.user_id; END IF;
    END IF:
      
    IF profid = 0 THEN
      i := 0; j := 0;
      LOOP
        i := position('' '' IN substring(name_s FROM j+1));
        j := j + i;
        EXIT WHEN (i = 0);
        -- RAISE NOTICE ''i = (%)'',i;
      END LOOP;
      last := substring(name_s FROM j+1);
      IF j > 0 THEN first := substring(name_s FROM 0 FOR j); ELSE first := ''''; END IF;
      INSERT INTO users(lastname, firstname, flags, lastlogin) VALUES (last, first, 4, NULL);
      profid := currval(''user_ids'');
    END IF;
    
    INSERT INTO professor_hooks (user_id,source,name) VALUES (profid,1,name_s);
    
    RETURN profid;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION prof_pidreplace(VARCHAR(28), VARCHAR(28), char(1), varchar(10)) RETURNS INTEGER AS '
  DECLARE
    firstname_s ALIAS FOR $1;
    lastname_s  ALIAS FOR $2;
    middle_s    ALIAS FOR $3;
    pid_s       ALIAS FOR $4;
    name_s VARCHAR(60);
    i INTEGER;
    j INTEGER;
    profid INTEGER;
    rec RECORD;
  BEGIN
    IF lower(lastname_s) = ''faculty'' OR pid_s = ''C000000001'' THEN
      RETURN 0;
    END IF;     
    
    SELECT INTO rec user_id FROM professor_hooks WHERE source = 2 AND firstname = firstname_s AND lastname = lastname_s AND middle = middle_s AND pid = pid_s;
    IF FOUND THEN RETURN rec.user_id; END IF;
    
    name_s := trim(firstname_s::text || '' '' || trim(middle_s::text || '' '' || lastname_s::text));
    profid := 0;
    
    SELECT INTO rec user_id FROM professor_hooks WHERE source = 1 AND name = name_s;
    IF FOUND THEN profid := rec.user_id; END IF;
    
    IF profid = 0 THEN
      SELECT INTO rec user_id FROM professor_hooks WHERE source IN (3,4) AND firstname = firstname_s AND lastname = lastname_s;
      IF FOUND THEN profid := rec.user_id; END IF;
    END IF;
    
    IF profid = 0 THEN
      INSERT INTO users(lastname, firstname, flags, lastlogin) VALUES (lastname_s, firstname_s, 4, NULL);
      profid := currval(''user_ids'');
    END IF;
    
    INSERT INTO professor_hooks (user_id,source,name,lastname,firstname,middle,pid) VALUES (profid,2,name_s,lastname_s,firstname_s,middle_s,pid_s);
    
    RETURN profid;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION prof_regreplace(VARCHAR(28), VARCHAR(28)) RETURNS INTEGER AS '
  DECLARE
    firstname_s ALIAS FOR $1;
    lastname_s  ALIAS FOR $2;
    name_s VARCHAR(60);
    i INTEGER;
    j INTEGER;
    profid INTEGER;
    rec RECORD;
  BEGIN
    IF lower(lastname_s) = ''faculty'' THEN
      RETURN 0;
    END IF;     
    
    SELECT INTO rec user_id FROM professor_hooks WHERE source = 4 AND firstname = firstname_s AND lastname = lastname_s;
    IF FOUND THEN RETURN rec.user_id; END IF;

    profid := 0;    

    SELECT INTO rec user_id FROM professor_hooks WHERE source IN (2,3) AND firstname = firstname_s AND lastname = lastname_s;
    IF FOUND THEN profid := rec.user_id; END IF;

    name_s := trim(firstname_s::text || '' '' || lastname_s::text);
    
    IF profid = 0 THEN
      SELECT INTO rec user_id FROM professor_hooks WHERE source = 1 AND name = name_s;
      IF FOUND THEN profid := rec.user_id; END IF;
    END IF;
    
    IF profid = 0 THEN
      INSERT INTO users(lastname, firstname, flags, lastlogin) VALUES (lastname_s, firstname_s, 4, NULL);
      profid := currval(''user_ids'');
    END IF;
    
    INSERT INTO professor_hooks (user_id,source,name,lastname,firstname) VALUES (profid,4,name_s,lastname_s,firstname_s);
    
    RETURN profid;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION course_find(TEXT) RETURNS INTEGER AS '
  DECLARE
    ccode ALIAS FOR $1;
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
      INSERT INTO subjects (code) VALUES (subj);
      subjectid := currval(''subject_ids'');
    END IF;
    
    SELECT INTO courseid course_id FROM courses WHERE subject_id = subjectid AND code = num AND divisioncode = dcode;
    IF NOT FOUND THEN
      INSERT INTO courses (subject_id, code, divisioncode) VALUES (subjectid,num,dcode);
      courseid := currval(''course_ids'');
    END IF;
    
    RETURN courseid;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION class_find(TEXT) RETURNS INTEGER AS '
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

CREATE FUNCTION strpos(text,text,integer) RETURNS integer AS '
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

CREATE FUNCTION login_parse(VARCHAR(12),VARCHAR(28), VARCHAR (28), VARCHAR(28),TEXT) RETURNS INTEGER AS '
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
    i INTEGER;
    acis_groupid INTEGER;
    rec RECORD;
    relevant INTEGER;
    last INTEGER;
    curtime timestamp;
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
      
      NOTICE ''(%)'',affil;
      
      IF char_length(affil) > 0 THEN
         
         -- Find the acis group id and class id
         
        classid := 0;
        SELECT INTO rec class_id, acis_group_id FROM acis_groups WHERE code = affil;
        IF FOUND THEN
          acis_groupid := rec.acis_group_id;
          IF rec.class_id THEN classid := rec.class_id; END IF;
        ELSE
          IF SUBSTRING(affil FROM 1 FOR 9) = ''CUcourse_'' THEN
            classid := class_find(SUBSTRING(affil FROM 10));
            RAISE NOTICE ''classid (%)'',classid;
          END IF;
          IF classid = 0 THEN
            INSERT INTO acis_groups (code) VALUES (affil);
          ELSE
            INSERT INTO acis_groups (code, class_id) VALUES (affil,classid);
          END IF;
          acis_groupid := currval(''acis_group_ids'');
        END IF;
      
        -- Update the class enrollment and AcIS affiliation
      
        INSERT INTO acis_affiliations(user_id, acis_group_id) VALUES (userid, acis_groupid);
        IF classid <> 0 THEN
          SELECT INTO rec class_id, status FROM enrollments WHERE user_id = userid AND class_id = classid;
          IF NOT FOUND THEN
            INSERT INTO enrollments(user_id,class_id,status,lastseen) VALUES (userid, classid, 1, curtime);
          ELSE
            UPDATE enrollments SET lastseen = curtime WHERE user_id = userid AND class_id = classid;
            IF rec.status < 1 THEN
              UPDATE enrollments SET status = 1 WHERE user_id = userid AND class_id = classid;  
            END IF;  
          END IF;
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
      RAISE NOTICE ''min %'',rec.min;
      RAISE NOTICE ''max %'',rec.max;
      RAISE NOTICE ''i %'',i;
      
      IF rec.min > i THEN
        relevant := rec.min;
      ELSE
        relevant := i;
      END IF;
      
      RAISE NOTICE ''relevant %'',relevant;
      
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
      
      i := (relevant - 1) / 3;
      RAISE NOTICE ''relevant year %'',i;
      i := ((relevant - 1) % 3) + 1;
      RAISE NOTICE ''relevant smst %'',i;
      
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
            e.lastseen < curtime
            AND
            ((cl.year * 3 + cl.semester) >= relevant)
        );
    
    END IF;
    
    -- is student or professor?
    
    SELECT INTO rec aa.user_id
    FROM acis_affiliations AS aa
    INNER JOIN acis_groups AS ag ON aa.acis_group_id = ag.acis_group_id
    WHERE 
      aa.user_id = userid
      AND
      (
        ag.code = ''CUinstructor'' OR
        ag.code = ''BCinstructor'' OR
        ag.code = ''CUNIX_instructor'' OR
        ag.code = ''CUNIX_Instructor''
      );
    
    IF FOUND THEN
      flags_s := flags_s | 4;
    ELSE  
      SELECT INTO rec aa.user_id
      FROM acis_affiliations AS aa
      INNER JOIN acis_groups AS ag ON aa.acis_group_id = ag.acis_group_id
      WHERE 
        aa.user_id = userid
        AND
        (
          ag.code = ''CUstudent''   OR
          ag.code = ''BCstudent''   OR
          ag.code = ''CPMCstudent'' OR
          ag.code = ''TCstudent''   OR
          ag.code = ''UTSstudent''
        );
      IF FOUND THEN
        flags_s := flags_s | 8;
      ELSE
        SELECT INTO rec MAX(status) FROM enrollments WHERE user_id = userid;
        IF FOUND AND rec.max IS NOT NULL THEN
          IF rec.max = 3 THEN
            flags_s := flags_s | 4;
          ELSE
            IF rec.max IN (1,2) THEN
              flags_s := flags_s | 8;
            END IF;  
          END IF;    
        END IF;
      END IF;
    END IF;
    
    UPDATE users SET flags = flags_s WHERE user_id = userid;
    RETURN userid;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION class_update(INTEGER,CHAR(3),SMALLINT,SMALLINT,VARCHAR(124),VARCHAR(60),VARCHAR(60),SMALLINT,INTEGER,INTEGER,INTEGER,INTEGER) RETURNS INTEGER AS '
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

CREATE FUNCTION course_update(INTEGER,SMALLINT,CHAR(1),VARCHAR(124),TEXT) RETURNS INTEGER AS '
  DECLARE
    subjectid ALIAS FOR $1;
    ccode     ALIAS FOR $2;
    dcode     ALIAS FOR $3;
    name_s    ALIAS FOR $4;
    info      ALIAS FOR $5;
    courseid INTEGER;
  BEGIN
    SELECT INTO courseid course_id FROM courses WHERE subject_id = subjectid AND code = ccode AND divisioncode = dcode;
    IF NOT FOUND THEN
      INSERT INTO courses (subject_id, code, divisioncode, name, information)
      VALUES (subjectid, ccode, dcode, name_s, info);
      RETURN currval(''course_ids'');
    ELSE
      IF name_s IS NOT NULL AND char_length(name_s) > 0 THEN
        UPDATE courses SET name = name_s WHERE course_id = courseid;      
      END IF;
      IF info IS NOT NULL AND char_length(info) > 0 THEN
        UPDATE courses SET information = info WHERE course_id = courseid;      
      END IF;
      RETURN courseid;
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION subject_update(CHAR(4),VARCHAR(124)) RETURNS INTEGER AS '
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

CREATE FUNCTION department_update(CHAR(4),VARCHAR(124)) RETURNS INTEGER AS '
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

CREATE FUNCTION division_update(VARCHAR(2),CHAR(1),VARCHAR(124)) RETURNS INTEGER AS '
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

CREATE FUNCTION school_update(VARCHAR(252)) RETURNS INTEGER AS '
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

CREATE FUNCTION enrollment_update(INTEGER,INTEGER,INTEGER,TIMESTAMP) RETURNS INTEGER AS '
  DECLARE
    userid ALIAS FOR $1;
    classid ALIAS FOR $2;
    status_s ALIAS FOR $3;
    tyme ALIAS FOR $4;
    i INTEGER;
  BEGIN
    SELECT INTO i status FROM enrollments WHERE user_id = userid AND class_id = classid;
    IF NOT FOUND THEN
      INSERT INTO enrollments (user_id, class_id, status, lastseen) VALUES (userid, classid, status_s, tyme);
      RETURN status_s;
    ELSE
      IF tyme IS NOT NULL THEN
        UPDATE enrollments SET lastseen = tyme WHERE user_id = userid AND class_id = classid;
      END IF;  
      IF status_s > i THEN
        UPDATE enrollments SET status = status_s WHERE user_id = userid AND class_id = classid;
        RETURN status_s;
      ELSE
        RETURN i;
      END IF;  
    END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION user_update(VARCHAR(12),VARCHAR(28),VARCHAR(28),VARCHAR(28),INTEGER,TIMESTAMP,INTEGER) RETURNS INTEGER AS '
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

CREATE FUNCTION professor_data_update(INTEGER,VARCHAR(252),VARCHAR(124),TEXT,TEXT,TEXT) RETURNS INTEGER AS '
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

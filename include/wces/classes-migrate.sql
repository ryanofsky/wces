DROP TABLE temp_subj;
DROP FUNCTION temp_subji(integer,integer);
DROP FUNCTION temp_subjr(integer);
DROP TABLE temp_dept;
DROP FUNCTION temp_depti(integer,integer);
DROP FUNCTION temp_deptr(integer);
DROP TABLE temp_div;
DROP FUNCTION temp_divi(integer,integer);
DROP FUNCTION temp_divr(integer);
DROP TABLE temp_class;
DROP FUNCTION temp_classi(integer,integer);
DROP FUNCTION temp_classr(integer);
DROP TABLE temp_course;
DROP FUNCTION temp_coursei(integer,char(1),integer);
DROP FUNCTION temp_courser(integer,char(1));
DROP TABLE temp_prof;
DROP FUNCTION temp_profi(integer,integer);
DROP FUNCTION temp_profr(integer);
DROP TABLE temp_sch;
DROP FUNCTION temp_schi(integer,integer);
DROP FUNCTION temp_schr(integer);
DROP TABLE temp_user;
DROP FUNCTION temp_useri(integer,integer);
DROP FUNCTION temp_userr(integer);

CREATE TABLE temp_subj
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES subjects(subject_id)
);

CREATE FUNCTION temp_subji(integer, integer) RETURNS integer AS '
  INSERT INTO temp_subj(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_subjr(integer) RETURNS integer AS '
  SELECT newid FROM temp_subj WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_dept
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES departments(department_id)
);

CREATE FUNCTION temp_depti(integer, integer) RETURNS integer AS '
  INSERT INTO temp_dept(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_deptr(integer) RETURNS integer AS '
  SELECT newid FROM temp_dept WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_div
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES divisions(division_id)
);

CREATE FUNCTION temp_divi(integer, integer) RETURNS integer AS '
  INSERT INTO temp_div(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_divr(integer) RETURNS integer AS '
  SELECT newid FROM temp_div WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_class
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES classes(class_id)
);

CREATE FUNCTION temp_classi(integer, integer) RETURNS integer AS '
  INSERT INTO temp_class(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_classr(integer) RETURNS integer AS '
  SELECT newid FROM temp_class WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_course
(
  oldid INTEGER NOT NULL,
  oldcode CHAR(1) NOT NULL,
  newid INTEGER UNIQUE NOT NULL REFERENCES courses(course_id),
  PRIMARY KEY(oldid,oldcode)
);

CREATE FUNCTION temp_coursei(integer, char(1), integer) RETURNS integer AS '
  INSERT INTO temp_course(oldid, oldcode, newid) VALUES ($1, $2, $3);
  SELECT $3;
' LANGUAGE 'sql';

CREATE FUNCTION temp_courser(integer, char(1)) RETURNS integer AS '
  SELECT newid FROM temp_course WHERE oldid = $1 AND oldcode = $2;
' LANGUAGE 'sql';

CREATE TABLE temp_prof
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES users(user_id)
);

CREATE FUNCTION temp_profi(integer, integer) RETURNS integer AS '
  INSERT INTO temp_prof(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_profr(integer) RETURNS integer AS '
  SELECT newid FROM temp_prof WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_sch
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES schools(school_id)
);

CREATE FUNCTION temp_schi(integer, integer) RETURNS integer AS '
  INSERT INTO temp_sch(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_schr(integer) RETURNS integer AS '
  SELECT newid FROM temp_sch WHERE oldid = $1;
' LANGUAGE 'sql';

CREATE TABLE temp_user
(
  oldid INTEGER NOT NULL PRIMARY KEY,
  newid INTEGER UNIQUE NOT NULL REFERENCES users(user_id)
);

CREATE FUNCTION temp_useri(integer, integer) RETURNS integer AS '
  INSERT INTO temp_user(oldid, newid) VALUES ($1, $2);
  SELECT $2;
' LANGUAGE 'sql';

CREATE FUNCTION temp_userr(integer) RETURNS integer AS '
  SELECT newid FROM temp_user WHERE oldid = $1;
' LANGUAGE 'sql';

<?

require_once("config/Columbia.inc");

$gateway_path = "/gateway";

$WCES_CONFIG_PAGE_INCLUDE = 'page.php';
$WCES_CONFIG_PAGE = 'GatewayPage';

$WCES_CONFIG_FACTORIES_INCLUDE = 'factories.php';

$WBES_LOGIN_HANDLER = "$gateway_path/login.php";
$WBES_LOGIN_LOGOUT = "$gateway_path/logout.php";
$WBES_LOGIN_BOUNCER = "https://oracle.seas.columbia.edu$gateway_path/bouncer.php";
$WBES_LOGIN_SECURE_HANDLER = "https://oracle.seas.columbia.edu$gateway_path/login.php";

$topics = array(3566 => "Pre-presentation survey" , 3567 => "Post-presentation survey");
$course_id = 1087;
$year = 2003;
$semester = 0;

/*

DROP TABLE gateway_topics;

CREATE TABLE gateway_departments
(
  gateway_department_id INTEGER NOT NULL PRIMARY KEY,
  department_name TEXT NOT NULL,
  department_people TEXT NOT NULL,
  ordinal INTEGER NOT NULL
);

INSERT INTO gateway_departments (gateway_department_id, department_name, department_people, ordinal)
SELECT DISTINCT ON (ordinal) ordinal, department_name, department_people, ordinal
FROM dp_topics
ORDER BY ordinal

CREATE TABLE gateway_topics
( topic_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('topic_ids'),
  course_id INTEGER NOT NULL,
  year SMALLINT NOT NULL,
  semester SMALLINT NOT NULL,
  item_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL,
  gateway_department_id INTEGER NOT NULL,
  begin_date TIMESTAMP NOT NULL,
  end_date TIMESTAMP NOT NULL,
  hidden BOOLEAN NOT NULL
);

INSERT INTO gateway_topics (topic_id, course_id, year, semester, item_id, specialization_id, gateway_department_id, begin_date, end_date, hidden)
SELECT m.topic_id, 1087, 2002, 2, m.item_id, t.specialization_id, t.ordinal, '2003-01-01', '2003-01-01', 'f'
FROM dp_topics AS t
LEFT JOIN misc_topics AS m ON m.question_period_id = 19 AND m.specialization_id = t.specialization_id;

DELETE FROM misc_topics WHERE topic_id IN (SELECT topic_id FROM gateway_topics);

DROP TABLE dp_topics;

--------------------------------
--------------------------
--------------------------------------
--------------------------------
-------------------
----------
-------------------------------------------------
-----------------------
-----------
-------------------------------
-----------


-- forgot the ordinal field

CREATE TEMPORARY TABLE temp_gateway_topics AS 
SELECT * FROM gateway_topics;

DROP TABLE gateway_topics;

CREATE TABLE gateway_topics
( topic_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('topic_ids'),
  course_id INTEGER NOT NULL,
  year SMALLINT NOT NULL,
  semester SMALLINT NOT NULL,
  item_id INTEGER NOT NULL,
  specialization_id INTEGER NOT NULL,
  gateway_department_id INTEGER NOT NULL,
  begin_date TIMESTAMP NOT NULL,
  end_date TIMESTAMP NOT NULL,
  hidden BOOLEAN NOT NULL
);

INSERT INTO gateway_topics (topic_id, course_id, year, semester, item_id, specialization_id, gateway_department_id, begin_date, end_date, hidden)
SELECT t.topic_id, t.course_id, t.year, t.semester, t.item_id, t.specialization_id, d.ordinal, t.begin_date, t.end_date, t.hidden
FROM temp_gateway_topics AS t
INNER JOIN dp_topics AS d USING (specialization_id)

DROP TABLE dp_topics;

--------------------------------
--------------------------
--------------------------------------
--------------------------------
-------------------
----------
-------------------------------------------------
-----------------------
-----------
-------------------------------
-----------

CREATE FUNCTION temp_make_spec(INTEGER) RETURNS INTEGER AS '
  INSERT INTO specializations (parent)
  VALUES ($1);
  SELECT currval(''specialization_ids'')::INTEGER;
' LANGUAGE 'sql';

INSERT INTO gateway_topics (course_id, year, semester, item_id, specialization_id, gateway_department_id, begin_date, end_date, hidden)
SELECT t.course_id, 2003, 0, t.item_id, temp_make_spec(s.parent), t.gateway_department_id, t.begin_date, t.end_date, t.hidden
FROM gateway_topics AS t
INNER JOIN specializations AS s USING (specialization_id)
WHERE t.year = 2002 AND t.semester =2 AND NOT t.hidden;

*/

CREATE TABLE answersets (
  answersetid int(11) NOT NULL auto_increment,
  questionsetid int(11) NOT NULL default '0',
  questionperiodid int(11) NOT NULL default '0',
  classid int(11) NOT NULL default '0',
  responses smallint(6) NOT NULL default '0',
  MC1a smallint(6) NOT NULL default '0',
  MC1b smallint(6) NOT NULL default '0',
  MC1c smallint(6) NOT NULL default '0',
  MC1d smallint(6) NOT NULL default '0',
  MC1e smallint(6) NOT NULL default '0',
  MC2a smallint(6) NOT NULL default '0',
  MC2b smallint(6) NOT NULL default '0',
  MC2c smallint(6) NOT NULL default '0',
  MC2d smallint(6) NOT NULL default '0',
  MC2e smallint(6) NOT NULL default '0',
  MC3a smallint(6) NOT NULL default '0',
  MC3b smallint(6) NOT NULL default '0',
  MC3c smallint(6) NOT NULL default '0',
  MC3d smallint(6) NOT NULL default '0',
  MC3e smallint(6) NOT NULL default '0',
  MC4a smallint(6) NOT NULL default '0',
  MC4b smallint(6) NOT NULL default '0',
  MC4c smallint(6) NOT NULL default '0',
  MC4d smallint(6) NOT NULL default '0',
  MC4e smallint(6) NOT NULL default '0',
  MC5a smallint(6) NOT NULL default '0',
  MC5b smallint(6) NOT NULL default '0',
  MC5c smallint(6) NOT NULL default '0',
  MC5d smallint(6) NOT NULL default '0',
  MC5e smallint(6) NOT NULL default '0',
  MC6a smallint(6) NOT NULL default '0',
  MC6b smallint(6) NOT NULL default '0',
  MC6c smallint(6) NOT NULL default '0',
  MC6d smallint(6) NOT NULL default '0',
  MC6e smallint(6) NOT NULL default '0',
  MC7a smallint(6) NOT NULL default '0',
  MC7b smallint(6) NOT NULL default '0',
  MC7c smallint(6) NOT NULL default '0',
  MC7d smallint(6) NOT NULL default '0',
  MC7e smallint(6) NOT NULL default '0',
  MC8a smallint(6) NOT NULL default '0',
  MC8b smallint(6) NOT NULL default '0',
  MC8c smallint(6) NOT NULL default '0',
  MC8d smallint(6) NOT NULL default '0',
  MC8e smallint(6) NOT NULL default '0',
  MC9a smallint(6) NOT NULL default '0',
  MC9b smallint(6) NOT NULL default '0',
  MC9c smallint(6) NOT NULL default '0',
  MC9d smallint(6) NOT NULL default '0',
  MC9e smallint(6) NOT NULL default '0',
  MC10a smallint(6) NOT NULL default '0',
  MC10b smallint(6) NOT NULL default '0',
  MC10c smallint(6) NOT NULL default '0',
  MC10d smallint(6) NOT NULL default '0',
  MC10e smallint(6) NOT NULL default '0',
  ABET1a smallint(6) NOT NULL default '0',
  ABET1b smallint(6) NOT NULL default '0',
  ABET1c smallint(6) NOT NULL default '0',
  ABET1d smallint(6) NOT NULL default '0',
  ABET1e smallint(6) NOT NULL default '0',
  ABET1f smallint(6) NOT NULL default '0',
  ABET2a smallint(6) NOT NULL default '0',
  ABET2b smallint(6) NOT NULL default '0',
  ABET2c smallint(6) NOT NULL default '0',
  ABET2d smallint(6) NOT NULL default '0',
  ABET2e smallint(6) NOT NULL default '0',
  ABET2f smallint(6) NOT NULL default '0',
  ABET3a smallint(6) NOT NULL default '0',
  ABET3b smallint(6) NOT NULL default '0',
  ABET3c smallint(6) NOT NULL default '0',
  ABET3d smallint(6) NOT NULL default '0',
  ABET3e smallint(6) NOT NULL default '0',
  ABET3f smallint(6) NOT NULL default '0',
  ABET4a smallint(6) NOT NULL default '0',
  ABET4b smallint(6) NOT NULL default '0',
  ABET4c smallint(6) NOT NULL default '0',
  ABET4d smallint(6) NOT NULL default '0',
  ABET4e smallint(6) NOT NULL default '0',
  ABET4f smallint(6) NOT NULL default '0',
  ABET5a smallint(6) NOT NULL default '0',
  ABET5b smallint(6) NOT NULL default '0',
  ABET5c smallint(6) NOT NULL default '0',
  ABET5d smallint(6) NOT NULL default '0',
  ABET5e smallint(6) NOT NULL default '0',
  ABET5f smallint(6) NOT NULL default '0',
  ABET6a smallint(6) NOT NULL default '0',
  ABET6b smallint(6) NOT NULL default '0',
  ABET6c smallint(6) NOT NULL default '0',
  ABET6d smallint(6) NOT NULL default '0',
  ABET6e smallint(6) NOT NULL default '0',
  ABET6f smallint(6) NOT NULL default '0',
  ABET7a smallint(6) NOT NULL default '0',
  ABET7b smallint(6) NOT NULL default '0',
  ABET7c smallint(6) NOT NULL default '0',
  ABET7d smallint(6) NOT NULL default '0',
  ABET7e smallint(6) NOT NULL default '0',
  ABET7f smallint(6) NOT NULL default '0',
  ABET8a smallint(6) NOT NULL default '0',
  ABET8b smallint(6) NOT NULL default '0',
  ABET8c smallint(6) NOT NULL default '0',
  ABET8d smallint(6) NOT NULL default '0',
  ABET8e smallint(6) NOT NULL default '0',
  ABET8f smallint(6) NOT NULL default '0',
  ABET9a smallint(6) NOT NULL default '0',
  ABET9b smallint(6) NOT NULL default '0',
  ABET9c smallint(6) NOT NULL default '0',
  ABET9d smallint(6) NOT NULL default '0',
  ABET9e smallint(6) NOT NULL default '0',
  ABET9f smallint(6) NOT NULL default '0',
  ABET10a smallint(6) NOT NULL default '0',
  ABET10b smallint(6) NOT NULL default '0',
  ABET10c smallint(6) NOT NULL default '0',
  ABET10d smallint(6) NOT NULL default '0',
  ABET10e smallint(6) NOT NULL default '0',
  ABET10f smallint(6) NOT NULL default '0',
  ABET11a smallint(6) NOT NULL default '0',
  ABET11b smallint(6) NOT NULL default '0',
  ABET11c smallint(6) NOT NULL default '0',
  ABET11d smallint(6) NOT NULL default '0',
  ABET11e smallint(6) NOT NULL default '0',
  ABET11f smallint(6) NOT NULL default '0',
  ABET12a smallint(6) NOT NULL default '0',
  ABET12b smallint(6) NOT NULL default '0',
  ABET12c smallint(6) NOT NULL default '0',
  ABET12d smallint(6) NOT NULL default '0',
  ABET12e smallint(6) NOT NULL default '0',
  ABET12f smallint(6) NOT NULL default '0',
  ABET13a smallint(6) NOT NULL default '0',
  ABET13b smallint(6) NOT NULL default '0',
  ABET13c smallint(6) NOT NULL default '0',
  ABET13d smallint(6) NOT NULL default '0',
  ABET13e smallint(6) NOT NULL default '0',
  ABET13f smallint(6) NOT NULL default '0',
  ABET14a smallint(6) NOT NULL default '0',
  ABET14b smallint(6) NOT NULL default '0',
  ABET14c smallint(6) NOT NULL default '0',
  ABET14d smallint(6) NOT NULL default '0',
  ABET14e smallint(6) NOT NULL default '0',
  ABET14f smallint(6) NOT NULL default '0',
  ABET15a smallint(6) NOT NULL default '0',
  ABET15b smallint(6) NOT NULL default '0',
  ABET15c smallint(6) NOT NULL default '0',
  ABET15d smallint(6) NOT NULL default '0',
  ABET15e smallint(6) NOT NULL default '0',
  ABET15f smallint(6) NOT NULL default '0',
  ABET16a smallint(6) NOT NULL default '0',
  ABET16b smallint(6) NOT NULL default '0',
  ABET16c smallint(6) NOT NULL default '0',
  ABET16d smallint(6) NOT NULL default '0',
  ABET16e smallint(6) NOT NULL default '0',
  ABET16f smallint(6) NOT NULL default '0',
  ABET17a smallint(6) NOT NULL default '0',
  ABET17b smallint(6) NOT NULL default '0',
  ABET17c smallint(6) NOT NULL default '0',
  ABET17d smallint(6) NOT NULL default '0',
  ABET17e smallint(6) NOT NULL default '0',
  ABET17f smallint(6) NOT NULL default '0',
  ABET18a smallint(6) NOT NULL default '0',
  ABET18b smallint(6) NOT NULL default '0',
  ABET18c smallint(6) NOT NULL default '0',
  ABET18d smallint(6) NOT NULL default '0',
  ABET18e smallint(6) NOT NULL default '0',
  ABET18f smallint(6) NOT NULL default '0',
  ABET19a smallint(6) NOT NULL default '0',
  ABET19b smallint(6) NOT NULL default '0',
  ABET19c smallint(6) NOT NULL default '0',
  ABET19d smallint(6) NOT NULL default '0',
  ABET19e smallint(6) NOT NULL default '0',
  ABET19f smallint(6) NOT NULL default '0',
  ABET20a smallint(6) NOT NULL default '0',
  ABET20b smallint(6) NOT NULL default '0',
  ABET20c smallint(6) NOT NULL default '0',
  ABET20d smallint(6) NOT NULL default '0',
  ABET20e smallint(6) NOT NULL default '0',
  ABET20f smallint(6) NOT NULL default '0',
  FR1 mediumtext NOT NULL,
  FR2 mediumtext NOT NULL,
  PRIMARY KEY (answersetid),
  UNIQUE KEY code(questionperiodid,questionsetid,classid)
);

CREATE TABLE classes (
  classid int(11) NOT NULL auto_increment,
  courseid int(11) NOT NULL default '0',
  section char(3) NOT NULL default '',
  divisioncode char(1) NOT NULL default '',
  year year(4) NOT NULL default '0000',
  semester enum('spring','summer','fall') NOT NULL default 'spring',
  name tinytext,
  professorid int(11) default NULL,
  students smallint(6) default NULL,
  time tinytext,
  location tinytext,
  callnumber int(11) default NULL,
  departmentid int(11) default NULL,
  divisionid int(11) default NULL,
  PRIMARY KEY (classid),
  UNIQUE KEY code(courseid,section,divisioncode,year,semester)
);

CREATE TABLE completesurveys (
  userid int(11) NOT NULL default '0',
  answersetid int(11) NOT NULL default '0',
  PRIMARY KEY (userid,answersetid)
);

CREATE TABLE courses (
  courseid int(11) NOT NULL auto_increment,
  subjectid int(11) NOT NULL default '0',
  code int(11) NOT NULL default '0',
  name tinytext,
  information text,
  departmentid int(11) default NULL,
  schoolid int(11) default NULL,
  PRIMARY KEY (courseid),
  UNIQUE KEY code(subjectid,code)
);

CREATE TABLE departments (
  departmentid int(11) NOT NULL auto_increment,
  code varchar(4) NOT NULL default '',
  name tinytext,
  PRIMARY KEY (departmentid),
  UNIQUE KEY code(code)
);

CREATE TABLE divisions (
  divisionid int(11) NOT NULL auto_increment,
  code char(2) NOT NULL default '',
  shortcode char(1) NOT NULL default '',
  name tinytext NOT NULL,
  PRIMARY KEY (divisionid),
  UNIQUE KEY name(name(255))
);

CREATE TABLE enrollments (
  classid int(11) default NULL,
  userid int(11) default NULL,
  UNIQUE KEY classid(classid,userid)
);

CREATE TABLE groupings (
  linkid int(11) NOT NULL default '0',
  linktype enum('classes','professors','subjects','departments','courses') NOT NULL default 'classes',
  questionsetid int(11) NOT NULL default '0',
  tarate enum('no','yes') NOT NULL default 'no',
  topicid int(11) default NULL,
  PRIMARY KEY (linkid,linktype,questionsetid)
);

CREATE TABLE ldapcache (
  userid int(11) NOT NULL default '0',
  cn tinytext,
  title tinytext,
  ou tinytext,
  PRIMARY KEY (userid)
);

CREATE TABLE professordupedata (
  professorid int(11) default NULL,
  first tinytext,
  middle char(1) default NULL,
  last tinytext,
  fullname tinytext,
  source enum('regweb','regpid','oracle','oldclasses') default NULL,
  pid varchar(10) default NULL,
  KEY professorid(professorid),
  KEY first(first(255)),
  KEY middle(middle),
  KEY last(last(255)),
  KEY fullname(fullname(255)),
  KEY source(source),
  KEY pid(pid)
);

CREATE TABLE professors (
  professorid int(11) NOT NULL auto_increment,
  userid int(11) default NULL,
  name tinytext,
  email tinytext,
  url tinytext,
  picname tinytext,
  statement text,
  profile text,
  education text,
  departmentid int(11) default NULL,
  PRIMARY KEY (professorid),
  UNIQUE KEY userid(userid)
);

CREATE TABLE questionperiods (
  questionperiodid int(11) NOT NULL auto_increment,
  year year(4) NOT NULL default '0000',
  semester enum('spring','summer','fall') NOT NULL default 'spring',
  periodstart date default NULL,
  periodend date default NULL,
  description tinytext,
  PRIMARY KEY (questionperiodid)
);

CREATE TABLE questionsets (
  questionsetid int(11) NOT NULL auto_increment,
  displayname tinytext,
  description tinytext,
  MC1 tinytext,
  MC2 tinytext,
  MC3 tinytext,
  MC4 tinytext,
  MC5 tinytext,
  MC6 tinytext,
  MC7 tinytext,
  MC8 tinytext,
  MC9 tinytext,
  MC10 tinytext,
  FR1 tinytext,
  ABET set('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16','17','18','19','20','21','22','23','24','25','26','27','28','29','30','31','32','33','34','35','36','37','38','39','40','41','42','43','44','45','46','47','48','49','50','51','52','53','54','55','56','57','58','59','60','61','62','63') default NULL,
  FR2 tinytext,
  type enum('private','public') NOT NULL default 'private',
  PRIMARY KEY (questionsetid)
);

CREATE TABLE schools (
  schoolid int(11) NOT NULL auto_increment,
  name tinytext NOT NULL,
  PRIMARY KEY (schoolid),
  UNIQUE KEY name(name(255))
);

CREATE TABLE sentmails (
  sentmailid int(11) NOT NULL auto_increment,
  sfrom tinytext,
  replyto tinytext,
  sto tinytext,
  title tinytext,
  body text,
  time timestamp(14) NOT NULL,
  PRIMARY KEY (sentmailid)
);

CREATE TABLE subjects (
  subjectid int(11) NOT NULL auto_increment,
  code varchar(4) NOT NULL default '',
  name tinytext,
  PRIMARY KEY (subjectid),
  UNIQUE KEY code(code)
);

CREATE TABLE tacomplete (
  questionperiodid int(11) NOT NULL default '0',
  classid int(11) NOT NULL default '0',
  userid int(11) NOT NULL default '0',
  PRIMARY KEY (questionperiodid,classid,userid)
);

CREATE TABLE taratings (
  taratingid int(11) NOT NULL auto_increment,
  tauserid int(11) default NULL,
  questionperiodid int(11) default NULL,
  classid int(11) default NULL,
  name tinytext,
  overall int(11) default NULL,
  knowledgeability int(11) default NULL,
  approachability int(11) default NULL,
  availability int(11) default NULL,
  communication int(11) default NULL,
  comments text,
  PRIMARY KEY (taratingid)
);

CREATE TABLE tausers (
  tauserid int(11) NOT NULL auto_increment,
  PRIMARY KEY (tauserid)
);

CREATE TABLE topics (
  topicid int(11) NOT NULL auto_increment,
  name tinytext,
  PRIMARY KEY (topicid)
);

CREATE TABLE users (
  userid int(11) NOT NULL auto_increment,
  cunix varchar(15) default NULL,
  email tinytext,
  status set('student','professor','administrator','barnard') NOT NULL default '',
  lastlogin datetime default NULL,
  isprofessor enum('false','true') NOT NULL default 'false',
  isadmin enum('false','true') NOT NULL default 'false',
  PRIMARY KEY (userid),
  UNIQUE KEY cunix(cunix)
);

CREATE TABLE warninglog (
  errorid int(11) NOT NULL auto_increment,
  date timestamp(14) NOT NULL,
  file tinytext,
  line int(11) default NULL,
  uni tinytext,
  dump text,
  PRIMARY KEY (errorid)
);

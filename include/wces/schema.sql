CREATE TABLE AnswerSets (
  answersetid int(11) NOT NULL auto_increment,
  questionsetid int(11) NOT NULL default '0',
  questionperiodid int(11) NOT NULL default '0',
  classid int(11) NOT NULL default '0',
  responses smallint(6) default NULL,
  MC1a smallint(6) default NULL,
  MC1b smallint(6) default NULL,
  MC1c smallint(6) default NULL,
  MC1d smallint(6) default NULL,
  MC1e smallint(6) default NULL,
  MC2a smallint(6) default NULL,
  MC2b smallint(6) default NULL,
  MC2c smallint(6) default NULL,
  MC2d smallint(6) default NULL,
  MC2e smallint(6) default NULL,
  MC3a smallint(6) default NULL,
  MC3b smallint(6) default NULL,
  MC3c smallint(6) default NULL,
  MC3d smallint(6) default NULL,
  MC3e smallint(6) default NULL,
  MC4a smallint(6) default NULL,
  MC4b smallint(6) default NULL,
  MC4c smallint(6) default NULL,
  MC4d smallint(6) default NULL,
  MC4e smallint(6) default NULL,
  MC5a smallint(6) default NULL,
  MC5b smallint(6) default NULL,
  MC5c smallint(6) default NULL,
  MC5d smallint(6) default NULL,
  MC5e smallint(6) default NULL,
  MC6a smallint(6) default NULL,
  MC6b smallint(6) default NULL,
  MC6c smallint(6) default NULL,
  MC6d smallint(6) default NULL,
  MC6e smallint(6) default NULL,
  MC7a smallint(6) default NULL,
  MC7b smallint(6) default NULL,
  MC7c smallint(6) default NULL,
  MC7d smallint(6) default NULL,
  MC7e smallint(6) default NULL,
  MC8a smallint(6) default NULL,
  MC8b smallint(6) default NULL,
  MC8c smallint(6) default NULL,
  MC8d smallint(6) default NULL,
  MC8e smallint(6) default NULL,
  MC9a smallint(6) default NULL,
  MC9b smallint(6) default NULL,
  MC9c smallint(6) default NULL,
  MC9d smallint(6) default NULL,
  MC9e smallint(6) default NULL,
  MC10a smallint(6) default NULL,
  MC10b smallint(6) default NULL,
  MC10c smallint(6) default NULL,
  MC10d smallint(6) default NULL,
  MC10e smallint(6) default NULL,
  FR1 mediumtext,
  FR2 mediumtext,
  PRIMARY KEY (answersetid),
  KEY questionperiodid(questionperiodid,questionsetid,classid)
);

CREATE TABLE Classes (
  classid int(11) NOT NULL auto_increment,
  courseid int(11) NOT NULL default '0',
  section char(3) NOT NULL default '',
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
  KEY courseid(courseid,section,year,semester)
);

CREATE TABLE CompleteSurveys (
  userid int(11) NOT NULL default '0',
  answersetid int(11) NOT NULL default '0',
  PRIMARY KEY (userid,answersetid)
);

CREATE TABLE Courses (
  courseid int(11) NOT NULL auto_increment,
  subjectid int(11) NOT NULL default '0',
  code int(11) default NULL,
  name tinytext,
  information text,
  departmentid int(11) default NULL,
  divisionid int(11) default NULL,
  schoolid int(11) default NULL,
  PRIMARY KEY (courseid),
  KEY subjectid(subjectid,code)
);

CREATE TABLE Departments (
  departmentid int(11) NOT NULL auto_increment,
  code varchar(4) NOT NULL default '',
  name tinytext,
  PRIMARY KEY (departmentid),
  KEY code(code)
);

CREATE TABLE Divisions (
  divisionid int(11) NOT NULL auto_increment,
  code char(2) default NULL,
  shortcode char(1) default NULL,
  name tinytext NOT NULL,
  PRIMARY KEY (divisionid)
);

CREATE TABLE Enrollments (
  userid int(11) NOT NULL default '0',
  classid int(11) NOT NULL default '0',
  PRIMARY KEY (userid,classid)
);

CREATE TABLE ErrorLog (
  time timestamp(14) NOT NULL,
  description text
);

CREATE TABLE Groupings (
  linkid int(11) NOT NULL default '0',
  linktype enum('classes','professors','subjects','departments','courses') NOT NULL default 'classes',
  questionsetid int(11) NOT NULL default '0',
  PRIMARY KEY (linkid,linktype,questionsetid)
);

CREATE TABLE ProfessorDupeData (
  professorid int(11) default NULL,
  first tinytext,
  middle char(1) default NULL,
  last tinytext,
  fullname tinytext,
  source enum('regweb','regpid','oracle','oldclasses') default NULL,
  pid varchar(10) default NULL
);

CREATE TABLE Professors (
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
  PRIMARY KEY (professorid)
);

CREATE TABLE QuestionPeriods (
  questionperiodid int(11) NOT NULL auto_increment,
  year year(4) NOT NULL default '0000',
  semester enum('spring','summer','fall') NOT NULL default 'spring',
  periodstart date default NULL,
  periodend date default NULL,
  description tinytext,
  PRIMARY KEY (questionperiodid)
);

CREATE TABLE QuestionSets (
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
  FR2 tinytext,
  type enum('private','public') NOT NULL default 'private',
  PRIMARY KEY (questionsetid)
);

CREATE TABLE Schools (
  schoolid int(11) NOT NULL auto_increment,
  name tinytext NOT NULL default '',
  PRIMARY KEY (schoolid)
);

CREATE TABLE Subjects (
  subjectid int(11) NOT NULL auto_increment,
  code varchar(4) NOT NULL default '',
  name tinytext,
  PRIMARY KEY (subjectid),
  KEY code(code)
);

CREATE TABLE Users (
  userid int(11) NOT NULL auto_increment,
  cunix varchar(15) default NULL,
  email tinytext,
  isprofessor enum('false','true') NOT NULL default 'false',
  isadmin enum('false','true') NOT NULL default 'false',
  PRIMARY KEY (userid)
);

CREATE TABLE surveys
(
  survey_id SERIAL PRIMARY KEY,
  link_id INTEGER NOT NULL
);

COMMENT ON TABLE surveys IS 'A survey is a list of survey_components. Each survey is linked to a set of objects by an entry in the links table.';

CREATE TABLE survey_components
(
  survey_component_id SERIAL PRIMARY KEY,
  survey_id INTEGER NOT NULL,
  component_id INTEGER NOT NULL,
  ordinal SMALLINT NOT NULL
);

COMMENT ON TABLE survey_components IS 'survey_components stores information about which components are in each survey and in what order the components appear.';

COMMENT ON COLUMN survey_components.ordinal IS 'ordinals are just integers used to quickly sort the components in a survey. No two survey_components in a survey should have the same ordinal.';

CREATE TABLE transforms
(
  transform_id SERIAL PRIMARY KEY,
  link_id INTEGER NOT NULL,
  survey_id INTEGER NOT NULL
);

COMMENT ON TABLE transforms IS 'each row represents a list of transform_operations that can be applied to a survey';

CREATE TABLE transform_operations
(
  transform_operation_id SERIAL PRIMARY KEY,
  transform_id INTEGER NOT NULL,
  component1_id INTEGER,
  component2_id INTEGER,
  ordinal SMALLINT NOT NULL,
  operation CHAR NOT NULL
);

COMMENT ON TABLE transform_operations IS 'A transform_operation contains instructions to add, remove, or swap components in a survey. Information about which survey is affected and what objects a group of transform_operations apply to is stored in the transform table.';

COMMENT ON COLUMN transform_operations.ordinal IS 'ordinals are integers used to specify what order the transform_operations in a transform are applied.';

COMMENT ON COLUMN transform_operations.operation IS 'The operation column determines the type of transformation to apply and the meaning of the question_id1 and question2 fields.
A value of 1 means *insert after*. More precisely, it means that question2 should be added to the survey and displayed after question1. If question2 already exists in the survey the instruction should be ignored. If question1 is NULL or does not exist in the survey, question2 should appear at the end of the survey.

A value of 2 means *insert before*. In other words, it means that question2 should be added to the survey and displayed before question1. If question2 already exists in the survey then the instruction should be ignored. If question1 is NULL, question2 should be inserted at the beginning of the survey. If question1 is not in the existing survey question2 should be inserted at the end.

A value of 3 means *delete*. If question1 exists in the survey, it should be removed, otherwise the instruction should be ignored. question2 should be set to NULL.

A value of 4 means *swap*. If question1 and question2 both exist in the survey, their ordering should be swapped. Otherwise the instruction should be ignored.';

CREATE TABLE survey_answers
(
  survey_answer_id SERIAL PRIMARY KEY,
  survey_id INTEGER NOT NULL,
  object_id INTEGER,
  question_period_id INTEGER NOT NULL
);

COMMENT ON TABLE survey_answers IS 'survey_answers is a collection of answers entered by one person taking a survey in a particular question_period.';

CREATE TABLE question_periods
(
  question_period_id SERIAL PRIMARY KEY,
  displayname VARCHAR(100),
  begindate TIME,
  enddate TIME
);

COMMENT ON TABLE question_periods IS 'a list of question periods';

COMMENT ON COLUMN question_periods.displayname IS 'an easy to recognize name for the question period like "Fall 2001 Midterm Evaluations"';

COMMENT ON COLUMN question_periods.begindate IS 'the date when the question_period begins';

COMMENT ON COLUMN question_periods.enddate IS 'the date when the question_period ends';

CREATE TABLE users
(
  user_id SERIAL PRIMARY KEY
);

COMMENT ON TABLE users IS 'The users table is just here so the system can keep track of who has already filled out surveys. It might also be used in the future to determine permissions and access rights.';

CREATE TABLE completions
(
  completion_id SERIAL PRIMARY KEY,
  user_id INTEGER NOT NULL,
  question_period_id INTEGER NOT NULL,
  survey_id INTEGER NOT NULL
);

COMMENT ON TABLE completions IS 'If a record exists for a particular user_id, question_period_id, and survey_id in this table then it means that the specified user filled out the specified survey in the specified question_period.';

CREATE TABLE links
(
  link_id SERIAL PRIMARY KEY
);

COMMENT ON TABLE links IS 'Links are the mechanism by which objects in the WBES database associate with external data. The links table is meant to be *abstract* -- not useful by itself but inherited by sub-tables that will have its properties in common. Other tables in the database reference external data by simply referencing a link_id number in this table. Depending on which type of subtable the link_id actually points to, the link can be associated with any type of information based on any type of criteria. For example, the each survey and transform in a course evaluation system would store link_id''s corresponding to certain subsets of the courses in an external database. Also, the user''s in the users table and address book entries used by the mass mailing utilities can store links that point to entries in another external database, possibly an LDAP server. There is no built in limit to the methods that can be used to convert a link into lists of the external objects and of finding the links associated with one or more external objects. The database right now includes support for two types of links that inherit from this table. The data_links table and data_links_cache are used to provide quick mapping of links to objects in a separate (but local) postgres database (and vice versa). The generic_links table is used to store links simple lists of items which don''t need to be associated with an existing database.';

CREATE TABLE objects
(
  object_id SERIAL PRIMARY KEY
);

COMMENT ON TABLE objects IS 'Objects are the destinations of links. A link can point to 0 or more objects. For example if a survey is created and linked to several classes, the survey_answer id will stores a field that indicates which one of the classes the answer applies to. Like the links table, the objects table can also be an abstract class. Unlike the links table, however, the other tables that reference object_id''s are not actually linked with foreign keys. If a link type requires that destination objects have integer ids (like the data_links link type), then those integer ids can be stored directly in the other tables. Normally, however, a proxy table would be created that inherits from objects.';

CREATE TABLE data_links
(
  link_table OID NOT NULL,
  link_function OID NOT NULL,
  is_dynamic BOOLEAN NOT NULL
)
INHERITS(links);

COMMENT ON TABLE data_links IS 'Data links will be used to link to rows from tables in a local (but separate) postgres database on conditions specified in a user defined function. The tables in the other database must have a normal sized INTEGER column that is a unique key.';

COMMENT ON COLUMN data_links.link_table IS 'This field stores the OID of the table which the link points to. For example if a survey is being linked to a set of classes then this field will store the OID of the classes table in the external database.';

COMMENT ON COLUMN data_links.link_function IS 'This field stores the OID of a function that will return a list of rows in the table being linked to. The function takes no arguments and returns a list of integer id''s for the appropriate rows int the table. For example, if the link is created to associate a survey with all of the classes in a particular department, the function will return the result of a simple SQL statment like: SELECT class_id FROM classes WHERE department = ''MATH''. The function can be of arbitrary complexity. Simple functions like the one just described might be created by a user-friendly GUI interface that allows the user to filter data based on some criteria. More complex linking functions could be created in SQL by a programmer or database administrator.';

COMMENT ON COLUMN data_links.is_dynamic IS 'For performance reasons described in data_links_cache, the results of data_links functions will be calculated in advance and cached. If the results of the functions need to be recalculated when the link_table is modified then this value should be set to TRUE, otherwise it should be FALSE. I''m not quite sure right now how we can determine if a table has been modified. There might be an adequate way of calculating this using some postgres internal table magic. A more likely approach would be to programmatically set a flag in the question_periods table when the external database contents change and then to regenerate the links_cache whenever it is needed and the flag is set. In most situations the relevant data in the external database will not be frequently changing during the question_period so the cache will rarely need to be rebuilt.';

CREATE TABLE data_links_cache
(
  link_id INTEGER NOT NULL,
  link_table OID NOT NULL,
  object_id INTEGER NOT NULL
);

COMMENT ON TABLE data_links_cache IS 'The user defined functions are able to quickly map a link to a list of objects. Often, however, what is needed is the reverse functionality. For example, when a user takes a survey, the system will start out with a list of objects related to the user and will then need to quickly get a list of which surveys and transformations are linked to those objects. Without some sort of cache, it would be neccessary to call the link functions of every survey in the system one by one and search inside their results for matching objects. Having a cache of the results of all functions is just a way of doing the work once and storing the results rather than doing the work with each and every page load.';

CREATE INDEX data_links_cache_index ON data_links_cache USING BTREE (object_id, link_table);

CREATE TABLE generic_links
(
  generic_link_id SERIAL PRIMARY KEY,
  link_names TEXT[]
);

COMMENT ON TABLE generic_links IS 'Used to link to objects that is not part of a relational database. The targets of the link are just a number of text strings which are stored in an array. For example, if a professor wanted to create a survey and link it to each of his course''s three textbooks, he could use a generic link with the names of the three textbooks. Each textbook name would be one element of an array. The array can also be set to NULL. If a survey was created with this type of link, it would mean that each person taking the survey would be able to enter in a string describing what the survey is about. (We used this technique to evaluate TAs because there is no information about them in the database.)';

COMMENT ON COLUMN generic_links.link_names IS 'see comment on the generic_links table.';

CREATE TABLE components
(
  component_id SERIAL PRIMARY KEY,
  component_type SMALLINT,
  component_text TEXT,
  is_html BOOLEAN NOT NULL,
  is_personal BOOLEAN NOT NULL,
  is_shared BOOLEAN NOT NULL,
  was_answered BOOLEAN NOT NULL
);

COMMENT ON TABLE components IS 'Abstract table inherited by other tables which store information about survey components. Survey components include questions as well as display items like headings, pictures and blocks of text.';

COMMENT ON COLUMN components.component_type IS 'Integer identifier for different question types.

1 - multiple choice question
2 - ratings question
3 - text response question
4 - picture
5 - block of text (or html)
6 - heading';

COMMENT ON COLUMN components.component_text IS 'Text of the question.';

COMMENT ON COLUMN components.is_html IS 'If this is set to true it means that the component_text and any other text associated with the question will be printed to the page unaltered. If this is set to false all text associated with the question will be converted to html by the system. Special characters like <, >, and & will be escaped and <br> tags will inserted at line endings.';

COMMENT ON COLUMN components.is_shared IS 'Will the answers to this question show up on the SEAS oracle.';

COMMENT ON COLUMN components.is_personal IS 'This is set to true if the question collects some kind of demographic information. When a question contains demographic information, there will be an option on the reporting utility to filter people''s other responses based on what they answered on the demographic questions.';

COMMENT ON COLUMN components.was_answered IS 'This flag will be set to true once a question has been answered at least once. (For non-question components like headings the flag will be set whenever the question appears on a survey that has been answered.) When true it is a signal to the system that the question should never be edited or deleted. Even after this has happened is_shared and is_personal flags should be considered mutable because they are more like reporting defaults than properties of the question itself.';

CREATE TABLE pictures
(
  location VARCHAR(16) NOT NULL,
  originalname VARCHAR(100) NOT NULL,
  format SMALLINT NOT NULL,
  width SMALLINT NOT NULL,
  height SMALLINT NOT NULL
)
INHERITS(components);

COMMENT ON TABLE pictures IS 'This component can display a picture that has been uploaded by the survey creator';

COMMENT ON COLUMN pictures.location IS 'stores the location of the picture on the web server';

COMMENT ON COLUMN pictures.format IS 'integer which identifies the format of the picture.

1 - PNG
2 - JPEG
3 - GIF';

COMMENT ON COLUMN pictures.width IS 'width of the picture in pixels';
COMMENT ON COLUMN pictures.height IS 'height of the picture in pixels';

CREATE TABLE multiple_choice
(
  choices text[],
  first_number REAL NOT NULL,
  last_number REAL NOT NULL,
  orientation SMALLINT NOT NULL,
  check_many BOOLEAN NOT NULL,
  is_required BOOLEAN NOT NULL
)
INHERITS(components);

COMMENT ON TABLE multiple_choice IS 'multiple choice component';

COMMENT ON COLUMN multiple_choice.choices IS 'Array of responses to choose from. If the is_html value of the component is true these can contain html.';

COMMENT ON COLUMN multiple_choice.first_number IS 'Sometimes it might be useful to treat the responses to multiple choice questions as numbers. For example if the choices on a question are "Not at all", "Slightly", "Somewhat", and "Very much," you can associate the first response with the number 0 and the last response with the number 3 and see be able to see the average and standard distribution on the final report.';

COMMENT ON COLUMN multiple_choice.last_number IS 'see comment for lowernumber';

COMMENT ON COLUMN multiple_choice.check_many IS 'true if the user can choose multiple answers';

COMMENT ON COLUMN multiple_choice.is_required IS 'true if an answer to the question is required';

COMMENT ON COLUMN multiple_choice.orientation IS 'describes the appearance of the choices
  1 - radio buttons or checkboxes listed horizontally
  2 - radio buttons or checkboxes choices listed vertically
  3 - a select box
  4 - a drop-down select box';

CREATE TABLE rating_items
(
  first_number INTEGER NOT NULL,
  last_number INTEGER NOT NULL,
  is_required BOOLEAN NOT NULL
)
INHERITS(components);

COMMENT ON TABLE rating_items IS 'a ratings_item is an item that is rated on a scale from the value of first_number to the value of last_number. When multiple ratings_items are next to each other on a survey with the same low number and high numbers they can all be edited together in a single component editor screen. They will also share a table on the survey and reporting pages.';

COMMENT ON COLUMN rating_items.first_number IS 'see description of table ratings_items.';
COMMENT ON COLUMN rating_items.last_number IS 'see description of table ratings_items.';
COMMENT ON COLUMN rating_items.is_required IS 'true if an answer to the question is required';

CREATE TABLE rating_descriptions
(
  first_text VARCHAR(100),
  last_text VARCHAR(100)
)
INHERITS(components);

COMMENT ON TABLE rating_descriptions IS 'The ratings_descriptions component stores brief fragments of text that describe what the low and high numbers mean on a ratings question. The descriptions will appear at the top of the tables descibed in ratings_items on surveys and reports as a reminder to the person filling out the survey or reading the report. On internal representations of the survey, the a rating_description component will precede a group of rating_item components.';

COMMENT ON COLUMN rating_descriptions.first_text IS 'see description of table ratings_descriptions.';
COMMENT ON COLUMN rating_descriptions.last_text IS 'see description of table ratings_descriptions.';

CREATE TABLE text_responses
(
  width SMALLINT NOT NULL,
  height SMALLINT NOT NULL
)
INHERITS(components);

COMMENT ON TABLE text_responses IS 'component type for text response questions';
COMMENT ON COLUMN text_responses.width IS 'width of the textarea field in characters';
COMMENT ON COLUMN text_responses.height IS 'height of the textarea field in characters';

CREATE TABLE component_answers
(
  component_answer_id SERIAL PRIMARY KEY,
  survey_answer_id INTEGER NOT NULL,
  component_id INTEGER NOT NULL
);

COMMENT ON TABLE component_answers IS 'abstract table whose descendants can store responses entered into survey components.';

CREATE TABLE numeric_answers
(
  answer INTEGER
)
INHERITS(component_answers);

COMMENT ON TABLE numeric_answers IS 'stores numeric responses for components';
COMMENT ON COLUMN numeric_answers.answer IS 'meaning depends on the component type';

CREATE TABLE text_answers
(
  answer INTEGER
)
INHERITS(component_answers);

COMMENT ON TABLE text_answers IS 'stores text responses for components';
COMMENT ON COLUMN text_answers.answer IS 'meaning depends on the component type';

CREATE TABLE legacy_answers
(
  answer INTEGER[] NOT NULL
)
INHERITS(component_answers);

COMMENT ON TABLE text_answers IS 'stores answers for old-style multiple choice and ratings question.';
COMMENT ON COLUMN text_answers.answer IS 'There is one position in the array for each possible choice or rating. The number stored in each position is the number of people who chose that option on the survey form.';

ALTER TABLE surveys ADD FOREIGN KEY (link_id) REFERENCES links(link_id);
ALTER TABLE survey_components ADD FOREIGN KEY (survey_id) REFERENCES surveys(survey_id);
ALTER TABLE survey_components ADD FOREIGN KEY (component_id) REFERENCES components(component_id);
ALTER TABLE transforms ADD FOREIGN KEY (link_id) REFERENCES links(link_id);
ALTER TABLE transforms ADD FOREIGN KEY (survey_id) REFERENCES surveys(survey_id);
ALTER TABLE transform_operations ADD FOREIGN KEY (transform_id) REFERENCES transforms(transform_id);
ALTER TABLE transform_operations ADD FOREIGN KEY (component1_id) REFERENCES components(component_id);
ALTER TABLE transform_operations ADD FOREIGN KEY (component2_id) REFERENCES components(component_id);
ALTER TABLE survey_answers ADD FOREIGN KEY (survey_id) REFERENCES surveys(survey_id);
ALTER TABLE survey_answers ADD FOREIGN KEY (question_period_id) REFERENCES question_periods(question_period_id);
ALTER TABLE completions ADD FOREIGN KEY (user_id) REFERENCES users(user_id);
ALTER TABLE completions ADD FOREIGN KEY (question_period_id) REFERENCES question_periods(question_period_id);
ALTER TABLE completions ADD FOREIGN KEY (survey_id) REFERENCES surveys(survey_id);
ALTER TABLE component_answers ADD FOREIGN KEY (survey_answer_id) REFERENCES survey_answers(survey_answer_id);
ALTER TABLE component_answers ADD FOREIGN KEY (component_id) REFERENCES components(component_id);

CREATE TABLE surveys
(
  survey_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('survey_ids'),
  topic_id
  branches INTEGER[],
  depth SMALLINT
);

CREATE SEQUENCE survey_ids INCREMENT 1 START 1;

CREATE INDEX topic_idx ON surveys(topic_id);

CREATE TABLE components
(
  component_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('component_ids'),
  type INTEGER NOT NULL
);

CREATE SEQUENCE component_ids INCREMENT 1 START 1;

COMMENT ON TABLE components IS 'Abstract table inherited by other tables which store information about survey components. Survey components include questions as well as display items like headings, pictures and blocks of text.';

COMMENT ON COLUMN components.component_type IS 'Integer identifier for different component types.';

CREATE TABLE major_components
(
  is_html BOOLEAN NOT NULL
  component_text TEXT
  visibility INTEGER
)
INHERITS(components);

CREATE TABLE text_responses
(
  component_text TEXT,
  rows SMALLINT NOT NULL,
  cols SMALLINT NOT NULL
)
INHERITS(major_components);

CREATE TABLE mult_choices
(
  first_number INTEGER,
  last_number INTEGER,
  choices TEXT[],
  flags INTEGER,
  rows INTEGER,
  is_html BOOLEAN NOT NULL,
  list_branch1 INTEGER NOT NULL,
  PRIMARY KEY (component_id)
)
INHERITS(major_components);

COMMENT ON COLUMN mult_choices.flags IS '
  0x01  is_numeric
  0x02  select_many
  0x04  last_other
  0x08  stacked
  0x10  vertical
';

CREATE TABLE mult_choice_questions
(
  question_text TEXT
)
INHERITS (components); -- these are not really full fledged components. the only inherit
                       -- from the component class so they can be used in the component_groups table

CREATE TABLE component_groups
(
  component_group_id INTEGER NOT NULL DEFAULT NEXTVAL('component_group_ids'),
  component_id INTEGER UNIQUE NOT NULL
);

COMMENT ON TABLE text_responses IS 'component type for text response questions';
COMMENT ON COLUMN text_responses.cols IS 'width of the textarea field in characters';
COMMENT ON COLUMN text_responses.rows IS 'height of the textarea field in characters';

CREATE TABLE component_answers
(
  component_answer_id INTEGER PRIMARY KEY DEFAULT NEXTVAL('component_answer_ids'),
  component_id INTEGER NOT NULL,
  user_id INTEGER,
  topic_id INTEGER
);

CREATE SEQUENCE component_answer_ids INCREMENT 1 START 1;

CREATE TABLE completions
(
  component_id INTEGER NOT NULL,
  topic_id INTEGER,
  user_id INTEGER NOT NULL
);

CREATE SEQUENCE completion_ids INCREMENT 1 START 1;

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

ALTER TABLE surveys ADD FOREIGN KEY (list_head) REFERENCES links(list_head);


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

FUNCTION component_group_insert(INTEGER, INTEGER)
DECLARE
  oldid ALIAS FOR $1;
  newid ALIAS FOR $2;
  g INTEGER;
  locked BOOLEAN;
BEGIN
  IF oldid IS NOT NULL AND newid IS NOT NULL THEN
    locked := 0;
    LOOP
      SELECT INTO g component_group_id FROM component_groups WHERE component_id = oldid;
      EXIT WHEN FOUND OR locked;
      SELECT INTO locked 1 FROM components WHERE component_id = oldid FOR UPDATE;
    END LOOP;
    IF NOT FOUND THEN
      INSERT INTO component_groups (component_id) VALUES (oldid);
      g := currval(''component_group_ids'');
    END IF;   
    INSERT INTO component_groups (component_group_id, component_id) VALUES (g, newid);
  END IF;
END;

FUNCTION text_response_update(INTEGER, INTEGER, BOOLEAN, TEXT, INTEGER, INTEGER, INTEGER)
DECLARE
  component_id_   ALIAS FOR $1;
  type_           ALIAS FOR $2;
  is_html_        ALIAS FOR $3;
  component_text_ ALIAS FOR $4;
  visibility_     ALIAS FOR $5;
  rows_           ALIAS FOR $6;
  cols_           ALIAS FOR $7;
  rec RECORD;
  c INTEGER;
BEGIN
  SELECT INTO rec component_id, is_html, component_text, visibility, rows, cols FROM text_responses WHERE component_id = component_id_ AND type = type_ FOR UPDATE;
  IF FOUND AND rec.is_html = is_html_ AND rec.component_text = component_text_ AND rec.visibility = visibility_ AND rec.rows = rows_ AND rec.cols = cols_) THEN
    RETURN rec.component_id;
  ELSE
    INSERT INTO text_responses(type, is_html, component_text, visibility, rows, cols) VALUES (type_, is_html_, component_text_, visibility_, rows_, cols_);
    c := currval(''component_ids'');
    component_group_insert(rec.component_id, c);
    RETURN c;
 END IF;
END;

FUNCTION major_component_update(INTEGER, INTEGER, BOOLEAN, TEXT, INTEGER)
DECLARE
  component_id_   ALIAS FOR $1;
  type_           ALIAS FOR $2;
  is_html_        ALIAS FOR $3;
  component_text_ ALIAS FOR $4;
  visibility_     ALIAS FOR $5;
  rec RECORD;
  c INTEGER;
BEGIN
  SELECT INTO rec component_id, is_html, component_text, visibility FROM text_responses WHERE component_id = component_id_ AND type = type_ FOR UPDATE;
  IF FOUND AND rec.is_html = is_html_ AND rec.component_text = component_text_ AND rec.visibility = visibility_) THEN
    RETURN rec.component_id;
  ELSE
    INSERT INTO text_responses(type, is_html, component_text, visibility) VALUES (type_, is_html_, component_text_, visibility_);
    c := currval(''component_ids'');
    component_group_insert(rec.component_id, c);
    RETURN c;
 END IF;
END;

FUNCTION choice_update(INTEGER, INTEGER, BOOLEAN, INTEGER,
                       INTEGER, INTEGER, INTEGER, TEXT,
                       TEXT[], BOOLEAN, BOOLEAN, BOOLEAN,
                       BOOLEAN, BOOLEAN, INTEGER)
DECLARE
  component_id_   ALIAS FOR $1;
  type_           ALIAS FOR $2;
  is_html_        ALIAS FOR $3;
  flags_          ALIAS FOR $4;
  choices         ALIAS FOR $5;
  first_number INTEGER,
  last_number INTEGER,
  rows INTEGER,
  is_html BOOLEAN NOT NULL,
  list_branch1 INTEGER NOT NULL,
  rec RECORD;
  c INTEGER;
BEGIN
  SELECT INTO rec component_id, is_html, component_text, visibility FROM text_responses WHERE component_id = component_id_ AND type = type_ FOR UPDATE;
  IF FOUND AND rec.is_html = is_html_ AND rec.component_text = component_text_ AND rec.visibility = visibility_) THEN
    RETURN rec.component_id;
  ELSE
    INSERT INTO text_responses(type, is_html, component_text, visibility) VALUES (type_, is_html_, component_text_, visibility_);
    c := currval(''component_ids'');
    component_group_insert(rec.component_id, c);
    RETURN c;
 END IF;
END;
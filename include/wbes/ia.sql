DROP TABLE ia_data;
DROP SEQUENCE ia_ids;
DROP FUNCTION ia_new(INTEGER[]);
DROP FUNCTION ia_delete(INTEGER);
DROP FUNCTION ia_double(TEXT);
DROP FUNCTION ia_grow(INTEGER, SMALLINT);
DROP FUNCTION ia_clone(INTEGER);
DROP FUNCTION ia_get(INTEGER, SMALLINT);
DROP FUNCTION ia_set(INTEGER, SMALLINT, INTEGER);
DROP FUNCTION ia_array(INTEGER);
DROP FUNCTION ia_print(INTEGER[]);
DROP FUNCTION ia_length(INTEGER);

CREATE TABLE ia_data
(
  ia_id INTEGER NOT NULL PRIMARY KEY DEFAULT NEXTVAL('ia_ids'),
  length SMALLINT NOT NULL,
  capacity SMALLINT NOT NULL,
  data INTEGER[] NOT NULL
);

CREATE SEQUENCE ia_ids INCREMENT 1 START 1;

CREATE FUNCTION ia_new() RETURNS INTEGER AS '
  BEGIN
    RETURN ia_new(''{}'');
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_new(INTEGER[]) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
    i SMALLINT;
  BEGIN
    i := 0;
    LOOP
      EXIT WHEN a[i+1] IS NULL;
      i = i + 1;
    END LOOP;
    INSERT INTO ia_data (length,capacity,data) VALUES (i,i,a);
    RETURN currval(''ia_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_delete(INTEGER) RETURNS INTEGER AS '
  DELETE FROM ia_data WHERE ia_id = $1;
' LANGUAGE 'sql';

CREATE FUNCTION ia_double(TEXT) RETURNS INTEGER[] AS '
  BEGIN
    RETURN SUBSTRING($1 FOR (char_length($1)-1)) || '','' || SUBSTRING($1 FROM 2);
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_print(INTEGER[]) RETURNS TEXT AS '
  BEGIN
    RETURN $1;
  END;  
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_grow(INTEGER, SMALLINT) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
    l ALIAS FOR $2;
    c INTEGER;
  BEGIN
    SELECT INTO c capacity FROM ia_data WHERE ia_id = a;
    IF l <= c THEN RETURN 1; END IF;
    
    IF c = 0 THEN
      c := 4;
      UPDATE ia_data SET capacity = c, data = ''{0,0,0,0}'' WHERE ia_id = a;
    END IF;
    
    LOOP
      EXIT WHEN l <= c;
      c := c * 2;
      UPDATE ia_data SET capacity = c, data = ia_double(ia_print(data)) WHERE ia_id = a;
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_clone(INTEGER) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
  BEGIN
    INSERT INTO ia_data (length,capacity,data) SELECT length, capacity, data FROM ia_data WHERE ia_id = a;
    RETURN currval(''ia_ids'');
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_get(INTEGER, SMALLINT) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
    i ALIAS FOR $2;
    rec RECORD;
  BEGIN
    SELECT INTO rec length, data[i] AS d FROM ia_data WHERE ia_id = a;
    IF i > rec.length THEN RETURN NULL; ELSE RETURN rec.d; END IF;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_reverse(INTEGER) AS '
  DECLARE
    a ALIAS FOR $1;
    rec RECORD;
    i INTEGER;
    l INTEGER;
    t INTEGER;
  BEGIN
    SELECT INTO rec length, capacity FROM ia_data WHERE ia_id = a;
    l := rec.length / 2;
    FOR i IN 1..l LOOP
      SELECT INTO t data[i] FROM ia_data WHERE ia_id = a;
      UPDATE ia_data SET data[i] = data[rec.length - i -1] WHERE ia_id = a;
      UPDATE ia_data SET data[rec.length - i - 1] = t WHERE ia_id = a;
    END LOOP;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';
  

CREATE FUNCTION ia_set(INTEGER, SMALLINT, INTEGER) RETURNS INTEGER AS '
  DECLARE
    a ALIAS FOR $1;
    i ALIAS FOR $2;
    v ALIAS FOR $3;
    rec RECORD;
  BEGIN
    SELECT INTO rec length, capacity FROM ia_data WHERE ia_id = a;
    IF i > rec.capacity THEN
      PERFORM ia_grow(a,i);
      UPDATE ia_data SET length = i, data[i] = v WHERE ia_id = a;
    ELSE IF i > rec.length THEN
      UPDATE ia_data SET length = i, data[i] = v WHERE ia_id = a;
    ELSE IF v IS NULL THEN
      UPDATE ia_data SET length = i - 1, data[i] = v WHERE ia_id = a;
    ELSE
      UPDATE ia_data SET data[i] = v WHERE ia_id = a;
    END IF; END IF; END IF;
    RETURN 1;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_array(INTEGER) RETURNS INTEGER[] AS '
  DECLARE
    a ALIAS FOR $1;
    rec RECORD;
  BEGIN
    SELECT INTO rec data[1:length] AS d FROM ia_data WHERE ia_id = a;
    RETURN rec.d;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_slice(INTEGER, INTEGER, INTEGER) RETURNS INTEGER[] AS '
  DECLARE
    a ALIAS FOR $1;
    s ALIAS FOR $2;
    e ALIAS FOR $3;
    rec RECORD;
  BEGIN
    SELECT INTO rec data[s:e] AS d FROM ia_data WHERE ia_id = a;
    RETURN rec.d;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_tarray(INTEGER) RETURNS INTEGER[] AS '
  DECLARE
    a ALIAS FOR $1;
    rec RECORD;
  BEGIN
    SELECT INTO rec data[1:length] AS d FROM ia_data WHERE ia_id = a;
    DELETE FROM ia_data WHERE ia_id = a;
    RETURN rec.d;
  END;
' LANGUAGE 'plpgsql';

CREATE FUNCTION ia_length(INTEGER) RETURNS SMALLINT AS '
  SELECT length FROM ia_data WHERE ia_id = $1;
' LANGUAGE 'sql';

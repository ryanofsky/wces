CREATE FUNCTION int_distf(integer[], integer) RETURNS integer[] AS
  '/cygdrive/l/server/shares/russ.esurveys.hn.org/etc/postgres/distribution.dll', 'int_distf'
LANGUAGE 'c' WITH (iscachable);

CREATE FUNCTION int_mdistf(integer[], integer[]) RETURNS integer[] AS
  '/cygdrive/l/server/shares/russ.esurveys.hn.org/etc/postgres/distribution.dll', 'int_mdistf'
LANGUAGE 'c' WITH (iscachable);

DROP FUNCTION int_distf(integer[], integer);
DROP FUNCTION int_mdistf(integer[], integer[]);

CREATE FUNCTION int_distf(integer[], integer) RETURNS integer[] AS
  '/home/httpd/etc/postgres/distribution.so', 'int_distf'
LANGUAGE 'c' WITH (iscachable);

CREATE FUNCTION int_mdistf(integer[], integer[]) RETURNS integer[] AS
  '/home/httpd/etc/postgres/distribution.so', 'int_mdistf'
LANGUAGE 'c' WITH (iscachable);

CREATE AGGREGATE choice_dist (
    basetype = INTEGER,
    stype = INTEGER[],
    sfunc = int_distf
);

CREATE AGGREGATE choice_dist (
    basetype = INTEGER[],
    stype = INTEGER[],
    sfunc = int_mdistf
);

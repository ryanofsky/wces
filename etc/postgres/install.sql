DROP AGGREGATE choice_dist INTEGER;
DROP AGGREGATE choice_dist INTEGER[];
DROP FUNCTION dist_insert(integer[], integer);
DROP FUNCTION dist_insert(integer[], integer[]);
DROP FUNCTION dist_sum(integer[], integer[]);

CREATE FUNCTION dist_insert(integer[], integer) RETURNS integer[] AS
  '/cygdrive/l/server/shares/russ.esurveys.hn.org/etc/postgres/distribution.dll', 'int_dist_insert_one'
LANGUAGE 'c' WITH (iscachable);

CREATE FUNCTION dist_insert(integer[], integer[]) RETURNS integer[] AS
  '/cygdrive/l/server/shares/russ.esurveys.hn.org/etc/postgres/distribution.dll', 'int_dist_insert_many'
LANGUAGE 'c' WITH (iscachable);

CREATE FUNCTION dist_sum(integer[], integer[]) RETURNS integer[] AS
  '/cygdrive/l/server/shares/russ.esurveys.hn.org/etc/postgres/distribution.dll', 'int_dist_sum'
LANGUAGE 'c' WITH (iscachable);

CREATE AGGREGATE choice_dist (
    basetype = INTEGER,
    stype = INTEGER[],
    sfunc = dist_insert
);

CREATE AGGREGATE choice_dist (
    basetype = INTEGER[],
    stype = INTEGER[],
    sfunc = dist_insert
);

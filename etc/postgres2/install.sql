DROP AGGREGATE mode INTEGER;
DROP AGGREGATE scat_dist INTEGER;
DROP FUNCTION int_scat_distf(integer[], integer);
DROP FUNCTION int_scat_modef(integer[]);

CREATE FUNCTION int_scat_distf(integer[], integer) RETURNS integer[] AS
  '/home/httpd/etc/postgres2/scattered_distribution.so', 'int_scat_distf'
LANGUAGE 'c' WITH (iscachable);

CREATE FUNCTION int_scat_modef(integer[]) RETURNS integer AS
  '/home/httpd/etc/postgres2/scattered_distribution.so', 'int_scat_modef'
LANGUAGE 'c' WITH (iscachable);

CREATE AGGREGATE mode (
    basetype = INTEGER,
    stype = INTEGER[],
    sfunc = int_scat_distf,
    finalfunc = int_scat_modef 
);

CREATE AGGREGATE scat_dist (
    basetype = INTEGER,
    stype = INTEGER[],
    sfunc = int_scat_distf
);


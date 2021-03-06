CREATE TABLE IF NOT EXISTS public.test_table (
    time TIMESTAMP WITHOUT TIME ZONE NOT NULL,
    test_double double PRECISION NULL
);

CREATE TABLE IF NOT EXISTS public.test_table2 (
    test_double1 double PRECISION NULL,
    test_double2 double PRECISION NULL,
    test_double3 double PRECISION NULL
);

CREATE TABLE IF NOT EXISTS public.test_table3 (
    test_double1 double PRECISION NULL,
    test_double2 double PRECISION NULL,
    test_double3 double PRECISION NULL,
    time TIMESTAMP WITHOUT TIME ZONE NOT NULL,
    test_string varchar(100),
    time2 TIMESTAMP WITHOUT TIME ZONE NOT NULL
);
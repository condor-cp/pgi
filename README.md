# Postgres C++ interfacing library : pgi

pgi is a header only library built on top of [libpqxx](https://github.com/jtv/libpqxx.git) and [cpp-yaml](https://github.com/jbeder/yaml-cpp.git). It is essentially a wrapper around libpqxx that allows configuration from yaml file and higher level methods interfacing C++ with postgres.

## Requirements

1. [libpqxx](https://github.com/jtv/libpqxx.git) installed
2. [cpp-yaml](https://github.com/jbeder/yaml-cpp.git) installed

## How to use

1. Create a config file with yaml format. An example is given in [config/database_config.yaml](config/database_config.yaml). See [config file fields](#configuration-file-fields) for details.

2. Include "pgi.h"

3. Create a DatabaseWorker instance, by providing a path to the configuration file.

4. Use DatabaseWorker methods. For example, insert columns from multiple std::map and print the content of the table :

```
    DatabaseWorker dbw(
        pgi_test::pgi_config_dir + "database_config.yaml", pgi_test::pgi_config_dir + "database_config_out.yaml");

    // Most general method to insert a row
    std::map<std::string, double> double_cols;
    double_cols["test_double1"] = 1.1;
    double_cols["test_double2"] = 2.2;
    double_cols["test_double3"] = 3.3;
    std::map<std::string, std::string> string_cols;
    string_cols["test_string"] = "Hello world!";
    std::map<std::string, time_point_t> time_cols;
    time_cols["time"] = tp;
    time_cols["time2"] = tp - 50s;
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);
    dbw.print("public.test_table3");
```

## Try it with docker-compose

A minimal working environment (for the sake of example + continuous deployment) can be found in [ci/](ci/). It sets up a minimal postgres database in one container, builds minimal example using pgi in another container and runs it.

```docker-compose up```

Will end with output :  

```
test_double1 |test_double2 |test_double3 |time                    |test_string  |time2                   |
1.1          |2.2          |3.3          |2021-12-01 11:32:38.112 |Hello world! |2021-12-01 11:31:48.112 |
1.1          |2.2          |3.3          |2021-12-01 11:32:38.112 |Hello world! |2021-12-01 11:31:48.112 |
1.1          |2.2          |3.3          |2021-12-01 11:32:38.112 |Hello world! |2021-12-01 11:31:48.112 |

```

## Configuration file fields 

+ connection : have to contain all the fields to establish database connection. Key/Values will be parsed to the connection string. See postgres documentation for the connection string [here](https://www.postgresql.org/docs/12/libpq-connect.html#LIBPQ-CONNSTRING). 

+ [tables] : list tables to explore at the construction of DatabaseWorker 

+ [field_length_mapping] : configure the length of the printed column according to postgres type  

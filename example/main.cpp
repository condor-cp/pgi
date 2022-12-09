#include <iostream>

#include <pgi.hpp>
#include "config_dir.hpp"
#include <chrono>
#include <execution>
using namespace pgi;

int main()
{
    using namespace std::chrono_literals;
    DatabaseWorker dbw(pgi_test::pgi_config_dir + "database_config.yaml");

    // Most general method to insert a row
    std::map<std::string, double> double_cols = {
        {"test_double1", 1.1},
        {"test_double2", 2.2},
        {"test_double3", 3.3},
    };
    std::map<std::string, std::string> string_cols = {{"test_string", "Hello world!"}};
    time_point_t tp = std::chrono::system_clock::now();
    std::map<std::string, time_point_t> time_cols = {
        {"time", tp},
        {"time2", tp - 50s},
    };
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);
    dbw.insert_from_maps("public.test_table3", double_cols, string_cols, time_cols);

    // Bulk insert of row
    std::map<std::string, std::vector<double>> double_cols_bulk;
    double_cols_bulk["test_double1"] = {1.1, 1.2, 1.3};
    double_cols_bulk["test_double2"] = {0.1, 0.2, 0.3};
    double_cols_bulk["test_double3"] = {2.1, 2.2, 2.3};
    std::map<std::string, std::vector<std::string>> string_cols_bulk;
    string_cols_bulk["test_string"] = {"Hello world!", "Hello earth!", "Hello"};
    std::map<std::string, std::vector<time_point_t>> time_cols_bulk;
    time_cols_bulk["time"] = {tp, tp - 1s, tp - 2s};
    time_cols_bulk["time2"] = {tp, tp - 5s, tp - 10s};
    dbw.bulk_insert_from_maps("public.test_table3", double_cols_bulk, string_cols_bulk, time_cols_bulk);

    // Test concurrency
    std::vector<int> vecint(5, 1);

    std::for_each(std::execution::par, vecint.begin(), vecint.end(),
        [&dbw, double_cols_bulk, string_cols_bulk, time_cols_bulk](int) {
            dbw.bulk_insert_from_maps("public.test_table3", double_cols_bulk, string_cols_bulk, time_cols_bulk);
        });
    dbw.print("public.test_table3");

    return 0;
}
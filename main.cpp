#include <iostream>

#include "pgi.hpp"
#include "config_dir.hpp"
#include <chrono>

using namespace pgi;

int main()
{
    DatabaseWorker dbw(
        pgi_test::pgi_config_dir + "database_config.yaml", pgi_test::pgi_config_dir + "database_config_out.yaml");

    std::vector<std::string> columns;
    columns.push_back("time");
    columns.push_back("test_double");
    dbw.select("public.test_table", columns);
    dbw.insert("public.test_table", "NOW()", 54.8);
    std::vector<double> y;
    y.push_back(3.5155);
    time_point_t tp = std::chrono::system_clock::now();
    dbw.insert("public.test_table", tp, y);

    dbw.insert("public.test_table2", 3.1, 0.2, 4.7);
    dbw.insert("public.test_table2", 3.5, 0.5, 4.9);
    std::vector<std::string> columns2;
    columns2.push_back("test_double1");
    columns2.push_back("test_double2");
    columns2.push_back("test_double3");
    pqxx::result r = dbw.select("public.test_table2", columns2);
    int const num_rows = std::size(r);
    for (int rownum=0; rownum < num_rows; ++rownum)
    {
        pqxx::row const row = r[rownum];
        int const num_cols = std::size(row);
        for (int colnum=0; colnum < num_cols; ++colnum)
        {
            pqxx::field const field = row[colnum];
            std::cout << field.c_str() << '\t';
        }
        std::cout << std::endl;
    }
    return 0;
}
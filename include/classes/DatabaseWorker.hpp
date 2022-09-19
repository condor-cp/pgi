#pragma once
#include <iostream>
#include <fstream>
#include "utl/string_utls.hpp"
#include "utl/datetime.hpp"
#include "utl/map_utls.hpp"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <thread>

namespace pgi {

class DatabaseWorker
{
public:
    /// Constructor requires a connection file and optionnaly a configuration file defining the tables to explore.
    DatabaseWorker(const std::string& connection_file, std::string configuration_file = "")
    {
        // Step 1 : Connection
        YAML::Node connection_config_root = YAML::LoadFile(connection_file);
        YAML::Node connection_config = connection_config_root["connection"];
        connect(connection_config);
        open_pipeline();

        // Step 2 (optionnal): Explore tables
        if (configuration_file.empty())
            configuration_file = connection_file;
        db_config_ = YAML::LoadFile(configuration_file);
        if (db_config_["tables"])
        {
            YAML::Node tables = db_config_["tables"];
            explore_tables(tables);
        }
    }

    pqxx::result select(const std::string& table_name,
        const std::vector<std::string> fields = std::vector<std::string>(),
        const std::string& condition = "",
        const std::string& order_by = "",
        const int& limit = 10000)
    {
        explore_if_unknown(table_name);
        std::stringstream ss;
        if (fields.size() == 0)
            ss << "SELECT *";
        else
        {
            ss << "SELECT \"" << fields[0] << "\"";
            for (size_t i = 1; i < fields.size(); i++)
                ss << ", "
                   << "\"" << fields[i] << "\"";
        }

        ss << " FROM " << table_name;
        if (!condition.empty())
            ss << " WHERE " << condition;
        if (!order_by.empty())
            ss << " ORDER BY " << order_by;
        ss << " LIMIT " << limit;
        pqxx::result r = execute(ss.str());

        if (std::size(r) == limit)
            std::cout << "Warning : fetch reached maximum number (" << limit << ")" << std::endl;

        return r;
    }

    pqxx::result select_all_columns(const std::string& table_name, const std::string& condition = "")
    {
        return select(table_name, std::vector<std::string>(), condition);
    }

    void print(pqxx::result r)
    {
        bool header_added = false;
        std::stringstream ss;
        int const num_rows = std::size(r);
        for (int rownum = 0; rownum < num_rows; ++rownum)
        {
            pqxx::row const row = r[rownum];
            ss << print_row(row, !header_added);
            if (!header_added)
                header_added = true;
            ss << "\n";
        }
        std::cout << ss.str();
    }

    std::string print_row(const pqxx::row& row, bool header = false)
    {
        const char fill = ' ';
        const char separator[3] = " |";
        std::stringstream ss;
        std::stringstream headerss;
        int const num_cols = std::size(row);
        for (int colnum = 0; colnum < num_cols; ++colnum)
        {
            pqxx::field const field = row[colnum];
            std::string column_name = field.name();
            std::string column_type = get_typname_from_oid(field.type());
            YAML::Node field_length_mapping = db_config_["field_length_mapping"];
            int field_width;
            if (field_length_mapping[column_type])
                field_width = field_length_mapping[column_type].as<int>();
            else
                field_width = 10;
            field_width = std::max<int>(column_name.length(), field_width);
            ss << std::left << std::setw(field_width) << std::setfill(fill) << utl::truncate(field.c_str(), field_width)
               << separator;
            if (header)
                headerss << std::left << std::setw(field_width) << std::setfill(fill)
                         << utl::truncate(column_name, field_width) << separator;
        }
        if (header)
        {
            headerss << "\n" << ss.str();
            return headerss.str();
        }
        else
        {
            return ss.str();
        }
    }

    void print(const std::string& table_name) { return print(select_all_columns(table_name)); }


    /// Inserts a row in a defined table from a full set of values.
    /// The order of values must be the same as in the table definition.
    template <typename... Args>
    void insert(const std::string& table_name, Args... values)
    {
        explore_if_unknown(table_name);
        std::stringstream ss;
        ss << insert_statement_first_part(table_name);

        ((ss << values << ", "), ...);
        ss.seekp(-2, ss.cur);
        ss << ')';
        execute(ss.str());
    }

    /// Inserts a row in a defined table from a multiple std::map<std::string, T>.
    template <typename... Args>
    void insert_from_maps(const std::string& table_name, Args... maps)
    {
        std::map<std::string, std::string> merged_maps = utl::merge_maps(maps...);
        std::stringstream columns, values, ss;
        columns << "(";
        values << "(";
        for (auto const& [key, val] : merged_maps)
        {
            columns << "\"" << key << "\", ";
            values << val << ", ";
        }
        columns.seekp(-2, columns.cur);
        columns << ")";
        values.seekp(-2, values.cur);
        values << ")";

        ss << "INSERT INTO " << table_name << " " << columns.str() << "VALUES " << values.str();
        explore_if_unknown(table_name);
        execute(ss.str());
    }

    /// Inserts a row in a defined table from a multiple std::map<std::string, std::vector<T>>.
    template <typename... Args>
    void bulk_insert_from_maps(const std::string& table_name, Args... maps)
    {
        std::map<std::string, std::vector<std::string>> merged_maps = utl::merge_maps(maps...);
        std::stringstream columns, ss;
        std::vector<std::shared_ptr<std::stringstream>> values;
        size_t bulk_len = merged_maps.begin()->second.size();
        columns << "(";

        for (size_t i = 0; i < bulk_len; i++)
        {
            std::shared_ptr<std::stringstream> valuesss(new std::stringstream());
            *valuesss << "(";
            values.push_back(valuesss);
        }

        for (auto const& [key, val_vec] : merged_maps)
        {
            columns << "\"" << key << "\", ";
            size_t i = 0;
            for (auto const& val : val_vec)
            {
                (*(values[i])) << val << ", ";
                i++;
            }
        }
        columns.seekp(-2, columns.cur);
        columns << ")";
        ss << "INSERT INTO " << table_name << " " << columns.str() << "VALUES ";
        for (size_t i = 0; i < bulk_len; i++)
        {
            (*(values[i])).seekp(-2, (*(values[i])).cur);
            (*(values[i])) << ")";
            ss << (*(values[i])).str() << ", ";
        }
        ss.seekp(-2, ss.cur);
        ss << "  ";
        explore_if_unknown(table_name);
        execute(ss.str());
    }

    /// Inserts a row in a defined table from a multiple std::map<std::string, T>.
    template <typename... Args>
    void update_from_maps(const std::string& table_name, const std::string& condition, Args... maps)
    {
        std::map<std::string, std::string> merged_maps = utl::merge_maps(maps...);
        std::stringstream ss;
        ss << "UPDATE " << table_name << " SET ";
        for (auto const& [key, val] : merged_maps)
            ss << "\"" << key << "\""
               << "=" << val << ", ";

        ss.seekp(-2, ss.cur);
        ss << " WHERE " << condition;
        explore_if_unknown(table_name);
        execute(ss.str());
    }


    // Specialization for vectors
    template <typename T>
    void insert(const std::string& table_name, const std::vector<T>& vector)
    {
        explore_if_unknown(table_name);
        std::stringstream ss;
        ss << insert_statement_first_part(table_name);

        for (T element : vector)
            ss << element << ", ";
        ss.seekp(-2, ss.cur);
        ss << ')';
        execute(ss.str());
    }

    // Specialization for timed vectors
    template <typename T>
    void insert(const std::string& table_name, time_point_t tp, const std::vector<T>& vector)
    {
        explore_if_unknown(table_name);
        std::stringstream ss;
        ss << insert_statement_first_part(table_name);
        ss << "'" << utl::ISO_8601(tp) << "', ";
        for (T element : vector)
            ss << element << ", ";
        ss.seekp(-2, ss.cur);
        ss << ')';
        execute(ss.str());
    }

    void clear(const std::string& table_name)
    {
        std::stringstream ss;
        ss << "TRUNCATE " << table_name << " CASCADE";
        execute(ss.str());
    }

    pqxx::result execute(const std::string& statement)
    {
        pqxx::result r;
        try
        {
            pqxx::pipeline::query_id qid;
            {
                std::lock_guard<std::mutex> guard(mutex_);
                qid = current_pipeline_->insert(statement);

                // These line should be out of critical section but
                // "Got more results from pipeline than there were queries" is raised when executing in parallel

                // while (!current_pipeline_->is_finished(qid)) ==> Doesn't work, always 0 ??!
                //     std::this_thread::sleep_for(std::chrono::milliseconds(100));

                r = current_pipeline_->retrieve(qid);
            }
        } catch (const std::exception& e)
        {
            std::cerr << "\nError : " << e.what() << "was raised while executing the following statement : \n"
                      << statement << '\n';
        }

        return r;
    }

    pqxx::row execute1(const std::string& statement)
    {
        pqxx::row row;
        pqxx::result r = execute(statement);
        if (std::size(r) > 0)
            row = r[0];
        return row;
    }

protected:
    void connect(YAML::Node connection_config)
    {
        try
        {
            std::stringstream ss;
            for (YAML::const_iterator it = connection_config.begin(); it != connection_config.end(); ++it)
                ss << it->first.as<std::string>() << "=" << it->second.as<std::string>() << " ";

            std::string connection_string = ss.str();

            std::shared_ptr<pqxx::connection> buff(new pqxx::connection(connection_string));
            c_ = buff;

        } catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    };

    void explore_tables(YAML::Node tables)
    {
        try
        {
            for (std::size_t i = 0; i < tables.size(); i++)
                get_column_details(tables[i].as<std::string>());
        } catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    void get_column_details(const std::string& table_name)
    {
        try
        {
            std::stringstream ss(table_name);
            std::string segment;
            std::vector<std::string> seglist;

            std::getline(ss, segment, '.');
            db_config_["tables_details"][table_name]["schema"] = segment;
            std::getline(ss, segment, '.');
            db_config_["tables_details"][table_name]["table"] = segment;


            pqxx::result r = execute(utl::string_format("SELECT * FROM %s LIMIT 0", table_name));

            for (size_t i = 0; i < size_t(r.columns()); i++)
            {
                db_config_["tables_details"][table_name]["columns"][std::string(r.column_name(i))] =
                    get_typname_from_oid(r.column_type(i));
            }

            r = execute(utl::string_format(
                "SELECT c.column_name, c.data_type "
                "FROM information_schema.table_constraints tc "
                "JOIN information_schema.constraint_column_usage AS ccu USING (constraint_schema, constraint_name) "
                "JOIN information_schema.columns AS c ON c.table_schema = '%s' "
                "  AND tc.table_name = '%s' AND ccu.column_name = c.column_name "
                "WHERE constraint_type = 'PRIMARY KEY';",
                db_config_["tables_details"][table_name]["schema"].as<std::string>(),
                db_config_["tables_details"][table_name]["table"].as<std::string>()));
            if (!r.empty())
                db_config_["tables_details"][table_name]["primary_key"] = r[0][0].as<std::string>();
            else
                db_config_["tables_details"][table_name]["primary_key"] = "_none_";

        } catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    std::string get_typname_from_oid(int oid)
    {
        pqxx::row row;
        row = execute1(utl::string_format("SELECT t.typname FROM pg_type t WHERE t.oid = %d", oid));
        return row[0].as<std::string>();
    }

    void drop_config_yaml(const std::string& output_file)
    {
        try
        {
            std::ofstream fout;
            fout.open(output_file);
            fout << db_config_ << "\n";
            fout.close();
        } catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    std::string insert_statement_first_part(const std::string& table_name)
    {
        explore_if_unknown(table_name);
        std::stringstream ss;
        ss << "INSERT INTO " << table_name << "(";
        for (YAML::const_iterator it = db_config_["tables_details"][table_name]["columns"].begin();
             it != db_config_["tables_details"][table_name]["columns"].end(); ++it)
        {
            // Include column only if it is not a primary key ewcept if it is of type timezone
            if ((it->first.as<std::string>() !=
                    db_config_["tables_details"][table_name]["primary_key"].as<std::string>()) ||
                (it->second.as<std::string>().find("timestamp") != std::string::npos))
            {
                ss << "\"" << it->first.as<std::string>() << "\", ";
            }
        }

        ss.seekp(-2, ss.cur);
        ss << ") VALUES(";
        return ss.str();
    }

    void explore_if_unknown(const std::string table_name)
    {
        if (db_config_["tables_details"][table_name])
            return;
        db_config_["tables"].push_back(table_name);
        explore_tables(db_config_["tables"]);
    }

    void open_pipeline()
    {
        try
        {
            std::shared_ptr<pqxx::work> work_buff(new pqxx::work(*c_));
            current_work_ = work_buff;
            std::shared_ptr<pqxx::pipeline> buff(new pqxx::pipeline(*current_work_));
            current_pipeline_ = buff;
        } catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    std::shared_ptr<pqxx::connection> c_;
    std::shared_ptr<pqxx::pipeline> current_pipeline_;
    std::shared_ptr<pqxx::work> current_work_;
    size_t n_connections_;

protected:
    YAML::Node db_config_;
    std::mutex mutex_;
};

}  // namespace pgi
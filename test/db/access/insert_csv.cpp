#ifndef DATA_IMPORTER_HPP
#define DATA_IMPORTER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <config.hpp>
#include <db/access/measurement.hpp>
#include <db/connector.hpp>

class data_importer
{
public:
    static void import_csv(measurement_manager &manager)
    {
        std::vector<std::string> mcc_list = {"310", "311", "312", "313", "314", "315"};
        std::string root_path = OCID_DSET_PATH;
        for (const auto &mcc : mcc_list)
        {
            std::string csv_file = root_path + "/" + mcc + ".csv";
            std::ifstream file(csv_file);
            if (!file.is_open())
            {
                throw std::runtime_error("Could not open dataset at: " + csv_file);
            }

            std::string line;
            int count = 0;
            std::cout << "Starting import from " << OCID_DSET_PATH << "..." << std::endl;

            while (std::getline(file, line))
            {
                if (line.empty())
                    continue;

                // Use stringstream to parse the line
                // Based on your previous sample: radio, mcc, mnc, lac, cellid, unit, lon, lat, range...
                std::stringstream ss(line);
                std::string field;
                measurement m;

                try
                {
                    // 1. Radio (String)
                    std::getline(ss, field, ',');
                    m.radio = field;

                    // 2. MCC (Int)
                    std::getline(ss, field, ',');
                    m.key.mcc = std::stoi(field);

                    // 3. MNC (Int)
                    std::getline(ss, field, ',');
                    m.key.mnc = std::stoi(field);

                    // 4. LAC (Int)
                    std::getline(ss, field, ',');
                    m.key.lac = std::stoi(field);

                    // 5. CellID (Int)
                    std::getline(ss, field, ',');
                    m.key.cellid = std::stoi(field);

                    // 6. Unit (Int)
                    std::getline(ss, field, ',');
                    m.stats_data.unit = std::stoi(field);

                    // 7. Lon (Double)
                    std::getline(ss, field, ',');
                    m.core_data.lon = std::stod(field);

                    // 8. Lat (Double)
                    std::getline(ss, field, ',');
                    m.core_data.lat = std::stod(field);

                    // 9. Range (Int)
                    std::getline(ss, field, ',');
                    m.core_data.range = std::stoi(field);

                    // 10. Samples (Int)
                    std::getline(ss, field, ',');
                    m.stats_data.samples = std::stoi(field);

                    // 11. Changeable (Int)
                    std::getline(ss, field, ',');
                    m.stats_data.changeable = std::stoi(field);

                    // 12. Created (Long -> ms)
                    std::getline(ss, field, ',');
                    int64_t created_ms = std::stoll(field) * 1000;
                    m.key.measured_at = created_ms;
                    m.stats_data.created_at = created_ms;

                    // 13. Updated (Long -> ms)
                    std::getline(ss, field, ',');
                    m.stats_data.updated_at = std::stoll(field) * 1000;

                    // 14. Avg Signal (Int) - Final field, no delimiter needed but safe to include
                    std::getline(ss, field, ',');
                    if (!field.empty())
                        m.stats_data.avg_signal = std::stoi(field);

                    // Execute insertion
                    measurement record = manager.get_measurement(m.key.mcc, m.key.mnc, m.key.lac, m.key.cellid, m.key.measured_at); // Test retrieval before insertion
                    std::string record_str = measurement_manager::to_string(record, false);
                    std::string expected_str = measurement_manager::to_string(m, false);
                    if (record_str != expected_str)
                    {
                        std::cout << "Record not found before insertion, as expected." << std::endl;
                        std::cout << "Inserting measurement: " << expected_str << std::endl;
                        manager.insert(m);
                    }
                    else
                    {
                        std::cout << "Record already exists before insertion: " << record_str << std::endl;
                    }

                    count++;
                    if (count % 1000 == 0)
                    {
                        std::cout << "Inserted " << count << " rows..." << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Skip malformed line " << count << ": " << e.what() << std::endl;
                }
            }

            std::cout << "Import Complete. Total rows: " << count << std::endl;
            file.close();
        }
    }
};

#endif // DATA_IMPORTER_HPP

int main()
{
    connector db;
    db.connect("172.18.0.2", "open_cell_id");
    measurement_manager manager(db);
    data_importer::import_csv(manager);
    return 0;
}
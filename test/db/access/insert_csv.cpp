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
        std::vector<std::string> mcc_list = {"310", "311", "312", "313", "314", "315", "316"};
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
                std::string temp;
                measurement m;

                try
                {
                    // 1. Radio (String)
                    ss >> m.radio;

                    // 2. MCC (Int)
                    ss >> m.key.mcc;

                    // 3. MNC (Int)
                    ss >> m.key.mnc;

                    // 4. LAC (Int)
                    ss >> m.key.lac;

                    // 5. CellID (Int)
                    ss >> m.key.cellid;

                    // 6. Unit (Int)
                    ss >> m.stats_data.unit;

                    // 7. Lon (Double)
                    ss >> m.core_data.lon;

                    // 8. Lat (Double)
                    ss >> m.core_data.lat;

                    // 9. Range (Int)
                    ss >> m.core_data.range;

                    // 10. Samples (Int)
                    ss >> m.stats_data.samples;

                    // 11. Changeable (Int)
                    ss >> m.stats_data.changeable;

                    // 12. Created (Timestamp in seconds -> convert to ms)
                    int64_t created_sec;
                    ss >> created_sec;
                    m.key.measured_at = created_sec * 1000;
                    m.stats_data.created_at = created_sec * 1000;

                    // 13. Updated (Timestamp in seconds -> ms)
                    int64_t updated_sec;
                    ss >> updated_sec;
                    m.stats_data.updated_at = updated_sec * 1000;

                    // 14. Avg Signal (Int)
                    ss >> m.stats_data.avg_signal;

                    // Execute insertion
                    manager.insert(m);

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
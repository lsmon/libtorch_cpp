#ifndef CELL_MEASUREMENT_HPP
#define CELL_MEASUREMENT_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <db/connector.hpp>
 
struct raw_measurement {
    std::string radio;      // Col 1
    int32_t mcc;            // Col 2
    int32_t mnc;            // Col 3
    int32_t lac;            // Col 4
    int32_t cellid;         // Col 5
    int32_t unit;           // Col 6
    double lon;             // Col 7
    double lat;             // Col 8
    int32_t range;          // Col 9
    int32_t samples;        // Col 10
    int32_t changeable;     // Col 11
    int64_t created;        // Col 12
    int64_t updated;        // Col 13
    int32_t avg_signal;     // Col 14
};

class raw_measurement_manager {
private:
    // Database connection and session would be members here
    connector& db;
public:
    raw_measurement_manager(connector& db_conn) : db(db_conn) {}

    void insert(const raw_measurement& m);

    std::vector<raw_measurement> get_measurements(int32_t mcc, int32_t mnc);

    void update_signal(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts, int32_t new_signal);

    void remove(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts);
};

#endif // CELL_MEASUREMENT_HPP
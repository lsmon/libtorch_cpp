#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <db/connector.hpp>

struct measurement {
    // Primary Key (Partition)
    int32_t mcc;
    int32_t mnc;
    // Primary Key (Clustering)
    int32_t lac;
    int32_t cellid;
    int64_t measured_at; // Timestamp in milliseconds

    // Location
    double lat;
    double lon;
    
    // Metadata
    std::string apiKey;
    std::string act;
    std::string devn;

    // Metrics
    int32_t signal;
    double rating;
    double speed;
    double direction;

    // Tech Specific (Optional - can be set to 0 or use std::optional)
    int32_t ta = 0, tac = 0, pci = 0, psc = 0;
    int32_t sid = 0, nid = 0, bid = 0;
    int32_t txp = 0, tsrf = 0;
};

class measurement_manager {
private:
    // Database connection and session would be members here
    connector& db;
public:
    measurement_manager(connector& db_conn) : db(db_conn) {}

    void insert(const measurement& m);

    std::vector<measurement> get_measurements(int32_t mcc, int32_t mnc);

    void update_signal(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts, int32_t new_signal);

    void remove(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts);
};

#endif // MEASUREMENT_HPP
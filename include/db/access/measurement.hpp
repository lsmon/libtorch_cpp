#ifndef MEASUREMENT_HPP
#define MEASUREMENT_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <db/connector.hpp>

struct column_names {
    const char *mcc = "mcc";
    const char *mnc = "mnc";
    const char *lac = "lac";
    const char *cellid = "cellid";
    const char *measured_at = "measured_at";
    const char *lat = "lat";
    const char *lon = "lon";
    const char *signal = "signal";
    const char *speed = "speed";
    const char *direction = "direction";
    const char *ta = "ta";
    const char *tac = "tac";
    const char *pci = "pci";
    const char *sid = "sid";
    const char *nid = "nid";
    const char *bid = "bid";
    const char *range = "range";
    const char *rating = "rating";
    const char *apikey = "apikey";
    const char *radio = "radio";
    const char *devn = "devn";
    const char *unit = "unit";
    const char *samples = "samples";
    const char *changeable = "changeable";
    const char *avg_signal = "avg_signal";
    const char *created_at = "created_at";
    const char *updated_at = "updated_at";
};

const column_names columns;

struct keys {
    int32_t mcc, mnc, lac, cellid;
    int64_t measured_at;

    keys() : mcc(0), mnc(0), lac(0), cellid(0), measured_at(0) {}
};

struct core {
    double lat, lon;
    double rating; 
    int32_t range;

    core() : lat(0), lon(0), rating(0), range(0) {}
};

struct stats {
    int32_t unit, samples, changeable, avg_signal;
    int64_t created_at, updated_at;

    stats() : unit(0), samples(0), changeable(0), avg_signal(0), created_at(0), updated_at(0) {}
};

struct signal_movement {
    int32_t signal;

    double speed, direction;

    signal_movement() : signal(0), speed(0), direction(0) {}
};

struct tech_specific {
    // 4G
    int32_t ta, tac, pci;

    // 3G
    int32_t sid, nid, bid;

    tech_specific() : ta(0), tac(0), pci(0), sid(0), nid(0), bid(0) {}
};

struct measurement {
    // PK
    keys key;

    // Core
    core core_data;

    // Strings
    std::string radio, apikey, devn;

    // Stats
    stats stats_data;

    // Signal/Movement
    signal_movement movement_data;

    // Tech Specific
    tech_specific tech;

    // Constructor to initialize everything to 0/empty
    measurement() : radio(""), apikey(""), devn("") {}
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
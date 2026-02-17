#include <db/measurement.hpp>

int main() {
    connector db;
    db.connect("172.18.0.2", "open_cell_id");

    MeasurementManager manager(db);

    // 1. Create
    measurement m;
    m.mcc = 310; m.mnc = 410; m.lac = 123; m.cellid = 456;
    m.measured_at = 1710000000000; // Current timestamp
    m.lat = 34.05; m.lon = -118.24;
    m.act = "LTE"; m.signal = -95;
    manager.insert(m);

    // 2. Read
    auto list = manager.get_measurements(310, 410);
    std::cout << "Found " << list.size() << " records for this provider." << std::endl;

    // 3. Update
    manager.update_signal(310, 410, 123, 456, 1710000000000, -80);

    // 4. Delete
    manager.remove(310, 410, 123, 456, 1710000000000);

    return 0;
}
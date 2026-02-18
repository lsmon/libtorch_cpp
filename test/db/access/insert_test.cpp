#include <db/access/measurement.hpp>

int main() {
    connector db;
    db.connect("172.18.0.2", "open_cell_id");

    measurement_manager manager(db);

    // 1. Create
    measurement m;
    m.key.mcc = 310; m.key.mnc = 410; m.key.lac = 123; m.key.cellid = 456;
    m.key.measured_at = 1710000000000; // Current timestamp
    m.core_data.lat = 34.05; m.core_data.lon = -118.24;
    m.radio = "LTE"; m.movement_data.signal = -95;
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
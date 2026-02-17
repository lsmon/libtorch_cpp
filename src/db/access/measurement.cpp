#include "db/access/measurement.hpp"

measurement_manager::insert(const measurement& m) {
    std::string cql = "INSERT INTO cell_measurements (mcc, mnc, lac, cellid, measured_at, lat, lon, apiKey, act, devn, signal, ta, tac, pci, sid, nid, bid) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    const CassPrepared* prepared = db.prepare(cql);
    CassStatement* statement = cass_prepared_bind(prepared);

    cass_statement_bind_int32(statement, 0, m.mcc);
    cass_statement_bind_int32(statement, 1, m.mnc);
    cass_statement_bind_int32(statement, 2, m.lac);
    cass_statement_bind_int32(statement, 3, m.cellid);
    cass_statement_bind_int64(statement, 4, m.measured_at);
    cass_statement_bind_double(statement, 5, m.lat);
    cass_statement_bind_double(statement, 6, m.lon);
    cass_statement_bind_string(statement, 7, m.apiKey.c_str());
    cass_statement_bind_string(statement, 8, m.act.c_str());
    cass_statement_bind_string(statement, 9, m.devn.c_str());
    cass_statement_bind_int32(statement, 10, m.signal);
    
    // Example of tech-specific binding
    cass_statement_bind_int32(statement, 11, m.ta);
    cass_statement_bind_int32(statement, 12, m.tac);
    cass_statement_bind_int32(statement, 13, m.pci);
    cass_statement_bind_int32(statement, 14, m.sid);
    cass_statement_bind_int32(statement, 15, m.nid);
    cass_statement_bind_int32(statement, 16, m.bid);

    CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    
    cass_statement_free(statement);
    cass_prepared_free(prepared);
    cass_future_free(future);
}

std::vector<measurement> measurement_manager::get_measurements(int32_t mcc, int32_t mnc) {
    std::vector<measurement> results;
    std::string query = "SELECT * FROM cell_measurements WHERE mcc = ? AND mnc = ?";
    
    CassStatement* statement = cass_statement_new(query.c_str(), 2);
    cass_statement_bind_int32(statement, 0, mcc);
    cass_statement_bind_int32(statement, 1, mnc);

    CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);

    if (cass_future_error_code(future) == CASS_OK) {
        const CassResult* result = cass_future_get_result(future);
        CassIterator* iterator = cass_iterator_from_result(result);

        while (cass_iterator_next(iterator)) {
            const CassRow* row = cass_iterator_get_row(iterator);
            measurement m;
            // Extracting data from row
            cass_value_get_int32(cass_row_get_column_by_name(row, "mcc"), &m.mcc);
            cass_value_get_int32(cass_row_get_column_by_name(row, "mnc"), &m.mnc);
            cass_value_get_int32(cass_row_get_column_by_name(row, "lac"), &m.lac);
            cass_value_get_int32(cass_row_get_column_by_name(row, "cellid"), &m.cellid);
            cass_value_get_int64(cass_row_get_column_by_name(row, "measured_at"), &m.measured_at);
            cass_value_get_double(cass_row_get_column_by_name(row, "lat"), &m.lat);
            cass_value_get_double(cass_row_get_column_by_name(row, "lon"), &m.lon);
            cass_value_get_string(cass_row_get_column_by_name(row, "apiKey"), &m.apiKey);
            cass_value_get_string(cass_row_get_column_by_name(row, "act"), &m.act);
            cass_value_get_string(cass_row_get_column_by_name(row, "devn"), &m.devn);
            cass_value_get_int32(cass_row_get_column_by_name(row, "signal"), &m.signal);
            cass_value_get_int32(cass_row_get_column_by_name(row, "ta"), &m.ta);
            cass_value_get_int32(cass_row_get_column_by_name(row, "tac"), &m.tac);
            cass_value_get_int32(cass_row_get_column_by_name(row, "pci"), &m.pci);
            cass_value_get_int32(cass_row_get_column_by_name(row, "sid"), &m.sid);
            cass_value_get_int32(cass_row_get_column_by_name(row, "nid"), &m.nid);
            cass_value_get_int32(cass_row_get_column_by_name(row, "bid"), &m.bid);
            results.push_back(m);
        }
        cass_result_free(result);
        cass_iterator_free(iterator);
    }
    
    cass_future_free(future);
    cass_statement_free(statement);
    return results;
}

void measurement_manager::update_signal(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts, int32_t new_signal) {
    std::string query = "UPDATE cell_measurements SET signal = ? WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND measured_at = ?";
    CassStatement* statement = cass_statement_new(query.c_str(), 6);
    cass_statement_bind_int32(statement, 0, new_signal);
    cass_statement_bind_int32(statement, 1, mcc);
    cass_statement_bind_int32(statement, 2, mnc);
    cass_statement_bind_int32(statement, 3, lac);
    cass_statement_bind_int32(statement, 4, cellid);
    cass_statement_bind_int64(statement, 5, ts);
    CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
}

void measurement_manager::remove(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts) {
    std::string query = "DELETE FROM cell_measurements WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND measured_at = ?";
    CassStatement* statement = cass_statement_new(query.c_str(), 5);
    cass_statement_bind_int32(statement, 0, mcc);
    cass_statement_bind_int32(statement, 1, mnc);
    cass_statement_bind_int32(statement, 2, lac);
    cass_statement_bind_int32(statement, 3, cellid);
    cass_statement_bind_int64(statement, 4, ts);
    CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
}
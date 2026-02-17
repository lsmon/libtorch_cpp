#include "db/access/raw_measurement.hpp"
#include <cassandra.h>

void raw_measurement_manager::insert(const raw_measurement &m)
{
    std::string cql = "INSERT INTO cell_measurements (radio, mcc, mnc, lac, cellid, unit, lon, lat, range, samples, changeable, created_at, updated_at, avg_signal) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

    const CassPrepared* prepared = db.prepare(cql);
    CassStatement* statement = cass_prepared_bind(prepared);

    cass_statement_bind_string(statement, 0, m.radio.c_str());
    cass_statement_bind_int32(statement, 1, m.mcc);
    cass_statement_bind_int32(statement, 2, m.mnc);
    cass_statement_bind_int32(statement, 3, m.lac);
    cass_statement_bind_int32(statement, 4, m.cellid);
    cass_statement_bind_int32(statement, 5, m.unit);
    cass_statement_bind_double(statement, 6, m.lon);
    cass_statement_bind_double(statement, 7, m.lat);
    cass_statement_bind_int32(statement, 8, m.range);
    cass_statement_bind_int32(statement, 9, m.samples);
    cass_statement_bind_int32(statement, 10, m.changeable);
    cass_statement_bind_int64(statement, 11, m.created * 1000); // Sec to Ms
    cass_statement_bind_int64(statement, 12, m.updated * 1000); // Sec to Ms
    cass_statement_bind_int32(statement, 13, m.avg_signal);

    CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    
    cass_statement_free(statement);
    cass_prepared_free(prepared);
    cass_future_free(future);
}

std::vector<raw_measurement> raw_measurement_manager::get_measurements(int32_t mcc, int32_t mnc) {
    std::vector<raw_measurement> results;
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
            raw_measurement m;
            // Extracting data from row
            cass_value_get_string(cass_row_get_column_by_name(row, "radio"), &m.radio);
            cass_value_get_int32(cass_row_get_column_by_name(row, "mcc"), &m.mcc);
            cass_value_get_int32(cass_row_get_column_by_name(row, "mnc"), &m.mnc);
            cass_value_get_int32(cass_row_get_column_by_name(row, "lac"), &m.lac);
            cass_value_get_int32(cass_row_get_column_by_name(row, "cellid"), &m.cellid);
            cass_value_get_int32(cass_row_get_column_by_name(row, "unit"), &m.unit);
            cass_value_get_double(cass_row_get_column_by_name(row, "lon"), &m.lon);
            cass_value_get_double(cass_row_get_column_by_name(row, "lat"), &m.lat);
            cass_value_get_int32(cass_row_get_column_by_name(row, "range"), &m.range);
            cass_value_get_int32(cass_row_get_column_by_name(row, "samples"), &m.samples);
            cass_value_get_int32(cass_row_get_column_by_name(row, "changeable"), &m.changeable);
            cass_value_get_int64(cass_row_get_column_by_name(row, "created_at"), &m.created);
            cass_value_get_int64(cass_row_get_column_by_name(row, "updated_at"), &m.updated);
            cass_value_get_int32(cass_row_get_column_by_name(row, "avg_signal"), &m.avg_signal);

            results.push_back(m);
        }
        cass_result_free(result);
        cass_iterator_free(iterator);
    }

    cass_future_free(future);
    cass_statement_free(statement);
    return results;
}

void raw_measurement_manager::update_signal(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts, int32_t new_signal) {
    std::string cql = "UPDATE cell_measurements SET avg_signal = ? WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND created_at = ?";
    
    const CassPrepared* prepared = db.prepare(cql);
    CassStatement* statement = cass_prepared_bind(prepared);

    cass_statement_bind_int32(statement, 0, new_signal);
    cass_statement_bind_int32(statement, 1, mcc);
    cass_statement_bind_int32(statement, 2, mnc);
    cass_statement_bind_int32(statement, 3, lac);
    cass_statement_bind_int32(statement, 4, cellid);
    cass_statement_bind_int64(statement, 5, ts * 1000); // Sec to Ms

     CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
    cass_prepared_free(prepared);
}

void raw_measurement_manager::remove(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts) {
    std::string cql = "DELETE FROM cell_measurements WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND created_at = ?";
    
    const CassPrepared* prepared = db.prepare(cql);
    CassStatement* statement = cass_prepared_bind(prepared);

    cass_statement_bind_int32(statement, 0, mcc);
    cass_statement_bind_int32(statement, 1, mnc);
    cass_statement_bind_int32(statement, 2, lac);
    cass_statement_bind_int32(statement, 3, cellid);
    cass_statement_bind_int64(statement, 4, ts * 1000); // Sec to Ms

     CassFuture* future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
    cass_prepared_free(prepared);
}
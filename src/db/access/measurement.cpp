#include "db/access/measurement.hpp"
#include "db/connector.hpp"

void measurement_manager::insert(const measurement &m)
{
    if (m.key.mcc == 0 || m.key.mnc == 0 || m.key.lac == 0 || m.key.cellid == 0 || m.key.measured_at == 0)
    {
        throw std::invalid_argument("Missing required fields for measurement insertion");
    }

    int field_count = 5; // Start with required fields
    std::string cql = "INSERT INTO measurements ( ";
    cql += "mcc, mnc, lac, cellid, measured_at, ";
    if (m.core_data.lat != 0)
    {
        cql += "lat, ";
        field_count++;
    }
    if (m.core_data.lon != 0)
    {
        cql += "lon, ";
        field_count++;
    }
    if (m.core_data.rating != 0)
    {
        cql += "rating, ";
        field_count++;
    }
    if (m.core_data.range != 0)
    {
        cql += "range, ";
        field_count++;
    }
    if (!m.apikey.empty())
    {
        cql += "apikey, ";
        field_count++;
    }
    if (!m.radio.empty())
    {
        cql += "radio, ";
        field_count++;
    }
    if (!m.devn.empty())
    {
        cql += "devn, ";
        field_count++;
    }
    if (m.stats_data.unit != 0)
    {
        cql += "unit, ";
        field_count++;
    }
    if (m.stats_data.samples != 0)
    {
        cql += "samples, ";
        field_count++;
    }
    if (m.stats_data.changeable != 0)
    {
        cql += "changeable, ";
        field_count++;
    }
    if (m.stats_data.avg_signal != 0)
    {
        cql += "avg_signal, ";
        field_count++;
    }
    if (m.stats_data.created_at != 0)
    {
        cql += "created_at, ";
        field_count++;
    }
    if (m.stats_data.updated_at != 0)
    {
        cql += "updated_at, ";
        field_count++;
    }
    if (m.movement_data.signal != 0)
    {
        cql += "signal, ";
        field_count++;
    }
    if (m.movement_data.speed != 0)
    {
        cql += "speed, ";
        field_count++;
    }
    if (m.movement_data.direction != 0)
    {
        cql += "direction, ";
        field_count++;
    }
    if (m.tech.tac != 0)
    {
        cql += "tac, ";
        field_count++;
    }
    if (m.tech.pci != 0)
    {
        cql += "pci, ";
        field_count++;
    }
    if (m.tech.sid != 0)
    {
        cql += "sid, ";
        field_count++;
    }
    if (m.tech.nid != 0)
    {
        cql += "nid, ";
        field_count++;
    }
    if (m.tech.bid != 0)
    {
        cql += "bid, ";
        field_count++;
    }
    cql.pop_back(); // Remove last space
    cql.pop_back(); // Remove last comma
    cql += ") VALUES (";
    for (size_t i = 0; i < field_count; i++)
    {
        cql += "?";
        if (i < field_count - 1)
            cql += ", ";
    }
    cql += ")";

    const CassPrepared *prepared = db.prepare_query(cql);
    CassStatement *statement = cass_prepared_bind(prepared);

    cass_statement_bind_int32_by_name(statement, columns.mcc, m.key.mcc);
    cass_statement_bind_int32_by_name(statement, columns.mnc, m.key.mnc);
    cass_statement_bind_int32_by_name(statement, columns.lac, m.key.lac);
    cass_statement_bind_int32_by_name(statement, columns.cellid, m.key.cellid);
    cass_statement_bind_int64_by_name(statement, columns.measured_at, m.key.measured_at);

    if (m.core_data.lat != 0)
        cass_statement_bind_double_by_name(statement, columns.lat, m.core_data.lat);
    if (m.core_data.lon != 0)
        cass_statement_bind_double_by_name(statement, columns.lon, m.core_data.lon);
    if (m.core_data.rating != 0)
        cass_statement_bind_double_by_name(statement, columns.rating, m.core_data.rating);
    if (m.core_data.range != 0)
        cass_statement_bind_int32_by_name(statement, columns.range, m.core_data.range);
    if (!m.apikey.empty())
        cass_statement_bind_string_by_name(statement, columns.apikey, m.apikey.c_str());
    if (!m.radio.empty())
        cass_statement_bind_string_by_name(statement, columns.radio, m.radio.c_str());
    if (!m.devn.empty())
        cass_statement_bind_string_by_name(statement, columns.devn, m.devn.c_str());
    if (m.stats_data.unit != 0)
        cass_statement_bind_int32_by_name(statement, columns.unit, m.stats_data.unit);
    if (m.stats_data.samples != 0)
        cass_statement_bind_int32_by_name(statement, columns.samples, m.stats_data.samples);
    if (m.stats_data.changeable != 0)
        cass_statement_bind_int32_by_name(statement, columns.changeable, m.stats_data.changeable);
    if (m.stats_data.avg_signal != 0)
        cass_statement_bind_int32_by_name(statement, columns.avg_signal, m.stats_data.avg_signal);
    if (m.stats_data.created_at != 0)
        cass_statement_bind_int64_by_name(statement, columns.created_at, m.stats_data.created_at);
    if (m.stats_data.updated_at != 0)
        cass_statement_bind_int64_by_name(statement, columns.updated_at, m.stats_data.updated_at);
    if (m.movement_data.signal != 0)
        cass_statement_bind_int32_by_name(statement, columns.signal, m.movement_data.signal);
    if (m.movement_data.speed != 0)
        cass_statement_bind_double_by_name(statement, columns.speed, m.movement_data.speed);
    if (m.movement_data.direction != 0)
        cass_statement_bind_double_by_name(statement, columns.direction, m.movement_data.direction);
    if (m.tech.ta != 0)
        cass_statement_bind_int32_by_name(statement, columns.ta, m.tech.ta);
    if (m.tech.tac != 0)
        cass_statement_bind_int32_by_name(statement, columns.tac, m.tech.tac);
    if (m.tech.pci != 0)
        cass_statement_bind_int32_by_name(statement, columns.pci, m.tech.pci);
    if (m.tech.sid != 0)
        cass_statement_bind_int32_by_name(statement, columns.sid, m.tech.sid);
    if (m.tech.nid != 0)
        cass_statement_bind_int32_by_name(statement, columns.nid, m.tech.nid);
    if (m.tech.bid != 0)
        cass_statement_bind_int32_by_name(statement, columns.bid, m.tech.bid);

    CassFuture *future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);

    cass_statement_free(statement);
    cass_prepared_free(prepared);
    cass_future_free(future);
}

std::vector<measurement> measurement_manager::get_measurements(int32_t mcc, int32_t mnc)
{
    std::vector<measurement> results;
    std::string query = "SELECT * FROM measurements WHERE mcc = ? AND mnc = ?";

    CassStatement *statement = cass_statement_new(query.c_str(), 2);
    cass_statement_bind_int32_by_name(statement, columns.mcc, mcc);
    cass_statement_bind_int32_by_name(statement, columns.mnc, mnc);

    CassFuture *future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);

    if (cass_future_error_code(future) == CASS_OK)
    {
        const CassResult *result = cass_future_get_result(future);
        CassIterator *iterator = cass_iterator_from_result(result);

        while (cass_iterator_next(iterator))
        {
            const CassRow *row = cass_iterator_get_row(iterator);
            measurement m;
            // Extracting data from row
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.mcc), &m.key.mcc);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.mnc), &m.key.mnc);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.lac), &m.key.lac);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.cellid), &m.key.cellid);
            cass_value_get_int64(cass_row_get_column_by_name(row, columns.measured_at), &m.key.measured_at);
            cass_value_get_double(cass_row_get_column_by_name(row, columns.lat), &m.core_data.lat);
            cass_value_get_double(cass_row_get_column_by_name(row, columns.lon), &m.core_data.lon);

            // 1. Get the CassValue pointer
            const CassValue *radio_val = cass_row_get_column_by_name(row, columns.radio);
            // 2. Check if the column exists AND is not null
            if (radio_val != nullptr && !cass_value_is_null(radio_val))
            {
                const char *radio_ptr;
                size_t radio_len;
                // 3. Now it is safe to extract the data
                cass_value_get_string(radio_val, &radio_ptr, &radio_len);
                m.radio = std::string(radio_ptr, radio_len);
            }
            else
            {
                // 4. Handle the NULL case (e.g., set to empty string)
                m.radio = "";
            }

            // 1. Get the CassValue pointer
            const CassValue *api_key_val = cass_row_get_column_by_name(row, columns.apikey);
            // 2. Check if the column exists AND is not null
            if (api_key_val != nullptr && !cass_value_is_null(api_key_val))
            {
                const char *api_key_ptr;
                size_t api_key_len;
                // 3. Now it is safe to extract the data
                cass_value_get_string(api_key_val, &api_key_ptr, &api_key_len);
                m.apikey = std::string(api_key_ptr, api_key_len);
            }
            else
            {
                // 4. Handle the NULL case (e.g., set to empty string)
                m.apikey = "";
            }

            // 1. Get the CassValue pointer
            const CassValue *devn_val = cass_row_get_column_by_name(row, columns.devn);
            // 2. Check if the column exists AND is not null
            if (devn_val != nullptr && !cass_value_is_null(devn_val))
            {
                const char *devn_ptr;
                size_t devn_len;
                // 3. Now it is safe to extract the data
                cass_value_get_string(devn_val, &devn_ptr, &devn_len);
                m.devn = std::string(devn_ptr, devn_len);
            }
            else
            {
                // 4. Handle the NULL case (e.g., set to empty string)
                m.devn = "";
            }

            cass_value_get_int32(cass_row_get_column_by_name(row, columns.ta), &m.tech.ta);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.tac), &m.tech.tac);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.pci), &m.tech.pci);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.sid), &m.tech.sid);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.nid), &m.tech.nid);
            cass_value_get_int32(cass_row_get_column_by_name(row, columns.bid), &m.tech.bid);
            results.push_back(m);
        }
        cass_result_free(result);
        cass_iterator_free(iterator);
    }

    cass_future_free(future);
    cass_statement_free(statement);
    return results;
}

void measurement_manager::update_signal(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts, int32_t new_signal)
{
    std::string query = "UPDATE measurements SET signal = ? WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND measured_at = ?";
    CassStatement *statement = cass_statement_new(query.c_str(), 6);
    cass_statement_bind_int32_by_name(statement, columns.signal, new_signal);
    cass_statement_bind_int32_by_name(statement, columns.mcc, mcc);
    cass_statement_bind_int32_by_name(statement, columns.mnc, mnc);
    cass_statement_bind_int32_by_name(statement, columns.lac, lac);
    cass_statement_bind_int32_by_name(statement, columns.cellid, cellid);
    cass_statement_bind_int64_by_name(statement, columns.measured_at, ts);
    CassFuture *future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
}

void measurement_manager::remove(int32_t mcc, int32_t mnc, int32_t lac, int32_t cellid, int64_t ts)
{
    std::string query = "DELETE FROM measurements WHERE mcc = ? AND mnc = ? AND lac = ? AND cellid = ? AND measured_at = ?";
    CassStatement *statement = cass_statement_new(query.c_str(), 5);
    cass_statement_bind_int32_by_name(statement, columns.mcc, mcc);
    cass_statement_bind_int32_by_name(statement, columns.mnc, mnc);
    cass_statement_bind_int32_by_name(statement, columns.lac, lac);
    cass_statement_bind_int32_by_name(statement, columns.cellid, cellid);
    cass_statement_bind_int64_by_name(statement, columns.measured_at, ts);
    CassFuture *future = cass_session_execute(db.get_session(), statement);
    cass_future_wait(future);
    cass_future_free(future);
    cass_statement_free(statement);
}
void json_helper::to_json(json &j, const keys &k)
{
    j = json{{"mcc", k.mcc}, {"mnc", k.mnc}, {"lac", k.lac}, {"cellid", k.cellid}, {"measured_at", k.measured_at}};
}

void json_helper::to_json(json &j, const core &c)
{
    j = json{{"lat", c.lat}, {"lon", c.lon}, {"rating", c.rating}, {"range", c.range}};
}

void json_helper::to_json(json &j, const stats &s)
{
    j = json{{"unit", s.unit}, {"samples", s.samples}, {"changeable", s.changeable}, {"avg_signal", s.avg_signal}, {"created_at", s.created_at}, {"updated_at", s.updated_at}};
}

void json_helper::to_json(json &j, const signal_movement &sm)
{
    j = json{{"signal", sm.signal}, {"speed", sm.speed}, {"direction", sm.direction}};
}

void json_helper::to_json(json &j, const tech_specific &t)
{
    j = json{{"ta", t.ta}, {"tac", t.tac}, {"pci", t.pci}, {"sid", t.sid}, {"nid", t.nid}, {"bid", t.bid}};
}

void json_helper::to_json(json &j, const measurement &m)
{
    // Map the structures
    j = json{
        {"mcc", m.key.mcc},
        {"mnc", m.key.mnc},
        {"lac", m.key.lac},
        {"cellid", m.key.cellid},
        {"measured_at", m.key.measured_at},
        {"lat", m.core_data.lat},
        {"lon", m.core_data.lon},
        {"rating", m.core_data.rating},
        {"range", m.core_data.range},

        {"apikey", m.apikey},
        {"radio", m.radio},
        {"devn", m.devn},

        {"unit", m.stats_data.unit},
        {"samples", m.stats_data.samples},
        {"changeable", m.stats_data.changeable},
        {"avg_signal", m.stats_data.avg_signal},
        {"created_at", m.stats_data.created_at},
        {"updated_at", m.stats_data.updated_at},

        {"signal", m.movement_data.signal},
        {"speed", m.movement_data.speed},
        {"direction", m.movement_data.direction},

        {"ta", m.tech.ta},
        {"tac", m.tech.tac},
        {"pci", m.tech.pci},
        {"sid", m.tech.sid},
        {"nid", m.tech.nid},
        {"bid", m.tech.bid}
    };
}

std::string json_helper::to_string(const measurement &m, bool prettyPrint)
{
    json j = json::object();
    this->to_json(j, m);
    if (prettyPrint) {
        return j.dump(4); // 4 spaces indentation
    }
    return j.dump(); // Minified
}

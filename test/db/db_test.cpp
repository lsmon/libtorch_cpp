#include "db/connector.hpp"

int main() {
    try {
        connector db;
        
        // 1. Connect
        db.connect("172.18.0.2");
        std::cout << "Connected!" << std::endl;

        // 2. Setup Environment
        db.execute_query("CREATE KEYSPACE IF NOT EXISTS dev_ks WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1};");
        db.execute_query("CREATE TABLE IF NOT EXISTS dev_ks.test_table (id int PRIMARY KEY, val text);");

        // 3. Use Prepared Statement for efficiency
        const CassPrepared* prepared = db.prepare_query("INSERT INTO dev_ks.test_table (id, val) VALUES (?, ?);");

        // 4. Bind and Execute
        CassStatement* statement = cass_prepared_bind(prepared);
        cass_statement_bind_int32(statement, 0, 101);
        cass_statement_bind_string(statement, 1, "Hello Cassandra 5.0");

        CassFuture* future = cass_session_execute(db.get_session(), statement);
        cass_future_wait(future);
        
        if (cass_future_error_code(future) == CASS_OK) {
            std::cout << "Data inserted successfully!" << std::endl;
        }

        // Cleanup prepared statement
        cass_future_free(future);
        cass_statement_free(statement);
        cass_prepared_free(prepared);

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

#include "db/connector.hpp"

void connector::check_future_error(CassFuture *future, const std::string &error_prefix)
{
    cass_future_wait(future);
    CassError code = cass_future_error_code(future);
    if (code != CASS_OK) {
        const char* message;
        size_t message_length;
        cass_future_error_message(future, &message, &message_length);
        std::string err(message, message_length);
        throw std::runtime_error(error_prefix + " | Reason: " + err);
    }    
}

connector::~connector()
{
    if (session) {
        CassFuture* close_future = cass_session_close(session);
        cass_future_wait(close_future);
        cass_future_free(close_future);
        cass_session_free(session);
    }
    if (cluster) {
        cass_cluster_free(cluster);
    }
}

void connector::connect(const std::string &hosts, const std::string &keyspace)
{
    cass_cluster_set_contact_points(cluster, hosts.c_str());

    CassFuture* connect_future = nullptr;
    if (keyspace.empty()) {
        connect_future = cass_session_connect(session, cluster);
    } else {
        connect_future = cass_session_connect_keyspace(session, cluster, keyspace.c_str());
    }

    check_future_error(connect_future, "Failed to connect to Cassandra");
    cass_future_free(connect_future);
}

void connector::execute_query(const std::string &query)
{
    CassStatement* statement = cass_statement_new(query.c_str(), 0);
    CassFuture* result_future = cass_session_execute(session, statement);
        
    check_future_error(result_future, "Query execution failed: " + query);
        
    cass_statement_free(statement);
    cass_future_free(result_future);
}

const CassPrepared *connector::prepare_query(const std::string &query)
{
    CassFuture* prepare_future = cass_session_prepare(session, query.c_str());
    check_future_error(prepare_future, "Failed to prepare statement");
        
    const CassPrepared* prepared = cass_future_get_prepared(prepare_future);
    cass_future_free(prepare_future);
    return prepared;
}

CassSession *connector::get_session() const
{
    return session;
}

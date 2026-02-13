#ifndef CASSANDRA_CONNECTOR_HPP
#define CASSANDRA_CONNECTOR_HPP

#include <cassandra.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

class connector {
private:
    CassCluster* cluster;
    CassSession* session;

    void check_future_error(CassFuture* future, const std::string& error_prefix);

public:
    connector() : cluster(nullptr), session(nullptr) 
    {
        cluster = cass_cluster_new();
        session = cass_session_new();
    }

    ~connector();

    void connect(const std::string& hosts, const std::string& keyspace = "");

    void execute_query(const std::string& query);

    const CassPrepared* prepare_query(const std::string& query);

    CassSession* get_session() const;
};

#endif // CASSANDRA_CONNECTOR_HPP
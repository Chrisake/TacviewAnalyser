#include "data_service.hpp"

#include "global.hpp"

std::unique_ptr<DataService> data_service;

std::unique_ptr<pqxx::connection> DataService::CreateConnection() {
  std::string connectionStr = std::format(
      "dbname={} user={} password={} "
      "hostaddr={} port={}",
      dbname, user, password, hostaddr, port);
  std::unique_ptr<pqxx::connection> cx = std::make_unique<pqxx::connection>(connectionStr);
  if (!cx->is_open()) {
    return nullptr;
  }
  return cx;
}

DataService::DataService(std::string_view dbname,
                         std::string_view user,
                         std::string_view password,
                         std::string_view hostaddr,
                         uint16_t         port)
    : dbname(dbname), user(user), password(password), hostaddr(hostaddr), port(port) {}

DataService::~DataService() {
  for (const auto &con : connection_pool) {
    con->close();
  }
}

void DataService::Init(uint16_t count) {
  for (int i = 0; i < count; ++i) {
    std::unique_ptr<pqxx::connection> cx = CreateConnection();
    if (cx) {
      connection_queue.push(cx.get());
      connection_pool.push_back(std::move(cx));
    } else {
      std::cerr << "Failed to create connection." << std::endl;
      return;
    }
  }
}

bool DataService::Test() {
  try {
    std::unique_ptr<pqxx::connection> cx(CreateConnection());
    if (cx) {
      std::cout << "Connected to the database." << std::endl;
    } else {
      std::cerr << "Failed to connect to the database." << std::endl;
      return false;
    }
    cx->close();
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return false;
  }
}

void DataService::ReleaseConnection(pqxx::connection *conn) {
  std::scoped_lock lock(connection_queue_lock);
  connection_queue.push(conn);
}

pqxx::connection *DataService::GetConnection() {
  std::scoped_lock lock(connection_queue_lock);
  if (connection_queue.empty()) {
    return nullptr;
  } else {
    pqxx::connection *conn = connection_queue.front();
    connection_queue.pop();
    return conn;
  }
}

std::unique_ptr<pqxx::result> DataService::ExecuteQuery(const std::string &query, const pqxx::params &params) {
  auto cx = GetConnection();
  if (!cx) {
    std::cerr << "Failed to get a connection from the pool." << std::endl;
    return nullptr;
  }
  std::unique_ptr<pqxx::result> res;
  try {
    pqxx::work tx{*cx};
    res = std::make_unique<pqxx::result>(tx.exec(query, params));
    tx.commit();
  } catch (const std::exception &e) {
    std::cerr << "Query execution failed: " << e.what() << std::endl;
  }
  ReleaseConnection(cx);
  return res;
}

#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <queue>
#include <string_view>
#include <vector>

class DataService {
 private:
  // Database connection parameters
  std::string                                    dbname;
  std::string                                    user;
  std::string                                    password;
  std::string                                    hostaddr;
  uint16_t                                       port;
  std::vector<std::unique_ptr<pqxx::connection>> connection_pool;
  std::queue<pqxx::connection *>                 connection_queue;
  std::mutex                                     connection_queue_lock;

  std::unique_ptr<pqxx::connection> CreateConnection();

 public:
  DataService() = default;
  DataService(std::string_view dbname,
              std::string_view user,
              std::string_view password,
              std::string_view hostaddr,
              uint16_t         port);
  ~DataService();

  void                          Init(uint16_t count);
  bool                          Test();
  void                          ReleaseConnection(pqxx::connection *conn);
  pqxx::connection             *GetConnection();
  std::unique_ptr<pqxx::result> ExecuteQuery(const std::string &query, const pqxx::params &params = {});
};

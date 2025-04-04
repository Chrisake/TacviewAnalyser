#include "TAWAServer.hpp"

#include "global.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void api::v1::Pilot::getPilots(
    [[maybe_unused]] const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback) const {
  auto qres = data_service->ExecuteQuery("SELECT t.* FROM main.pilot t LIMIT 501");
  if (!qres) {
    auto resp = HttpResponse::newHttpResponse(
        drogon::HttpStatusCode::k500InternalServerError,
        drogon::CT_APPLICATION_JSON);
    json res = {
        {"result", "error"},
        {"message", "Internal Server Error"},
    };
    resp->setBody(res.dump());
    callback(resp);
    return;
  } else {
    json pilotArray = {};
    for (auto row : *qres) {
      json pilot = {
          {"id", row["id"].as<int>()},
          {"username", row["username"].as<std::string>()}
      };
      pilotArray.push_back(pilot);
    }
    json res = {
        {"result", "ok"},
        {"pilots", pilotArray},
    };
    auto resp = HttpResponse::newHttpResponse(drogon::HttpStatusCode::k200OK,
                                              drogon::CT_APPLICATION_JSON);
    resp->setBody(res.dump());
    callback(resp);
    return;
  }
}

#include "TAWAServer.hpp"

void api::v1::User::getInfo(
    [[maybe_unused]] const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback, int userId) const {
  Json::Value json;
  json["result"] = "ok";
  json["message"] = "GetInfo: UserId[" + std::to_string(userId) + "]";
  auto resp = HttpResponse::newHttpJsonResponse(json);
  callback(resp);
}

void api::v1::User::getDetailInfo(
    [[maybe_unused]] const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback, int userId) const {
  Json::Value json;
  json["result"] = "ok";
  json["message"] = "GetDetailInfo: UserId[" + std::to_string(userId) + "]";
  auto resp = HttpResponse::newHttpJsonResponse(json);
  callback(resp);
}

void api::v1::User::newUser(
    [[maybe_unused]] const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    std::string&& userName) {
  Json::Value json;
  json["result"] = "ok";
  json["message"] = "NewUser";
  json["username"] = userName;
  json["userId"] = rand() % 1000;
  auto resp = HttpResponse::newHttpJsonResponse(json);
  callback(resp);
}

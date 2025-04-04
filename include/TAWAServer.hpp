#pragma once
#include <drogon/HttpController.h>

using namespace drogon;
namespace api {
namespace v1 {
class Pilot : public drogon::HttpController<Pilot> {
 public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(Pilot::getPilots, "/", Get);  // path is /api/v1/Pilot/
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void getPilots(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback) const;

 public:
  Pilot() = default;
};
}  // namespace v1
}  // namespace api
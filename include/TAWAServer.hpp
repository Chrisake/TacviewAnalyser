#pragma once
#include <drogon/HttpController.h>

using namespace drogon;
namespace api {
namespace v1 {
class User : public drogon::HttpController<User> {
 public:
  METHOD_LIST_BEGIN
  // use METHOD_ADD to add your custom processing function here;
  METHOD_ADD(User::getInfo, "/{id}", Get);  // path is /api/v1/User/{arg1}
  METHOD_ADD(User::getDetailInfo, "/{id}/detailinfo",
             Get);  // path is /api/v1/User/{arg1}/detailinfo
  METHOD_ADD(User::newUser, "/{name}", Post);  // path is /api/v1/User/{arg1}
  METHOD_LIST_END
  // your declaration of processing function maybe like this:
  void getInfo(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback,
               int userId) const;
  void getDetailInfo(const HttpRequestPtr &req,
                     std::function<void(const HttpResponsePtr &)> &&callback,
                     int userId) const;
  void newUser(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback,
               std::string &&userName);

 public:
  User() { LOG_DEBUG << "User constructor!"; }
};
}  // namespace v1
}  // namespace api
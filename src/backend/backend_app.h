
#pragma once

#include "application.h"

#include "db.h"
#include "db/dao.h"
#include "db/json_serializer.h"
#include "db/model_relations.h"
#include "model_meta.h"
#include <crow.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

using json_pointer = nlohmann::json::json_pointer;
using namespace std::literals::string_literals;

namespace Meta = Sphinx::Db::Meta;

namespace Sphinx::Backend {

class BackendApp : public Application {

public:
  using Application::Application;

  BackendApp(const std::string &application_name,
             const std::vector<std::string> &args);

  int run() override;

protected:
  Sphinx::Db::connection_config prepare_db_config();

  Sphinx::Backend::Db::DAO dao_;

  crow::SimpleApp app_;

  const json_pointer REST_API_PORT_PATH = "/server/port"_json_pointer;
  const json_pointer REST_API_VERSION_PATH = "/server/version"_json_pointer;
  const json_pointer DUMP_LEVEL_PATH = "/debug/dump_level"_json_pointer;
  const std::uint16_t REST_API_PORT;
  const std::string REST_API_VERSION;

  const int dump_indent_;

private:
  template <typename... Methods>
  auto &add_route(const std::string &path, const Methods... methods)
  {
    const auto route = REST_API_VERSION + path;
    logger()->debug("Adding route {} (methods:{})", route,
                    ((" "s + crow::method_name(methods)) + ...));
    return app_.route_dynamic(REST_API_VERSION + path).methods(methods...);
  }
  void add_users_routes();
  void add_courses_routes();
  void add_modules_routes();
  void add_test_routes();

  std::string get_users();
  std::string get_courses();
  std::string get_modules();
  std::string get_modules(typename Meta::IdColumn_t<Model::Course> course_id);

  //----------------------------------------------------------------------
  template <typename T>
  bool is_entity_exists(typename Meta::IdColumn<T>::type entity_id)
  {
    return dao_.exists<T>(entity_id);
  }
  //----------------------------------------------------------------------
  template <typename T>
  T get_entity(typename Meta::IdColumn<T>::type entity_id)
  {
    auto entity = dao_.get_by_id<T>(entity_id);
    return entity;
  }
  //----------------------------------------------------------------------
  template <typename T>
  T update_entity(typename Meta::IdColumn<T>::type /* entity_id */,
                  const nlohmann::json & /* entity_json */)
  {
    NOT_IMPLEMENTED_YET();
  }
  //----------------------------------------------------------------------
  template <typename T>
  Meta::IdColumn_t<T> create_entity(const T & /* entity */)
  {
    static_assert(assert_false<T>::value, "Not implemented");
  }

  //----------------------------------------------------------------------

  template <typename Entity>
  void update_subentities(Entity &entity, Meta::IdColumn_t<Entity> id)
  {
    auto subentities_links = entity.get_many_links();
    auto func = [&id](auto &subentities2) {
      using RemoteKey =
          typename Sphinx::Db::LinkMany<decltype(subentities2)>::remote_key;
      constexpr auto n = RemoteKey::n;
      if (subentities2) {
        for (auto &subentity : *subentities2) {
          std::get<n>(subentity.get_columns()).value = id;
        }
      }
    };
    Sphinx::Utils::for_each_in_tuple(subentities_links, func);
  }

  //----------------------------------------------------------------------
  template <typename Entity, typename Func>
  void for_each_subentity(Entity &entity, Func &&func)
  {
    auto subentities_links = entity.get_many_links();
    Sphinx::Utils::for_each_in_tuple(
        subentities_links, [func = std::move(func)](auto &subentities) {
          if (subentities)
            func(*subentities);
        });
  }
  //----------------------------------------------------------------------
  template <typename Entity>
  void create_subentities(Entity &entity)
  {
    auto func = [this](auto &subentities3) {
      this->create_entities(subentities3);
    };
    for_each_subentity(entity, func);
  }
  //----------------------------------------------------------------------
  template <typename T>
  void create_entities(std::vector<T> &entities)
  {
    std::for_each(entities.begin(), entities.end(), [this](auto &entity) {
      auto entity_id = this->create_entity(entity);
      this->update_subentities(entity, entity_id);
      this->create_subentities(entity);

    });
  }
  //----------------------------------------------------------------------
  template <typename T>
  void create_entities(const nlohmann::json &data)
  {
    auto entities = deserialize_entities<T>(data, true);
    create_entities(entities);
  }

  //----------------------------------------------------------------------
  template <typename T>
  void create_entities(const std::string &data)
  {
    return create_entities<T>(nlohmann::json::parse(data));
  }

  //----------------------------------------------------------------------
  template <typename T>
  void deserialize_subentities(const nlohmann::json &data, T &entity)
  {
    auto links = entity.get_many_links();

    auto func = [this, &data, &entity](auto &link) {
      using Sphinx::Db::LinkMany;
      constexpr auto field_name = LinkMany<decltype(link)>::name;
      if (data.count(field_name) == 0) {
        link = std::nullopt;
      }
      else {
        using RemoteEntity = typename LinkMany<decltype(link)>::remote_entity;
        link = this->deserialize_entities<RemoteEntity>(data[field_name]);
      }
    };
    Sphinx::Utils::for_each_in_tuple(links, func);
  }

  //----------------------------------------------------------------------
  template <typename T>
  std::vector<T> deserialize_entities(const nlohmann::json &data,
                                      const bool include_subentities = false)
  {
    std::vector<T> entities;
    auto func = [this, &include_subentities](const auto &entity_data) {
      using Sphinx::Db::Json::from_json;
      if (include_subentities) {
        auto entity = from_json<T>(entity_data);
        deserialize_subentities(entity_data, entity);
        return entity;
      }
      else {
        return from_json<T>(entity_data);
      }
    };
    if (data.is_array()) {
      entities.reserve(data.size());
      std::transform(data.begin(), data.end(), std::back_inserter(entities),
                     func);
    }
    else {
      entities.emplace_back(func(data));
    }
    return entities;
  }

  //----------------------------------------------------------------------
  template <typename T>
  std::vector<T> deserialize_entities(const std::string &data,
                                      const bool include_subentities = false)
  {
    return deserialize_entities<T>(nlohmann::json::parse(data),
                                   include_subentities);
  }

  std::string find_users(std::string name);
};
} // namespace Sphinx::Backend

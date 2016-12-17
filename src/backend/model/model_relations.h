#pragma once

#include "db/model_relations.h"
#include "model.h" // for Course, Course::modules, Modules

namespace {
using Sphinx::Db::LinkManyInfo;
}
namespace {
using Sphinx::Backend::Model::Course;
using Sphinx::Backend::Model::Module;
using Sphinx::Backend::Model::Modules;
} // namespace

namespace Sphinx::Db {

//----------------------------------------------------------------------
template <>
struct LinkManyType<decltype(Course::modules)> {
  static constexpr char name[] = "modules";
  using type = LinkManyInfo<Modules,
                            decltype(Module::course_id),
                            decltype(Course::id),
                            name>;
};

} // namespace Sphinx::Db

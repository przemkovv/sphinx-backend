#pragma once

#include "model.h"
#include "model_meta.h"

namespace {
using Sphinx::Db::LinkManyInfo;
}
namespace {
  using Sphinx::Backend::Model::Course;
  using Sphinx::Backend::Model::Module;
  using Sphinx::Backend::Model::Modules;
}

namespace Sphinx::Db {

//----------------------------------------------------------------------
template <>
struct LinkManyType<decltype(Course::modules)> {
  using type = LinkManyInfo<Modules,
                        decltype(Module::course_id),
                        decltype(Course::id),
                        Course::Course_::modules>;
};



} // namespace Sphinx::Backend::Model::Meta

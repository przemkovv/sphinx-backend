#pragma once

#include "model_meta.h"

#include <fmt/format.h>

namespace Sphinx::Db {

struct lt_operator {
  static constexpr char name[] = "<";
};
struct le_operator {
  static constexpr char name[] = "<=";
};
struct eq_operator {
  static constexpr char name[] = "=";
};

struct neq_operator {
  static constexpr char name[] = "<>";
};

template <typename Column, typename Operator>
struct condition {
  using column = Column;
  using op = Operator;
  using value_type = typename Column::type;

  const Column &col;
  const value_type value;

  condition(Column &&c, Operator &&op, value_type val) : col(c), value(val) {}

  std::string str(std::string placeholder = "")
  {
    if (placeholder.empty())
      return fmt::format("{0} {1} {2}", c.name, op.name, value);
    else
      return fmt::format("{0} {1} {2}", c.name, op.name, placeholder);
  }
};
}

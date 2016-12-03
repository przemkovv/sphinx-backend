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
  using column_t = Column;
  using operator_t = Operator;
  using value_type = typename Column::type;

  const Column &col;
  const value_type value;

  condition(const Column &c, const Operator &op, value_type val) : col(c), value(val) {}

  std::string str(std::string placeholder = "") const
  {
    const std::string column_name = column_t::name;
    const std::string operator_name = operator_t::name;
    if (placeholder.empty())
      return fmt::format("{0} {1} '{2}'", column_name, operator_name, value);
    else
      return fmt::format("{0} {1} {2}", column_name, operator_name, placeholder);
  }
};
}

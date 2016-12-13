#pragma once

#include <fmt/format.h>
#include <string>

#include "model_meta.h"

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
struct like_operator {
  static constexpr char name[] = "LIKE";
};

template <typename Column, typename Operator>
struct condition {
  using column_t = Column;
  using operator_t = Operator;
  using value_type = typename Column::type;

  const Column &col;
  const value_type value;

  condition(const Column &c, const Operator & /* op */, value_type val)
    : col(c), value(val)
  {
  }

  std::string str(std::string placeholder = "") const
  {
    constexpr auto column_name = get_column_name<column_t>();
    constexpr auto operator_name = operator_t::name;
    if (placeholder.empty())
      return fmt::format("{0} {1} '{2}'", column_name, operator_name, value);
    else
      return fmt::format("{0} {1} {2}", column_name, operator_name,
                         placeholder);
  }
};
}

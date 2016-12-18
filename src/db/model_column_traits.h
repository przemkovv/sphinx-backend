#pragma once

#include <type_traits>

namespace Sphinx::Db::Meta {

//----------------------------------------------------------------------
template <typename T, typename... Traits>
struct contains : std::false_type {
};
template <typename T, typename... Traits>
struct contains<T, std::tuple<Traits...>>
    : std::disjunction<std::is_same<T, Traits>...> {
};

template <typename T, typename... Traits>
constexpr bool contains_v = contains<T, Traits...>::value;

//----------------------------------------------------------------------
struct primarykey_tag {
};
struct optional_tag {
};
struct autoincrement_tag {
};
struct foreignkey_tag {
};

//----------------------------------------------------------------------
template <typename... Traits>
constexpr bool has_optional_v = contains_v<optional_tag, Traits...>;

//----------------------------------------------------------------------
template <typename C>
struct is_primarykey : contains<primarykey_tag, typename C::traits> {
};
template <typename C>
struct is_optional : contains<optional_tag, typename C::traits> {
};
template <typename C>
struct is_autoincrement : contains<autoincrement_tag, typename C::traits> {
};
template <typename C>
struct is_foreignkey : contains<foreignkey_tag, typename C::traits> {
};

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_primarykey_v = is_primarykey<C>::value;
template <typename C>
constexpr bool is_optional_v = is_optional<std::remove_reference_t<C>>::value;
template <typename C>
constexpr bool is_autoincrement_v = is_autoincrement<C>::value;
template <typename C>
constexpr bool is_foreignkey_v = is_foreignkey<C>::value;

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_autoincrement_c(C &&)
{
  return is_autoincrement_v<typename std::remove_reference_t<C>>;
}

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_foreignkey_c(C &&)
{
  return is_foreignkey_v<typename std::remove_reference_t<C>>;
}

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_primarykey_c(C &&)
{
  return is_primarykey_v<typename std::remove_reference_t<C>>;
}

//----------------------------------------------------------------------
template <typename C>
constexpr bool is_optional_c(C &&)
{
  return is_optional_v<typename std::remove_reference_t<C>>;
}
} // namespace Sphinx::Db::a

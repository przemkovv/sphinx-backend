
#pragma once

#include "pprint.hpp"

#include <boost/program_options.hpp>

#include <future>
#include <iostream>
#include <string>

namespace Sphinx {

enum class ExitCode
/// Commonly used exit status codes.
/// Based on the definitions in the 4.3BSD <sysexits.h> header file.
{ OK = 0,           /// successful termination
  USAGE = 64,       /// command line usage error
  DATAERR = 65,     /// data format error
  NOINPUT = 66,     /// cannot open input
  NOUSER = 67,      /// addressee unknown
  NOHOST = 68,      /// host name unknown
  UNAVAILABLE = 69, /// service unavailable
  SOFTWARE = 70,    /// internal software error
  OSERR = 71,       /// system error (e.g., can't fork)
  OSFILE = 72,      /// critical OS file missing
  CANTCREAT = 73,   /// can't create (user) output file
  IOERR = 74,       /// input/output error
  TEMPFAIL = 75,    /// temp failure; user is invited to retry
  PROTOCOL = 76,    /// remote error in protocol
  NOPERM = 77,      /// permission denied
  CONFIG = 78       /// configuration error
};


/// @brief Auxiliary function to create a shared unique ptr.
template <class T, class... Args>
std::shared_ptr<std::unique_ptr<T>> make_shared_unique(Args &&... args)
{
  return std::make_shared<std::unique_ptr<T>>(
      std::make_unique<T>(std::forward<Args>(args)...));
}
/// @brief Auxiliary function to create a shared unique ptr.
template <class T, class... Args>
std::shared_ptr<std::unique_ptr<T>>
make_shared_unique_from_unique(std::unique_ptr<T> ptr)
{
  return std::make_shared<std::unique_ptr<T>>(ptr);
}

template <typename Task1, typename Task2>
auto while_do(const Task1 &task_while, const Task2 &task_do)
{

  auto future = std::async(std::launch::async, task_while);
  std::future_status status;
  do {
    task_do();
    status = future.wait_for(std::chrono::milliseconds(5));
  } while (status != std::future_status::ready);

  task_do();

  return future.get();
}

std::string escape_control_characters(const std::string &input);

template <typename Task> struct on_scope_exit {
  on_scope_exit(const Task &f) : f_(f) {}
  on_scope_exit<Task> &operator=(const on_scope_exit<Task> &) = delete;
  on_scope_exit(const on_scope_exit<Task> &) = delete;
  on_scope_exit(on_scope_exit<Task> &&) = default;

  ~on_scope_exit() { f_(); }
  const Task &f_;
};

template <typename Task> on_scope_exit<Task> do_on_scope_exit(const Task &f)
{
  return on_scope_exit<Task>(f);
}

#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1##arg2
#define SCOPE_EXIT(code)                                                       \
  auto STRING_JOIN2(on_scope_exit_, __LINE__) =                                \
      do_on_scope_exit([&]() { code; })

} // namespace Sphinx

namespace std {

std::ostream &operator<<(std::ostream &out,
                         const boost::program_options::variable_value &v);

} // namespace std

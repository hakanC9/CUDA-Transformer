#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static constexpr char LOGGER_CONSOLE[] = "logger_console";

inline std::shared_ptr<spdlog::logger> getLogger()
{
  auto log = spdlog::get(LOGGER_CONSOLE);
  if (log == nullptr)
  {
    log = spdlog::stdout_color_mt(LOGGER_CONSOLE);
    spdlog::set_pattern("[%l] %v");
  }

  return log;
}

#define GET_LOGGER() ::getLogger()

#define LOG_TRACE(msg, ...) GET_LOGGER()->trace(msg, ##__VA_ARGS__);
#define LOG_DEBUG(msg, ...) GET_LOGGER()->debug(msg, ##__VA_ARGS__);
#define LOG_INFO(msg, ...) GET_LOGGER()->info(msg, ##__VA_ARGS__);
#define LOG_WARN(msg, ...) GET_LOGGER()->warn(msg, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) GET_LOGGER()->error(msg, ##__VA_ARGS__);
#define LOG_CRITICAL(msg, ...) GET_LOGGER()->critical(msg, ##__VA_ARGS__);

#define LOAD_ENV_LEVELS() spdlog::cfg::load_env_levels();

inline spdlog::level::level_enum getLogLevel()
{
  auto log = spdlog::get(LOGGER_CONSOLE);

  if (log == nullptr)
  {
    log = spdlog::get("default");
    if (log == nullptr)
    {
      log = spdlog::stdout_color_mt("default");
    }
    log->critical("getLogLevel - logger with name '{}' does not exist", LOGGER_CONSOLE);
  }

  return log->level();
}

#define SET_LOG_LEVEL(loglevel)                       \
  switch (loglevel)                                   \
  {                                                   \
  case 0:                                             \
    GET_LOGGER()->set_level(spdlog::level::trace);    \
    break;                                            \
  case 1:                                             \
    GET_LOGGER()->set_level(spdlog::level::debug);    \
    break;                                            \
  case 2:                                             \
    GET_LOGGER()->set_level(spdlog::level::info);     \
    break;                                            \
  case 3:                                             \
    GET_LOGGER()->set_level(spdlog::level::warn);     \
    break;                                            \
  case 4:                                             \
    GET_LOGGER()->set_level(spdlog::level::err);      \
    break;                                            \
  case 5:                                             \
    GET_LOGGER()->set_level(spdlog::level::critical); \
    break;                                            \
  case 6:                                             \
    GET_LOGGER()->set_level(spdlog::level::off);      \
    break;                                            \
  default:                                            \
    assert(0 && "No such log level");                 \
  };

#define FLUSH_LOGGER() GET_LOGGER()->flush();
#define DROP_ALL_LOGGERS() spdlog::drop_all();

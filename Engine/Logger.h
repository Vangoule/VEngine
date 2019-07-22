#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Log.h"

static Logger::Logger<Logger::FileLogPolicy> m_log("log.txt");



#ifdef LOGGING_LEVEL_1

#ifdef LOGGING_THREADSAFE

#define LOG m_log.PrintThreadSafe< Logger::SeverityLevel::Engine >
#define GLOG m_log.PrintThreadSafe< Logger::SeverityLevel::Game >
#define ELOG m_log.PrintThreadSafe< Logger::SeverityLevel::Error >
#define LOG_WARN m_log.PrintThreadSafe< Logger::SeverityLevel::Warning >

#else

#define LOG m_log.Print< Logger::SeverityLevel::Engine >
#define GLOG m_log.Print< Logger::SeverityLevel::Game >
#define ELOG m_log.Print< Logger::SeverityLevel::Error >
#define LOG_WARN m_log.Print< Logger::SeverityLevel::Warning >

#endif


#else

#define LOG(...) 
#define LOG_ERR(...)
#define LOG_WARN(...)

#endif


#endif


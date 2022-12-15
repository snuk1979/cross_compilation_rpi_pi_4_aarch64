#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <SoapySDR/Logger.hpp>

struct Log {
  Log(const std::string& funcName) : mFuncName(funcName) {
    SoapySDR::logf(SOAPY_SDR_NOTICE, "Enter: %s", mFuncName.c_str());
  }
  ~Log() {
    SoapySDR::logf(SOAPY_SDR_NOTICE, "Exit: %s", mFuncName.c_str());
  }
  const std::string mFuncName;
};

#define LOG_FUNC() Log log(__PRETTY_FUNCTION__)

#define LOG_EXP(WHAT)                                \
  SoapySDR::logf(SOAPY_SDR_FATAL,                    \
                 "*************** Exception: %s %s", \
                 WHAT,                               \
                 "***************\n");

#endif  // __UTILITY_H__
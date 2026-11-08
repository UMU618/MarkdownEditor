#pragma once

#include <vcruntime.h>

#include "build_time.h"

#define FILE_VERSION BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR
#define PRODUCT_VERSION BUILD_YEAR, BUILD_MONTH, BUILD_DAY, 0

#define FILE_VERSION_STR                              \
  _CRT_STRINGIZE(BUILD_YEAR)                          \
  "." _CRT_STRINGIZE(BUILD_MONTH) "." _CRT_STRINGIZE( \
      BUILD_DAY) "." _CRT_STRINGIZE(BUILD_HOUR)
#define PRODUCT_VERSION_STR \
  _CRT_STRINGIZE(BUILD_YEAR) "." _CRT_STRINGIZE(BUILD_MONTH) "." _CRT_STRINGIZE(BUILD_DAY) "." "0"

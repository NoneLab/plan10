#include <feature.h>
#include <common.h>
extern "C" int from_cpp() { return FEATURE_VALUE + COMMON_VALUE; }

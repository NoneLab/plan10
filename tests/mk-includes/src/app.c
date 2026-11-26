#include <feature.h>
#include <common.h>
#include <generated.h>
#include <runtime_marker.h>
#if RUNTIME_EXPERIMENTAL != EXPECT_EXPERIMENTAL
#error wrong libc header selected
#endif
#ifdef EXPECT_PRIVATE
#include <private_math.h>
#endif
extern int core(void);
extern int runtime(void);
int main(void) { return core() + runtime() + COMMON_VALUE; }

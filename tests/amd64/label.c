/* AMD64 setlabel/gotolabel ABI continuation test, run in host userspace. */
#include "u.h"
#include "../../sys/src/9/port/lib.h"
#include "../../sys/src/9/pc64/mem.h"
#include "../../sys/src/9/pc64/dat.h"
#include "../../sys/src/9/pc64/fns.h"

static Label saved;
static volatile int passes;

int
main(void)
{
    int result;

    result = setlabel(&saved);
    if(result == 0){
        passes++;
        gotolabel(&saved);
    }

    return result == 1 && passes == 1 ? 0 : 1;
}

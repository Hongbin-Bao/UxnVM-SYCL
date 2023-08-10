#include "../uxn.h"
#include "controller.h"
/**
 * 
 * Description:
 * 
 * Created by: Hongbin Bao
 * Created on: 2023/7/11 19:44
 * 
 */
void
controller_down(Uxn *u, Uint8 *d, Uint8 mask,cl::sycl::queue& deviceQueue)
{
    if(mask) {
        d[2] |= mask;
        uxn_eval(u, PEEK2(d),deviceQueue);
    }
}

void
controller_up(Uxn *u, Uint8 *d, Uint8 mask,cl::sycl::queue& deviceQueue)
{
    if(mask) {
        d[2] &= (~mask);
        uxn_eval(u, PEEK2(d),deviceQueue);
    }
}

void
controller_key(Uxn *u, Uint8 *d, Uint8 key,cl::sycl::queue& deviceQueue)
{
    if(key) {
        d[3] = key;
        uxn_eval(u, PEEK2(d),deviceQueue);
        d[3] = 0x00;
    }
}

/**
 *
 * Description: A simple CLI (command line interface) implementation of the UXN virtual machine.
 *
 * Created by: Hongbin Bao
 * Created on: 2023/7/10 17:17
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
#include <CL/sycl.hpp>
#include <cstdlib>
#include "uxn.h"
#include "devices/system.h"
#include "devices/file.h"
#include "devices/datetime.h"
#include "iostream"
/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/


//这些掩码用于设备输入输出操作
Uint16 deo_mask[] = {0xc028, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
Uint16 dei_mask[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
/**
* Function reads device data based on address
  * This function first checks the upper 4 bits of the device address (implemented through the & operation).
  * For example: if the high 4 bits of the address are 0xc0, call the datetime_dei function to operate. Otherwise, it reads the corresponding value directly from the virtual machine's device array.
 * @param u
 * @param addr
 * @return
 */
Uint8
uxn_dei(Uxn *u, Uint8 addr)
{
    switch(addr & 0xf0) {
        case 0xc0: return datetime_dei(u, addr);
    }
    return u->dev[addr];
}
/**
* Function is responsible for writing data to the device
* Decompose the device address into high 4 bits and low 4 bits. Then call the corresponding device operation function based on the value of the high 4 bits.
 * @param u
 * @param addr
 */
void
uxn_deo(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0;
    switch(d) {
        case 0x00: system_deo(u, &u->dev[d], p); break;
        case 0x10: console_deo(&u->dev[d], p); break;
        case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break;
        case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break;
    }
}
/**
 * Check the command line parameters. If the ROM file name is not provided, an error will be reported and exit. Then, it initializes a UXN virtual machine and allocates space for the virtual machine's memory. Next, it loads the program from the ROM file into the virtual machine,
 * Then set some status of the device, and then start executing the program in the virtual machine. If the virtual machine program requires parameters, it will be obtained from the command line parameters. Finally, when the virtual machine stops running, the allocated memory is released and the status of the virtual machine is returned.
 * argc is an integer parameter used to represent the number of command line parameters, while argv is a pointer to a string pointer used to represent a string array of command line parameters.
 * @param argc
 * @param argv
 * @return
 */
int
main(int argc, char **argv)
{

    // Print original arguments
    std::cout << "Original arguments: " << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    cl::sycl::queue deviceQueue(cl::sycl::default_selector{});
    //cl::sycl::queue deviceQueue(cl::sycl::gpu_selector {});

    // malloc shared memory for Uxn  by SYCL USM
    Uxn* u = cl::sycl::malloc_shared<Uxn>(1, deviceQueue);
    //Uxn u;
    int i = 1;
    if(i == argc)
        return system_error("usage", "uxncli file.rom [args..]");

    uint8_t* p = malloc_shared<uint8_t>(0x10000 * RAM_PAGES, deviceQueue);
    if(!uxn_boot(u, p))
        return system_error("Boot", "Failed");
    if(!system_load(u, argv[i++]))
        return system_error("Load", "Failed");
    u->dev[0x17] = argc - i;
    Uint16* pc = cl::sycl::malloc_shared<Uint16>(1, deviceQueue);
    *pc = PAGE_PROGRAM;
    u->params = cl::sycl::malloc_shared<Params>(sizeof(Params),deviceQueue);
    u->queue = deviceQueue;
    if(uxn_eval(u, *pc)) {
        for(; i < argc; i++) {
            char *p = argv[i];
            while(*p) console_input(u, *p++, CONSOLE_ARG);
            console_input(u, '\n', i == argc - 1 ? CONSOLE_END : CONSOLE_EOA);
        }
        while(!u->dev[0x0f]) {
            int c = fgetc(stdin);
            if(c != EOF) console_input(u, (Uint8)c, CONSOLE_STD);
        }
    }
    free(u->ram);
    return u->dev[0x0f] & 0x7f;
}

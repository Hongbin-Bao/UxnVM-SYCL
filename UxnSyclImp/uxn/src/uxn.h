/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/
#define PAGE_PROGRAM 0x0100 // 定义编程页大小为0x0100

/* clang-format off */

#define POKE2(d, v) { (d)[0] = (v) >> 8; (d)[1] = (v); } // 定义一个宏POKE2，用于将16位值v写入到地址d
#define PEEK2(d) ((d)[0] << 8 | (d)[1]) // 定义一个宏PEEK2，用于从地址d读取16位值
#include <CL/sycl.hpp>

/* clang-format on */

typedef unsigned char Uint8; // 定义8位无符号整数类型Uint8
typedef signed char Sint8; // 定义8位有符号整数类型Sint8
typedef unsigned short Uint16; // 定义16位无符号整数类型Uint16
typedef signed short Sint16; // 定义16位有符号整数类型Sint16
typedef unsigned int Uint32; // 定义32位无符号整数类型Uint32

typedef struct { // 定义一个Stack结构体
    Uint8 dat[255], ptr; // dat数组用于存储数据，ptr是一个指针，用于指向当前栈顶
} Stack;

typedef struct {
    Uint16 pc;
    Stack *s;
    Stack *z;
    int t;
    int n;
    int l;
    int k;
    int tmp;
    int opc;
    int ins;
    int ret;
    int haltCode;
    bool yield;
} Params;

typedef struct Uxn { // 定义一个Uxn结构体
    Uint8 *ram, dev[256]; // ram是一个指向内存的指针，dev数组用于模拟输入/输出设备
    Stack wst, rst; // wst和rst是两个栈，分别为工作栈和返回栈
    Uint8 (*dei)(struct Uxn *u, Uint8 addr); // dei是一个函数指针，用于处理输入设备的操作
    void (*deo)(struct Uxn *u, Uint8 addr); // deo是一个函数指针，用于处理输出设备的操作
    Params *params;
    //cl::sycl::queue queue;
} Uxn;

/* required functions */

extern Uint8 uxn_dei(Uxn *u, Uint8 addr); // 定义一个外部函数uxn_dei，用于处理输入设备的操作
extern void uxn_deo(Uxn *u, Uint8 addr,cl::sycl::queue& deviceQueue); // 定义一个外部函数uxn_deo，用于处理输出设备的操作
extern int uxn_halt(Uxn *u, Uint8 instr, Uint8 err, Uint16 addr,cl::sycl::queue& deviceQueue); // 定义一个外部函数uxn_halt，用于处理计算机出错的情况
extern Uint16 dei_mask[]; // 定义一个外部数组dei_mask，用于处理输入设备的掩码
extern Uint16 deo_mask[]; // 定义一个外部数组deo_mask，用于处理输出设备的掩码

/* built-ins */

int uxn_boot(Uxn *u, Uint8 *ram); // 定义一个函数uxn_boot，用于初始化Uxn结构体并设置其内存块
int uxn_eval(Uxn *u, Uint16 pc,cl::sycl::queue& deviceQueue); // 定义一个函数uxn_eval，用于执行Uxn的代码

//int uxn_eval_gpu(Uxn *u, Uint16 pc);
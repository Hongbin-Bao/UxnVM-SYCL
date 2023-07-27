/**
 *
 * Description:
 *
 * Created by: Hongbin Bao
 * Created on: 2023/7/10 17:17
 *
 */
#include "uxn.h"
#include <CL/sycl.hpp>
#include <iostream>

/*
Copyright (u) 2022-2023 Devine Lu Linvega, Andrew Alderwick, Andrew Richards

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define T s->dat[s->ptr - 1]
#define N s->dat[s->ptr - 2]
#define L s->dat[s->ptr - 3]
#define H2 PEEK2(s->dat + s->ptr - 3)
#define T2 PEEK2(s->dat + s->ptr - 2)
#define N2 PEEK2(s->dat + s->ptr - 4)
#define L2 PEEK2(s->dat + s->ptr - 6)

/* Registers

[ . ][ . ][ . ][ L ][ N ][ T ] <
[ . ][ . ][ . ][   H2   ][ T ] <
[   L2   ][   N2   ][   T2   ] <

*/

//#define HALT(c) { return uxn_halt(u, ins, (c), pc - 1); }
#define HALT(c) { }
//#define SET(mul, add) { if(mul > s->ptr) HALT(1) tmp = (mul & k) + add + s->ptr; if(tmp > 254) HALT(2) s->ptr = tmp; }
#define SET(mul, add) {  tmp = (mul & k) + add + s->ptr; s->ptr = tmp; }

#define PUT(o, v) { s->dat[(Uint8)(s->ptr - 1 - (o))] = (v); }


#define PUT2(o, v) { tmp = (v); s->dat[(Uint8)(s->ptr - o - 2)] = tmp >> 8; s->dat[(Uint8)(s->ptr - o - 1)] = tmp; }
//#define PUSH(x, v) { z = (x); if(z->ptr > 254) HALT(2) z->dat[z->ptr++] = (v); }
#define PUSH(x, v) { z = (x);z->dat[z->ptr++] = (v); }

//#define PUSH2(x, v) { z = (x); if(z->ptr > 253) HALT(2) tmp = (v); z->dat[z->ptr] = tmp >> 8; z->dat[z->ptr + 1] = tmp; z->ptr += 2; }
#define PUSH2(x, v) { z = (x);  tmp = (v); z->dat[z->ptr] = tmp >> 8; z->dat[z->ptr + 1] = tmp; z->ptr += 2; }

#define DEO(a, b) { u->dev[(a)] = (b); if((deo_mask[(a) >> 4] >> ((a) & 0xf)) & 0x1) uxn_deo(u, (a)); }
#define DEI(a, b) { PUT((a), ((dei_mask[(b) >> 4] >> ((b) & 0xf)) & 0x1) ? uxn_dei(u, (b)) : u->dev[(b)]) }

//void DEI_func(int a, int b, Uxn* u) {
//    int index = b >> 4;
//    int shift = b & 0xf;
//    int mask_value = dei_mask[index] >> shift;
//    int is_set = mask_value & 0x1;
//
//    int value;
//    if (is_set) {
//        value = uxn_dei(u, b);
//    } else {
//        value = u->dev[b];
//    }
//
//    PUT(a, value);
//}

void PUT_F(Stack* s, Uint8 o, Uint8 v) {
    Uint8 temp = s->ptr -1 -o;
    s->dat[temp] = v;
}
//# DEI(0a, tb)
//# t int
void DEI_F(Uxn* u, Stack* s, Uint8 a, Uint8 b, Uint16* dei_mask) {
        Uint8 val;
        if ((dei_mask[b >> 4] >> (b & 0xf)) & 0x1) {
            //val = u->dei(u, b);
            val = uxn_dei(u,b);
        } else {
            val = u->dev[b];
        }
        PUT_F(s, a, val);
}

void DEO_F(Uxn* u, Uint8 a, Uint8 b, Uint16* deo_mask) {
        u->dev[a] = b;
        if ((deo_mask[a >> 4] >> (a & 0xf)) & 0x1) {
            u->deo(u, a);
        }
}




class my_kernel;
//int uxn_eval(Uxn *u, Uint16 pc){
//
//
////    // 选择设备并创建SYCL队列
////    cl::sycl::queue q (cl::sycl::default_selector{});
////
////
////    cl::sycl::device dev = q.get_device();
////    std::cout << "Running on "
////              << dev.get_info<cl::sycl::info::device::name>()
////              << "\n";
//
//    cl::sycl::queue queue; // Initialize a SYCL queue
//    int* message = cl::sycl::malloc_shared<int>(1,queue);
//    *message = 0; // Initialize message
//    int* t = cl::sycl::malloc_shared<int>(1, queue);
//    int* n = cl::sycl::malloc_shared<int>(1, queue);
//    int* l = cl::sycl::malloc_shared<int>(1, queue);
//    int* k = cl::sycl::malloc_shared<int>(1, queue);
//    int* tmp = cl::sycl::malloc_shared<int>(1, queue);
//    int* opc = cl::sycl::malloc_shared<int>(1, queue);
//    int* ins = cl::sycl::malloc_shared<int>(1, queue);
//    Stack* s = cl::sycl::malloc_shared<Stack>(1, queue);
//    Stack* z =  cl::sycl::malloc_shared<Stack>(1, queue);
//
//
//    queue.submit([&](cl::sycl::handler& cgh) {
//        cgh.single_task<class my_kernel>([=]() mutable {
//
//            int t, n, l, k, tmp, opc, ins;
//            Uint8* ram =  u->ram;
//            //Stack *s, *z;  // 定义两个指向Stack的指针
//
//            // 如果pc为0或者u的dev数组的第16个元素（0x0f）非零，则函数返回0
//            if(!pc || u->dev[0x0f]){
//
//            }
//
//            bool should_break = false;
//
//            for(;;) {
//                ins = ram[pc++] & 0xff;
//                k = ins & 0x80 ? 0xff : 0;
//                s = ins & 0x40 ? &u->rst : &u->wst;
//                opc = !(ins & 0x1f) ? (0 - (ins >> 5)) & 0xff : ins & 0x3f;
//                switch(opc) {  // 开始switch语句，对opc进行分支处理
//                    /* IMM */
//                    case 0x00: /* BRK   */ *message = 1; break;
//                    case 0xff: /* JCI   */ pc += !!s->dat[--s->ptr] * PEEK2(ram + pc) + 2; break;
//                    case 0xfe: /* JMI   */ pc += PEEK2(ram + pc) + 2; break;
//
//                    // TODO  PUSH2 have  HALT  uxn_halt need   return
//                    case 0xfd: /* JSI   */ PUSH2(&u->rst, pc + 2) pc += PEEK2(ram + pc) + 2; break;
//                    case 0xfc: /* LIT   */ PUSH(s, ram[pc++]) break;
//                    case 0xfb: /* LIT2  */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
//                    case 0xfa: /* LITr  */ PUSH(s, ram[pc++]) break;
//                    case 0xf9: /* LIT2r */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
//                        /* ALU */
//                    case 0x01: /* INC  */ t=T;            SET(1, 0) PUT(0, t + 1) break;
//                    case 0x21:            t=T2;           SET(2, 0) PUT2(0, t + 1) break;
//                    case 0x02: /* POP  */                 SET(1,-1) break;
//                    case 0x22:                            SET(2,-2) break;
//                    case 0x03: /* NIP  */ t=T;            SET(2,-1) PUT(0, t) break;
//                    case 0x23:            t=T2;           SET(4,-2) PUT2(0, t) break;
//                    case 0x04: /* SWP  */ t=T;n=N;        SET(2, 0) PUT(0, n) PUT(1, t) break;
//                    case 0x24:            t=T2;n=N2;      SET(4, 0) PUT2(0, n) PUT2(2, t) break;
//                    case 0x05: /* ROT  */ t=T;n=N;l=L;    SET(3, 0) PUT(0, l) PUT(1, t) PUT(2, n) break;
//                    case 0x25:            t=T2;n=N2;l=L2; SET(6, 0) PUT2(0, l) PUT2(2, t) PUT2(4, n) break;
//                    case 0x06: /* DUP  */ t=T;            SET(1, 1) PUT(0, t) PUT(1, t) break;
//                    case 0x26:            t=T2;           SET(2, 2) PUT2(0, t) PUT2(2, t) break;
//                    case 0x07: /* OVR  */ t=T;n=N;        SET(2, 1) PUT(0, n) PUT(1, t) PUT(2, n) break;
//                    case 0x27:            t=T2;n=N2;      SET(4, 2) PUT2(0, n) PUT2(2, t) PUT2(4, n) break;
//                    case 0x08: /* EQU  */ t=T;n=N;        SET(2,-1) PUT(0, n == t) break;
//                    case 0x28:            t=T2;n=N2;      SET(4,-3) PUT(0, n == t) break;
//                    case 0x09: /* NEQ  */ t=T;n=N;        SET(2,-1) PUT(0, n != t) break;
//                    case 0x29:            t=T2;n=N2;      SET(4,-3) PUT(0, n != t) break;
//                    case 0x0a: /* GTH  */ t=T;n=N;        SET(2,-1) PUT(0, n > t) break;
//                    case 0x2a:            t=T2;n=N2;      SET(4,-3) PUT(0, n > t) break;
//                    case 0x0b: /* LTH  */ t=T;n=N;        SET(2,-1) PUT(0, n < t) break;
//                    case 0x2b:            t=T2;n=N2;      SET(4,-3) PUT(0, n < t) break;
//                    case 0x0c: /* JMP  */ t=T;            SET(1,-1) pc += (Sint8)t; break;
//                    case 0x2c:            t=T2;           SET(2,-2) pc = t; break;
//                    case 0x0d: /* JCN  */ t=T;n=N;        SET(2,-2) pc += !!n * (Sint8)t; break;
//                    case 0x2d:            t=T2;n=L;       SET(3,-3) if(n) pc = t; break;
//                    case 0x0e: /* JSR  */ t=T;            SET(1,-1) PUSH2(&u->rst, pc) pc += (Sint8)t; break;
//                    case 0x2e:            t=T2;           SET(2,-2) PUSH2(&u->rst, pc) pc = t; break;
//                    case 0x0f: /* STH  */ t=T;            SET(1,-1) PUSH((ins & 0x40 ? &u->wst : &u->rst), t) break;
//                    case 0x2f:            t=T2;           SET(2,-2) PUSH2((ins & 0x40 ? &u->wst : &u->rst), t) break;
//                    case 0x10: /* LDZ  */ t=T;            SET(1, 0) PUT(0, ram[t]) break;
//                    case 0x30:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + t)) break;
//                    case 0x11: /* STZ  */ t=T;n=N;        SET(2,-2) ram[t] = n; break;
//                    case 0x31:            t=T;n=H2;       SET(3,-3) POKE2(ram + t, n) break;
//                    case 0x12: /* LDR  */ t=T;            SET(1, 0) PUT(0, ram[pc + (Sint8)t]) break;
//                    case 0x32:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + pc + (Sint8)t)) break;
//                    case 0x13: /* STR  */ t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n; break;
//                    case 0x33:            t=T;n=H2;       SET(3,-3) POKE2(ram + pc + (Sint8)t, n) break;
//                    case 0x14: /* LDA  */ t=T2;           SET(2,-1) PUT(0, ram[t]) break;
//                    case 0x34:            t=T2;           SET(2, 0) PUT2(0, PEEK2(ram + t)) break;
//                    case 0x15: /* STA  */ t=T2;n=L;       SET(3,-3) ram[t] = n; break;
//                    case 0x35:            t=T2;n=N2;      SET(4,-4) POKE2(ram + t, n) break;
//
//                    // TODO
//                    //case 0x16: /* DEI  */ t=T;            SET(1, 0) DEI(0, t) break;
//                    case 0x16: /* DEI  */ t=T;            *message = 2;SET(1, 0) return;//SET(1, 0) DEI(0, t) break;
//                    case 0x36:            t=T;            *message = 3;SET(1, 1) return;//SET(1, 1) DEI(1, t) DEI(0, t + 1) break;
//                    case 0x17: /* DEO  */ t=T;n=N;        *message = 4;SET(2,-2) return;//SET(2,-2) DEO(t, n) break;
//                    case 0x37:            t=T;n=N;l=L;    *message = 5;SET(3,-3) return;//SET(3,-3) DEO(t, l) DEO(t + 1, n) break;
//
//
//                    case 0x18: /* ADD  */ t=T;n=N;        SET(2,-1) PUT(0, n + t) break;
//                    case 0x38:            t=T2;n=N2;      SET(4,-2) PUT2(0, n + t) break;
//                    case 0x19: /* SUB  */ t=T;n=N;        SET(2,-1) PUT(0, n - t) break;
//                    case 0x39:            t=T2;n=N2;      SET(4,-2) PUT2(0, n - t) break;
//                    case 0x1a: /* MUL  */ t=T;n=N;        SET(2,-1) PUT(0, n * t) break;
//                    case 0x3a:            t=T2;n=N2;      SET(4,-2) PUT2(0, n * t) break;
//                    case 0x1b: /* DIV  */ t=T;n=N;        SET(2,-1) if(!t) HALT(3) PUT(0, n / t) break;
//                    case 0x3b:            t=T2;n=N2;      SET(4,-2) if(!t) HALT(3) PUT2(0, n / t) break;
//                    case 0x1c: /* AND  */ t=T;n=N;        SET(2,-1) PUT(0, n & t) break;
//                    case 0x3c:            t=T2;n=N2;      SET(4,-2) PUT2(0, n & t) break;
//                    case 0x1d: /* ORA  */ t=T;n=N;        SET(2,-1) PUT(0, n | t) break;
//                    case 0x3d:            t=T2;n=N2;      SET(4,-2) PUT2(0, n | t) break;
//                    case 0x1e: /* EOR  */ t=T;n=N;        SET(2,-1) PUT(0, n ^ t) break;
//                    case 0x3e:            t=T2;n=N2;      SET(4,-2) PUT2(0, n ^ t) break;
//                    case 0x1f: /* SFT  */ t=T;n=N;        SET(2,-1) PUT(0, n >> (t & 0xf) << (t >> 4)) break;
//                    case 0x3f:            t=T;n=H2;       SET(3,-1) PUT2(0, n >> (t & 0xf) << (t >> 4)) break;
//                }
//            }
//        });
//    }).wait();
//
//    if(*message == 1){
//        cl::sycl::free(message,queue);
//        return 1;
//    }
//    if(*message == 2){
//
//        DEI(0, *t);
//        uxn_eval(u,pc);
//    }
//    if(*message ==3){
//        DEI(1, *t); DEI(0, *t + 1);
//        uxn_eval(u,pc);
//    }
//    if(*message == 4){
//        DEO(*t, *n);
//        uxn_eval(u,pc);
//    }
//
//    if(*message == 5){
//        DEO(*t, *l) DEO(*t + 1, *n);
//        uxn_eval(u,pc);
//    }
//    else{
//        cl::sycl::free(message,queue);
//        return 0;
//    }
//}




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


void gpu_k(Params &params,Uxn *u){

    Uint8 *ram = u->ram;
    Uint16 &pc = params.pc;
    Stack *&s = params.s;
    Stack *&z = params.z;
    int &t = params.t;
    int &n = params.n;
    int &l = params.l;
    int &k = params.k;
    int &tmp = params.tmp;
    int &opc = params.opc;
    int &ins = params.ins;
    int &ret = params.ret;
    int &haltCode = params.haltCode;
    bool &yield = params.yield;
    for(;;) {  // 无限循环，直到满足某个退出条件才会跳出

        ins = ram[pc++] & 0xff;  // 获取ram中pc指定的元素值，并与0xff做与操作，然后pc自增
        k = ins & 0x80 ? 0xff : 0;  // 若ins的最高位（第8位）为1，则k为0xff，否则为0
        s = ins & 0x40 ? &u->rst : &u->wst;  // 若ins的第7位为1，则s指向u的rst成员，否则指向wst成员
        opc = !(ins & 0x1f) ? (0 - (ins >> 5)) & 0xff : ins & 0x3f;  // 如果ins的最低5位全为0，则opc等于ins右移5位后的负值与0xff的与运算结果，否则等于ins与0x3f的与运算结果

        switch(opc) {  // 开始switch语句，对opc进行分支处理
            /* IMM */
            case 0x00: /* BRK   */ ret =1; return ;
            case 0xff: /* JCI   */ pc += !!s->dat[--s->ptr] * PEEK2(ram + pc) + 2; break;
            case 0xfe: /* JMI   */ pc += PEEK2(ram + pc) + 2; break;
            case 0xfd: /* JSI   */ PUSH2(&u->rst, pc + 2) pc += PEEK2(ram + pc) + 2; break;
            case 0xfc: /* LIT   */ PUSH(s, ram[pc++]) break;
            case 0xfb: /* LIT2  */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
            case 0xfa: /* LITr  */ PUSH(s, ram[pc++]) break;
            case 0xf9: /* LIT2r */ PUSH2(s, PEEK2(ram + pc)) pc += 2; break;
                /* ALU */
            case 0x01: /* INC  */ t=T;            SET(1, 0) PUT(0, t + 1) break;
            case 0x21:            t=T2;           SET(2, 0) PUT2(0, t + 1) break;
            case 0x02: /* POP  */                 SET(1,-1) break;
            case 0x22:                            SET(2,-2) break;
            case 0x03: /* NIP  */ t=T;            SET(2,-1) PUT(0, t) break;
            case 0x23:            t=T2;           SET(4,-2) PUT2(0, t) break;
            case 0x04: /* SWP  */ t=T;n=N;        SET(2, 0) PUT(0, n) PUT(1, t) break;
            case 0x24:            t=T2;n=N2;      SET(4, 0) PUT2(0, n) PUT2(2, t) break;
            case 0x05: /* ROT  */ t=T;n=N;l=L;    SET(3, 0) PUT(0, l) PUT(1, t) PUT(2, n) break;
            case 0x25:            t=T2;n=N2;l=L2; SET(6, 0) PUT2(0, l) PUT2(2, t) PUT2(4, n) break;
            case 0x06: /* DUP  */ t=T;            SET(1, 1) PUT(0, t) PUT(1, t) break;
            case 0x26:            t=T2;           SET(2, 2) PUT2(0, t) PUT2(2, t) break;
            case 0x07: /* OVR  */ t=T;n=N;        SET(2, 1) PUT(0, n) PUT(1, t) PUT(2, n) break;
            case 0x27:            t=T2;n=N2;      SET(4, 2) PUT2(0, n) PUT2(2, t) PUT2(4, n) break;
            case 0x08: /* EQU  */ t=T;n=N;        SET(2,-1) PUT(0, n == t) break;
            case 0x28:            t=T2;n=N2;      SET(4,-3) PUT(0, n == t) break;
            case 0x09: /* NEQ  */ t=T;n=N;        SET(2,-1) PUT(0, n != t) break;
            case 0x29:            t=T2;n=N2;      SET(4,-3) PUT(0, n != t) break;
            case 0x0a: /* GTH  */ t=T;n=N;        SET(2,-1) PUT(0, n > t) break;
            case 0x2a:            t=T2;n=N2;      SET(4,-3) PUT(0, n > t) break;
            case 0x0b: /* LTH  */ t=T;n=N;        SET(2,-1) PUT(0, n < t) break;
            case 0x2b:            t=T2;n=N2;      SET(4,-3) PUT(0, n < t) break;
            case 0x0c: /* JMP  */ t=T;            SET(1,-1) pc += (Sint8)t; break;
            case 0x2c:            t=T2;           SET(2,-2) pc = t; break;
            case 0x0d: /* JCN  */ t=T;n=N;        SET(2,-2) pc += !!n * (Sint8)t; break;
            case 0x2d:            t=T2;n=L;       SET(3,-3) if(n) pc = t; break;
            case 0x0e: /* JSR  */ t=T;            SET(1,-1) PUSH2(&u->rst, pc) pc += (Sint8)t; break;
            case 0x2e:            t=T2;           SET(2,-2) PUSH2(&u->rst, pc) pc = t; break;
            case 0x0f: /* STH  */ t=T;            SET(1,-1) PUSH((ins & 0x40 ? &u->wst : &u->rst), t) break;
            case 0x2f:            t=T2;           SET(2,-2) PUSH2((ins & 0x40 ? &u->wst : &u->rst), t) break;
            case 0x10: /* LDZ  */ t=T;            SET(1, 0) PUT(0, ram[t]) break;
            case 0x30:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + t)) break;
            case 0x11: /* STZ  */ t=T;n=N;        SET(2,-2) ram[t] = n; break;
            case 0x31:            t=T;n=H2;       SET(3,-3) POKE2(ram + t, n) break;
            case 0x12: /* LDR  */ t=T;            SET(1, 0) PUT(0, ram[pc + (Sint8)t]) break;
            case 0x32:            t=T;            SET(1, 1) PUT2(0, PEEK2(ram + pc + (Sint8)t)) break;
            case 0x13: /* STR  */ t=T;n=N;        SET(2,-2) ram[pc + (Sint8)t] = n; break;
            case 0x33:            t=T;n=H2;       SET(3,-3) POKE2(ram + pc + (Sint8)t, n) break;
            case 0x14: /* LDA  */ t=T2;           SET(2,-1) PUT(0, ram[t]) break;
            case 0x34:            t=T2;           SET(2, 0) PUT2(0, PEEK2(ram + t)) break;
            case 0x15: /* STA  */ t=T2;n=L;       SET(3,-3) ram[t] = n; break;
            case 0x35:            t=T2;n=N2;      SET(4,-4) POKE2(ram + t, n) break;

            case 0x16: /* DEI  */{
                t=T;
                SET(1, 0);
                yield = true;

//
//                DEI(0, t)
                return ;
//                break;
            }

                //SET(1, 0) gpuParams1.status =2; return gpuParams1;//DEI(0, t) break;
            case 0x36:            t=T;            SET(1, 1) DEI(1, t) DEI(0, t + 1) break;
            case 0x17: /* DEO  */ t=T;n=N;        SET(2,-2) DEO(t, n) break;
            case 0x37:            t=T;n=N;l=L;    SET(3,-3) DEO(t, l) DEO(t + 1, n) break;

            case 0x18: /* ADD  */ t=T;n=N;        SET(2,-1) PUT(0, n + t) break;
            case 0x38:            t=T2;n=N2;      SET(4,-2) PUT2(0, n + t) break;
            case 0x19: /* SUB  */ t=T;n=N;        SET(2,-1) PUT(0, n - t) break;
            case 0x39:            t=T2;n=N2;      SET(4,-2) PUT2(0, n - t) break;
            case 0x1a: /* MUL  */ t=T;n=N;        SET(2,-1) PUT(0, n * t) break;
            case 0x3a:            t=T2;n=N2;      SET(4,-2) PUT2(0, n * t) break;
            case 0x1b: /* DIV  */ t=T;n=N;        SET(2,-1) if(!t) HALT(3) PUT(0, n / t) break;
            case 0x3b:            t=T2;n=N2;      SET(4,-2) if(!t) HALT(3) PUT2(0, n / t) break;
            case 0x1c: /* AND  */ t=T;n=N;        SET(2,-1) PUT(0, n & t) break;
            case 0x3c:            t=T2;n=N2;      SET(4,-2) PUT2(0, n & t) break;
            case 0x1d: /* ORA  */ t=T;n=N;        SET(2,-1) PUT(0, n | t) break;
            case 0x3d:            t=T2;n=N2;      SET(4,-2) PUT2(0, n | t) break;
            case 0x1e: /* EOR  */ t=T;n=N;        SET(2,-1) PUT(0, n ^ t) break;
            case 0x3e:            t=T2;n=N2;      SET(4,-2) PUT2(0, n ^ t) break;
            case 0x1f: /* SFT  */ t=T;n=N;        SET(2,-1) PUT(0, n >> (t & 0xf) << (t >> 4)) break;
            case 0x3f:            t=T;n=H2;       SET(3,-1) PUT2(0, n >> (t & 0xf) << (t >> 4)) break;
        }
    }
}

int gpu(Uxn *u, Uint16 pc_){


    Params params = {pc_, nullptr, nullptr, 0};

    Uint8 *ram = u->ram;
    Uint16 &pc = params.pc;
    Stack *&s = params.s;
    Stack *&z = params.z;
    int &t = params.t;
    int &n = params.n;
    int &l = params.l;
    int &k = params.k;
    int &tmp = params.tmp;
    int &opc = params.opc;
    int &ins = params.ins;
    int &ret = params.ret;
    int &haltCode = params.haltCode;
    bool &yield = params.yield;

    if(!pc || u->dev[0x0f]) {
        return 0;
    }

    gpu_k(params,u);


    while(yield){
        yield = 0;
        DEI(0, t)
        gpu_k(params,u);
    }

    return ret;

}

int
uxn_eval(Uxn *u, Uint16 pc)
{
    return gpu(u,pc);
}














/**
 * 先初始化指向Uxn的指针u，将其所指向的内存区域全部清零。
 * 然后将传入的ram指针赋值给Uxn结构体的ram成员，即设置Uxn的内存块为ram。
 * 最后返回1，表示函数执行成功
 * @param u
 * @param ram
 * @return
 */
int uxn_boot(Uxn *u, Uint8 *ram) // 定义一个函数，输入是指向Uxn结构体的指针u和指向一个8位无符号整数的指针ram
{
    Uint32 i; // 定义一个32位无符号整数i，用于循环计数
    char *cptr = (char *)u; // 定义一个字符指针cptr，并将u的地址转换为char*类型后赋值给cptr

    for(i = 0; i < sizeof(*u); i++) // 循环，从0开始，直到i等于Uxn结构体的大小（字节数），每次循环i加1
        cptr[i] = 0; // 将cptr指向的内存区域的当前位置（cptr + i）清零

    u->ram = ram; // 将输入的ram指针赋值给Uxn结构体的ram成员

    return 1; // 返回1，表示函数执行成功
}



/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/
#define PAGE_PROGRAM 0x0100 // Define the programming page size as 0x0100

/* clang-format off */

#define POKE2(d, v) { (d)[0] = (v) >> 8; (d)[1] = (v); } //Define a macro POKE2 to write the 16-bit value v to address d
#define PEEK2(d) ((d)[0] << 8 | (d)[1]) //Define a macro PEEK2 to read the 16-bit value from address d

/* clang-format on */

typedef unsigned char Uint8; //Define 8-bit unsigned integer type Uint8
typedef signed char Sint8; // Define the 8-bit signed integer type Sint8
typedef unsigned short Uint16; // Define the 16-bit unsigned integer type Uint16
typedef signed short Sint16; //Define the 16-bit signed integer type Sint16
typedef unsigned int Uint32; // Define the 32-bit unsigned integer type Uint32

typedef struct { // Define a Stack structure
    Uint8 dat[255], ptr; // The dat array is used to store data, and ptr is a pointer used to point to the top of the current stack.
} Stack;

typedef struct Uxn { // Define a Uxn structure
    Uint8 *ram, dev[256]; // ram is a pointer to memory, and the dev array is used to simulate input/output devices
    Stack wst, rst; // wst and rst are two stacks, the work stack and the return stack respectively.
    Uint8 (*dei)(struct Uxn *u, Uint8 addr); // dei is a function pointer used to handle the operation of the input device
    void (*deo)(struct Uxn *u, Uint8 addr); // deo is a function pointer used to handle the operation of the output device
} Uxn;

/* required functions */

extern Uint8 uxn_dei(Uxn *u, Uint8 addr); // Define an external function uxn_dei to handle the operation of the input device
extern void uxn_deo(Uxn *u, Uint8 addr); // Define an external function uxn_deo to handle the operation of the output device
extern int uxn_halt(Uxn *u, Uint8 instr, Uint8 err, Uint16 addr); //Define an external function uxn_halt to handle computer errors.
extern Uint16 dei_mask[]; // Define an external array dei_mask for processing the mask of the input device
extern Uint16 deo_mask[]; // Define an external array deo_mask for processing the mask of the output device

/* built-ins */

int uxn_boot(Uxn *u, Uint8 *ram); //Define a function uxn_boot to initialize the Uxn structure and set its memory block
int uxn_eval(Uxn *u, Uint16 pc); //Define a function uxn_eval for executing Uxn code


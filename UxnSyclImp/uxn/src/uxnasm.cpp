/**
 * 
 * Description:
 * 
 * Created by: Hongbin Bao
 * Created on: 2023/7/10 17:14
 * 
 */
#include <stdio.h>

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define TRIM 0x0100   //0x0100  256
#define LENGTH 0x10000  // 0x10000   65536

typedef unsigned char Uint8;
typedef signed char Sint8;
typedef unsigned short Uint16;
/**
 * 在编程中，Macro（宏）是一种用于自动化的方式，它可以让你定义一段代码，并使用一个名称来引用这段代码。
 * 然后在程序中，每次使用这个名称，编译器就会自动将它替换为定义的代码段。

在你给出的代码中，Macro被定义为一个结构体，包含以下字段：

char name[0x40]：宏的名字，使用一个长度为64的字符数组存储（0x40在十六进制中等于64）。
char items[0x40][0x40]：宏的条目，这是一个二维字符数组，
 可以存储64个长度为64的字符串，可能是表示宏可以包含的最多条目数量。
Uint8 len：宏的长度，可能表示宏包含的条目数量。
举个例子，假设你有一段经常需要使用的代码，这段代码的功能是计算两个数的和。
 你可以将这段代码定义为一个宏，例如定义为ADD(a, b)，
 然后在程序中每次需要计算两个数的和时，就可以直接使用ADD(a, b)，编译器会自动将ADD(a, b)替换为对应的代码段。
 */
typedef struct {
    char name[0x40], items[0x40][0x40];
    Uint8 len;
} Macro;
/**
 * 在汇编语言中，标签（Label）是一个用于标识某个特定位置或一段代码的符号或名称。
 * 当你在编写汇编代码时，可能需要在代码的某些位置设置标签，以便在其他地方引用这些位置
 * start:          ; This is a label
    MOV AX, 10  ; Assembly instruction
    JMP start   ; Jump to the 'start' label
    在这个例子中，start: 是一个标签，它标记了一段代码的开始位置。在 JMP start 这一行，
    JMP 是一个指令，它告诉程序跳转到 start 标签所标记的位置。这就是标签在汇编语言中的作用。
    在你给出的代码中，Label 结构体用于存储这样的标签，每个标签都有一个名称（用于在代码中引用）
    和一个地址（标签在程序中的位置）。标签还有一个 refs 字段，可能用于存储这个标签被引用的次数。
 */
typedef struct {
    char name[0x40];
    Uint16 addr, refs;
} Label;
/**
 * char name[0x40]：这是一个字符数组，长度为64（0x40在十六进制中等于64），用于存储引用的名称。

char rune：一个字符，它的具体含义和用途依赖于你的程序的上下文，但从其他代码段的一些提示来看，这可能是一个用于标识特定类型的引用的字符。

Uint16 addr：这是一个16位无符号整数，用于存储引用的地址。

简单地说，这个Reference结构体可能用于表示汇编语言中的引用，例如你可能要引用一个地址、一个标签（label），或者其他的符号。这个结构体就用来存储这些引用的信息。

举个例子，假设你的程序中有一个标签叫做LOOP_START，它的地址是0x1234，你可能会这样创建一个引用：

c
Copy code
Reference ref;
strcpy(ref.name, "LOOP_START");
ref.rune = ':';
ref.addr = 0x1234;
这样，当你在后面的代码中看到一个:字符，你就知道这是一个引用，需要去查找名为LOOP_START的标签，然后用它的地址0x1234来代替。
 */
typedef struct {
    char name[0x40], rune;
    Uint16 addr;
} Reference;
/**
 * Uint8 data[LENGTH]：这个字段是一个数组，用于存储机器码或汇编代码。

unsigned int ptr, length：这两个字段可能是用于跟踪 data 数组的使用情况。

ptr 可能是下一个要写入 data 的位置，length 可能表示 data 中已使用的字节的数量。

Uint16 llen, mlen, rlen：这三个字段可能分别表示程序中标签（Label）、宏（Macro）和引用（Reference）的数量。

Label labels[0x400]：这个字段是一个数组，存储程序中所有的标签。

Macro macros[0x100]：这个字段是一个数组，存储程序中所有的宏。

Reference refs[0x800]：这个字段是一个数组，存储程序中所有的引用。

char scope[0x40]：这个字段可能表示当前的范围或上下文，比如当前正在解析的函数或模块的名字。

这个结构体基本上包含了一个程序的所有元素和信息，可以说是一个完整程序的内部表示形式。
 Program 结构体的一个实例（比如 p）就表示一个程序。
 */
typedef struct {
    Uint8 data[LENGTH];
    unsigned int ptr, length;
    Uint16 llen, mlen, rlen;
    Label labels[0x400];
    Macro macros[0x100];
    Reference refs[0x800];
    char scope[0x40];
} Program;

Program p;

/* clang-format off */

static char ops[][4] = {
        "LIT", "INC", "POP", "NIP", "SWP", "ROT", "DUP", "OVR",
        "EQU", "NEQ", "GTH", "LTH", "JMP", "JCN", "JSR", "STH",
        "LDZ", "STZ", "LDR", "STR", "LDA", "STA", "DEI", "DEO",
        "ADD", "SUB", "MUL", "DIV", "AND", "ORA", "EOR", "SFT"
};

static int   scmp(char *a, char *b, int len) { int i = 0; while(a[i] == b[i]) if(!a[i] || ++i >= len) return 1; return 0; } /* string compare */
static int   sihx(char *s) { int i = 0; char c; while((c = s[i++])) if(!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) return 0; return i > 1; } /* string is hexadecimal */
static int   shex(char *s) { int n = 0, i = 0; char c; while((c = s[i++])) if(c >= '0' && c <= '9') n = n * 16 + (c - '0'); else if(c >= 'a' && c <= 'f') n = n * 16 + 10 + (c - 'a'); return n; } /* string to num */
static int   slen(char *s) { int i = 0; while(s[i]) i++; return i; } /* string length */
static int   spos(char *s, char c) { Uint8 i = 0, j; while((j = s[i++])) if(j == c) return i; return -1; } /* character position */
static char *scpy(char *src, char *dst, int len) { int i = 0; while((dst[i] = src[i]) && i < len - 2) i++; dst[i + 1] = '\0'; return dst; } /* string copy */
static char *scat(char *dst, const char *src) { char *ptr = dst + slen(dst); while(*src) *ptr++ = *src++; *ptr = '\0'; return dst; } /* string cat */

/* clang-format on */

static int parse(char *w, FILE *f);

static int
error(const char *name, const char *msg)
{
    fprintf(stderr, "%s: %s\n", name, msg);
    return 0;
}

static char *
sublabel(char *src, char *scope, char *name)
{
    if(slen(scope) + slen(name) >= 0x3f) {
        error("Sublabel length too long", name);
        return NULL;
    }
    return scat(scat(scpy(scope, src, 0x40), "/"), name);
}

static Macro *
findmacro(char *name)
{
    int i;
    for(i = 0; i < p.mlen; i++)
        if(scmp(p.macros[i].name, name, 0x40))
            return &p.macros[i];
    return NULL;
}

static Label *
findlabel(char *name)
{
    int i;
    for(i = 0; i < p.llen; i++)
        if(scmp(p.labels[i].name, name, 0x40))
            return &p.labels[i];
    return NULL;
}

static Uint8
findopcode(char *s)
{
    int i;
    for(i = 0; i < 0x20; i++) {
        int m = 0;
        if(!scmp(ops[i], s, 3))
            continue;
        if(!i) i |= (1 << 7); /* force keep for LIT */
        while(s[3 + m]) {
            if(s[3 + m] == '2')
                i |= (1 << 5); /* mode: short */
            else if(s[3 + m] == 'r')
                i |= (1 << 6); /* mode: return */
            else if(s[3 + m] == 'k')
                i |= (1 << 7); /* mode: keep */
            else
                return 0; /* failed to match */
            m++;
        }
        return i;
    }
    return 0;
}

static int
makemacro(char *name, FILE *f)
{
    Macro *m;
    char word[0x40];
    if(findmacro(name))
        return error("Macro duplicate", name);
    if(sihx(name) && slen(name) % 2 == 0)
        return error("Macro name is hex number", name);
    if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
        return error("Macro name is invalid", name);
    if(p.mlen == 0x100)
        return error("Macros limit exceeded", name);
    m = &p.macros[p.mlen++];
    scpy(name, m->name, 0x40);
    while(fscanf(f, "%63s", word) == 1) {
        if(word[0] == '{') continue;
        if(word[0] == '}') break;
        if(word[0] == '%')
            return error("Macro error", name);
        if(m->len >= 0x40)
            return error("Macro size exceeded", name);
        scpy(word, m->items[m->len++], 0x40);
    }
    return 1;
}

static int
makelabel(char *name)
{
    Label *l;
    if(findlabel(name))
        return error("Label duplicate", name);
    if(sihx(name) && (slen(name) == 2 || slen(name) == 4))
        return error("Label name is hex number", name);
    if(findopcode(name) || scmp(name, "BRK", 4) || !slen(name))
        return error("Label name is invalid", name);
    if(p.llen == 0x400)
        return error("Labels limit exceeded", name);
    l = &p.labels[p.llen++];
    l->addr = p.ptr;
    l->refs = 0;
    scpy(name, l->name, 0x40);
    return 1;
}

static int
makereference(char *scope, char *label, char rune, Uint16 addr)
{
    char subw[0x40], parent[0x40];
    Reference *r;
    if(p.rlen >= 0x800)
        return error("References limit exceeded", label);
    r = &p.refs[p.rlen++];
    if(label[0] == '&') {
        if(!sublabel(subw, scope, label + 1))
            return error("Invalid sublabel", label);
        scpy(subw, r->name, 0x40);
    } else {
        int pos = spos(label, '/');
        if(pos > 0) {
            Label *l;
            if((l = findlabel(scpy(label, parent, pos))))
                l->refs++;
        }
        scpy(label, r->name, 0x40);
    }
    r->rune = rune;
    r->addr = addr;
    return 1;
}

static int
writebyte(Uint8 b)
{
    if(p.ptr < TRIM)
        return error("Writing in zero-page", "");
    else if(p.ptr > 0xffff)
        return error("Writing after the end of RAM", "");
    else if(p.ptr < p.length)
        return error("Memory overwrite", "");
    p.data[p.ptr++] = b;
    p.length = p.ptr;
    return 1;
}

static int
writeopcode(char *w)
{
    return writebyte(findopcode(w));
}

static int
writeshort(Uint16 s, int lit)
{
    if(lit)
        if(!writebyte(findopcode("LIT2"))) return 0;
    return writebyte(s >> 8) && writebyte(s & 0xff);
}

static int
writelitbyte(Uint8 b)
{
    return writebyte(findopcode("LIT")) && writebyte(b);
}
/**
 * 这段代码定义了一个函数doinclude，它接收一个字符串参数filename，这个字符串代表了一个文件名。这个函数的主要功能是打开这个文件，并逐个解析文件中的每一个字符串（也就是每个词或者标记），然后关闭文件。

以下是该函数的具体步骤：

使用fopen函数打开文件，打开模式为只读模式（"r"）。

如果文件不存在，fopen函数返回NULL，此时调用error函数报错，并返回。

如果文件成功打开，它会使用一个while循环，通过fscanf函数从文件中读取字符串，每次读取一个，最多读取63个字符。

对每一个读取到的字符串w，调用parse函数进行处理。parse函数之前已经定义过，它用来解析字符串并执行对应的操作。

如果parse函数返回失败（返回0），则doinclude函数也返回失败，并调用error函数报错。

当文件中的所有字符串都处理完毕后，使用fclose函数关闭文件，并返回成功（1）。

总的来说，这个函数的主要作用是解析文件，并对其中的每个字符串进行处理。它在汇编编程中非常重要，因为它允许开发者在一个文件中引用另一个文件，这使得代码组织和管理更加方便
 * @param filename
 * @return
 */
static int
doinclude(const char *filename)
{
    FILE *f;
    char w[0x40];
    if(!(f = fopen(filename, "r")))
        return error("Include missing", filename);
    while(fscanf(f, "%63s", w) == 1)
        if(!parse(w, f))
            return error("Unknown token", w);
    fclose(f);
    return 1;
}
/**
 * 这个函数接收两个参数，一个是char *w，另一个是FILE *f。char *w是一个指向字符的指针，可能指向的是要解析的单词或标记。FILE *f是一个指向文件的指针，该文件可能包含要解析的代码或数据。

该函数首先检查w的长度是否超过63，如果超过则返回错误。

然后该函数使用一个switch语句对w[0]进行检查。w[0]是w的第一个字符，这可能是一个标识符，用来决定如何解析w的其余部分。switch语句包含多个case，每个case处理一个可能的标识符。

这里的case语句涵盖了大量的情况，例如：

'('：这可能表示一个注释，函数将读取并忽略直到匹配的)出现。
'~'：这可能表示一个包含语句，函数将试图包含w指定的文件。
'%'：这可能表示一个宏，函数将尝试创建一个新的宏。
'@'：这可能表示一个标签，函数将尝试创建一个新的标签。
'&'：这可能表示一个子标签，函数将尝试创建一个新的子标签。
'_'、','、'-'、'.'、':'、';'、'?'、'!'：这些都可能表示某种引用或字面量，函数将尝试创建一个新的引用并写入一个字节或者短整数值。
在处理所有可能的特殊标识符之后，如果w不符合任何特殊情况，函数将试图将其解析为一个操作码、原始字节、原始短整数或宏。如果都不是，函数将默认创建一个新的引用，并写入一个指定的值。

最后，如果没有出错，函数将返回1表示成功。如果在处理过程中出现任何错误，函数将返回错误信息并停止执行。
 * @param w
 * @param f
 * @return
 */
static int
parse(char *w, FILE *f)
{
    int i;
    char word[0x40], subw[0x40], c;
    Label *l;
    Macro *m;
    if(slen(w) >= 63)
        return error("Invalid token", w);
    switch(w[0]) {
        case '(': /* comment */
            if(slen(w) != 1) fprintf(stderr, "-- Malformed comment: %s\n", w);
            i = 1; /* track nested comment depth */
            while(fscanf(f, "%63s", word) == 1) {
                if(slen(word) != 1)
                    continue;
                else if(word[0] == '(')
                    i++;
                else if(word[0] == ')' && --i < 1)
                    break;
            }
            break;
        case '~': /* include */
            if(!doinclude(w + 1))
                return error("Invalid include", w);
            break;
        case '%': /* macro */
            if(!makemacro(w + 1, f))
                return error("Invalid macro", w);
            break;
        case '|': /* pad-absolute */
            if(sihx(w + 1))
                p.ptr = shex(w + 1);
            else if(w[1] == '&') {
                if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
                    return error("Invalid sublabel", w);
                p.ptr = l->addr;
            } else {
                if(!(l = findlabel(w + 1)))
                    return error("Invalid label", w);
                p.ptr = l->addr;
            }
            break;
        case '$': /* pad-relative */
            if(sihx(w + 1))
                p.ptr += shex(w + 1);
            else if(w[1] == '&') {
                if(!sublabel(subw, p.scope, w + 2) || !(l = findlabel(subw)))
                    return error("Invalid sublabel", w);
                p.ptr += l->addr;
            } else {
                if(!(l = findlabel(w + 1)))
                    return error("Invalid label", w);
                p.ptr += l->addr;
            }
            break;
        case '@': /* label */
            if(!makelabel(w + 1))
                return error("Invalid label", w);
            scpy(w + 1, p.scope, 0x40);
            break;
        case '&': /* sublabel */
            if(!sublabel(subw, p.scope, w + 1) || !makelabel(subw))
                return error("Invalid sublabel", w);
            break;
        case '#': /* literals hex */
            if(sihx(w + 1) && slen(w) == 3)
                return writelitbyte(shex(w + 1));
            else if(sihx(w + 1) && slen(w) == 5)
                return writeshort(shex(w + 1), 1);
            else
                return error("Invalid hex literal", w);
            break;
        case '_': /* raw byte relative */
            makereference(p.scope, w + 1, w[0], p.ptr);
            return writebyte(0xff);
        case ',': /* literal byte relative */
            makereference(p.scope, w + 1, w[0], p.ptr + 1);
            return writelitbyte(0xff);
        case '-': /* raw byte absolute */
            makereference(p.scope, w + 1, w[0], p.ptr);
            return writebyte(0xff);
        case '.': /* literal byte zero-page */
            makereference(p.scope, w + 1, w[0], p.ptr + 1);
            return writelitbyte(0xff);
        case ':':
        case '=': /* raw short absolute */
            makereference(p.scope, w + 1, w[0], p.ptr);
            return writeshort(0xffff, 0);
        case ';': /* literal short absolute */
            makereference(p.scope, w + 1, w[0], p.ptr + 1);
            return writeshort(0xffff, 1);
        case '?': /* JCI */
            makereference(p.scope, w + 1, w[0], p.ptr + 1);
            return writebyte(0x20) && writeshort(0xffff, 0);
        case '!': /* JMI */
            makereference(p.scope, w + 1, w[0], p.ptr + 1);
            return writebyte(0x40) && writeshort(0xffff, 0);
        case '"': /* raw string */
            i = 0;
            while((c = w[++i]))
                if(!writebyte(c)) return 0;
            break;
        case '[':
        case ']':
            if(slen(w) == 1) break; /* else fallthrough */
        default:
            /* opcode */
            if(findopcode(w) || scmp(w, "BRK", 4))
                return writeopcode(w);
                /* raw byte */
            else if(sihx(w) && slen(w) == 2)
                return writebyte(shex(w));
                /* raw short */
            else if(sihx(w) && slen(w) == 4)
                return writeshort(shex(w), 0);
                /* macro */
            else if((m = findmacro(w))) {
                for(i = 0; i < m->len; i++)
                    if(!parse(m->items[i], f))
                        return 0;
                return 1;
            } else {
                makereference(p.scope, w, ' ', p.ptr + 1);
                return writebyte(0x60) && writeshort(0xffff, 0);
            }
    }
    return 1;
}

static int
resolve(void)
{   // 标签
    Label *l;
    int i;
    Uint16 a;
    for(i = 0; i < p.rlen; i++) {
        Reference *r = &p.refs[i];
        switch(r->rune) {
            //对于'_'、','的情况：它认为它是相对引用，因此它计算相对偏移量（标签地址和引用地址之间的差值减2）。
            // 如果差异太大而无法放入 a 中Sint8，则会返回错误。然后将该引用存储在Sint8地址为r->addr的变量中p.data
            case '_':
            case ',':
                if(!(l = findlabel(r->name)))
                    return error("Unknown relative reference", r->name);
                p.data[r->addr] = (Sint8)(l->addr - r->addr - 2);
                if((Sint8)p.data[r->addr] != (l->addr - r->addr - 2))
                    return error("Relative reference is too far", r->name);
                l->refs++;
                break;
                // 对于“-”、“.”的情况：它是零页引用，因此标签地址的低 8 位存储在r->addr中p.data。
            case '-':
            case '.':
                if(!(l = findlabel(r->name)))
                    return error("Unknown zero-page reference", r->name);
                p.data[r->addr] = l->addr & 0xff;
                l->refs++;
                break;
                // 对于“:”、“=”、“;”的情况：它是绝对引用。
                // 这里，标签的地址以 16 位大端数的形式存储在p.data该地址中（先是高字节，然后是低字节）。r->addr
            case ':':
            case '=':
            case ';':
                if(!(l = findlabel(r->name)))
                    return error("Unknown absolute reference", r->name);
                p.data[r->addr] = l->addr >> 0x8;
                p.data[r->addr + 1] = l->addr & 0xff;
                l->refs++;
                break;
                //对于“?”、“!”的情况：也是绝对引用，但标签地址和引用地址之间的差减 2 存储为 16 位大端数字。
            case '?':
            case '!':
            default:
                if(!(l = findlabel(r->name)))
                    return error("Unknown absolute reference", r->name);
                a = l->addr - r->addr - 2;
                p.data[r->addr] = a >> 0x8;
                p.data[r->addr + 1] = a & 0xff;
                l->refs++;
                break;
        }
    }
    return 1;
}
/**
 *
 * 该函数的每个部分的作用如下：
 * char w[0x40];w声明一个大小为0x40（十进制 64）的字符数组。这将保存从程序集文件中读取的每个标记。
 * p.ptr = 0x100;将ptr某个全局结构的字段设置p为0x100（十进制 256）。p及其ptr字段未在此代码段中定义，但基于汇编程序中的常见模式，ptr可以表示内存指针或类似的概念。
 * scpy("on-reset", p.scope, 0x40);似乎正在将字符串“on-reset”复制到 的scope字段p，大概使用不会溢出目的地的安全复制函数。
 * while(fscanf(f, "%62s", w) == 1)一次从文件中读取最多 62 个非空白字符f，并将它们存储在w. 它会一直持续到fscanf无法读取令牌为止，这可能是因为它已到达文件末尾。
 * if(slen(w) > 0x3d || !parse(w, f))检查每个读取的令牌。slen(w)大概计算 , 的字符串长度w，0x3d十进制为 61。如果令牌的长度超过 61，或者parse(w, f)返回 0（可能表明parse失败），则调用error("Invalid token", w)并返回其结果。
 * 如果它遍历整个文件而没有出现错误，return resolve();则执行。该resolve函数未在此代码段中定义，但在汇编程序中，这样的函数通常负责解析符号，例如标签的地址。
 * @param f
 * @return
 */
static int
assemble(FILE *f)
{
    char w[0x40];
    p.ptr = 0x100;
    scpy("on-reset", p.scope, 0x40);
    while(fscanf(f, "%62s", w) == 1)
        if(slen(w) > 0x3d || !parse(w, f))
            return error("Invalid token", w);
    return resolve();
}
/**
 * 作用是遍历所有的标签（labels）并打印出一些汇编代码的统计信息。

代码的主要步骤如下：

循环遍历 p.labels 数组，这个数组存储了所有的标签。

如果标签的第一个字符是大写的（也就是ASCII码在'A'和'Z'之间），那么忽略它并进入下一个循环。这个规则似乎是用来忽略设备名或其他特定种类的标签。

如果某个标签没有被引用过（p.labels[i].refs 的值为 0），那么将这个标签的名字打印到标准错误输出流，前缀为 -- Unused label:。

最后，向标准错误输出流打印一条总结消息，包含以下信息：

汇编的文件名。
汇编后的字节大小（p.length - TRIM）以及这个大小占用 6502 CPU 的内存容量的百分比。6502 是一种8位的微处理器，最大可以寻址64KB的内存，所以这里将总大小除以 652.80（即64KB的百分之一）来计算使用的百分比。
总的标签数量 (p.llen)。
总的宏数量 (p.mlen)。
 * @param filename
 */
static void
review(char *filename)
{
    int i;
    for(i = 0; i < p.llen; i++)
        if(p.labels[i].name[0] >= 'A' && p.labels[i].name[0] <= 'Z')
            continue; /* Ignore capitalized labels(devices) */
        else if(!p.labels[i].refs)
            fprintf(stderr, "-- Unused label: %s\n", p.labels[i].name);
    fprintf(stderr,
            "Assembled %s in %d bytes(%.2f%% used), %d labels, %d macros.\n",
            filename,
            p.length - TRIM,
            (p.length - TRIM) / 652.80,
            p.llen,
            p.mlen);
}

static void
writesym(char *filename)
{
    int i;
    char symdst[0x60];
    FILE *fp;
    if(slen(filename) > 0x60 - 5)
        return;
    fp = fopen(scat(scpy(filename, symdst, slen(filename) + 1), ".sym"), "w");
    if(fp != NULL) {
        for(i = 0; i < p.llen; i++) {
            Uint8 hb = p.labels[i].addr >> 8, lb = p.labels[i].addr & 0xff;
            fwrite(&hb, 1, 1, fp);
            fwrite(&lb, 1, 1, fp);
            fwrite(p.labels[i].name, slen(p.labels[i].name) + 1, 1, fp);
        }
    }
    fclose(fp);
}

int
main(int argc, char *argv[])
// argc保存命令行上提供的参数的计数，并且argv是这些参数的数组
{   // src（source的缩写，意为源）和dst（destination的缩写，意为目标）
    FILE *src, *dst;
    // argc表示命令行参数的计数，并且argv是这些参数的数组。数组的第一个元素（索引 0）始终是程序本身的名称，因此如果argc小于 3，则意味着未提供足够的参数
    // 因为正常的程序调用应该是这样的："uxnasm input.tal output.rom"，其中"uxnasm"是程序名，"input.tal"和"output.rom"是两个参数，所以总共应该有3个命令行参数。
    if(argc < 3)
        return !error("usage", "uxnasm input.tal output.rom");
    if(!(src = fopen(argv[1], "r")))
        return !error("Invalid input", argv[1]);
    if(!assemble(src))
        return !error("Assembly", "Failed to assemble rom.");
    if(!(dst = fopen(argv[2], "wb")))
        return !error("Invalid Output", argv[2]);
    if(p.length <= TRIM)
        return !error("Assembly", "Output rom is empty.");
    fwrite(p.data + TRIM, p.length - TRIM, 1, dst);
    review(argv[2]);
    writesym(argv[2]);
    return 0;
}

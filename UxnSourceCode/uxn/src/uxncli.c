#include <stdio.h>
#include <stdlib.h>

#include "uxn.h"
#include "devices/system.h"
#include "devices/file.h"
#include "devices/datetime.h"

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/
// 一个简单的UXN虚拟机的CLI（命令行界面）实现。

//这些掩码用于设备输入输出操作
Uint16 deo_mask[] = {0xc028, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
Uint16 dei_mask[] = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};
/**
 * 函数根据地址读取设备的数据
 * 这个函数首先检查设备地址的高4位（通过&操作实现）。
 * 在这个例子中，如果地址的高4位为0xc0，则调用datetime_dei函数进行操作。否则，它会直接从虚拟机的设备数组中读取相应的值。
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
 * 函数负责将数据写入设备
 * 将设备地址分解为高4位和低4位。然后根据高4位的值来调用对应的设备操作函数。
 * 在这个例子中，如果高4位是0x00，那么就调用system_deo函数，
 * 如果是0x10，那么就调用console_deo函数，如果是0xa0或0xb0，则调用file_deo函数。
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
 * 它检查命令行参数，如果没有提供ROM文件名，则会报错并退出。然后，它初始化一个UXN虚拟机，并为虚拟机的内存分配空间。接着，它从ROM文件中加载程序到虚拟机，
 * 然后设置设备的一些状态，之后开始执行虚拟机中的程序。如果虚拟机程序需要参数，那么它将会从命令行参数中获取。最后，当虚拟机停止运行时，释放分配的内存，并返回虚拟机的状态
 * argc 是整数参数，用于表示命令行参数的数量，而 argv 是一个指向字符串指针的指针，用于表示命令行参数的字符串数组。
 * @param argc
 * @param argv
 * @return
 */
int
main(int argc, char **argv)
{
	Uxn u;
	int i = 1;
	if(i == argc)
		return system_error("usage", "uxncli file.rom [args..]");
	if(!uxn_boot(&u, (Uint8 *)calloc(0x10000 * RAM_PAGES, sizeof(Uint8))))
		return system_error("Boot", "Failed");
	if(!system_load(&u, argv[i++]))
		return system_error("Load", "Failed");
	u.dev[0x17] = argc - i;
	if(uxn_eval(&u, PAGE_PROGRAM)) {
		for(; i < argc; i++) {
			char *p = argv[i];
			while(*p) console_input(&u, *p++, CONSOLE_ARG);
			console_input(&u, '\n', i == argc - 1 ? CONSOLE_END : CONSOLE_EOA);
		}
		while(!u.dev[0x0f]) {
			int c = fgetc(stdin);
			if(c != EOF) console_input(&u, (Uint8)c, CONSOLE_STD);
		}
	}
	free(u.ram);
	return u.dev[0x0f] & 0x7f;
}

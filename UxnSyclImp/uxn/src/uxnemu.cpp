/**
 * 
 * Description:
 * 
 * Created by: Hongbin Bao
 * Created on: 2023/7/10
 * 
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <CL/sycl.hpp>
#include "uxn.h"

#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wtypedef-redefinition"
#include <SDL.h>
#include <iostream>
#include <hipSYCL/sycl/queue.hpp>

#include "devices/system.h"
#include "devices/screen.h"
#include "devices/audio.h"
#include "devices/file.h"
#include "devices/controller.h"
#include "devices/mouse.h"
#include "devices/datetime.h"

#define MOVE_THRESHOLD 5

//#define SMOOTH_FACTOR 0.5

#if defined(_WIN32) && defined(_WIN32_WINNT) && _WIN32_WINNT > 0x0602
#include <processthreadsapi.h>
#elif defined(_WIN32)
#include <windows.h>
#include <string.h>
#endif
#ifndef __plan9__
#define USED(x) (void)(x)
#endif
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

/*
Copyright (c) 2021-2023 Devine Lu Linvega, Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

#define WIDTH 64 * 8
#define HEIGHT 40 * 8
#define PAD 2
#define TIMEOUT_MS 334
#define BENCH 0

static SDL_Window *emu_window;
static SDL_Texture *emu_texture;
static SDL_Renderer *emu_renderer;
static SDL_AudioDeviceID audio_id;
static SDL_Rect emu_frame;
static SDL_Thread *stdin_thread;

Uint16 deo_mask[] = {0xff28, 0x0300, 0xc028, 0x8000, 0x8000, 0x8000, 0x8000, 0x0000, 0x0000, 0x0000, 0xa260, 0xa260, 0x0000, 0x0000, 0x0000, 0x0000};
Uint16 dei_mask[] = {0x0000, 0x0000, 0x003c, 0x0014, 0x0014, 0x0014, 0x0014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x07ff, 0x0000, 0x0000, 0x0000};

/* devices */

static Uint32 stdin_event, audio0_event, zoom = 1;
static Uint64 exec_deadline, deadline_interval, ms_interval;

char *rom_path;

static int
clamp(int val, int min, int max)
{
    return (val >= min) ? (val <= max) ? val : max : min;
}

static Uint8
audio_dei(int instance, Uint8 *d, Uint8 port)
{
    if(!audio_id) return d[port];
    switch(port) {
        case 0x4: return audio_get_vu(instance);
        case 0x2: POKE2(d + 0x2, audio_get_position(instance)); /* fall through */
        default: return d[port];
    }
}

static void
audio_deo(int instance, Uint8 *d, Uint8 port, Uxn *u)
{
    if(!audio_id) return;
    if(port == 0xf) {
        SDL_LockAudioDevice(audio_id);
        audio_start(instance, d, u);
        SDL_UnlockAudioDevice(audio_id);
        SDL_PauseAudioDevice(audio_id, 0);
    }
}

// 输入函数实现
Uint8
uxn_dei(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0; // 提取设备编号和端口号
    switch(d) {  // 判断设备编号
        case 0x20: return screen_dei(u, addr); // 屏幕设备输入
        case 0x30: return audio_dei(0, &u->dev[d], p); // 音频设备输入1
        case 0x40: return audio_dei(1, &u->dev[d], p); // 音频设备输入2
        case 0x50: return audio_dei(2, &u->dev[d], p); // 音频设备输入3
        case 0x60: return audio_dei(3, &u->dev[d], p); // 音频设备输入4
        case 0xc0: return datetime_dei(u, addr); // 时间日期设备输入
    }
    return u->dev[addr];  // 无匹配设备，返回指定地址的设备状态
}

// 输出函数实现
void
uxn_deo(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0;  // 提取设备编号和端口号
    switch(d) {  // 判断设备编号
        case 0x00:  // 系统设备
            system_deo(u, &u->dev[d], p);  // 系统设备输出
            if(p > 0x7 && p < 0xe)
                screen_palette(&u->dev[0x8]);  // 屏幕颜色输出
            break;
        case 0x10: console_deo(&u->dev[d], p); break; // 控制台设备输出
        case 0x20: screen_deo(u->ram, &u->dev[d], p); break; // 屏幕设备输出
        case 0x30: audio_deo(0, &u->dev[d], p, u); break; // 音频设备输出1
        case 0x40: audio_deo(1, &u->dev[d], p, u); break; // 音频设备输出2
        case 0x50: audio_deo(2, &u->dev[d], p, u); break; // 音频设备输出3
        case 0x60: audio_deo(3, &u->dev[d], p, u); break; // 音频设备输出4
        case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break; // 文件设备输出1
        case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break; // 文件设备输出2
    }
}


#pragma mark - Generics

static void
audio_callback(void *u, Uint8 *stream, int len)
{
    int instance, running = 0;
    Sint16 *samples = (Sint16 *)stream;
    USED(u);
    SDL_memset(stream, 0, len);
    for(instance = 0; instance < POLYPHONY; instance++)
        running += audio_render(instance, samples, samples + len / 2);
    if(!running)
        SDL_PauseAudioDevice(audio_id, 1);
}

void
audio_finished_handler(int instance)
{
    SDL_Event event;
    event.type = audio0_event + instance;
    SDL_PushEvent(&event);
}

static int
stdin_handler(void *p)
{
    SDL_Event event;
    USED(p);
    event.type = stdin_event;
    while(read(0, &event.cbutton.button, 1) > 0 && SDL_PushEvent(&event) >= 0)
        ;
    return 0;
}

static void
set_window_size(SDL_Window *window, int w, int h)
{
    SDL_Point win, win_old;
    SDL_GetWindowPosition(window, &win.x, &win.y);
    SDL_GetWindowSize(window, &win_old.x, &win_old.y);
    if(w == win_old.x && h == win_old.y) return;
    SDL_RenderClear(emu_renderer);
    /* SDL_SetWindowPosition(window, (win.x + win_old.x / 2) - w / 2, (win.y + win_old.y / 2) - h / 2); */
    SDL_SetWindowSize(window, w, h);
}

static int
set_size(void)
{
    emu_frame.x = PAD;
    emu_frame.y = PAD;
    emu_frame.w = uxn_screen.width;
    emu_frame.h = uxn_screen.height;
    if(emu_texture != NULL)
        SDL_DestroyTexture(emu_texture);
    SDL_RenderSetLogicalSize(emu_renderer, uxn_screen.width + PAD * 2, uxn_screen.height + PAD * 2);
    emu_texture = SDL_CreateTexture(emu_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, uxn_screen.width, uxn_screen.height);
    if(emu_texture == NULL || SDL_SetTextureBlendMode(emu_texture, SDL_BLENDMODE_NONE))
        return system_error("SDL_SetTextureBlendMode", SDL_GetError());
    if(SDL_UpdateTexture(emu_texture, NULL, uxn_screen.pixels, sizeof(Uint32)) != 0)
        return system_error("SDL_UpdateTexture", SDL_GetError());
    set_window_size(emu_window, (uxn_screen.width + PAD * 2) * zoom, (uxn_screen.height + PAD * 2) * zoom);
    return 1;
}

static void
redraw(void)
{
    if(emu_frame.w != uxn_screen.width || emu_frame.h != uxn_screen.height)
        set_size();
    screen_redraw();
    if(SDL_UpdateTexture(emu_texture, NULL, uxn_screen.pixels, uxn_screen.width * sizeof(Uint32)) != 0)
        system_error("SDL_UpdateTexture", SDL_GetError());
    SDL_RenderCopy(emu_renderer, emu_texture, NULL, &emu_frame);
    SDL_RenderPresent(emu_renderer);
}

static int
init(void)
{
    SDL_AudioSpec as;
    SDL_zero(as);
    as.freq = SAMPLE_FREQUENCY;
    as.format = AUDIO_S16SYS;
    as.channels = 2;
    as.callback = audio_callback;
    as.samples = 512;
    as.userdata = NULL;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
        return system_error("sdl", SDL_GetError());
    emu_window = SDL_CreateWindow("Uxn", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (WIDTH + PAD * 2) * zoom, (HEIGHT + PAD * 2) * zoom, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if(emu_window == NULL)
        return system_error("sdl_window", SDL_GetError());
    emu_renderer = SDL_CreateRenderer(emu_window, -1, 0);
    if(emu_renderer == NULL)
        return system_error("sdl_renderer", SDL_GetError());
    SDL_SetRenderDrawColor(emu_renderer, 0x00, 0x00, 0x00, 0xff);
    audio_id = SDL_OpenAudioDevice(NULL, 0, &as, NULL, 0);
    if(!audio_id)
        system_error("sdl_audio", SDL_GetError());
    if(SDL_NumJoysticks() > 0 && SDL_JoystickOpen(0) == NULL)
        system_error("sdl_joystick", SDL_GetError());
    stdin_event = SDL_RegisterEvents(1);
    audio0_event = SDL_RegisterEvents(POLYPHONY);
    SDL_DetachThread(stdin_thread = SDL_CreateThread(stdin_handler, "stdin", NULL));
    SDL_StartTextInput();
    SDL_ShowCursor(SDL_DISABLE);
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    ms_interval = SDL_GetPerformanceFrequency() / 1000;
    deadline_interval = ms_interval * TIMEOUT_MS;
    return 1;
}

#pragma mark - Devices

/* Boot */
/**
 * 主要负责启动Uxn系统并加载rom文件
 * @param u
 * @param rom
 * @param queue
 * @return
 */
static int
start(Uxn *u, char *rom, int queue,cl::sycl::queue& deviceQueue)
{   // 释放之前的内存
    //free(u->ram);
    cl::sycl::free(u->ram, deviceQueue);
    //cl::sycl::free(pc,deviceQueue);


    //cl::sycl::queue deviceQueue; // Initialize a SYCL queue

    uint8_t* p = malloc_shared<uint8_t>(0x10000 * RAM_PAGES, deviceQueue);
    //uint16_t* pc = malloc_shared<uint16_t>(1, deviceQueue);
    // *pc = PAGE_PROGRAM;
    // 启动uxn 为其分配内存空间
//    if(!uxn_boot(u, (Uint8 *)calloc(0x10000 * RAM_PAGES, sizeof(Uint8))))
    if(!uxn_boot(u, p))
        return system_error("Boot", "Failed to start uxn.");
    // 加载rom 文件
    if(!system_load(u, rom))
        return system_error("Boot", "Failed to load rom.");
    // 将命令行参数数量 存入设备内存中的特定位置
    u->dev[0x17] = queue;
    // 设置执行截止时间
    exec_deadline = SDL_GetPerformanceCounter() + deadline_interval;

    Uint16* pc = cl::sycl::malloc_shared<Uint16>(1, deviceQueue);
    *pc = PAGE_PROGRAM;
    u->params = cl::sycl::malloc_shared<Params>(sizeof(Params),deviceQueue);
    u->queue = deviceQueue;
    // 执行rom 中的程序
    if(!uxn_eval(u, *pc))
        return system_error("Boot", "Failed to eval rom.");

    // 将窗口标题设置为rom 文件名
    SDL_SetWindowTitle(emu_window, rom);
    cl::sycl::free(pc, deviceQueue);
    return 1;
}

static void
set_zoom(Uint8 z)
{
    if(z >= 1) {
        zoom = z;
        set_window_size(emu_window, (uxn_screen.width + PAD * 2) * zoom, (uxn_screen.height + PAD * 2) * zoom);
    }
}

static void
capture_screen(void)
{
    const Uint32 format = SDL_PIXELFORMAT_RGB24;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    /* SDL_PIXELFORMAT_RGB24 */
	Uint32 Rmask = 0x000000FF;
	Uint32 Gmask = 0x0000FF00;
	Uint32 Bmask = 0x00FF0000;
#else
    /* SDL_PIXELFORMAT_BGR24 */
    Uint32 Rmask = 0x00FF0000;
    Uint32 Gmask = 0x0000FF00;
    Uint32 Bmask = 0x000000FF;
#endif
    time_t t = time(NULL);
    char fname[64];
    int w, h;
    SDL_Surface *surface;
    SDL_GetRendererOutputSize(emu_renderer, &w, &h);
    surface = SDL_CreateRGBSurface(0, w, h, 24, Rmask, Gmask, Bmask, 0);
    SDL_RenderReadPixels(emu_renderer, NULL, format, surface->pixels, surface->pitch);
    strftime(fname, sizeof(fname), "screenshot-%Y%m%d-%H%M%S.bmp", localtime(&t));
    SDL_SaveBMP(surface, fname);
    SDL_FreeSurface(surface);
    fprintf(stderr, "Saved %s\n", fname);
    fflush(stderr);
}

static void
restart(Uxn *u,cl::sycl::queue& deviceQueue)
{
    screen_resize(WIDTH, HEIGHT);
    if(!start(u, "launcher.rom", 0,deviceQueue))
        start(u, rom_path, 0,deviceQueue);
}

static Uint8
get_button(SDL_Event *event)
{
    switch(event->key.keysym.sym) {
        case SDLK_LCTRL: return 0x01;
        case SDLK_LALT: return 0x02;
        case SDLK_LSHIFT: return 0x04;
        case SDLK_HOME: return 0x08;
        case SDLK_UP: return 0x10;
        case SDLK_DOWN: return 0x20;
        case SDLK_LEFT: return 0x40;
        case SDLK_RIGHT: return 0x80;
    }
    return 0x00;
}

static Uint8
get_button_joystick(SDL_Event *event)
{
    return 0x01 << (event->jbutton.button & 0x3);
}

static Uint8
get_vector_joystick(SDL_Event *event)
{
    if(event->jaxis.value < -3200)
        return 1;
    if(event->jaxis.value > 3200)
        return 2;
    return 0;
}

static Uint8
get_key(SDL_Event *event)
{
    int sym = event->key.keysym.sym;
    SDL_Keymod mods = SDL_GetModState();
    if(sym < 0x20 || sym == SDLK_DELETE)
        return sym;
    if(mods & KMOD_CTRL) {
        if(sym < SDLK_a)
            return sym;
        else if(sym <= SDLK_z)
            return sym - (mods & KMOD_SHIFT) * 0x20;
    }
    return 0x00;
}

static void
do_shortcut(Uxn *u, SDL_Event *event,cl::sycl::queue& deviceQueue)
{
    if(event->key.keysym.sym == SDLK_F1)
        set_zoom(zoom == 3 ? 1 : zoom + 1);
    else if(event->key.keysym.sym == SDLK_F2)
        system_inspect(u);
    else if(event->key.keysym.sym == SDLK_F3)
        capture_screen();
    else if(event->key.keysym.sym == SDLK_F4)
        restart(u,deviceQueue);
}

/**
 * 在代码中，添加一个MouseState类型的静态变量来保存当前鼠标的状态，包括鼠标的位置，滚轮的状态，以及鼠标的按键状态。
 * 然后，将处理鼠标移动，鼠标滚轮移动，鼠标按键按下，和鼠标按键释放的代码统一放在一个函数mouse_state_update中。
 * 在每一次SDL事件循环中，如果发现有鼠标的相关事件，我就更新MouseState变量的状态，并调用mouse_state_update函数处理这个事件。
 * 同时，保留了上一次鼠标的状态，只有当鼠标的状态发生改变的时候，才会调用相关的处理函数。这样，就可以减少不必要的函数调用和设备与主机间的通讯。
 * *
 * 这种优化方法的思路是减少对设备的操作。对设备的每一次操作，都会涉及到设备与主机之间的数据传输，这是一种相对昂贵的操作。
 * 如果能够通过在主机端预处理数据，减少对设备的操作，就可以提高效率。
 * 通过在主机端保存鼠标的状态，并只在状态发生改变的时候调用处理函数，从而减少了对设备的操作，提高了效率。
 * 同时，通过合并处理多种鼠标事件的代码，还减少了代码的冗余。
 * 在原来的代码中，处理鼠标移动，鼠标滚轮移动，鼠标按键按下，
 * 和鼠标按键释放的代码是分散在不同的地方的。在代码中，将这些代码统一放在了一个函数中，减少了代码的冗余，提高了代码的可读性和可维护性。
 */
// 定义鼠标状态的数据结构，包含了当前和上次鼠标的位置、按键状态、滚轮位置等信息
typedef struct MouseState {
    int x, y, button;  // 当前鼠标的位置和按键状态
    int lastX, lastY, lastButton;  // 上次鼠标的位置和按键状态
    bool isButtonDown;  // 鼠标是否按下的状态
    SDL_Point wheel, lastWheel;  // 当前和上次滚轮的位置
    int accumulatedDeltaX, accumulatedDeltaY;  // Accumulated mouse movement
} MouseState;

// 更新并处理鼠标状态的函数
void mouse_state_update(Uxn* u, MouseState* ms, int eventType) {
    // 如果鼠标位置发生了变化，更新鼠标位置
    if(ms->x != ms->lastX || ms->y != ms->lastY)
        mouse_pos(u, &u->dev[0x90], ms->x, ms->y);
    // 如果滚轮位置发生了变化，处理滚轮滚动事件
    if(ms->wheel.x != ms->lastWheel.x || ms->wheel.y != ms->lastWheel.y)
        mouse_scroll(u, &u->dev[0x90], ms->wheel.x, ms->wheel.y);

    // 如果检测到鼠标按键释放事件，处理按键释放
    if(eventType == SDL_MOUSEBUTTONUP)
        mouse_up(u, &u->dev[0x90], ms->button);
        // 如果检测到鼠标按键按下事件，处理按键按下
    else if(eventType == SDL_MOUSEBUTTONDOWN)
        mouse_down(u, &u->dev[0x90], ms->button);

    // 更新上次的鼠标状态为当前状态，为下一次比较做准备
    ms->lastX = ms->x;
    ms->lastY = ms->y;
    ms->lastWheel = ms->wheel;
}




static int
handle_events(Uxn *u,cl::sycl::queue& deviceQueue)
{
    SDL_Event event;
    static MouseState ms = {0};

    while(SDL_PollEvent(&event)) {
        /* Window */
        if(event.type == SDL_QUIT){
            SDL_Quit();
            exit(0);
        }
//            return 0;
        else if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_EXPOSED)
            redraw();
        else if(event.type == SDL_DROPFILE) {
            screen_resize(WIDTH, HEIGHT);
            start(u, event.drop.file, 0, deviceQueue);
            SDL_free(event.drop.file);
        }
            /* Audio */
        else if(event.type >= audio0_event && event.type < audio0_event + POLYPHONY)
            uxn_eval(u, PEEK2(&u->dev[0x30 + 0x10 * (event.type - audio0_event)]));
            /* Mouse */
//        else if(event.type == SDL_MOUSEMOTION)
//            mouse_pos(u, &u->dev[0x90], clamp(event.motion.x - PAD, 0, uxn_screen.width - 1), clamp(event.motion.y - PAD, 0, uxn_screen.height - 1));
//        else if(event.type == SDL_MOUSEBUTTONUP)
//            mouse_up(u, &u->dev[0x90], SDL_BUTTON(event.button.button));
//        else if(event.type == SDL_MOUSEBUTTONDOWN)
//            mouse_down(u, &u->dev[0x90], SDL_BUTTON(event.button.button));
//        else if(event.type == SDL_MOUSEWHEEL)
//            mouse_scroll(u, &u->dev[0x90], event.wheel.x, event.wheel.y);
        else if(event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEWHEEL)
        {
            switch(event.type) {
                case SDL_MOUSEMOTION:
                {
                    // Calculate movement delta
                    int deltaX = event.motion.x - ms.lastX;
                    int deltaY = event.motion.y - ms.lastY;

                    // Update accumulated movement
                    ms.accumulatedDeltaX += deltaX;
                    ms.accumulatedDeltaY += deltaY;

                    // If accumulated movement exceeds the threshold
                    if(abs(ms.accumulatedDeltaX) > MOVE_THRESHOLD || abs(ms.accumulatedDeltaY) > MOVE_THRESHOLD) {
                        ms.x = clamp(event.motion.x - PAD, 0, uxn_screen.width - 1);
                        ms.y = clamp(event.motion.y - PAD, 0, uxn_screen.height - 1);

                        // Reset accumulated movement
                        ms.accumulatedDeltaX = 0;
                        ms.accumulatedDeltaY = 0;
                    }

//                    // 计算移动距离
//                    int deltaX = event.motion.x - ms.lastX;
//                    int deltaY = event.motion.y - ms.lastY;
//
//                    // 更新累计移动距离
//                    ms.accumulatedDeltaX += deltaX;
//                    ms.accumulatedDeltaY += deltaY;
//
//                    // 应用累计移动距离的一部分
//                    int appliedDeltaX = (int)(ms.accumulatedDeltaX * SMOOTH_FACTOR);
//                    int appliedDeltaY = (int)(ms.accumulatedDeltaY * SMOOTH_FACTOR);
//
//                    // 更新鼠标位置
//                    ms.x = clamp(ms.x + appliedDeltaX - PAD, 0, uxn_screen.width - 1);
//                    ms.y = clamp(ms.y + appliedDeltaY - PAD, 0, uxn_screen.height - 1);
//
//                    // 更新剩余的累计移动距离
//                    ms.accumulatedDeltaX -= appliedDeltaX;
//                    ms.accumulatedDeltaY -= appliedDeltaY;
//                    break;
                }
                    break;
                case SDL_MOUSEBUTTONUP:
                    ms.button = SDL_BUTTON(event.button.button);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    ms.button = SDL_BUTTON(event.button.button);
                    break;
                case SDL_MOUSEWHEEL:
                    ms.wheel.x = event.wheel.x;
                    ms.wheel.y = event.wheel.y;
                    break;
            }
            mouse_state_update(u, &ms, event.type);
        }

            /* Controller */
        else if(event.type == SDL_TEXTINPUT)
            controller_key(u, &u->dev[0x80], event.text.text[0]);
        else if(event.type == SDL_KEYDOWN) {
            int ksym;
            if(get_key(&event))
                controller_key(u, &u->dev[0x80], get_key(&event));
            else if(get_button(&event))
                controller_down(u, &u->dev[0x80], get_button(&event));
            else
                do_shortcut(u, &event,deviceQueue);
            ksym = event.key.keysym.sym;
            if(SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_KEYUP, SDL_KEYUP) == 1 && ksym == event.key.keysym.sym) {
                return 1;
            }
        } else if(event.type == SDL_KEYUP)
            controller_up(u, &u->dev[0x80], get_button(&event));
        else if(event.type == SDL_JOYAXISMOTION) {
            Uint8 vec = get_vector_joystick(&event);
            if(!vec)
                controller_up(u, &u->dev[0x80], (3 << (!event.jaxis.axis * 2)) << 4);
            else
                controller_down(u, &u->dev[0x80], (1 << ((vec + !event.jaxis.axis * 2) - 1)) << 4);
        } else if(event.type == SDL_JOYBUTTONDOWN)
            controller_down(u, &u->dev[0x80], get_button_joystick(&event));
        else if(event.type == SDL_JOYBUTTONUP)
            controller_up(u, &u->dev[0x80], get_button_joystick(&event));
        else if(event.type == SDL_JOYHATMOTION) {
            /* NOTE: Assuming there is only one joyhat in the controller */
            switch(event.jhat.value) {
                case SDL_HAT_UP:
                    controller_down(u, &u->dev[0x80], 0x10);
                    break;
                case SDL_HAT_DOWN:
                    controller_down(u, &u->dev[0x80], 0x20);
                    break;
                case SDL_HAT_LEFT:
                    controller_down(u, &u->dev[0x80], 0x40);
                    break;
                case SDL_HAT_RIGHT:
                    controller_down(u, &u->dev[0x80], 0x80);
                    break;
                case SDL_HAT_LEFTDOWN:
                    controller_down(u, &u->dev[0x80], 0x40 | 0x20);
                    break;
                case SDL_HAT_LEFTUP:
                    controller_down(u, &u->dev[0x80], 0x40 | 0x10);
                    break;
                case SDL_HAT_RIGHTDOWN:
                    controller_down(u, &u->dev[0x80], 0x80 | 0x20);
                    break;
                case SDL_HAT_RIGHTUP:
                    controller_down(u, &u->dev[0x80], 0x80 | 0x10);
                    break;
                case SDL_HAT_CENTERED:
                    /* Set all directions to down */
                    controller_up(u, &u->dev[0x80], 0x10 | 0x20 | 0x40 | 0x80);
                    break;
                default:
                    /* Ignore */
                    break;
            }
        }
            /* Console */
        else if(event.type == stdin_event)
            console_input(u, event.cbutton.button, CONSOLE_STD);
    }
    return 1;
}

static int
run(Uxn *u,cl::sycl::queue& deviceQueue)
{
    Uint64 next_refresh = 0;
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 frame_interval = SDL_GetPerformanceFrequency() / 60;
    for(;;) {
        Uint16 screen_vector;
        /* .System/halt */
        if(u->dev[0x0f])
            return system_error("Run", "Ended.");
        now = SDL_GetPerformanceCounter();
        exec_deadline = now + deadline_interval;
        int event_result = handle_events(u, deviceQueue);
        if(event_result == 0)
            return 0;
        else if(event_result == -1)
            break;
        screen_vector = PEEK2(&u->dev[0x20]);
        if(BENCH || now >= next_refresh) {
            now = SDL_GetPerformanceCounter();
            next_refresh = now + frame_interval;
            uxn_eval(u, screen_vector);
            if(uxn_screen.x2)
                redraw();
        }
        if(BENCH)
            ;
        else if(screen_vector || uxn_screen.x2) {
            Uint64 delay_ms = (next_refresh - now) / ms_interval;
            if(delay_ms > 0) SDL_Delay(delay_ms);
        }
    }
    return 1;
}

#define WIDTH 512
#define HEIGHT 384
#define PAD 10

int main(int argc, char **argv)
{

//    const char *args[] = {
//            "./bin/uxnemu",
//            "-2x",
//            //"-o",
//            "bin/piano.rom"
//    };
//
//    argc = sizeof(args) / sizeof(args[0]);
//    argv = new char*[argc]; // Allocate dynamic array
//
//    // Copy args to argv
//    for (int i = 0; i < argc; i++) {
//        argv[i] = const_cast<char*>(args[i]);
//    }
    // Print original arguments
    std::cout << "Original arguments: " << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }
    // 创建一个标志来表示是否执行一次性的计算
    bool run_once = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--once") == 0) {
            run_once = true;
            for (int j = i; j < argc - 1; j++) {
                argv[j] = argv[j + 1];
            }
            argc--; // Decrease the count of arguments
            i--; // Check the current index again in next iteration
        }
    }
    std::cout << "run_once: " << std::boolalpha << run_once << std::endl;
    // Print modified arguments
    std::cout << "\nModified arguments: " << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }


    // 初始化显示模式变量
    SDL_DisplayMode DM;

    // 创建一个Uxn类型的变量u，并对其进行初始化
    cl::sycl::queue deviceQueue(cl::sycl::default_selector{});
    //cl::sycl::queue deviceQueue(cl::sycl::gpu_selector {});

    // Uint8* ram = cl::sycl::malloc_shared<Uint8>(1,deviceQueue);
    // malloc shared memory for Uxn  by SYCL USM
    Uxn* u = cl::sycl::malloc_shared<Uxn>(1, deviceQueue);

    // u 指针       *u 指针 取 他指向地址的 值
    // &u 指针变量在内存的地址

    //**u
    // 内存区域全部写0
    //*u = {0};
    //Uxn u = {0};
    // 创建并初始化索引i
    int i = 1;

    // 如果初始化失败 返回错误
    if(!init())
        return system_error("Init", "Failed to initialize emulator.");
    /* default resolution */
    // 设置默认屏幕分辨率
    screen_resize(WIDTH, HEIGHT);
    /* default zoom */
    // 检查是否存在缩放选项
    if(argc > 1 && (strcmp(argv[i], "-1x") == 0 || strcmp(argv[i], "-2x") == 0 || strcmp(argv[i], "-3x") == 0))
        set_zoom(argv[i++][1] - '0');
    else if(SDL_GetCurrentDisplayMode(0, &DM) == 0)
        set_zoom(DM.w / 1280);
    /* load rom */
    // 检查是否提供了rom 文件路径
    if(i == argc)
        return system_error("usage", "uxnemu [-2x][-3x] file.rom [args...]");
    rom_path = argv[i++]; // 存储rom路径

    // 如果启动失败 返回错误
    if(!start(u, rom_path, argc - i,deviceQueue))
        return system_error("Start", "Failed");
    /* read arguments */
    // 读取命令行参数
    for(; i < argc; i++) {
        char *p = argv[i];
        while(*p) console_input(u, *p++, CONSOLE_ARG);
        console_input(u, '\n', i == argc - 1 ? CONSOLE_END : CONSOLE_EOA);
    }
    /* start rom */
    // 根据 run_once 标志选择执行模式
    if(run_once) {
        // Execute all computations at once and return the result
//        uxn_eval(u, PEEK2(u->dev[0x20]));
    } else {
        // Enter the SDL loop and process events
        run(u,deviceQueue);
    }
    /* finished */
#ifdef _WIN32
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
	TerminateThread((HANDLE)SDL_GetThreadID(stdin_thread), 0);
#elif !defined(__APPLE__)
    close(0); /* make stdin thread exit */
#endif

    // 清理SDL资源

    SDL_Quit();
    return 0;
}


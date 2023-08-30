/**
 * 
 * Description:uxnemu implementation
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


#include "devices/system.h"
#include "devices/screen.h"
#include "devices/audio.h"
#include "devices/file.h"
#include "devices/controller.h"
#include "devices/mouse.h"
#include "devices/datetime.h"

#define MOVE_THRESHOLD 1

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

//Input function implementation
Uint8
uxn_dei(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0; // Extract the device number and port number
    switch(d) {  // Determine the device number
        case 0x20: return screen_dei(u, addr); //Screen device input
        case 0x30: return audio_dei(0, &u->dev[d], p); //Audio device input 1
        case 0x40: return audio_dei(1, &u->dev[d], p); //Audio device input 2
        case 0x50: return audio_dei(2, &u->dev[d], p); //Audio device input 3
        case 0x60: return audio_dei(3, &u->dev[d], p); //Audio device input 4
        case 0xc0: return datetime_dei(u, addr); // time and date device input
    }
    return u->dev[addr];  // No matching device, return the device status of the specified address
}
/**
 * Output function implementation
 * @param u
 * @param addr
 */

void
uxn_deo(Uxn *u, Uint8 addr)
{
    Uint8 p = addr & 0x0f, d = addr & 0xf0;  // Extract the device number and port number
    switch(d) {  // Determine the device number
        case 0x00:  // system devices
            system_deo(u, &u->dev[d], p);  // system device output
            if(p > 0x7 && p < 0xe)
                screen_palette(&u->dev[0x8]);  // screen color output
            break;
        case 0x10: console_deo(&u->dev[d], p); break; //Console device output
        case 0x20: screen_deo(u->ram, &u->dev[d], p); break; //Screen device output
        case 0x30: audio_deo(0, &u->dev[d], p, u); break; // audio device output 1
        case 0x40: audio_deo(1, &u->dev[d], p, u); break; // audio device output 2
        case 0x50: audio_deo(2, &u->dev[d], p, u); break; // audio device output 3
        case 0x60: audio_deo(3, &u->dev[d], p, u); break; // audio device output 4
        case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break; //  File device output 1
        case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break; //  File device output 2
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
 * Mainly responsible for starting the Uxn system and loading rom files
 * @param u
 * @param rom
 * @param queue
 * @return
 */
static int
start(Uxn *u, char *rom, int queue,cl::sycl::queue& deviceQueue)
{

    cl::sycl::free(u->ram, deviceQueue);
    //cl::sycl::free(pc,deviceQueue);




    uint8_t* p = malloc_shared<uint8_t>(0x10000 * RAM_PAGES, deviceQueue);

    if(!uxn_boot(u, p))
        return system_error("Boot", "Failed to start uxn.");
    // Load rom file
    if(!system_load(u, rom))
        return system_error("Boot", "Failed to load rom.");
    // Store the number of command-line arguments to a specific location in device memory
    u->dev[0x17] = queue;
    //Set execution deadline
    exec_deadline = SDL_GetPerformanceCounter() + deadline_interval;

    Uint16* pc = cl::sycl::malloc_shared<Uint16>(1, deviceQueue);
    *pc = PAGE_PROGRAM;
    u->params = cl::sycl::malloc_shared<Params>(sizeof(Params),deviceQueue);
    u->queue = deviceQueue;
    // Execute the program in rom
    if(!uxn_eval(u, *pc))
        return system_error("Boot", "Failed to eval rom.");

    // Set window title to rom filename
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
* In the code, add a static variable of type MouseState to save the current mouse state, including the mouse position, the wheel state, and the mouse button state.
  * Then, put the code that handles mouse movement, mouse wheel movement, mouse button press, and mouse button release into one function mouse_state_update.
  * In each SDL event loop, if a mouse-related event is found, I update the state of the MouseState variable and call the mouse_state_update function to handle the event.
  * At the same time, the last mouse state is retained. Only when the mouse state changes, the relevant processing function will be called. In this way, unnecessary function calls and communication between the device and the host can be reduced.
  * *
  * The idea of this optimization method is to reduce the operation of the equipment. Every operation on the device involves data transmission between the device and the host, which is a relatively expensive operation.
  * If the data can be preprocessed on the host side and the operations on the device can be reduced, efficiency can be improved.
  * By saving the mouse state on the host side and calling the processing function only when the state changes, the operations on the device are reduced and efficiency is improved.
  * At the same time, code redundancy is also reduced by merging codes that handle multiple mouse events.
  * In the original code, handle mouse movement, mouse wheel movement, mouse button press,
  * The code for mouse button release is scattered in different places. In the code, these codes are unified into one function, which reduces the redundancy of the code and improves the readability and maintainability of the code.
 */
// Define the data structure of the mouse status, including the current and last mouse positions, button status, wheel position and other information
typedef struct MouseState {
    int x, y, button;  //Current mouse position and button status
    int lastX, lastY, lastButton;  // The last mouse position and button state
    bool isButtonDown;  // Whether the mouse is pressed or not
    SDL_Point wheel, lastWheel;  //Current and last scroll wheel positions
    int accumulatedDeltaX, accumulatedDeltaY;  // Accumulated mouse movement
} MouseState;

// Function to update and handle mouse state
void mouse_state_update(Uxn* u, MouseState* ms, int eventType) {
    //If the mouse position changes, update the mouse position
    if(ms->x != ms->lastX || ms->y != ms->lastY)
        mouse_pos(u, &u->dev[0x90], ms->x, ms->y);
    //If the wheel position changes, handle the wheel scroll event
    if(ms->wheel.x != ms->lastWheel.x || ms->wheel.y != ms->lastWheel.y)
        mouse_scroll(u, &u->dev[0x90], ms->wheel.x, ms->wheel.y);

    // If a mouse button release event is detected, handle the button release
    if(eventType == SDL_MOUSEBUTTONUP)
        mouse_up(u, &u->dev[0x90], ms->button);
        // If a mouse button press event is detected, handle the button press
    else if(eventType == SDL_MOUSEBUTTONDOWN)
        mouse_down(u, &u->dev[0x90], ms->button);

    //Update the last mouse state to the current state to prepare for the next comparison
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

//                   // Calculate moving distance
//                    int deltaX = event.motion.x - ms.lastX;
//                    int deltaY = event.motion.y - ms.lastY;
//
//                    //Update cumulative movement distance
//                    ms.accumulatedDeltaX += deltaX;
//                    ms.accumulatedDeltaY += deltaY;
//
//                   // apply a fraction of the cumulative distance traveled
//                    int appliedDeltaX = (int)(ms.accumulatedDeltaX * SMOOTH_FACTOR);
//                    int appliedDeltaY = (int)(ms.accumulatedDeltaY * SMOOTH_FACTOR);
//
//                    //Update mouse position
//                    ms.x = clamp(ms.x + appliedDeltaX - PAD, 0, uxn_screen.width - 1);
//                    ms.y = clamp(ms.y + appliedDeltaY - PAD, 0, uxn_screen.height - 1);
//
//                   // Update the remaining cumulative movement distance
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
    // Create a flag to indicate whether to perform a one-time calculation
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


    //Initialize display mode variables
    SDL_DisplayMode DM;

    //Create a Uxn type variable u and initialize it
    cl::sycl::queue deviceQueue(cl::sycl::default_selector{});
    //cl::sycl::queue deviceQueue(cl::sycl::gpu_selector {});

    // Uint8* ram = cl::sycl::malloc_shared<Uint8>(1,deviceQueue);
    // malloc shared memory for Uxn  by SYCL USM
    Uxn* u = cl::sycl::malloc_shared<Uxn>(1, deviceQueue);

    // u pointer *u pointer takes the value of the address it points to
    // &u The address of the pointer variable in memory


    // 创建并初始化索引i
    int i = 1;

    //If initialization fails, return error
    if(!init())
        return system_error("Init", "Failed to initialize emulator.");
    /* default resolution */
    // Set the default screen resolution
    screen_resize(WIDTH, HEIGHT);
    /* default zoom */
    // check if zoom option exists
    if(argc > 1 && (strcmp(argv[i], "-1x") == 0 || strcmp(argv[i], "-2x") == 0 || strcmp(argv[i], "-3x") == 0))
        set_zoom(argv[i++][1] - '0');
    else if(SDL_GetCurrentDisplayMode(0, &DM) == 0)
        set_zoom(DM.w / 1280);
    /* load rom */
    // Check if rom file path is provided
    if(i == argc)
        return system_error("usage", "uxnemu [-2x][-3x] file.rom [args...]");
    rom_path = argv[i++]; // storage rom path

    // If startup fails, return an error
    if(!start(u, rom_path, argc - i,deviceQueue))
        return system_error("Start", "Failed");
    /* read arguments */

    for(; i < argc; i++) {
        char *p = argv[i];
        while(*p) console_input(u, *p++, CONSOLE_ARG);
        console_input(u, '\n', i == argc - 1 ? CONSOLE_END : CONSOLE_EOA);
    }
    /* start rom */
    //Select execution mode based on run_once flag
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

    // Clean up SDL resources

    SDL_Quit();
    return 0;
}


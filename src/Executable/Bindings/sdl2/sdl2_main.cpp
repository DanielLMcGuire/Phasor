// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <SDL2/SDL.h>
#include <PhasorString.hpp>
#include <iostream>
#include <stdexcept>

PhasorValue phasor_sdl_init(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_Init requires 1 argument: flags (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_Init argument 1 (flags) must be an integer");
    }
    int flags = (int)phasor_to_int(argv[0]);
    int result = SDL_Init(flags);
    return phasor_make_int(result);
}

PhasorValue phasor_sdl_quit(PhasorVM*, int, const PhasorValue*)
{
    SDL_Quit();
    return phasor_make_null();
}

PhasorValue phasor_sdl_create_window(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 6) { [[unlikely]]
        throw std::runtime_error("SDL_CreateWindow requires 6 arguments: title (string), x (int), y (int), w (int), h (int), flags (int)");
    }
    if (!phasor_is_string(argv[0]) || !phasor_is_int(argv[1]) || !phasor_is_int(argv[2]) || 
        !phasor_is_int(argv[3]) || !phasor_is_int(argv[4]) || !phasor_is_int(argv[5])) { [[unlikely]]
        throw std::runtime_error("SDL_CreateWindow: Invalid argument types (Expected: string, int, int, int, int, int)");
    }
    const char* title = phasor_to_string(argv[0]);
    int x = (int)phasor_to_int(argv[1]);
    int y = (int)phasor_to_int(argv[2]);
    int w = (int)phasor_to_int(argv[3]);
    int h = (int)phasor_to_int(argv[4]);
    uint32_t flags = (uint32_t)phasor_to_int(argv[5]);
    
    SDL_Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
    return phasor_make_int((intptr_t)window);
}

PhasorValue phasor_sdl_destroy_window(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_DestroyWindow requires 1 argument: window_handle (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_DestroyWindow arg 1 must be an integer (window handle)");
    }
    SDL_Window* window = (SDL_Window*)(intptr_t)phasor_to_int(argv[0]);
    if (window) { [[likely]]
        SDL_DestroyWindow(window);
    }
    return phasor_make_null();
}

PhasorValue phasor_sdl_delay(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_Delay requires 1 argument: ms (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_Delay arg 1 must be an integer (milliseconds)");
    }
    Uint32 ms = (Uint32)phasor_to_int(argv[0]);
    SDL_Delay(ms);
    return phasor_make_null();
}

static const char* kEventKeys[] = {
    "type", "timestamp",
    "key_sym", "key_scancode", "key_mod", "key_repeat",
    "mouse_x", "mouse_y", "mouse_xrel", "mouse_yrel",
    "mouse_button", "mouse_clicks", "mouse_state",
    "wheel_x", "wheel_y", "wheel_direction",
    "window_event_id", "window_data1", "window_data2"
};
constexpr size_t kEventFieldCount = sizeof(kEventKeys) / sizeof(kEventKeys[0]);

PhasorValue phasor_sdl_poll_event(PhasorVM*, int, const PhasorValue*)
{
    static PhasorValue values[kEventFieldCount];

    for (size_t i = 0; i < kEventFieldCount; ++i) {
        values[i] = phasor_make_int(0);
    }

    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        values[0] = phasor_make_int((int64_t)event.type);
        values[1] = phasor_make_int((int64_t)event.common.timestamp);

        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                values[2] = phasor_make_int(event.key.keysym.sym);
                values[3] = phasor_make_int(event.key.keysym.scancode);
                values[4] = phasor_make_int(event.key.keysym.mod);
                values[5] = phasor_make_int(event.key.repeat);
                break;

            case SDL_MOUSEMOTION:
                values[6]  = phasor_make_int(event.motion.x);
                values[7]  = phasor_make_int(event.motion.y);
                values[8]  = phasor_make_int(event.motion.xrel);
                values[9]  = phasor_make_int(event.motion.yrel);
                values[12] = phasor_make_int(event.motion.state);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                values[6]  = phasor_make_int(event.button.x);
                values[7]  = phasor_make_int(event.button.y);
                values[10] = phasor_make_int(event.button.button);
                values[11] = phasor_make_int(event.button.clicks);
                break;

            case SDL_MOUSEWHEEL:
                values[13] = phasor_make_int(event.wheel.x);
                values[14] = phasor_make_int(event.wheel.y);
                values[15] = phasor_make_int(event.wheel.direction);
                break;

            case SDL_WINDOWEVENT:
                values[16] = phasor_make_int(event.window.event);
                values[17] = phasor_make_int(event.window.data1);
                values[18] = phasor_make_int(event.window.data2);
                break;

            default:
                break;
        }
    }

    return phasor_make_struct("SDLEvent", kEventKeys, values, kEventFieldCount);
}

PhasorValue phasor_sdl_get_error(PhasorVM*, int argc, const PhasorValue *)
{
    if (argc > 1) { [[unlikely]] 
        throw std::runtime_error("SDL_GetError requires 0 arguments.");
    }
    static Phasor::string error = SDL_GetError();
    return phasor_make_string(error.c_str());
}

PhasorValue phasor_sdl_create_renderer(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 3) { [[unlikely]]
        throw std::runtime_error("SDL_CreateRenderer requires 3 arguments: window (int), index (int), flags (int)");
    }
    if (!phasor_is_int(argv[0]) || !phasor_is_int(argv[1]) || !phasor_is_int(argv[2])) { [[unlikely]]
        throw std::runtime_error("SDL_CreateRenderer: Invalid argument types (Expected: int, int, int)");
    }
    
    SDL_Window* window = (SDL_Window*)(intptr_t)phasor_to_int(argv[0]);
    int index = (int)phasor_to_int(argv[1]);
    uint32_t flags = (uint32_t)phasor_to_int(argv[2]);
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, index, flags);
    return phasor_make_int((intptr_t)renderer);
}

PhasorValue phasor_sdl_destroy_renderer(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_DestroyRenderer requires 1 argument: renderer_handle (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_DestroyRenderer arg 1 must be an integer (renderer handle)");
    }
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    if (renderer) { [[likely]]
        SDL_DestroyRenderer(renderer);
    }
    return phasor_make_null();
}

PhasorValue phasor_sdl_set_render_draw_color(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 5) { [[unlikely]]
        throw std::runtime_error("SDL_SetRenderDrawColor requires 5 arguments: renderer (int), r (int), g (int), b (int), a (int)");
    }
    for (int i = 0; i < 5; ++i) {
        if (!phasor_is_int(argv[i])) { [[unlikely]]
            throw std::runtime_error("SDL_SetRenderDrawColor: All arguments must be integers (0-255)");
        }
    }
    
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    Uint8 r = (Uint8)phasor_to_int(argv[1]);
    Uint8 g = (Uint8)phasor_to_int(argv[2]);
    Uint8 b = (Uint8)phasor_to_int(argv[3]);
    Uint8 a = (Uint8)phasor_to_int(argv[4]);
    
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    return phasor_make_null();
}

PhasorValue phasor_sdl_render_clear(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_RenderClear requires 1 argument: renderer (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_RenderClear arg 1 must be an integer (renderer handle)");
    }
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    SDL_RenderClear(renderer);
    return phasor_make_null();
}

PhasorValue phasor_sdl_render_present(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 1) { [[unlikely]]
        throw std::runtime_error("SDL_RenderPresent requires 1 argument: renderer (int)");
    }
    if (!phasor_is_int(argv[0])) { [[unlikely]]
        throw std::runtime_error("SDL_RenderPresent arg 1 must be an integer (renderer handle)");
    }
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    SDL_RenderPresent(renderer);
    return phasor_make_null();
}

PhasorValue phasor_sdl_render_draw_line(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 5) { [[unlikely]]
        throw std::runtime_error("SDL_RenderDrawLine requires 5 arguments: renderer (int), x1 (int), y1 (int), x2 (int), y2 (int)");
    }
    for (int i = 0; i < 5; ++i) {
        if (!phasor_is_int(argv[i])) { [[unlikely]]
            throw std::runtime_error("SDL_RenderDrawLine: All arguments must be integers");
        }
    }
    
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    int x1 = (int)phasor_to_int(argv[1]);
    int y1 = (int)phasor_to_int(argv[2]);
    int x2 = (int)phasor_to_int(argv[3]);
    int y2 = (int)phasor_to_int(argv[4]);
    
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    return phasor_make_null();
}

PhasorValue phasor_sdl_render_draw_rect(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 5) { [[unlikely]]
        throw std::runtime_error("SDL_RenderDrawRect requires 5 arguments: renderer (int), x (int), y (int), w (int), h (int)");
    }
    for (int i = 0; i < 5; ++i) {
        if (!phasor_is_int(argv[i])) { [[unlikely]]
            throw std::runtime_error("SDL_RenderDrawRect: All arguments must be integers");
        }
    }
    
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    SDL_Rect rect;
    rect.x = (int)phasor_to_int(argv[1]);
    rect.y = (int)phasor_to_int(argv[2]);
    rect.w = (int)phasor_to_int(argv[3]);
    rect.h = (int)phasor_to_int(argv[4]);
    
    SDL_RenderDrawRect(renderer, &rect);
    return phasor_make_null();
}

PhasorValue phasor_sdl_render_fill_rect(PhasorVM*, int argc, const PhasorValue *argv)
{
    if (argc < 5) { [[unlikely]]
        throw std::runtime_error("SDL_RenderFillRect requires 5 arguments: renderer (int), x (int), y (int), w (int), h (int)");
    }
    for (int i = 0; i < 5; ++i) {
        if (!phasor_is_int(argv[i])) { [[unlikely]]
            throw std::runtime_error("SDL_RenderFillRect: All arguments must be integers");
        }
    }
    
    SDL_Renderer* renderer = (SDL_Renderer*)(intptr_t)phasor_to_int(argv[0]);
    SDL_Rect rect;
    rect.x = (int)phasor_to_int(argv[1]);
    rect.y = (int)phasor_to_int(argv[2]);
    rect.w = (int)phasor_to_int(argv[3]);
    rect.h = (int)phasor_to_int(argv[4]);
    
    SDL_RenderFillRect(renderer, &rect);
    return phasor_make_null();
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
    api->register_function(vm, "SDL_Init", phasor_sdl_init);
    api->register_function(vm, "SDL_Quit", phasor_sdl_quit);
    api->register_function(vm, "SDL_CreateWindow", phasor_sdl_create_window);
    api->register_function(vm, "SDL_DestroyWindow", phasor_sdl_destroy_window);
    api->register_function(vm, "SDL_Delay", phasor_sdl_delay);
    api->register_function(vm, "SDL_PollEvent", phasor_sdl_poll_event);
    api->register_function(vm, "SDL_GetError", phasor_sdl_get_error);

    api->register_function(vm, "SDL_CreateRenderer", phasor_sdl_create_renderer);
    api->register_function(vm, "SDL_DestroyRenderer", phasor_sdl_destroy_renderer);
    api->register_function(vm, "SDL_SetRenderDrawColor", phasor_sdl_set_render_draw_color);
    api->register_function(vm, "SDL_RenderClear", phasor_sdl_render_clear);
    api->register_function(vm, "SDL_RenderPresent", phasor_sdl_render_present);
    api->register_function(vm, "SDL_RenderDrawLine", phasor_sdl_render_draw_line);
    api->register_function(vm, "SDL_RenderDrawRect", phasor_sdl_render_draw_rect);
    api->register_function(vm, "SDL_RenderFillRect", phasor_sdl_render_fill_rect);
}
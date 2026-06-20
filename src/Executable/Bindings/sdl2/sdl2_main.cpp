#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <stdexcept>

PhasorValue phasor_sdl_init(PhasorVM*, int argc, const PhasorValue *argv) {
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

PhasorValue phasor_sdl_quit(PhasorVM*, int, const PhasorValue*) {
    SDL_Quit();
    return phasor_make_null();
}

PhasorValue phasor_sdl_create_window(PhasorVM*, int argc, const PhasorValue *argv) {
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

PhasorValue phasor_sdl_destroy_window(PhasorVM*, int argc, const PhasorValue *argv) {
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

PhasorValue phasor_sdl_delay(PhasorVM*, int argc, const PhasorValue *argv) {
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

PhasorValue phasor_sdl_poll_event(PhasorVM*, int, const PhasorValue*) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) { [[unlikely]]
        return phasor_make_int(event.type);
    }
    return phasor_make_int(0); 
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm) {
    api->register_function(vm, "SDL_Init", phasor_sdl_init);
    api->register_function(vm, "SDL_Quit", phasor_sdl_quit);
    api->register_function(vm, "SDL_CreateWindow", phasor_sdl_create_window);
    api->register_function(vm, "SDL_DestroyWindow", phasor_sdl_destroy_window);
    api->register_function(vm, "SDL_Delay", phasor_sdl_delay);
    api->register_function(vm, "SDL_PollEvent", phasor_sdl_poll_event);
#ifdef _DEBUG
    api->log(vm, phasor_make_string("SDL2 Bindings loaded successfully."));
#endif
}
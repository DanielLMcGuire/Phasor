#pragma once
#ifndef PLATFORM_OVERRIDE
#if defined(__ORBIS__) || defined(_GAMING_XBOX_XBOXONE)
#define PLATFORM_LEGACY_GAME
#elif defined(_GAMING_DESKTOP)
#define PLATFORM_DESKTOP_GAME
#elif defined(_GAMING_XBOX_SCARLETT) || defined(__PROSPERO__)
#define PLATFORM_MODERN_GAME
#else
#define PLATFORM_DESKTOP_STANDARD
#endif
#endif

#if defined(PLATFORM_LEGACY_GAME)
#define MAX_REGISTERS 4
#elif defined(PLATFORM_DESKTOP_GAME) || defined(PLATFORM_MODERN_GAME)
#define MAX_REGISTERS 8
#elif defined(PLATFORM_DESKTOP_STANDARD)
#define MAX_REGISTERS 16
#endif

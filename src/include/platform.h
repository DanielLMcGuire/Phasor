#pragma once
#if (defined(__ORBIS__) || defined(_XBOX_UNIT)) && !defined(_XBOX_SCARLET)
#define PLATFORM_LEGACY_GAME
#elif defined(_GAMING_DESKTOP)
#define PLATFORM_DESKTOP_GAME
#elif defined(_GAMING_XBOX) || defined(__PROSPERO__)
#define PLATFORM_MODERN_GAME
#else
#define PLATFORM_DESKTOP_STANDARD
#endif

#if defined(PLATFORM_LEGACY_GAME)
#define MAX_REGISTERS 8
#elif defined(PLATFORM_DESKTOP_GAME) || defined(PLATFORM_MODERN_GAME)
#define MAX_REGISTERS 16
#elif defined(PLATFORM_DESKTOP_STANDARD)
#define MAX_REGISTERS 32
#endif
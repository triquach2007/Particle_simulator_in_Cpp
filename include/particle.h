#pragma once

#include <SDL3/SDL_rect.h>

struct Particle
{
    SDL_FPoint position;
    SDL_FPoint velocity;
    SDL_FPoint acceleration;
};
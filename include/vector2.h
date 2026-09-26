#pragma once

#include <iostream>
#include <SDL3/SDL_rect.h>

template<typename T>
struct Vector2
{
    T x, y;
};

SDL_FPoint operator+(const SDL_FPoint&, const SDL_FPoint&);
SDL_FPoint& operator+=(SDL_FPoint&, const SDL_FPoint&);

SDL_FPoint operator-(const SDL_FPoint&, const SDL_FPoint&);
SDL_FPoint& operator-=(SDL_FPoint&, const SDL_FPoint&);

bool operator==(const SDL_FPoint&, const SDL_FPoint&);

template<typename T>
SDL_FPoint operator*(const SDL_FPoint& LHS, const T& RHS)
{
    return {
        LHS.x * RHS,
        LHS.y * RHS
    };
}
template<typename T>
SDL_FPoint operator*(const T& LHS, const SDL_FPoint& RHS)
{
    return RHS * LHS;
}
template<typename T>
SDL_FPoint& operator*=(SDL_FPoint& LHS, const T& RHS)
{
    LHS.x *= RHS;
    LHS.y *= RHS;
    return LHS;
}

float vector_length_squared(const SDL_FPoint&);
float vector_length(const SDL_FPoint&);
float dot_product(const SDL_FPoint&, const SDL_FPoint&);
SDL_FPoint vector_normalize(const SDL_FPoint&);
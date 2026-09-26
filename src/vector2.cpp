#include "../include/vector2.h"

SDL_FPoint operator+(const SDL_FPoint& LHS, const SDL_FPoint& RHS)
{
    return {
        LHS.x + RHS.x,
        LHS.y + RHS.y
    };
}
SDL_FPoint& operator+=(SDL_FPoint& LHS, const SDL_FPoint& RHS)
{
    LHS.x += RHS.x;
    LHS.y += RHS.y;

    return LHS;
}

SDL_FPoint operator-(const SDL_FPoint& LHS, const SDL_FPoint& RHS)
{
    return {
        LHS.x - RHS.x,
        LHS.y - RHS.y
    };
}
SDL_FPoint& operator-=(SDL_FPoint& LHS, const SDL_FPoint& RHS)
{
    LHS.x -= RHS.x;
    LHS.y -= RHS.y;

    return LHS;
}

bool operator==(const SDL_FPoint& LHS, const SDL_FPoint& RHS)
{
    return LHS.x == RHS.x && LHS.y == RHS.y;
}

float vector_length_squared(const SDL_FPoint& v)
{
    return SDL_powf(v.x, 2) + SDL_pow(v.y, 2);
}

float vector_length(const SDL_FPoint& v)
{
    return SDL_sqrtf(SDL_pow(v.x, 2) + SDL_pow(v.y, 2));
}
float dot_product(const SDL_FPoint& a, const SDL_FPoint& b)
{
    return a.x * b.x + a.y * b.y;
}
SDL_FPoint vector_normalize(const SDL_FPoint& v)
{
    float length = vector_length(v);
    return {
        v.x / length,
        v.y / length
    };
}
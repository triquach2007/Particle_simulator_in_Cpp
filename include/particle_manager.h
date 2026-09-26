#pragma once

#include <iostream>
#include <vector>
#include "./grid.h"
#include "./particle.h"
using namespace std;

class Particle_manager
{
private:
    vector<Particle*> particles;
    Grid grid;
    vector<SDL_FPoint> render_buffer;

public:
    Particle_manager();
    ~Particle_manager();

    // Getters
    vector<Particle*>* get_particles();
    Grid* get_grid();
    Particle* operator[](const size_t&) const;
    const vector<SDL_FPoint>& get_render_buffer();

    // Additional Functions
    size_t get_count() const;
    void append_particle(Particle*);
    void remove_particle(const size_t&);

    void update_grid();
    void update(const float&);
};
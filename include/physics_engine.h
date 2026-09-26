#pragma once

#include "../include/particle_manager.h"

class Physics_engine
{
private:
    Particle_manager* pm;

public:
    Physics_engine(Particle_manager*);
    ~Physics_engine();

    void update(const float&);
    void update_particle_positions(const float&);
    
    void apply_gravity();
    
    void check_collision_between_two_particles(Particle*, Particle*);
    void check_collision_between_two_cells(Cell*, Cell*);
    void check_collision_with_window();
    void check_collision_between_cells();

};
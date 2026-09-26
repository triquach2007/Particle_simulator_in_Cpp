#include <cassert>
#include "../include/particle_manager.h"
#include "../include/vector2.h"

Particle_manager::Particle_manager()
{
    this->particles = {};
}
Particle_manager::~Particle_manager()
{
    size_t count = this->particles.size();
    if (count == 0)
        return;

    for (size_t i = 0; i < count; i++)
    {
        delete this->particles[i];
    }
    cout << "deleted particles" << endl;
}

// Getters
vector<Particle*>* Particle_manager::get_particles()
{
    return &(this->particles);
}
Grid* Particle_manager::get_grid()
{
    return &(this->grid);
}
Particle* Particle_manager::operator[](const size_t& i) const
{
    return this->particles[i];
}
const vector<SDL_FPoint>& Particle_manager::get_render_buffer()
{
    this->render_buffer.clear();
    for (auto& p:this->particles)
    {
        this->render_buffer.push_back(p->position);
    }

    return this->render_buffer;
}

// Additional Functions
size_t Particle_manager::get_count() const
{
    return this->particles.size();
}
void Particle_manager::append_particle(Particle* p)
{
    this->particles.push_back(p);
    this->grid.insert(p);
}
void Particle_manager::remove_particle(const size_t& i)
{
    delete this->particles[i];
    this->particles.erase(this->particles.begin() + i);
}

void Particle_manager::update_grid()
{
    this->grid.clear();
    for (Particle*& p:this->particles)
    {
        this->grid.insert(p);
    }
}
void Particle_manager::update(const float& dt)
{
    for (Particle*& p:this->particles)
    {
        p->velocity += p->acceleration * dt;
        p->position += p->velocity * dt;
    }
}
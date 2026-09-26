#include <math.h>
#include "../include/physics_engine.h"
#include "../include/vector2.h"
#include "../include/constant.h"

Physics_engine::Physics_engine(Particle_manager* pm)
{
    this->pm = pm;
}
Physics_engine::~Physics_engine()
{

}

void Physics_engine::update(const float& dt)
{
    this->apply_gravity();
    this->check_collision_between_cells();
    
    this->pm->update(dt);
    this->check_collision_with_window();
    this->pm->update_grid();
}

void Physics_engine::apply_gravity()
{
    for (auto& p:*(this->pm->get_particles()))
    {
        p->acceleration = {0, 9.8};
    }
}
void Physics_engine::check_collision_between_two_particles(Particle* p1, Particle* p2)
{
    const float min_distance = 2.0f;
    const float restitution = 0.15f;

    SDL_FPoint delta = p2->position - p1->position;
    float distance_squared = vector_length_squared(delta);

    // Avoid normalizing a zero-length vector.
    if (distance_squared <= 0.0f)
        return;

    float min_distance_squared = min_distance * min_distance;

    // Not colliding.
    if (distance_squared >= min_distance_squared)
        return;

    float distance = SDL_sqrtf(distance_squared);
    SDL_FPoint normal = delta * (1/distance);

    // --------------------------------------------------
    // Positional correction
    // --------------------------------------------------

    float overlap = min_distance - distance;
    SDL_FPoint separation = normal * (overlap * 0.5f);

    p1->position -= separation;
    p2->position += separation;

    // --------------------------------------------------
    // Velocity response
    // --------------------------------------------------

    SDL_FPoint relative_velocity = p1->velocity - p2->velocity;

    float velocity_along_normal =
        dot_product(relative_velocity, normal);

    // Already moving apart; don't apply another impulse.
    if (velocity_along_normal >= 0.0f)
        return;

    // Equal-mass collision impulse.
    float impulse = -(1.0f + restitution) * velocity_along_normal * 0.5f;

    SDL_FPoint impulse_vector = normal * impulse;

    p1->velocity += impulse_vector;
    p2->velocity -= impulse_vector;
}
void Physics_engine::check_collision_with_window()
{
    for (auto& p:*(this->pm->get_particles()))
    {
        if (p->position.y > (WINDOW_HEIGHT - 1))
        {
            p->position.y = WINDOW_HEIGHT - 1;
            p->velocity.y *= -0.1f;
        }

        if (p->position.x < (1))
        {
            p->position.x = 1;
            p->velocity.x *= -0.1f;
        } else if (p->position.x > (WINDOW_WIDTH - 1))
        {
            p->position.x = WINDOW_WIDTH - 1;
            p->velocity.x *= -0.1f;
        }
    }
}
void Physics_engine::check_collision_between_cells()
{
    Grid* grid = this->pm->get_grid();

    for (int cell_index = 0; cell_index < CELL_COUNT; ++cell_index)
    {
        Cell* c1 = grid->get_cell(cell_index);

        // -----------------------------------------
        // Collisions between particles in same cell
        // -----------------------------------------

        for (size_t i = 0; i < c1->size(); ++i)
        {
            for (size_t j = i + 1; j < c1->size(); ++j)
            {
                check_collision_between_two_particles(
                    (*c1)[i],
                    (*c1)[j]
                );
            }
        }

        // -----------------------------------------
        // Collisions with neighboring cells
        // -----------------------------------------

        Vector2<int> cell_coord =
            grid->get_cell_coord(cell_index);

        for (Direction direction = RIGHT;
             direction <= DOWN_LEFT;
             ++direction)
        {
            int neighbor_index =
                grid->get_neighbor_index(
                    cell_coord.x,
                    cell_coord.y,
                    direction
                );

            if (neighbor_index == -1)
                continue;

            Cell* c2 = grid->get_cell(neighbor_index);

            check_collision_between_two_cells(c1, c2);
        }
    }
}
void Physics_engine::check_collision_between_two_cells(Cell* c1, Cell* c2)
{
    for (Particle* p1:*c1)
        for (Particle*& p2:*c2)
            check_collision_between_two_particles(p1, p2);
}
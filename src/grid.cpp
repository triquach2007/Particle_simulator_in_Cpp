#include <cassert>
#include "../include/grid.h"

Grid::Grid()
{
    this->cells.resize(CELL_COUNT);
}

Grid::~Grid()
{
}

vector<Cell>* Grid::get_cells()
{
    return &this->cells;
}

Vector2<int> Grid::get_cell_coord(const int& i)
{
    assert(i >= 0 && i < CELL_COUNT);

    return {
        i % GRID_COLS,
        i / GRID_COLS
    };
}

int Grid::get_cell_index(const int& x, const int& y)
{
    assert(x >= 0 && x < GRID_COLS);
    assert(y >= 0 && y < GRID_ROWS);

    return y * GRID_COLS + x;
}

int Grid::get_neighbor_index(
    const int& x,
    const int& y,
    const Direction& direction)
{
    const Offset offset = getOffset(direction);

    const int newX = x + offset.col;
    const int newY = y + offset.row;

    if (newX < 0 || newX >= GRID_COLS ||
        newY < 0 || newY >= GRID_ROWS)
    {
        return -1;
    }

    return get_cell_index(newX, newY);
}

Cell* Grid::get_cell(const int& i)
{
    assert(i >= 0 && i < CELL_COUNT);

    return &this->cells[i];
}

void Grid::clear()
{
    for (Cell& cell : this->cells)
        cell.clear();
}

void Grid::insert(const Particle* p)
{
    if (p == nullptr)
        return;

    int x = static_cast<int>(p->position.x / CELL_WIDTH);
    int y = static_cast<int>(p->position.y / CELL_HEIGHT);

    // Particle is outside the grid.
    if (x < 0 || x >= GRID_COLS ||
        y < 0 || y >= GRID_ROWS)
    {
        return;
    }

    int cell_index = get_cell_index(x, y);

    get_cell(cell_index)->push_back(const_cast<Particle*>(p));
}

Offset getOffset(Direction direction)
{
    switch (direction)
    {
        case Direction::RIGHT:
            return {1, 0};

        case Direction::DOWN:
            return {0, 1};

        case Direction::DOWN_RIGHT:
            return {1, 1};

        case Direction::DOWN_LEFT:
            return {-1, 1};
    }

    return {0, 0};
}
Direction operator++(Direction& direction)
{
    direction = static_cast<Direction>(
        static_cast<int>(direction) + 1
    );

    return direction;
}
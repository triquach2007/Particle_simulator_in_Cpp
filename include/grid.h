#pragma once

#include <iostream>
#include <vector>
#include "./constant.h"
#include "./vector2.h"
#include "./particle.h"
using namespace std;

typedef vector<Particle*> Cell;
typedef enum
{
    RIGHT,
    DOWN,
    DOWN_RIGHT,
    DOWN_LEFT
} Direction;
struct Offset
{
    int row, col;
};

class Grid
{
    private:
    vector<Cell> cells;
    
    public:
    Grid();
    ~Grid();
    
    vector<Cell>* get_cells();
    Vector2<int> get_cell_coord(const int&);
    int get_cell_index(const int&, const int&);
    int get_neighbor_index(const int&, const int&, const Direction&);
    Cell* get_cell(const int&);
    
    void clear();
    void insert(const Particle*);
};

Offset getOffset(Direction direction);
Direction operator++(Direction&);
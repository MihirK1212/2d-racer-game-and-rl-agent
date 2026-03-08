#ifndef RECTANGLE_OBB_H
#define RECTANGLE_OBB_H

#include "vector.h"

struct RectangleOBB { // oriented bounding box of a rectangle
    Vector2D corners[4];
    Vector2D axes[2];   // the two unique edge normals 
};

#endif
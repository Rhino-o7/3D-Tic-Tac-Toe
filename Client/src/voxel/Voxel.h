#pragma once
#include <cstdint>


enum class VoxelType : std::uint8_t
{
	Air = 0,
	Solid,
    X,
    O,
	Other
};

struct VoxelNeighbors
{
    VoxelType left;
    VoxelType right;
    VoxelType top;
    VoxelType bottom;
    VoxelType front;
    VoxelType back;
};


struct VoxelFace {
    const float frontFace[12]{
        0, 0, 1,
        1, 0, 1,
        1, 1, 1,
        0, 1, 1,
    };

    const float backFace[12]{
        1, 0, 0,
        0, 0, 0,
        0, 1, 0,
        1, 1, 0,
    };

    const float leftFace[12]{
        0, 0, 0,
        0, 0, 1,
        0, 1, 1,
        0, 1, 0,
    };

    const float rightFace[12]{
        1, 0, 1,
        1, 0, 0,
        1, 1, 0,
        1, 1, 1,
    };

    const float topFace[12]{
        0, 1, 1,
        1, 1, 1,
        1, 1, 0,
        0, 1, 0,
    };

    const float bottomFace[12]{
        0, 0, 0,
        1, 0, 0,
        1, 0, 1,
        0, 0, 1 
    };
};
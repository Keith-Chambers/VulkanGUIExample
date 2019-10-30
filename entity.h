#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

typedef uint32_t Entity32;

// NOTE: Assume reordering Entities is cheap, always try and do this instead of moving actual data

struct RelativeDataLocation
{
    uint32_t offsetBytes;   // Offset from allocated memory
    uint16_t spanElements;  // The number of elements being grouped
    uint16_t strideBytes;   // Can be a power of 2 to reduce memory required
                            // Or even, to reduce by half
};



struct EntitySystemHandle
{
    Entity32 nextEntity = 0;
    uint64_t unusedEntities[5]; // = 5 * 64

    uint8_t * verticesComponentBasePtr;
    uint32_t numberVerticesComponents;
    RelativeDataLocation verticesComponent[30];

    uint16_t exampleTimeUpdateListSize = 0;
    Entity32 exampleTimeUpdateList[10];
};

//Entity32 requestEntity(EntitySystemHandle& handle);
//void releaseEntity(EntitySystemHandle& handle, Entity32 entity);

#endif // ENTITY_H

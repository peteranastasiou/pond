#pragma once

#include <stdint.h>

enum class OpCode {
    RETURN,
};

struct Chunk {
    uint8_t * code;
};


// modules.hpp
#pragma once
#include "../memory/memory.h"
#include "../gui/zdraw/zdraw/zdraw.hpp"
#include <atomic>

extern float viewMatrix[16];
extern std::string configName;
extern std::atomic<int> isAiming;

struct QAngle {
    float x; // Pitch
    float y; // Yaw
    float z; // Roll
};

extern QAngle punchAngle;

inline std::atomic<float> punchDeltaX { 0.f };
inline std::atomic<float> punchDeltaY { 0.f };

namespace reader {
    void handler();
}

namespace esp {
    void handler(RECT rect);
    
    extern bool enabled;
    extern bool enableNameESP;
    extern bool enableHealthESP;
    extern bool enableBoxes;
    extern bool borders;
    extern bool teams;

    extern zdraw::rgba color;
    extern zdraw::rgba nameColor;
    extern zdraw::rgba healthColor;
}

namespace tracers {
    void handler(RECT rect);
    extern float offset;
    extern bool enabled;
    extern int style;
    extern zdraw::rgba color;
    extern bool teams;
    extern int centered;
}

namespace bones {
    void handler(RECT rect);
    extern bool enabled;
    extern zdraw::rgba color, headColor, filledColor;
    extern bool head, filled, teams;
}

namespace aimassist {
    void handler();
    void render(RECT rect);
    extern bool enabled, renderRadio, renderDeadZone, deadZoneEnabled;
    extern float radio, deadZone, smoothing;
}

namespace triggerbot {
    void handler();
    extern bool enabled;
    extern float delay;
    extern bool randomization;
}

namespace rcs {
    void handler();
    extern bool enabled;
    extern float smoothing;
}

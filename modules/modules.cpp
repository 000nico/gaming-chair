#include "modules.hpp"
#include "../gui/zdraw/zdraw/zdraw.hpp"

std::atomic<int> isAiming{ 0 };
std::string configName = "config";

namespace esp {
    bool enabled = false;
    bool enableNameESP = false;
    bool enableHealthESP = false;
    bool enableBoxes = false;
    bool borders = false;
    bool teams = false;

    zdraw::rgba color = {255, 255, 255, 255};
    zdraw::rgba nameColor = {255, 255, 255, 255};
    zdraw::rgba healthColor = {0, 255, 0, 255};
}

namespace tracers {
    bool enabled = false;
    int style = 0;
    float offset = 80;
    zdraw::rgba color = {255, 255, 255, 255};
    bool teams = false;
    int centered = true;
}

namespace bones {
    bool enabled = false;
    zdraw::rgba color = {255, 255, 255, 255};
    zdraw::rgba headColor   = {255, 182, 213, 255};
    zdraw::rgba filledColor = {255, 182, 213, 150};
    bool head = false, filled = false, teams = false;
}

namespace aimassist {
    bool enabled = false, renderRadio = false, renderDeadZone = false, deadZoneEnabled = false;
    float radio = 100.0f, deadZone = 20.0f;
    float smoothing = 15.0f;
}

namespace triggerbot {
    bool enabled = false;
    float delay = 100.0f;
    bool randomization;
}

namespace rcs {
    bool enabled = false;
    float smoothing = 20.0f;
}

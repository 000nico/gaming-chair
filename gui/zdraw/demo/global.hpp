#pragma once

// core
#include <../zdraw/zdraw.hpp>
#include <../zdraw/zui/zui.hpp>
#include <../zdraw/zscene/zscene.hpp>

#include "menu/menu.hpp"
#include "render/render.hpp"

namespace zEntry {
    int zMain();
}

namespace render::window {
    inline bool menu_open = false;
    void update_clickthrough();
    void sync_to_cs2();
}
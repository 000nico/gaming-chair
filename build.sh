#!/bin/bash

CXX="/usr/bin/ccache /ucrt64/bin/g++"
CXXFLAGS="-O1 -std=c++20 -msse2 -DNOMINMAX \
  -I/ucrt64/include \
  -Igui/zdraw \
  -Igui/zdraw/zdraw \
  -Igui/zdraw/zdraw/external \
  -Igui/zdraw/zdraw/external/freetype"
LDFLAGS="-static -L/ucrt64/lib -LC:/msys64/ucrt64/lib/x64 \
  -Wl,--subsystem,console"

LIBS="-ld3d11 -ldxgi -ld3dcompiler -lntdll -lWebView2Loader \
      -lole32 -loleaut32 -lshlwapi -lversion -ldwmapi \
      -lfreetype -lharfbuzz -lgraphite2 -lpng -lbrotlidec -lbrotlicommon \
      -lbz2 -lz -lwindowscodecs \
      -lusp10 -lrpcrt4 -ldwrite -luuid \
      -lgdi32 -lwinmm"
SRCS=(
  "entry_point.cpp"
  "proxy/proxy/proxy.cpp"
  "overlay/overlay/overlay.cpp"
  "overlay/duplication/duplication.cpp"
  "direct_x/direct_x.cpp"
  "gui/gui.cpp"
  "gui/imgui/imgui_demo.cpp"
  "gui/imgui/imgui_draw.cpp"
  "gui/imgui/imgui_impl_dx11.cpp"
  "gui/imgui/imgui_impl_win32.cpp"
  "gui/imgui/imgui_tables.cpp"
  "gui/imgui/imgui_widgets.cpp"
  "gui/imgui/imgui.cpp"
  "gui/zdraw/zdraw/zdraw.cpp"
  "gui/zdraw/zdraw/zui/zui.cpp"
  "gui/zdraw/zdraw/zscene/zscene.cpp"
  "gui/zdraw/demo/entry.cpp"
  "gui/zdraw/demo/menu/menu.cpp"
  "gui/zdraw/demo/render/render.cpp"
  "modules/reader/entity_reader.cpp"
  "modules/modules.cpp"
  "memory/memory.cpp"
  "modules/esp/esp.cpp"
  "modules/tracers/tracers.cpp"
  "modules/bones/bones.cpp"
  "modules/triggerbot/triggerbot.cpp"
  "modules/aimbot/aimbot.cpp"
  "modules/aimbot/renderAim.cpp"
  "modules/rcs/rcs.cpp"
  "protector/upp.cpp"
  "protector/thread or dll protection/dll protection.cpp"
  "protector/helper/sdk.cpp"
  "protector/find target/find target.cpp"
  "protector/anti suspend/anti suspend.cpp"
  "protector/anti debug/anti debug.cpp"
  "config_manager/config_manager.cpp"
)

mkdir -p .objs
OBJS=()
RELINK=0

for src in "${SRCS[@]}"; do
  obj=".objs/$(echo "$src" | tr '/' '_').o"
  OBJS+=("$obj")

  if [[ "$src" == *"zscene"* ]]; then
    EXTRA=""
  else
    EXTRA="-include cmath"
  fi

  if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
    echo "Compilando $src..."
    $CXX $CXXFLAGS $EXTRA -c "$src" -o "$obj" || exit 1
    RELINK=1
  fi
done

if [ $RELINK -eq 1 ]; then
  echo "Linkeando..."
  $CXX $LDFLAGS "${OBJS[@]}" $LIBS -o "gaming chair.exe" || exit 1
  echo "Listo!"
else
  echo "Nada cambio, no hay nada que recompilar."
fi
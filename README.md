# gaming-chair

**gaming-chair** is an external, read-only cheat for counter-strike 2 made to provide good visuals and maintaining a low detection profile. the project implements handle hijacking, dwm bypass to avoid composition delay, and band 4 overlay

---

## technical architecture

### 1. stealth memory acquisition (handle hijacking)
to mitigate risks associated with opening handles directly from the cheat process (VAC does strict handle checks), **gaming-chair** utilizes a proxy to read memory:
- **[process-proxy-hijacking](https://github.com/000nico/process-proxy-hijacking):** uses themes, from svchost, which already has a handle

### 2. no-delay, fullscreen overlay (band 4 overlay and DWM bypass)
standard external overlays typically experience significant composition latency (30-50ms) and can not render on fullscreen mode. **gaming-chair** addresses this by using:
- **[band-4-window-creation](https://github.com/000nico/band-4-window-creation):** this repo injects a payload into `explorer.exe` to create a window within **z-order band 4 (`ZWID_IMMERSIVE_NOTIFICATION`)**. 
- windows created in this band are rendered above almost all other system elements, including exclusive fullscreen applications, without requiring the game to be in windowed or borderless mode.
- **IDXGIOutputDuplication:** by combining band 4 window with the desktop duplication API, the cheat achieves a visual latency of **0-1ms** (thanks to bypassing DWM), ensuring visuals remain synchronized with the game every frame

---

## demo taken with my phone because the cheat its stream proof by design

| gui | visuals |
| :---: | :---: |
| <img src="https://i.imgur.com/HeFgtFw.jpeg" width="700"> | <img src="https://i.imgur.com/TtGeb5w.jpeg" width="700"> |
--- 
## modules

- esp
- tracers
- bones
- aimbot
- triggerbot
- rcs
- config manager
- destruct

---
## warning 
- as mentioned before, the cheat uses band 4 windows, recorders like OBS, Action, etc, normally can not record these
- as if that were not enough, the cheat uses `WDA_EXCLUDEFROMCAPTURE`, not as a stream-proof feature, when using screen mirroring, if you don't use this to exclude your overlay from the capture, it generates an infinite duplication loop.
- my point is that this can't be recorded without a capture card. I read on some internet forum that you can record it if you use Nvidia Shadowplay, and then activate the game bar after you start recording, but I haven't tried it.

- i don't recommend using aimbot; VAC Live is good enough to detect robotic movements
- it might not work correctly if your windows is debloated. Mine is, and it works, but I can't guarantee it will work on everyone, especially regarding the proxy process for reading
- configs are stored in `user/documents/gamingchair`

- if your game is laggy after opening the cheat, even if your fps are still high, your overlay is running at low fps `(this should not happen on modern GPU)`
- in case this happens, lower your FPS limit. Im not sure why this happens even if you, for example, have a 60hz monitor and 180fps. Capping the fps will help anyways

- you may want to update the offsets manually to use this, their variables are located in `modules/reader/reader.cpp`
___

## disclaimer
im not responsible if you get banned, you should not use this on online mode


not ur average shit fr

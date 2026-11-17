# Eclipse RotF — World Director QA Status
## November 17, 2026 — Evening Cron Review

### SOLARA CATHEDRAL ENVIRONMENT DESIGN

**STATUS: DESIGN COMPLETE (v3) — BUILD BLOCKED ON USER EDITOR SESSION**

---

## Current State (Verified This Run)

| Check | Result |
|-------|--------|
| UE5.8 running? | NO — only Epic Launcher processes |
| Remote API (port 30010) | NOT RESPONDING |
| C: drive free | 37GB (99% — still critical) |
| K: drive free | 3.1TB (16% used) |
| Design doc v3 | COMPLETE — 494 lines, 22.8KB |
| DefaultEngine.ini | VERIFIED — all RX 6800 settings correct |
| MegascansPlugin in .uproject | CONFIRMED |
| Content/Environments/ | EMPTY |
| Content/Materials/ | EMPTY |
| K:/Megascans/Solara/ | 4 empty subdirs (Architecture, Decals, Foliage, Materials) |
| SOLARA_CATHEDRAL_AAA | 38MB broken placeholder (NOT Gothic) |
| solara.py | 13.9KB — ready but never executed |
| Desktop screenshots | 13 files, all from Nov 7 (10 days stale) |
| Git | Clean except .Aura + QA docs |
| Build progress since Nov 11 | NONE |

## Design Doc v3 — What's Covered (COMPLETE)

1. **Architecture spec**: 5-zone 300m playable path (Vista → Approach → Nave → Grace Site → Boss Arena)
2. **Gothic elements**: pointed arches, ribbed vaults, flying buttresses, stained glass, gargoyles, tracery
3. **Material palette**: weathered basalt, aged sandstone, brass/bronze, stained glass, moss/lichen
4. **Nanite plan**: 20M source tris → 1.5M visible per frame (RX 6800 budget)
5. **Lumen config**: software GI (RDNA2 no DXR), DownsampleFactor=2, ScreenSpaceGI=True
6. **Volumetric fog**: god rays from ceiling breach, ExponentialHeightFog, 0.015 density
7. **Lighting plan**: warm amber directional (10 lux, 5500K), violet sky light fill, ACES tonemapper
8. **Post-process**: bloom 1.2, vignette 0.6, contrast 1.1, saturation 0.85, film grain 0.15
9. **Megascans shopping list**: 20 assets (10 architectural, 6 materials, 4 foliage/debris)
10. **Performance budget**: 16.6ms (60 FPS) — Nanite 5ms, Lumen 5ms, fog 1.5ms, shadows 2ms, post 1.5ms
11. **VRAM budget**: 8GB textures, 1GB Lumen cache, 2GB Nanite, 0.5GB VSM, 2GB RT, 2.5GB OS
12. **Environmental storytelling**: 8 story beats (abandoned offerings → shattered clock → energy veins)
13. **World Partition plan**: 4 Data Layers, 25,600-unit cells, distance-based streaming
14. **Fallback quality tiers**: High (Lumen software 60fps) → Medium (SSR+SSAO) → Low (baked)
15. **5-phase implementation roadmap**

## 5 Build Blockers (All Require User at UE5.8 Editor)

1. **UE5.8 launch** — not running; needs manual start
2. **Quixel/Fab Epic login** — interactive auth, cannot do from CLI/cron
3. **solara.py execution** — `py content/python/solara.py` in editor console
4. **Megascans download + import + Nanite enable** — GUI workflow in editor
5. **In-engine screenshots + stat gpu profiling** — requires PIE/edit mode

## Performance Concerns (Honest)

1. **Lumen software on RDNA2** is the biggest risk — 5ms budget is aggressive, may need 7ms+ → drop to ~50 FPS
2. **Volumetric fog god rays** are central to cathedral identity but expensive — GridSize=8 helps
3. **Stained glass translucency** — limit to 6-8 visible panes, forward-rendered
4. **Nanite 1.5M visible** is tight — relies on line-of-sight occlusion in nave
5. **No profiling done** — all budgets are estimates, not verified numbers
6. **C: at 99%** — editor may struggle to launch/compile; all large data must go to K:

## What Jamil Needs To Do

1. Launch UE5.8 (Megascans plugin will activate on restart)
2. Login to Quixel Bridge (Window → Quixel Bridge → Sign In with Epic account)
3. Download assets from the shopping list (Section 6.2 of design doc v3)
4. Run `py content/python/solara.py` in editor console for lighting setup
5. Dress the level with Megascans assets
6. Enable Nanite on all imported static meshes
7. Capture screenshots + run `stat gpu` for profiling

Once the cathedral is built in-engine with verified screenshots, I can finalize the performance report with real numbers.

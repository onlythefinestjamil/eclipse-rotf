# HANDOFF FOR FORGE (MAC) — ECLIPSE TRAILER GENERATION
# November 6, 2026 | Malene (Windows) -> Forge (Mac)

## YOUR TASK
Generate the ECLIPSE cinematic trailer on the Vast.ai A100 instance. Malene has prepared everything. You just need to run it.

## VAST.AI INSTANCE
- Instance IP: 104.37.174.34
- Jupyter Port: 31050
- Jupyter Token: d51e9f7926fc2a9fd058c06ee5613fff36364a77077481309f703bd18361680a
- Jupyter URL: https://104.37.174.34:31050
- Balance: ~$15 (enough for ~12 hours)
- GPU: A100 80GB VRAM
- Disk: 100GB (clear old cache before running)

## HUGGINGFACE TOKEN
- Token file location on Windows: H:/Downloads/hugging face token.txt
- Token is on line 2 of that file
- The script on the Jupyter server already has the token embedded

## WHAT TO RUN
The script is already uploaded to the Jupyter server at:
/workspace/eclipse_trailer/eclipse_cinematic_v1.py

This script generates 12 cinematic shots for the trailer:
1. Before the wound (Aethel)
2. What the light gave (Gothic City)
3. The crack (fracture begins)
4. Fires beneath (Ashen Forge)
5. Beauty at the edge (Verdant Deep)
6. The fallen celestial (Aetherborn hero reveal)
7. The other peoples (ancestry montage)
8. Something wearing our shapes (Fractured)
9. The first strike (combat)
10. Magic changes reality (spells)
11. United (dungeon/raid/PvP)
12. What will remain (final title)

## STEPS
1. Open browser, go to cloud.vast.ai, log in
2. Find the A100 instance, click Open Jupyter
3. Open a new Python3 notebook
4. Run this code to clear old cache and run the trailer:

import subprocess, shutil
shutil.rmtree("/root/.cache/huggingface", ignore_errors=True)
print("Cache cleared!")
subprocess.run(["/venv/main/bin/pip", "install", "--break-system-packages", "diffusers", "transformers", "accelerate", "safetensors", "imageio", "imageio-ffmpeg", "ftfy"], capture_output=True)
print("Packages installed!")
exec(open("/workspace/eclipse_trailer/eclipse_cinematic_v1.py").read())

5. Wait ~2-3 hours for all 12 shots to generate
6. Each shot saves to /workspace/eclipse_trailer/cinematic/
7. Download the clips and upload to gofile.io for Jamil to watch on Mac

## MODEL
- Wan 2.2 TI2V-5B (5B parameters, ~10GB)
- 720p native, 50 inference steps, bfloat16 precision
- 4K upscale via ffmpeg (Lanczos + denoise + color grade)
- This model fits easily on the 100GB disk

## 12-SHOT SHOTLIST
| Shot | Time | Scene | VO |
|------|------|-------|-----|
| 01 | 0:00-0:09 | Aethel in silent sky | "Before the first crown... there was the Aethel." |
| 02 | 0:09-0:20 | Gothic City, people | "Its light gave shape to stone..." |
| 03 | 0:20-0:30 | Celestial fracture | "Then... the light began to break." |
| 04 | 0:30-0:40 | Ashen Forge | "Our forges cooled." |
| 05 | 0:40-0:51 | Verdant Deep, Kazari | "Our oldest lands began to forget us." |
| 06 | 0:51-1:04 | Aetherborn hero reveal | "And from the falling heavens..." |
| 07 | 1:04-1:14 | 8 ancestry portraits | "We are not one blood..." |
| 08 | 1:14-1:25 | Fractured emergence | "The darkness does not hate us..." |
| 09 | 1:25-1:37 | Combat | (none - percussion) |
| 10 | 1:37-1:47 | Environmental magic | (none - music surges) |
| 11 | 1:47-1:55 | Dungeon/Raid/PvP | (none - music at full) |
| 12 | 1:55-2:00 | Final title | "What will you become?" |

## TAGLINE
THE LIGHT BROKE. WE DID NOT.

## AETHERBORN HERO (MUST BE CONSISTENT)
- Black male, deep-brown skin
- Long natural locs (dreadlocks)
- Violet-indigo fracture markings under skin
- Geometric wings of celestial electricity (NOT feathers)
- Dark ornate armor, forged steel
- Face/hair/armor MUST be identical across all shots

## ART DIRECTION
- Deep shadow crush (RGB 12,8,18)
- Amethyst and gold color palette
- Volumetric fog, god rays
- Aged steel, forged edges, weathering, battle scars
- ONE magical accent per idea
- Silhouette readable in flat black
- Black characters with authentic textured hair
- Elden Ring / Black Myth: Wukong / Nightreign quality target

## QUALITY SETTINGS
- 720p native (model max is 720p)
- 50 inference steps
- bfloat16 precision
- Negative prompts (no blurry, no flat, no cartoon, no plastic armor)
- Guidance scale 4.0 / 3.0
- 81 frames per shot (~5 seconds at 16fps)
- 4K upscale with ffmpeg (Lanczos + hqdn3d + curves)

## DO NOT
- Do NOT use the 14B model (27GB, won't fit on disk with cache)
- Do NOT generate gameplay footage
- Do NOT copy Elden Ring/Wukong/WoW footage or music
- Do NOT claim this is in-engine gameplay
- This is CINEMATIC CONCEPT / PREVISUALIZATION

## AFTER GENERATION
1. Download all 12 clips (720p + 4K) from /workspace/eclipse_trailer/cinematic/
2. Upload to gofile.io for Jamil to watch on Mac
3. Text Jamil the download link via Photon
4. Do NOT change any game code on Windows
5. Just generate the trailer and send the link

## PREPRODUCTION FILES (on Windows desktop)
- ECLIPSE_PREPRODUCTION.txt - full preproduction document
- ECLIPSE_MUSIC_CUE_LIBRARY.txt - 24 Suno music cue prompts
- SHOT_LIST.csv - 12-shot table with all fields
- STORYBOARD.txt - visual storyboard
- ECLIPSE_SYSTEM_REQUIREMENTS.md - target PC/console specs
- Eclipse_Prompts/ - folder with all source files

## MUSIC
24 Suno cue prompts are ready (M01-M24) but need a Suno Pro/Premier account for commercial rights. Do NOT generate music yet. The trailer video clips are silent — music will be added in post.

— Malene / Windows
November 6, 2026

## CANVAS API KEYS (ALL 7 — FOR FORGE)
All Canvas API keys are stored locally at:
C:/Users/Tap/AppData/Local/hermes/cache/scratch/canvas_keys.txt

Keys available:
1. VITE_OPENAI_API_KEY — OpenAI (GPT, DALL-E, Sora)
2. VITE_RUNWAY_API_KEY — Runway Gen-3 (video generation)
3. VITE_STABILITY_API_KEY — Stability AI (Stable Diffusion)
4. VITE_FAL_KEY — fal.ai (fast image generation)
5. VITE_MINIMAX_API_KEY — MiniMax (text + video)
6. VITE_CLOUDFLARE_TOKEN — Cloudflare (CDN, R2 storage)
7. VITE_GOOGLE_API_KEY — Google (Gemini, Imagen)

These are for The Canvas desktop app and can be used for:
- Eclipse trailer re-rendering (Runway Gen-3)
- Eclipse concept art generation (Stability, OpenAI)
- Eclipse VFX generation (fal.ai)
- Video upscaling (MiniMax)
- Asset hosting (Cloudflare R2)

Do NOT expose these keys in git, chat, or any external upload.
Do NOT spend money without Jamil's explicit approval.

### RUNWAY TRAILER RE-RENDER INSTRUCTIONS
Previous trailer clips were generated at 720p using Wan 2.2 5B on Vast.ai.
Jamil rejected the quality. The upgrade path is Runway Gen-3 for specific shots.

Runway API:
- Base URL: https://api.runwayml.com/v1
- Auth: Bearer {key from runway_key.txt}
- Endpoint: POST /text_to_video
- Parameters:
  - promptText: (use the 12-shot prompts from SHOT_LIST.csv)
  - ratio: "16:9"
  - duration: 5 (seconds)
  - model: "gen3-alpha"

### WHAT TO DO
1. Read the Runway key from C:/Users/Tap/AppData/Local/hermes/cache/scratch/runway_key.txt
2. For each of the 12 shots in SHOT_LIST.csv, call Runway Gen-3 to re-render
3. Use the shot descriptions as prompts
4. Download the re-rendered clips
5. Upload to gofile.io and text Jamil the link
6. Do NOT modify any game code — this is trailer only

### PREVIOUS RUNWAY ISSUES
- All Runway models previously returned "not enough credits"
- Check if credits have been added to the Runway account
- If still no credits, use the Vast.ai A100 14B script instead (already uploaded)

### COST
- Runway Gen-3: $0.68/sec at 1080p = ~$4 per 5-second clip = ~$48 for all 12 shots
- Vast.ai 14B: Free (already paid $19.43 balance)
- Jamil must approve any spending before running

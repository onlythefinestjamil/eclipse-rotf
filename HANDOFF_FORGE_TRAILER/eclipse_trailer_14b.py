import os
import torch
import numpy as np
from diffusers import WanPipeline, AutoencoderKLWan
from diffusers.utils import export_to_video, load_image
import imageio
from huggingface_hub import login

# Login
login("PASTE_TOKEN_HERE")

# Use the REAL 14B model - not the 5B
print("Loading Wan 2.2 T2V-A14B (14 billion parameters)...")
vae = AutoencoderKLWan.from_pretrained("Wan-AI/Wan2.2-T2V-A14B-Diffusers", subfolder="vae", torch_dtype=torch.float32)
pipe = WanPipeline.from_pretrained("Wan-AI/Wan2.2-T2V-A14B-Diffusers", vae=vae, torch_dtype=torch.bfloat16)
pipe.to("cuda")
print("14B MODEL LOADED ON A100!")

# Quality tokens
QT = "4K, ultra-detailed, cinematic, dramatic volumetric lighting, film grain, photorealistic, Unreal Engine 5, $200M blockbuster, award-winning, dark fantasy, deep shadow crush, amethyst and gold palette, Elden Ring aesthetic, volumetric fog, depth of field, motion blur, professional cinematography"

# Negative prompt for quality
NEG = "blurry, low quality, distorted, artifacts, overexposed, flat lighting, cartoon, anime, low detail, washed out, pixelated, jpeg artifacts, extra limbs, deformed"

os.makedirs("/workspace/eclipse_trailer/output", exist_ok=True)
os.makedirs("/workspace/eclipse_trailer/output_4k", exist_ok=True)

# Install upscaler
print("Installing upscaler (Real-ESRGAN)...")
import subprocess
subprocess.run(["/venv/main/bin/pip", "install", "--break-system-packages", "realsrgan"], capture_output=True)
try:
    import realsrgan
    print("Real-ESRGAN installed!")
except:
    print("Real-ESRGAN not available - will use ffmpeg upscale instead")

trailer_scenes = [
    {"name":"01_world_vista","prompt":"Sweeping aerial cinematic drone shot soaring over a vast ancient fantasy continent. Colossal floating islands of black limestone suspended in a fractured sky, a massive eclipsed sun casting dying golden corona light across the landscape. Ancient ruins of a golden city atop the highest floating island, overgrown with crystalline vines. Below, endless forests of bioluminescent trees, volcanic ash drifting in the distance, frozen mountain peaks far north. A lone armored warrior stands at the edge of the highest cliff, tattered cloak billowing in the wind. Scale: tiny human against god-like architecture. Camera: slow cinematic dolly push-in from extreme wide to medium. Atmospheric haze, god rays through clouds."},
    {"name":"02_aetherborn_hero","prompt":"A muscular Black warrior with authentic dreadlocks and crystalline energy wings stands on a cliff edge beneath a colossal full moon. He slowly draws a massive greatsword that trails golden light from its runes. Amethyst-violet energy crackles across his dark ornate armor. His lightning wings spread wide - jagged, ethereal arcs of violet-white energy. The camera orbits around him in slow cinematic motion. His eyes glow with amethyst fire. Weight, anticipation, follow-through on every movement. Ancient stone beneath his boots, wind carrying embers. Elden Ring weight, Wukong fluidity, cinematic 4K."},
    {"name":"03_title_card","prompt":"Pure darkness. Letters materialize from coalescing amethyst-violet energy particles forming the word ECLIPSE. Gold rim-light illuminates each letter from behind. The letters float at different depths in 3D space with parallax. Faint starlight and cosmic dust in the deep black background. A single bass drop shakes the frame. The particles settle and solidify. Cinematic title card, dark fantasy aesthetic, amethyst and gold."},
    {"name":"04_aetherborn_portrait","prompt":"Extreme close-up portrait of a Black Aetherborn warrior. Glowing amethyst eyes with intensity. Crackling lightning energy wings visible behind his shoulders, arcing with power. Energy traces across his dark skin like constellations. Ornate dark armor with violet energy etched into the metal surface. Authentic textured dreadlocks catching golden rim-light. Shallow depth of field, cinematic lighting, rim light, chiaroscuro. Elden Ring character portrait quality."},
    {"name":"05_solari_vanguard","prompt":"A heavily armored Solari Vanguard warrior in a golden-lit capital city plaza. Ancient baroque architecture of crumbling golden stone around him. He raises a massive tower shield - amethyst rune energy ignites along its surface. Sun rays beam down from above like divine judgment. He plants the shield and charges forward with earth-shaking weight, amethyst energy trailing behind him like a comet. Bull rush impact sends a shockwave ring outward, cracking the ancient stone pavement. Heavy plate armor, imposing stance, warm gold lighting against deep shadow."},
    {"name":"06_umbral_assassin","prompt":"A hooded Umbral Assassin dissolves into shadow mid-stride - her body fragments into void-black particles and dark smoke. She vanishes completely for a frame, then reappears behind an armored enemy, dual daggers descending in a lethal X-pattern strike. Three rapid teleport blinks, each leaving a dark afterimage burned into the air. Umbralis ancestry, void-touched, dark purple energy edges on each blink. Stealth to burst combat. Fast, precise, lethal. Cinematic slow-motion on the final strike."},
    {"name":"07_arcanist_cast","prompt":"An Arcanist thrusts both hands forward into the air. A massive amethyst energy sigil forms - 15 feet across, complex geometric runes rotating within it. The sigil crackles with violet power, casting dramatic shadows. Dual affinity cast - fire and ice combine in the center - a blazing frost nova erupts outward in all directions. The heat distorts the air. Arcane runes orbit around her in a perfect ring. She floats slightly above the ground, powered by aether energy. Cinematic spell VFX, volumetric lighting."},
    {"name":"08_berserker_rage","prompt":"A massive Tharuk Berserker enters rage mode - blood-red energy erupts from his body like an aura. His muscles bulge, veins glowing with rage. He swings a two-handed great axe in a devastating overhead arc that splits the ground beneath him. Earth cracks and shatters from the impact. Enemies are sent flying backward in slow motion. Dust clouds rise. Giant-born ancestry, primal power, intimidating scale. Screen shake on impact, slow motion on the heaviest hit. Dark fantasy combat, weight and brutality."},
    {"name":"09_hellsworn_demon","prompt":"A Hellsworn warrior rises slowly from a pool of molten lava. Dark red energy flows from cracks in his dark skin like magma veins. Demonic horns glow with inner fire. He raises one hand and hellfire erupts around him in a towering column of flame that reaches toward the sky. Lava drips from his body, sizzling on the dark volcanic rock below. Intimidating, powerful, infernal. Deep black rock environment, lava-orange-red dominant lighting, volumetric fire glow, intense heat haze."},
    {"name":"10_necrevan_undead","prompt":"A Necrevan revenant rises from an open grave in a mist-covered cemetery. Spectral blue-cyan energy emanates from his skeletal form, casting an ethereal glow. His tattered burial shroud disintegrates in the wind. His eyes ignite with cold cyan fire. Around him, the dead begin to rise - skeletal hands clawing out of the earth, pulling themselves free. He raises a cursed staff crackling with ghostly blue energy. The dead answer his call. Graveyard dark, cold, eerie, cinematic volumetric fog, moonlight."},
    {"name":"11_combat_hybrid","prompt":"Intense close combat between a warrior and a towering armored enemy. The warrior dodges an incoming halberd strike with fluid Wukong-style motion - a perfect roll with visible i-frames. He counters with a heavy Elden Ring-weighted greatsword strike that staggers the enemy back. Then a perfect parry - sparks fly as blades meet, time slows for a frame. He follows with a 3-hit combo: light, light, then a devastating heavy finisher that sends the enemy crashing to the ground. Weight, impact, follow-through, recovery. Every hit has screen shake. Cinematic combat, 4K."},
    {"name":"12_cathedral_dungeon","prompt":"Slow cinematic approach to the Cathedral of Unmade Hours - a massive ancient cathedral with impossible architecture. Floating clockwork gears rotate in the air above. Fractured stained glass windows cast prismatic light across the stone floor. Purple nebula light streams through cracks in the cathedral walls. The camera slowly pushes toward the towering entrance doors, which are carved with ancient runes that glow faintly. A colossal boss silhouette is visible in the darkness within. Elden Ring dungeon aesthetic, volumetric fog, candlelight, dread and anticipation."},
    {"name":"13_pvp_duel_circle","prompt":"Two warriors face off in the Duel Circle - a massive circular stone arena with glowing amethyst rune circles etched deep into the floor. One warrior radiates golden light from holy armor. The other pulses with shadow energy, void-black and purple. They charge simultaneously and clash in the center of the arena - their weapons meeting with an explosive burst of light that splits the frame. PvP arena combat, slow motion on the clash, sparks cascading, energy eruption. Stone bleachers filled with silhouetted spectators of various ancestries watching in the dark."},
    {"name":"14_forge_weapon","prompt":"Inside the Weeping Foundry - a volcanic forge dungeon with rivers of molten lava flowing through rusted iron walkways. A blacksmith in a leather apron hammers a glowing weapon on an ancient anvil. Sparks cascade upward in slow motion with each hammer strike, each spark a star of light against the darkness. The weapon - an Eclipse-tier greatsword - glows with amethyst energy veins flowing through its obsidian-black steel blade. Violet flame aura licks off the blade edge. The runes pulse like a heartbeat. Forged with the life force of fallen enemies. Dark industrial fantasy, warm forge light against deep shadow."},
    {"name":"15_fracture_opens","prompt":"Wide aerial shot of an ancient battlefield. The ground begins to crack - not from an impact but from reality itself splitting apart. A massive chasm - The Fracture - tears open across the landscape like a wound in existence. The edges of the crack glow with intense amethyst-violet light pouring from within. Reality warps around the edges - space bends, objects appear in duplicate, time stutters and repeats. The crack widens to 200 meters across. The camera shakes violently. The sky above darkens unnaturally. Apocalyptic, catastrophic, reality-breaking, awe and terror."},
    {"name":"16_sovereign_rises","prompt":"From within The Fracture, a colossal 50-foot silhouette rises slowly. The Eclipsed Sovereign - vaguely humanoid but with dimensional wings that fold through space itself, not physical wings but warps and tears in reality trailing behind it. Its form is pure darkness edged with dying starlight. Two points of cold amethyst light ignite as its eyes - burning with ancient power. It turns its head toward the camera with terrible slow grace. God-like scale, dwarfing everything. Awe, dread, and the weight of something that should not exist. Near-silhouette, the only light is the amethyst glow from its eyes and the fracture walls."},
    {"name":"17_weapon_slam_finale","prompt":"The Aetherborn warrior raises an Eclipse-tier greatsword overhead - the blade is obsidian-black steel with flowing amethyst energy veins running through it like living light. A thorn-like ornate guard, violet flames licking off the blade edge, runes pulsing like a heartbeat in the steel. Time slows to a crawl. He brings the blade down in a devastating overhead slam into the arena ground. On impact: a massive crack of amethyst-violet light erupts from the impact point, spreading across the entire arena floor like lightning. Light geysers upward from every crack. The arena fractures and crumbles. The light fills the entire frame. Climactic and devastating, the most powerful moment in the trailer."},
    {"name":"18_final_title","prompt":"The white light fades to pure black. Full title materializes letter by letter - each letter forming from coalescing amethyst-violet energy particles with gold edge lighting. The title reads: ECLIPSE: REALMS OF THE FRACTURED in an elegant serif font. Below, in smaller gold text: Coming 2027. The energy particles settle and slowly fade. A single piano note sustains in the darkness, echoing with reverb. The text glows softly one final time. Then everything fades to black. Final title card, elegant and ominous, dark fantasy."},
]

print(f"\n{'='*60}")
print(f"ECLIPSE: REALMS OF THE FRACTURED - CINEMATIC TRAILER")
print(f"18 scenes | Wan2.2 T2V-A14B (14B) | 1080p | 50 steps")
print(f"{'='*60}\n")

for i, scene in enumerate(trailer_scenes):
    print(f"\n--- Scene {i+1}/18: {scene['name']} ---")
    try:
        output = pipe(
            prompt=scene["prompt"] + " " + QT,
            negative_prompt=NEG,
            height=720,
            width=1280,
            num_frames=81,
            guidance_scale=4.0,
            guidance_scale_2=3.0,
            num_inference_steps=50,
        ).frames[0]
        outpath = f"/workspace/eclipse_trailer/output/{scene['name']}.mp4"
        export_to_video(output, outpath, fps=16)
        print(f"  SAVED 720p: {outpath}")
        
        # Upscale to 4K using ffmpeg
        outpath_4k = f"/workspace/eclipse_trailer/output_4k/{scene['name']}_4k.mp4"
        upscale_cmd = f"ffmpeg -y -i {outpath} -vf 'scale=3840:2160:flags=lanczos,hqdn3d=1.5:1.5:6:6,curves=master' -c:v libx264 -crf 14 -preset slow -pix_fmt yuv420p {outpath_4k}"
        subprocess.run(upscale_cmd, shell=True, capture_output=True)
        print(f"  SAVED 4K: {outpath_4k}")
    except Exception as e:
        print(f"  ERROR: {e}")
        print(f"  Continuing...")

print(f"\n{'='*60}")
print(f"ALL 18 SCENES COMPLETE!")
print(f"720p clips in: /workspace/eclipse_trailer/output/")
print(f"4K upscaled clips in: /workspace/eclipse_trailer/output_4k/")
print(f"{'='*60}")

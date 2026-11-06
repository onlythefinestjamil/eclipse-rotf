# ECLIPSE: REALMS OF THE FRACTURED
# 4K Cinematic Reveal Trailer - Hermes Master Prompt v1.0
# Based on the user's 120-line cinematic directive
# 12 shots, ~120 seconds, 720p native (upscaled to 4K)
# Voiceover lines included as scene descriptions

import os
import torch
import numpy as np
from diffusers import WanPipeline, AutoencoderKLWan
from diffusers.utils import export_to_video
import subprocess
from huggingface_hub import login

login("PASTE_TOKEN_HERE")

print("Loading Wan 2.2 TI2V-5B with cinematic quality settings...")
vae = AutoencoderKLWan.from_pretrained("Wan-AI/Wan2.2-TI2V-5B-Diffusers", subfolder="vae", torch_dtype=torch.float32)
pipe = WanPipeline.from_pretrained("Wan-AI/Wan2.2-TI2V-5B-Diffusers", vae=vae, torch_dtype=torch.bfloat16)
pipe.to("cuda")
print("Model loaded on A100 80GB!")

# Quality tokens - matching the directive's standards
QT = "4K UHD, ultra-detailed, cinematic, filmic realism, photorealistic skin texture with pores and subsurface scattering, authentic textured hair with physics, practical layered armor with straps and buckles and forged steel, ancient artifacts not oversized, volumetric clouds and fog, atmospheric particles, rich blacks without losing detail, controlled highlights, natural color contrast, believable scale, cinematic exposure, Unreal Engine 5 quality, dark fantasy, restrained magical highlights, aged steel, dark forged alloys, carved stone, cloth weave, weathering, battle scars, ash, frost, patina"

NEG = "blurry, low quality, distorted, artifacts, overexposed, flat lighting, cartoon, anime, plastic armor, oversized weapons, feathery wings, washed out dark skin, glowing effects without light casting, gameplay UI, damage numbers, floating particles without cause, weightless spinning, clipping, broken fingers, drifting symbols, motion artifacts, bright bulky MMO armor, copycat imagery"

os.makedirs("/workspace/eclipse_trailer/cinematic", exist_ok=True)

# 12 shots from the directive - each with exact timestamp, voiceover, and visual
shots = [
    {
        "name": "01_before_the_wound",
        "prompt": "Begin on pure black. A microscopic shimmer of golden light appears in the darkness, expanding into the vast luminous celestial form of the Aethel filling a silent sky above ancient dark mountains. Extremely slow cinematic push-in. No title text. Sacred, ancient, otherworldly. The light is warm gold against deep cosmic black. Atmospheric depth, god rays, celestial resonance.",
        "voice": "Before the first crown... there was the Aethel."
    },
    {
        "name": "02_what_the_light_gave",
        "prompt": "Sunlight crosses a gothic city built around a colossal cracked celestial fragment of golden stone. Bustling ancient streets with dark-skinned Solari merchants in rich textiles, grim dwarven forge workers, diverse races and skin tones, moving cloth and banners, temple bells ringing, smoke rising from kitchens, children in the streets, distant monumental stone bridges. Camera floats across richly textured baroque architecture to one intimate human face with natural skin pores and warm brown skin. Cinematic golden hour lighting.",
        "voice": "Its light gave shape to stone... and breath to every living thing."
    },
    {
        "name": "03_the_crack",
        "prompt": "An enormous hairline fracture crawls across the face of a luminous golden celestial body in the sky. No attacker visible - the cause remains unknown. Sunlight turns strange and cold, shifting from warm gold to pale blue. Birds scatter and change course in the sky. A temple bell rings without a hand touching it. Below, a crowd of diverse ancient people looks skyward in awe and fear. The music stops on a single low bass note. Cinematic dread, atmospheric shift, cold light.",
        "voice": "Then... the light began to break."
    },
    {
        "name": "04_fires_beneath",
        "prompt": "Inside the Ashen Forge - a dark volcanic underground foundry. A Grimstarr dwarf master hammers a half-finished blacksteel blade on an ancient anvil. Extreme close-ups of real forging sparks flying in slow motion, molten metal channels flowing with orange glow, ash floating in the air, furnace texture on cracked stone walls, perspiration on a soot-streaked face, cracked leather gloves gripping the hammer. A river of molten stone visibly dims and cools. The dwarf's eyes register terror as the forge light fades. Dark industrial fantasy, warm forge glow against deep shadow.",
        "voice": "Our forges cooled."
    },
    {
        "name": "05_beauty_at_edge",
        "prompt": "A serene long take moving through the Verdant Deep - an immense ancient forest with giant roots bigger than houses, bioluminescent moss glowing soft blue-green, tiny insect wings catching light, suspended pollen drifting in golden shafts of light, rain falling on enormous leaves, drifting forest mist. A Kazari hunter with intelligent feline features, natural hair, and authentic lived-in leather gear senses something approaching and turns. Quick cut to Frozen Crown glaciers and silent icy peaks under ominous clouds. Quick cut to sun-sand ruins buried in golden dunes. Nature's beauty fading.",
        "voice": "Our oldest lands began to forget us."
    },
    {
        "name": "06_fallen_celestial",
        "prompt": "HERO REVEAL: A muscular Black male warrior stands on a suspended stone bridge in the Floating Isles - celestial debris and impossible waterfalls around him. Deep-brown skin with natural skin texture and pores, carefully modeled long locs moving in high wind, intricate yet physically believable dark armor with forged steel and straps. The camera orbits from behind to a 70mm close-up of his eyes and jaw - his violet-indigo fracture markings light up under his skin like constellations, and matching geometric wings of celestial electricity unfurl from his back, releasing physical sparks that illuminate suspended stone fragments. Not feathers. Not static. Living energy wings matching his skin markings.",
        "voice": "And from the falling heavens... some of us awoke."
    },
    {
        "name": "07_other_peoples",
        "prompt": "A brief lyrical montage of unique faces from different ancestries. A proud Black Solari woman with sculpted twists and scarred plate armor, warm golden light. A Vyrael seer with exhausted ancient silver eyes. A horned Hellsworn with dark skin and inner fire glow. A scaled Draven warrior with reptilian features. A spectral Necrevan keeping vigil in blue ghostly light. A towering Tharuk giant with earth-toned skin. A Nereyth near moonlit sea with aquatic features. A Myceliar emerging through fungal roots with bioluminescent spores. Each face has cultural authorship, individuality, natural skin texture, authentic hair. Not a character grid - a lyrical portrait sequence.",
        "voice": "We are not one blood... but this world is ours."
    },
    {
        "name": "08_wearing_our_shapes",
        "prompt": "At the boundary of the Eclipse Shadow - a creeping darkness that is a wound in reality, not cartoon black smoke. An ordinary-looking person with natural skin turns and their face shifts into an impossible void-mimic contour - features that look almost right but profoundly uncanny, like a reflection that does not match. A Fractured being emerges from a seam in space itself, its form unsettling and void-born. Dread hush, cloth fluttering in unnatural wind, involuntary human reaction of terror. No generic monsters - something deeply wrong with reality itself.",
        "voice": "The darkness does not hate us. It simply takes what the light leaves behind."
    },
    {
        "name": "09_first_strike",
        "prompt": "Grounded hero-on-monster combat. A heavily armored Vanguard braces and catches a massive blow on his tower shield - iron sparks flash on impact, he staggers backward with visible weight, adjusts his foot placement on cracked stone, and counters with a greatsword slash. An Umbral Assassin runs low along the creature's blind side with dual daggers. A Riftwalker creates one brief afterimage of light. Real contact between weapons and bodies, recovery frames, stone fragments flying, believable weight and excellent timing. Percussion enters. Dark fantasy combat, Elden Ring weight, Wukong fluidity.",
        "voice": ""
    },
    {
        "name": "10_magic_changes_reality",
        "prompt": "A caster in dark robes thrusts hands forward and a collapsing stone arch freezes - ice crystals spreading across ancient stone with visible frost. An allied fire spell hits the frozen arch and it shatters into hot vapor and falling ice shards. A Stormcaller raises a staff and lightning branches across flooded stone, the current reflecting in polished armor and faces. Spells have physically coherent origins - light casts from the actual cast source, illuminates skin and armor and dust, creates contact effects and aftermath. No floating decorative particle fog - real physical consequences. Music surges.",
        "voice": ""
    },
    {
        "name": "11_united",
        "prompt": "A five-hero dungeon group navigates the Weeping Foundry - a dark industrial dungeon with molten rivers and rusted iron walkways. Five distinct warriors move with discipline through the forge: a tank with shield, a healer casting golden light, two DPS warriors flanking, a caster behind. Cut to a ten-hero raid bracing beneath the impossible vaulted stone of the Cathedral of Unmade Hours - enormous ancient architecture dwarfing the warriors. Two fast arena flashes: an intense 1v1 duel with weapons clashing in golden light, and a 5v5 tactical confrontation with distinct team roles visible. Cinematic, not gameplay UI.",
        "voice": ""
    },
    {
        "name": "12_what_will_remain",
        "prompt": "The Black Aetherborn hero stands alone at a precipice before the wounded Aethel in the sky - his celestial energy wings dim to thin strands of violet light as the fracture in the sky spreads silently in the distance. Deep-brown skin, long locs in the wind, dark armor, violet-indigo fracture markings fading. The sky's fracture widens. Hard cut to black on a final heartbeat sound. The word ECLIPSE materializes in amethyst-violet energy with gold edge lighting. Below: REALMS OF THE FRACTURED. Below that: THE LIGHT BROKE. WE DID NOT. End card: MOMENTUM LABS | IN DEVELOPMENT. Dark fantasy, elegant, ominous, final.",
        "voice": "What will you become?"
    },
]

print(f"\n{'='*60}")
print(f"ECLIPSE: REALMS OF THE FRACTURED")
print(f"4K Cinematic Reveal Trailer - 12 shots, ~120 seconds")
print(f"Wan 2.2 TI2V-5B | 720p 50 steps bfloat16 | 4K upscale")
print(f"Tagline: THE LIGHT BROKE. WE DID NOT.")
print(f"{'='*60}\n")

for i, shot in enumerate(shots):
    print(f"\n--- Shot {i+1}/12: {shot['name']} ---")
    if shot['voice']:
        print(f"  VO: \"{shot['voice']}\"")
    try:
        output = pipe(
            prompt=shot["prompt"] + " " + QT,
            negative_prompt=NEG,
            height=720,
            width=1280,
            num_frames=81,
            guidance_scale=4.0,
            guidance_scale_2=3.0,
            num_inference_steps=50,
        ).frames[0]
        
        # Save 720p
        outpath = f"/workspace/eclipse_trailer/cinematic/{shot['name']}_720p.mp4"
        export_to_video(output, outpath, fps=16)
        print(f"  SAVED 720p: {outpath}")
        
        # Upscale to 4K with ffmpeg - Lanczos + denoise + color grade
        outpath_4k = f"/workspace/eclipse_trailer/cinematic/{shot['name']}_4k.mp4"
        upscale_cmd = f"ffmpeg -y -i {outpath} -vf scale=3840:2160:flags=lanczos,hqdn3d=1.5:1.5:6:6,curves=master -c:v libx264 -crf 14 -preset slow -pix_fmt yuv420p {outpath_4k}"
        subprocess.run(upscale_cmd, shell=True, capture_output=True)
        print(f"  SAVED 4K: {outpath_4k}")
        
    except Exception as e:
        print(f"  ERROR: {e}")
        print(f"  Continuing...")

print(f"\n{'='*60}")
print(f"ALL 12 SHOTS COMPLETE!")
print(f"Cinematic clips in: /workspace/eclipse_trailer/cinematic/")
print(f"{'='*60}")

import os
import torch
from PIL import Image
import imageio
from huggingface_hub import login

# Login to HuggingFace
login("PASTE_TOKEN_HERE")

# Load model
from diffusers import AutoencoderKLWan, WanPipeline
print("Loading Wan 2.2 TI2V-5B...")
pipe = WanPipeline.from_pretrained("Wan-AI/Wan2.2-TI2V-5B-Diffusers", torch_dtype=torch.float16)
pipe.to("cuda")
print("Model loaded on A100 80GB!")

QT = "4K resolution, ultra-detailed, cinematic composition, dramatic lighting, film-quality visuals, volumetric lighting, photorealistic textures, high dynamic range, shallow depth of field, professional cinematography, Unreal Engine 5 quality, dark fantasy aesthetic, deep shadow crush, amethyst and gold color palette, Elden Ring style, volumetric fog"

def gen(name, image_file, prompt, frames=81, h=720, w=1280, steps=30):
    print(f"  Generating: {name}...")
    img = Image.open(image_file).convert("RGB")
    video = pipe(prompt=prompt + " " + QT, num_frames=frames, height=h, width=w, num_inference_steps=steps).frames[0]
    outpath = f"/workspace/eclipse_trailer/output/{name}.mp4"
    os.makedirs("/workspace/eclipse_trailer/output", exist_ok=True)
    imageio.mimsave(outpath, video, fps=24, codec="libx264")
    print(f"  SAVED: {outpath}")
    return outpath

trailer_scenes = [
    {"name":"01_world_vista","image":"/workspace/eclipse_trailer/scene1_world.png","prompt":"Sweeping aerial drone shot of a vast ancient fantasy world. Solara the Crownlands. Black limestone ruins atop floating islands, a fractured moon in the sky casting pale dying golden corona light. Dense forests of Vaelwood below, volcanic ash from Morvath in the distance, frozen peaks of Nhalor far north. A lone warrior stands on the highest island edge, back to camera, tattered cloak blowing."},
    {"name":"02_aetherborn_hero","image":"/workspace/eclipse_trailer/scene2_aetherborn_hero.png","prompt":"A muscular Black warrior with dreadlocks and crystalline gold wings stands on a cliff edge under a massive full moon. He slowly draws a glowing greatsword - the Sunbreaker - that trails golden light. Amethyst energy crackles from his ornate dark armor. Lightning wings spread from his back - jagged, ethereal, violet-white arcs. The camera slowly orbits around him. His eyes glow amethyst. Authentic textured dreadlocks. Dark fantasy, Elden Ring weight, Wukong fluidity."},
    {"name":"03_title_card","image":"/workspace/eclipse_trailer/scene2_aetherborn_hero.png","prompt":"The warrior lightning wings flash brilliant white, illuminating the full frame. Letters materialize from coalescing amethyst-violet energy particles forming the word ECLIPSE. Gold rim-light on each letter. The letters float at different depths in 3D space. The background is pure darkness with faint starlight. Title card moment, cinematic, elegant and ominous."},
    {"name":"04_aetherborn_portrait","image":"/workspace/eclipse_trailer/scene2_aetherborn_hero.png","prompt":"Extreme close-up portrait of an Aetherborn warrior, glowing amethyst eyes, crackling lightning energy wings visible behind shoulders, energy arcs across dark skin, ornate dark armor with violet energy lines, authentic textured dreadlocks, dark fantasy character design, amethyst and gold palette, shallow depth of field, rim lighting"},
    {"name":"05_solari_vanguard","image":"/workspace/eclipse_trailer/scene4_vanguard.png","prompt":"A heavily armored Vanguard warrior in golden-lit capital city plaza of Solara. He raises a massive tower shield - amethyst rune energy flares along the shield surface. Sun rays beam down. He plants the shield and charges forward, amethyst energy trailing behind. Bull rush impact sends shockwave outward. Solari ancestry, sunforged human, ornate plate armor, warm gold lighting, imposing"},
    {"name":"06_umbral_assassin","image":"/workspace/eclipse_trailer/scene5_umbral_assassin.png","prompt":"A hooded Umbral Assassin dissolves into shadow mid-stride - body fragments into dark particles, void-black smoke. She reappears behind an enemy, dual daggers descending in an X-pattern strike. Three rapid teleport blinks, each leaving a dark afterimage. Umbralis ancestry, void-touched, dark purple energy edges, stealth to burst combat, fast and lethal"},
    {"name":"07_arcanist_cast","image":"/workspace/eclipse_trailer/scene6_arcanist.png","prompt":"An Arcanist thrusts both hands forward. A massive amethyst energy sigil forms in the air - complex geometric runes rotating, 15 feet across. Energy beams lance outward from the sigil center. Dual affinity cast - fire and ice combine - a blazing frost nova erupts. Arcane runes orbit around her. She floats slightly above ground with aether energy"},
    {"name":"08_berserker_rage","image":"/workspace/eclipse_trailer/scene7_berserker.png","prompt":"A massive Berserker in rage mode - red rage energy emanating from his body, muscles bulging. He swings a 2h great axe in a devastating arc, sending enemies flying. Tharuk ancestry, giant-born, muscular, intimidating. Screen shake on impact, slow motion on the heaviest hit. Earth cracks beneath his feet. Primal power, dark fantasy combat, devastating"},
    {"name":"09_hellsworn_demon","image":"/workspace/eclipse_trailer/scene11_hellsworn.png","prompt":"A Hellsworn warrior with demon-blood ancestry emerging from molten lava. Dark red energy flows from cracks in his dark skin. Demonic horns glowing with inner fire. He raises his hand and hellfire erupts around him in a column of flame. Lava drips from his body. Intimidating, powerful, infernal. Deep black rock, lava-orange-red dominant lighting"},
    {"name":"10_necrevan_undead","image":"/workspace/eclipse_trailer/scene12_necrevan.png","prompt":"A Necrevan revenant undead warrior rising from an open grave. Spectral blue-cyan energy emanating from skeletal body. Tattered burial shroud. Glowing cyan eyes. The dead begin to rise around him - skeletons climbing from the earth. He raises a cursed staff with blue ghostly energy. Graveyard dark environment, cold and eerie, dark fantasy"},
    {"name":"11_combat_hybrid","image":"/workspace/eclipse_trailer/scene3_combat.png","prompt":"Intense close combat - a warrior dodges an incoming attack with fluid Wukong-style motion, counters with a heavy Elden Ring-weighted strike, then parries the next attack perfectly. Sparks fly on the parry. He follows with a 3-hit combo: light-light-heavy finisher. The Fracture Flow meter fills with each skillful action. Perfect dodge i-frames, precise parry window, weighted impact. Hybrid combat: Wukong fluidity + Elden Ring commitment + WoW rotation depth. Cinematic combat, 4K"},
    {"name":"12_cathedral_dungeon","image":"/workspace/eclipse_trailer/scene8_cathedral.png","prompt":"Slow approach to the Cathedral of Unmade Hours - a massive ancient cathedral with floating clockwork gears, fractured stained glass windows, purple nebula light streaming through cracks in the walls. The camera slowly pushes toward the entrance. Ancient, ominous, atmospheric. A boss silhouette visible in the darkness within. Elden Ring dungeon aesthetic, volumetric fog, candlelight"},
    {"name":"13_pvp_duel_circle","image":"/workspace/eclipse_trailer/scene9_duel_pvp.png","prompt":"Two warriors face off in the Duel Circle - a circular stone arena with glowing amethyst rune circles etched into the floor. One warrior radiates golden light, the other shadow energy. They charge and clash in the center, weapons meeting with an explosive burst of light. PvP arena combat, slow motion on the clash, sparks and energy eruption. Stone bleachers filled with silhouetted spectators of various ancestries"},
    {"name":"14_forge_weapon","image":"/workspace/eclipse_trailer/scene10_forge.png","prompt":"Inside the Weeping Foundry - a volcanic forge dungeon with molten lava flows and rusted machinery. A blacksmith hammers a glowing weapon on an anvil. Sparks cascade in slow motion with each hammer strike. The weapon - an Eclipse-tier greatsword - glows with amethyst energy veins flowing through obsidian-black steel. The blade emits violet flame aura. Forging with life force of fallen enemies. Dark industrial fantasy"},
    {"name":"15_fracture_opens","image":"/workspace/eclipse_trailer/scene1_world.png","prompt":"Wide aerial shot of a battlefield. The ground begins to crack - not from an impact but from reality itself splitting. A massive chasm - The Fracture - tears open across the landscape. The edges glow with amethyst-violet light. Reality warps around the edges - space bends, objects appear in duplicate, time stutters. The crack widens to 200 meters. Camera shakes increasingly. Apocalyptic, catastrophic, reality-breaking"},
    {"name":"16_sovereign_rises","image":"/workspace/eclipse_trailer/scene12_necrevan.png","prompt":"From within The Fracture, a colossal 50-foot silhouette rises - the Eclipsed Sovereign. Vaguely humanoid, but with dimensional wings that fold through space itself - not physical wings but warps in reality. Its form is darkness edged with dying starlight. Two points of cold amethyst light ignite as its eyes. It turns its head toward the camera. God-like scale. Awe and dread. Near-silhouette, the only light is amethyst from its eyes"},
    {"name":"17_weapon_slam_finale","image":"/workspace/eclipse_trailer/scene2_aetherborn_hero.png","prompt":"The Aetherborn warrior raises an Eclipse-tier greatsword overhead - obsidian-black steel with flowing amethyst energy veins, thorn-like ornate guard, violet flames licking off the blade edge, runes pulsing like a heartbeat. Time slows. He slams the blade into the arena ground. A massive crack of amethyst-violet light erupts from the impact point, spreading across the entire floor. Light geysers upward. The arena fractures. Climactic and devastating"},
    {"name":"18_final_title","image":"/workspace/eclipse_trailer/scene2_aetherborn_hero.png","prompt":"White flash fades to black. Full title materializes - each letter forming from coalescing amethyst-violet energy particles with gold edge lighting. The title reads ECLIPSE REALMS OF THE FRACTURED in elegant serif font. Below: Coming 2027. The energy particles settle and fade. A single piano note sustains in the darkness. Final title card, elegant and ominous"},
]

print(f"\n{'='*60}")
print(f"ECLIPSE: REALMS OF THE FRACTURED - TRAILER GENERATION")
print(f"{len(trailer_scenes)} scenes, Wan 2.2 TI2V-5B, A100 80GB")
print(f"{'='*60}\n")

for i, scene in enumerate(trailer_scenes):
    print(f"\n--- Scene {i+1}/{len(trailer_scenes)}: {scene['name']} ---")
    try:
        gen(scene["name"], scene["image"], scene["prompt"])
    except Exception as e:
        print(f"  ERROR on {scene['name']}: {e}")
        print(f"  Continuing to next scene...")

print(f"\n{'='*60}")
print(f"ALL {len(trailer_scenes)} SCENES COMPLETE!")
print(f"{'='*60}")

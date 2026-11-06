# ECLIPSE: REALMS OF THE FRACTURED
## Original Soundtrack + Full Cinematic Preproduction Bible, V1.0

**Momentum Labs / Illustrators Entertainment LLC**  
**Hermes handoff** | November 2026 | **CONFIDENTIAL: internal creative planning**  

**Scope:** Suno music generation prompts for game environments, exploration, PvE, PvP, cinematics, bosses, cities and character identity; original motif-development methodology; audio middleware and technical handoff; and an exhaustive preproduction checklist for the approximately 120-second 4K cinematic reveal trailer.

**Source hierarchy:** 1) `ECLIPSE_MASTER_BIBLE_V6.pdf` (sound direction on PDF p. 81; social/music context p. 82; major lore/race/zone sections); 2) `ECLIPSE_Hermes_4K_Cinematic_Reveal_Trailer_Prompt_V1.pdf` (12-shot treatment and voiceover); 3) this document (additional production proposals). **Do not silently rewrite locked canon.**

**Important:** This is creative and technical direction, not proof that the music has been generated, that any asset is game-ready, or that the trailer exists. The proposed cues, tempo, motif, assignments, and milestones below are new production recommendations. Secure the applicable commercial permissions and keep provenance records.

---

# 1. What the canon actually specifies

V6's **Sound Design Directive** calls for a base sound of **dark-fantasy orchestral music with African percussion**; combat with drums, string tension and vocal chants; bosses with full orchestra, choir and soloists; aggressive PvP strings and percussion; ambient strings, nature and wind-like instrument colors during exploration; and dissonant dimensional sonorities around the Fracture. It also identifies distinct sound signatures for fire, ice, lightning, shadow, holy, water, nature, soul, temporal, earth, wind and poison (PDF p. 81).

The proposed 120-second reveal cinematic follows 12 shots: Aethel -> Gothic City -> first fracture -> Ashen Forge -> Verdant Deep -> Aetherborn on Floating Isles -> the peoples -> Fractured emergence -> melee combat -> environmental magic -> 5-player dungeon/10-player raid and PvP -> final title. Its closing tagline is **THE LIGHT BROKE. WE DID NOT.** These are sourced from the prior cinematic prompt rather than invented here.

The V6 document internally uses the old nine-peoples lore framing while also proposing a **13-ancestry playable roster** and different class counts. Hermes must create a **Canon/Production Reconciliation Matrix** and obtain a creative-director decision for conflicts before producing race-specific theme libraries; the existing locked lore should not be changed by guesswork.

# 2. Original musical signature: a three-part sonic grammar

**Part A - The Aethel:** Design an original five-note motif that sounds luminous yet incomplete. Produce an audio reference performed by a human in MIDI or recorded instrument as a documented, rights-cleared seed, rather than hoping text generation reproduces a tune. The leitmotif must survive different instrumentation and harmonies.

**Part B - The living peoples:** Use acoustic detail, physically performed rhythm, and distinct regional ensembles. African percussion is an explicit V6 direction, but not a substitute for cultural specificity. If commissioning performers, identify precise instruments, traditions, credits and context. Avoid using a single undifferentiated ‘tribal’ sound for every race.

**Part C - The Fracture:** A texture, not a catchy melody: partially erased overtones, detuned bells, sound arriving before its physical source, sub-bass drones, reversed reflections, breath-like distant voices, and selective silence. Do not bury gameplay cues beneath wall-to-wall dissonance.

**Signature motif design exercise (proposal):** Create 3 fully original five-note candidates from an approved human musician or MIDI sketch. Select the motif by blind listening, mood match and recognizability. Preserve motif ID, notated pitches, intervals, rhythmic cells, key-independent MIDI, BPM reference, stems, ownership chain and notes on transformations. Name it `MOTIF_AETHEL_01` only when approved. Do not copy or prompt a melody from any existing soundtrack.

**Mixing philosophy:** A game-world soundtrack should often be quieter than the player expects: environments need air for dialogue, footsteps and combat telegraphs. Reserve maximum orchestral impact for irreversible narrative beats, major boss phases and the cinematic crest.

# 3. How to use Suno for this project (current November 2026 workflow)

1. Use **Custom / Instrumental** for most environmental and combat cues, with a clear title and a style prompt. For choir without words, specify *wordless/phonetic choir* and manually reject hallucinated lyrics. Voiceover should be recorded and licensed separately.
2. Use the newest available Suno model for committed cues; use a lighter or experimental variant for ideation if useful. Recent official Suno documentation lists v6, v6-wild and v6-mini. For repeatability, log model variant, parameters, creation date, account plan and seed/reference inputs.
3. Generate 4–8 candidates per cue (more for the main theme), audition in context with image or animatic, and **choose**. One prompt is a creative brief, not an assurance of the output.
4. Reuse only owned/authorized motif audio or MIDI. Where permitted, upload the original motif sketch to guide new themes. Do not upload another game's soundtrack, trailer audio, or confidential full lore files without authorization and terms review.
5. Use Studio/editor features, where entitled, to **Replace Section, Extend, trim/crop and export**. Extract stems for production mixing; examine separation artifacts before relying on them. Native multitrack recording or a composer's DAW session provides more precise layering when available.
6. Export **WAV/stems** where the current plan permits, preserve original file and license records, and master in a DAW. For game implementation create musical stems and genuine loops, not one uneditable long MP3.
7. Separate **underscore music** from **sound effects, ambient beds, dialogue and foley**. Generate optional isolated whooshes/impacts with an authorized SFX tool; do not assume a song-generation model produces clean technical game SFX.
8. Preserve music's dynamic range; make trailer-specific mixes where narration, sword contact and wing-reveal VFX can breathe.

**Commercial-rights caution (verified November 2026):** Suno's current official help says Pro/Premier music has commercial-use rights when properly obtained under the plan, including use in games and film. The current September 2026 rules emphasize paid-plan authorized downloads. Free-tier outputs are noncommercial; upgrading later does not automatically retroactively license them. A commercial-use grant does **not** automatically guarantee U.S. copyright protection or exclusive originality. Review your exact plan and Terms before releasing the trailer or shipping the game. Sources: https://help.suno.com/en/articles/9601665 , https://help.suno.com/en/articles/13926081 , https://help.suno.com/en/articles/13925249 , https://help.suno.com/en/articles/2746945 , https://suno.com/terms/ .

**Recommended source metadata:** `TrackID, CueName, CreativeBrief, ApprovedMotif, SunoModel, GenerationPrompt, CandidateURL/ID, GeneratedAt, PlanTierAtGeneration, AuthorizedDownloadDate, FileHash, WAVPath, StemPaths, BPM, Key, TimeSignature, LoopPoints, LicenseProof, HumanEdits, Approver, Status`.

# 4. Main title theme - paste-ready Suno Custom-mode style prompt

**Title:** *The Last Light of the Aethel*  
**Setting:** Custom mode; **Instrumental ON**; target 3-4 minutes (edit to fit).

> Compose an entirely original, emotionally unforgettable dark-fantasy orchestral game main theme for ECLIPSE: REALMS OF THE FRACTURED, a world shaped by a celestial creative force whose light is slowly fracturing. Monumental symphonic dark fantasy, sacred cosmic wonder, tragic grandeur, intricate acoustic ensemble, distinct West African-inspired hand-drum interlocks performed with human groove, layered djembe and dunun alongside deep cinematic bass drums, passionate solo cello, intimate low strings, soaring but restrained violins, warm French horns, dark brass, harp, spectral bowed glass, celestial bell partials, distant wordless mixed choir, subtly detuned resonance, and very restrained analog sub-bass. Start almost silent with a fragile, memorable original five-note melody; introduce suspended harmonies and measured percussion; build an emotionally devastating orchestral statement, then return to solitary unresolved tones. Dynamics matter more than loudness. Original melodic construction, varied arrangement, musical space for character dialogue and foley. Wonder > reverence > dread > grief > defiance > fragile hope. Realistic acoustic depth, controlled low end, organic ensemble detail, beautiful quiet passages, no pop structure, no EDM/trap, no intelligible lyrics, no endless repetitive ostinato, no generic trailer booms, no imitation of recognizable films or games.

**Acceptance test:** Someone who has never heard the theme should be able to hum its signature motif after two listens, and the melody must still work on solo cello with no percussion or choir.

# 5. Suno cue library - individual paste-ready prompts

**How to use:** For each cue below, copy the **Style prompt** into Suno Custom mode, turn Instrumental ON, and select a track title. These are separate generations and sound-design starting points. The stated BPM is **creative guidance**; verify the resulting tempo and edit or regenerate if it is wrong. Full audio loop points, dynamic variants, crossfades and cue lengths must be built in a DAW and tested in Unreal.

## M01 | Gothic City: Breath of the Crowns

**Suggested tempo:** 72–82 BPM  
**Usage:** Day hub; safety, intrigue, life  
**Style prompt (copy into Suno):**

> Original fantasy-city ambient orchestral score with spacious warm strings, solo cello and soft woodwind, distant handbells, gentle plucked strings and subtle hand-drum pulse; architectural grandeur without triumphal fanfare; warm public-life humanity, merchants, rooflines, cathedral shadow, the enormous cracked Aethel fragment overlooking the city. Slow moving harmony and a barely audible allusion to the original Aethel motif. Intimate and majestic, very low rhythmic density, no sudden swells or vocals, no aggressive percussion, seamless-loop-friendly sustained ending.

**Deliverable variants:** Explore daytime / quiet interior / short transition into night.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M02 | Gothic City: The Last Market Lantern

**Suggested tempo:** 56–66 BPM  
**Usage:** Night hub; melancholic safety  
**Style prompt (copy into Suno):**

> A nocturnal variation of the Gothic City theme: low viola and cello, soft distant bells, sparse harp harmonics, thin night air, footsteps between ancient arches and the shadow of a dying celestial light. Retain the core melodic identity at half speed but let harmonies remain unresolved. Warmth in an immense cold city, solitary and understated. No climactic drums, no recognizable lyrics, little high-frequency energy, long reverb tails, loop-friendly last cadence.

**Deliverable variants:** Night hub / tavern exterior / threatened nighttime overlay.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M03 | Ashen Forge: The Heart Below

**Suggested tempo:** 86–98 BPM  
**Usage:** Grimstarr forges; heat and craftsmanship  
**Style prompt (copy into Suno):**

> Grounded industrial dark fantasy underscoring with warm bowed low strings, earthy struck iron, hand-hammer polyrhythms, deep drum skins, rough bass woodwind, distant furnace roar like pitched breath, small human work-song fragments without intelligible words. Create a patient working rhythm that feels old, sacred and physical, and a slight harmonic sinking that suggests cooling forges. Not rock, not sci-fi machinery, no constant huge brass, distinct mallet timbres. Able to loop beneath crafting dialogue.

**Deliverable variants:** Ambient forge / active forge / emergency molten-dimming layer.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M04 | Verdant Deep: Root Memory

**Suggested tempo:** 58–70 BPM  
**Usage:** Living forest; wonder, alertness  
**Style prompt (copy into Suno):**

> Vast, intimate organic fantasy exploration soundtrack: bowed wooden textures, flute-like airy harmonics, delicate harp, string harmonics, hollow resonant wood, gentle percussion with irregular pulse, soft breath-like wordless textures, suspended melody under shimmering moss and ancient forest canopy. Nature feels alive and watchful, not whimsical or cute. Sparse instrumentation, changing microtextures, spacious frequencies leaving room for birds and rain; end designed for a gentle loop.

**Deliverable variants:** Day forest / twilight / predator sensed.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M05 | Sun-Sand Wastes: Cities Beneath the Dust

**Suggested tempo:** 74–88 BPM  
**Usage:** Desert ruins; hidden scholarship  
**Style prompt (copy into Suno):**

> Original desert-ruin dark fantasy underscore built from dry bowed string drones, low wooden winds, plucked string ostinatos, delicate metal resonances, slow rolling frame-drum patterns, brittle heat-scorched harmonics, intermittent deep bass notes. Suggest buried knowledge, wind-carved stones, long distances and an oasis that may not be real. Avoid stock “exotic” scale clichés; emotional motif should be melancholy, proud and enigmatic. Sparse, evolving and loop-friendly.

**Deliverable variants:** Wilderness / ruin interior / sandstorm tension.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M06 | Frozen Crown: The Sky Without Dragons

**Suggested tempo:** 48–62 BPM  
**Usage:** Ice, loss and silence  
**Style prompt (copy into Suno):**

> Emotionally austere orchestral northern mountain ambience: lonely solo cello, glassy string harmonics, thin icy air, restrained low horn, bowed metal, extreme dynamic space and silences. Sound of a sky emptied of ancient dragons, vast glaciers and stone halls. A slow fragment of the Aethel motif emerges through a cold suspended chord then breaks. Minimal pulse, no bright heroic choir, loopable and quiet enough for wind and distant cracking ice.

**Deliverable variants:** Glacier / mountain pass / ancestral memory.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M07 | Floating Isles: Wings of Last Light

**Suggested tempo:** 64–78 BPM  
**Usage:** Aetherborn; celestial beauty and loss  
**Style prompt (copy into Suno):**

> Otherworldly yet tactile celestial fantasy music for suspended islands and a near-extinct luminous people. Delicate solo cello, wordless crystalline female-and-male choir, layered string harmonics, subdued slow hand drums, water-glass resonance, distant brass bloom. An original five-note Aethel motif arrives through inverted harmony, and the sound expands in a breathtaking but intimate swell as violet-indigo energy wings unfold from a fallen celestial warrior. No lyrics, no constant fanfare, dynamic restraint and space for electrical wing crackles.

**Deliverable variants:** Exploration / wing awakening / grief reprise.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M08 | Eclipse Shadow: Where Light Refuses

**Suggested tempo:** 40–56 BPM  
**Usage:** The Fracture; dissonant not evil  
**Style prompt (copy into Suno):**

> Sparse psychological cosmic-darkness score for an expanding absence of light, not an evil creature. Detuned bass clarinet, bowed cymbal, unstable string clusters, distant breath textures, filtered heartbeats, silence interrupting low drones, a harmonic resonance that seems to disappear before resolving. Small acoustic details and uneasy stereo space, no typical horror stingers, no constant loud drones, no slasher music, no words. Minimal loop that can be layered with gameplay threat pulses.

**Deliverable variants:** Ambient void / mimic nearby / imminent breach.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M09 | Sunken Realms: The Water Remembers

**Suggested tempo:** 60–72 BPM  
**Usage:** Nereyth aquatic setting (proposed extension)  
**Style prompt (copy into Suno):**

> Deep underwater dark-fantasy ambience using resonant low cello, glassy mallet tones, plucked harp, airy choir without words and soft watery filtered percussion. Slow undulating phrasing and harmonic currents should suggest a civilization below tidal light and coral structures without relying on cliché tropical rhythms. The Aethel motif appears as distant fragments reflected through water. Rich low-mid depth, no sudden drums, subtle looping finale.

**Deliverable variants:** Surface shore / underwater / flooded sanctum.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M10 | Myceliar: The Choir of Spores

**Suggested tempo:** 52–68 BPM  
**Usage:** Symbiotic forest culture (proposed extension)  
**Style prompt (copy into Suno):**

> Strange, beautiful living-forest underscore based on breathy woodwinds, bowed wood, muted marimba-like organic resonance, granular falling droplets and fine string tremolo. Tiny interlocking rhythmic cells spread and answer one another as fungal networks grow. The music should feel sentient, patient and rooted rather than monstrous; no generic sinister swamp theme and no explosive climax. Very spare instrumental layers suitable for seamless looping.

**Deliverable variants:** Forest colony / growth ritual / ecological danger.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M11 | The Peoples: Thirteen Paths

**Suggested tempo:** 78–92 BPM  
**Usage:** Shared unity; ancestry montage  
**Style prompt (copy into Suno):**

> Original orchestral suite representing distinct peoples living under one fractured sky. Start with solo hand drum and cello; add small, contrasting instrument families over time, weaving them into one coherent harmonic language without assigning stereotypes or pretending one instrument represents an ethnicity. Gentle wordless voices unite only near the end. Emotional communal strength, not propaganda or a march; use a clear melodic link to the Aethel motif.

**Deliverable variants:** Character selection / ancestry introduction / alliance scene.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M12 | Skirmish: Steel Against the Fractured

**Suggested tempo:** 118–132 BPM  
**Usage:** Small combat; precise telegraphs  
**Style prompt (copy into Suno):**

> Tactical dark fantasy combat underscore with taut staccato low strings, deep hand percussion and irregular accents, sparse brass responses, a pulsing distorted bowed bass tone, and brief spaces for weapon impacts and enemy calls. Strong rhythmic drive but never wall-to-wall sound; clear 4-bar phrasing for adaptive loops. Unpredictable but readable, skill-based melee intensity, no rock guitars, no dance beat, no endless choir or fake trailer braams.

**Deliverable variants:** Anticipation / active skirmish / elite combat / clear ending.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M13 | Weeping Foundry: The Furnace Has a Memory

**Suggested tempo:** 100–112 BPM  
**Usage:** 5-player dungeon; corrupted forge  
**Style prompt (copy into Suno):**

> Dark forge dungeon music where manual hammer rhythms slowly lose synchronization, with grinding metal percussion, cello ostinato, contrabass, muted low brass, restrained choir fragments, volcanic rumble and ghostlike harmonics. Dangerous but deeply crafted; suggest the sacred workshops collapsing into the Fracture. Build sparse and modular, with distinct encounter pulse and boss escalation layer; room for five players’ combat cues.

**Deliverable variants:** Dungeon exploration / trash packs / miniboss / boss.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M14 | Cathedral of Unmade Hours: The Ninth Bell

**Suggested tempo:** 94–108 BPM  
**Usage:** 10-player raid; altered time  
**Style prompt (copy into Suno):**

> Enormous ritual dark-fantasy raid soundtrack: massive cathedral reverberation, tolling bells that skip expected beats, deep orchestra, string tremolo, precise hand percussion, male-and-female wordless choir singing dissonant clusters, fragments of the Aethel theme stretched into unnaturally long notes. Three phases: solemn entry, temporal instability and relentless final judgment. Spectacular scale but distinct attack cue openings; no unintelligible choir syllables treated as invented lore language.

**Deliverable variants:** Raid intro / phase A / phase B / final phase / victory.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M15 | Aetherborn: What Fell From the Sky

**Suggested tempo:** 68–82 BPM  
**Usage:** Hero motif; power and mortality  
**Style prompt (copy into Suno):**

> Character-driven elegy for a muscular dark-skinned Aetherborn whose luminous skin fractures and celestial lightning wings are the last light of a fading creator. Begin with a single expressive bowed melody and dry heartbeat percussion, then rising harmonic overtones and a restrained choir. A major chord tries to emerge from an unresolved minor texture, becoming huge for one brief wing-awakening climax before returning to fragile solitary strings. Never constant bombast; brave, wounded, magnificent.

**Deliverable variants:** Private sorrow / wing transformation / final hero shot.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M16 | Arena: No Crown Without Skill

**Suggested tempo:** 136–148 BPM  
**Usage:** 1v1, 2v2, 3v3 and 5v5 ranked  
**Style prompt (copy into Suno):**

> Competitive dark fantasy PvP music with sharp rhythmic precision, urgent low string ostinatos, interlocking deep hand-drum patterns, dry brass jabs and subtle atmospheric metallic resonance. Aggressive yet focused: no overwhelming choir, no distracting melody during clutch CC timings, no broad epic explosions every 10 seconds. Athletic, tense and suited to seamless 8-bar looping; create intro, sustained duel and last-stand layers.

**Deliverable variants:** Match ready / active fight / final seconds / win and loss stingers.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M17 | Battleground: Standards in Ash

**Suggested tempo:** 122–136 BPM  
**Usage:** 10v10 and objective battles  
**Style prompt (copy into Suno):**

> Large-scale objective-driven dark fantasy battleground score: powerful but restrained low strings, disciplined interlocking hand drums and orchestral percussion, distant brass horns, fragmented battle motif, occasional wordless choral lift. Conveys teamwork, flanking, flags, siege objectives and a contested capture point without turning into generic war trailer music. Rhythmic sections should map to neutral capture, advantage, overtime and victory states.

**Deliverable variants:** Neutral / push / comeback / overtime / score.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M18 | Fracture Expedition: The Night Takes Shape

**Suggested tempo:** 108–122 BPM  
**Usage:** Timed cooperative expedition  
**Style prompt (copy into Suno):**

> Dark-fantasy survival expedition cue with rhythmic hand percussion, ticking processed plucks, anxious string ostinato and gradually dissonant resonances. Early sections feel curious and opportunistic; later layers reveal unstable reality and approaching danger. Create clearly separable movement layers for safe gathering, closing threat, elite pursuit, final boss and extraction. Moderate dynamic range, avoid unbroken climax.

**Deliverable variants:** Discovery / escalation / close threat / extraction.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M19 | Eclipse Avatar: The Unanswered Choice

**Suggested tempo:** 70 rising to 116 BPM  
**Usage:** Final campaign confrontation; spoiler-sensitive  
**Style prompt (copy into Suno):**

> Grand tragic celestial boss score centered on the original Aethel motif now stretched and harmonically fractured. Distant human cello, cathedral-sized wordless choir, grave low brass, irregular ritual percussion, surging strings, sudden pauses, huge deep-space resonance. Emotion is grief and impossible responsibility, not simply good versus evil. Three movements: revelation without explicit answer, irreversible combat and suspended moral choice. Never disclose the hidden true nature of the Avatar through lyrics or track titles in public builds.

**Deliverable variants:** Secret production cue; phase transitions, spoiler-safe.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M20 | Character Creator: Who Will You Become?

**Suggested tempo:** 55–68 BPM  
**Usage:** Class and ancestry creation interface  
**Style prompt (copy into Suno):**

> Soft, aspirational dark fantasy music for creating a hero: gentle harp and solo viola, intimate low cello, a little choir breath, subtle bowed-glass celestial ring, and a simple fragment of the Aethel melody. Emotional wonder, possibilities, vulnerability and a sense of a world waiting. No loud climax, no lyrics, static spatial image appropriate for a menu, carefully constructed loop ending.

**Deliverable variants:** Menu loop / race selection preview / confirmation stinger.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M21 | Guild Hall: The Fires We Keep

**Suggested tempo:** 66–76 BPM  
**Usage:** Housing, social, crafting respite  
**Style prompt (copy into Suno):**

> Warm acoustic fantasy ambience with gentle hand percussion, woodwind, cello, plucked strings and occasional low communal humming without lyrics. A sense of believable people sharing a safe room while the dangerous world continues outside; small human-scale rhythms and laughter-like instrumental gestures, not tavern novelty music. Calm, friendly, minimal; no solo vocals. Seamless long-form loop.

**Deliverable variants:** Guild hall / quiet housing / festival adaptation.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M22 | Forge: Name the Blade

**Suggested tempo:** 84–96 BPM  
**Usage:** Crafting and legendary acquisition  
**Style prompt (copy into Suno):**

> Ceremonial crafting underscore with heavy but soft struck iron, heartbeat percussion, low viola, short brass swells and a sparse, original three-note motif that builds toward the naming strike. Close-up tactile instrument detail, carefully measured hammer rhythms, no music overpowering blade impact Foley. Structure into waiting loop, action sequence and short forge completion sting; controlled low end.

**Deliverable variants:** Craft loop / successful forge / rare forge variation.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M23 | The Last Breath: Defeat and Grace

**Suggested tempo:** 46–60 BPM  
**Usage:** Death, checkpoint and recovery  
**Style prompt (copy into Suno):**

> A profoundly restrained dark fantasy lament: one bowed cello line, distant piano-like resonances, an unresolved cluster that opens into one slender warm note. Very short and emotionally precise. Leave generous silence after the death beat. No cheesy sadness or excessively long choir; usable as an 8–15 second sting with a companion ambient recovery loop.

**Deliverable variants:** Death stinger / rest point / regained control.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

## M24 | Trailer: The Light Broke. We Did Not.

**Suggested tempo:** Variable  
**Usage:** Approximately 120-second trailer cue  
**Style prompt (copy into Suno):**

> Original through-composed dark fantasy cinematic score synchronized to a precise 120-second twelve-shot storyboard: 0:00–0:30 sacred celestial awe and an unheard fracture; 0:30–0:51 forge and forest grief; 0:51–1:14 celestial wing awakening and disparate peoples; 1:14–1:25 near-silence as a void mimic emerges; 1:25–1:47 precision combat and environment-changing magic, controlled percussion crescendo; 1:47–1:55 five-hero dungeon and ten-hero raid scale; 1:55–2:00 almost complete silence and final five-note Aethel motif with logo impact. Distinct acoustic low strings, original hand-drum polyrhythms, expressive choir without words, wounded horns, measured silence, realistic dynamics, no pop beat or generic rise-only trailer structure. Reserve frequencies for voiceover, foley and impact sync. Create as sections and finish to picture in editor/DAW; precise synchronization is an editorial task, not guaranteed by text.

**Deliverable variants:** Trailer-specific cue; section stems and voiceover-safe gaps.  
**Production review:** Preserve identity with the Aethel theme where appropriate; check audible motifs, clean source, musical dynamics, no unwanted words, no stolen/overly familiar phrases, sufficient SFX and dialogue space, looping feasibility, and proof of commercial authorization.

# 6. 120-second trailer music spotting and cue sheet

This table follows the exact 12-shot timing from the already-approved V1 trailer concept. The timestamps are **editorial targets**, not preexisting completed recordings.

| Shot | Time | Picture | Musical intention | Sound-design/voice priority |
|---|---|---|---|---|
| 01 | 00:00-00:09 | Aethel in silent sky | Almost silence, single spectral harmonic | Opening line, airy celestial resonance |
| 02 | 00:09-00:20 | Gothic City and people | Warm low strings; first five-note seed | Ambient market, bells, footsteps, voice |
| 03 | 00:20-00:30 | Celestial fracture | Motif dislocates; cut to low suspended note | Cracking light, lone bell, VO breath |
| 04 | 00:30-00:40 | Ashen Forge | Iron percussion in 3 uneven hits | Hammer sparks, heat, cooling river, VO |
| 05 | 00:40-00:51 | Verdant Deep / glimpses | Forest textures; delicate cello lament | Leaves, rain, animals, VO |
| 06 | 00:51-01:04 | Aetherborn and wing reveal | Motif large but only brief climax | Skin-energy charge, wings, wind, VO |
| 07 | 01:04-01:14 | Many peoples | Rhythm becomes shared pulse | One human detail per cut, VO |
| 08 | 01:14-01:25 | Fractured mimic | Almost all music ducks into dread silence | Organic void shift, cloth and breath, VO |
| 09 | 01:25-01:37 | Shield and sword battle | Percussion and short string ostinato enter | Hit transients, guard counters, footsteps |
| 10 | 01:37-01:47 | Frost/fire/lightning reactions | Music swells with staggered attacks | Frozen arch, vapor, lightning on water |
| 11 | 01:47-01:55 | 5-player dungeon, 10-player raid, PvP | Biggest 8-second orchestral statement | Party call, spell punctuation, cinematic edit |
| 12 | 01:55-02:00 | Final hero and title | Sudden silence; five-note identity/title impact | Whisper ‘What will you become?’ then logo |

**Voiceover rule:** Test intelligibility before mastering the score. In music stems, preserve a dialogue-safe spectral window rather than merely turning down the finished mix. Design the final hit AFTER the spoken tag rather than colliding with it.

# 7. In-game adaptive music specification

**Core player states:** `SAFE_HUB`, `WILDERNESS_CALM`, `WILDERNESS_UNEASE`, `ENEMY_DETECTED`, `SKIRMISH`, `ELITE`, `BOSS_PHASE_1`, `BOSS_PHASE_2`, `BOSS_FINAL`, `VICTORY`, `DEFEAT`, `EXTRACTION`, `PVP_READY`, `PVP_ACTIVE`, `PVP_OVERTIME`, `SOCIAL`. Each region should map those states to compatible music sources.

**Layers per cue:** `BASE_AMBIENCE`, `HARMONY`, `MELODY`, `RHYTHM`, `DANGER`, `CHOIR`, `IMPACT_STING`, `VICTORY_STING`. These may be genuine compositions recorded as stems or acceptable stem extractions; each layer must be time aligned with a shared reference clock, key, BPM, length and zero start.

**Game event examples:** entering the Verdant Deep fades a calm bed; awareness rises when the Kazari hears a Fractured; combat adds percussion only on a bar boundary; enemy death removes combat layer after the next phrase; a world boss forces an authored transition into its own motif. PvP uses neutral audible states regardless of item rarity; avoid masking enemy cast and dodge cues.

**Implementation targets (engineering proposal):** Use Unreal 5.8 MetaSounds for controllable sound sources, **Quartz** for quantized music playback, Audio Modulation for ducking/intensity, and sound classes/submixes for menu, dialogue, effects and music routing. Configure `BossIntensity`, `DangerLevel`, `HealthCritical`, `RegionID`, `PvpState`, `ExpeditionTimeRemaining` as parameter inputs. Evaluate whether a separate interactive-music middleware package is justified; never assume it is needed.

**Technical audio deliverables:** 48 kHz / 24-bit WAV assets where feasible; streaming/compressed runtime versions made from masters; synced stems with precise start offset; original `BPM`, `Key`, `TimeSignature`, `LoopStartSample`, `LoopEndSample`, `BarsPerLoop`, `TailHandling`, `TransitionBarCount`, `CueID`, `RegionID`, `GameState`, `ThreatLevel`, `StingerName`, `SourceRights`. Test headphones, stereo TV speakers and console-like playback; check loudness, mono fold-down and accessibility sliders.

**Mandatory in-engine QA:** Avoid phase drift, clicks at loop boundaries, overlong crossfades, redundant full-mix layering, combat music too loud for spell telegraphs, stacked elite cues, queue-to-combat latency, repeated stingers, restart-on-camera cuts and audio breakup during scene transitions.

# 8. Comprehensive cinematic preproduction, before final rendering

**Scope:** These are the stages Hermes must complete before full-resolution final production. Some preproduction stages involve a small **proof shot, look-dev scene, temporary voice, sample music** or 5–8 second test render; those are deliberately permitted as low-cost validation, not full production.

| # | Preproduction stage | Required actions | Gate deliverable |
|---:|---|---|---|
| 01 | **Project authorization and deliverable contract** | Confirm owner, audience, usage, confidentiality, trailer objective, signoff authority, rights to supplied images and V6 bible. | Project charter; confidentiality and permissions log. |
| 02 | **Source intake and version audit** | Hash/index V6, V1 trailer, higher-resolution art, references, prior revisions, and all uploaded character materials. | Reference inventory with exact source/version. |
| 03 | **Canon consistency audit** | Crosswalk 9 lore peoples vs 13 gameplay ancestries; 17 classes vs earlier 8/12 references; four- vs five-player and eight- vs ten-player old group descriptions. Escalate conflicts without silent retcons. | Canon reconciliation matrix and proposed decision list. |
| 04 | **Creative objective, audience, and marketing positioning** | Decide whether reveal is narrative teaser, investment pitch, recruitment asset or public Coming Soon site promo. Distinguish mood from gameplay claims. | One-page creative and marketing brief. |
| 05 | **Reference decomposition and IP boundary** | Study reference trailer for pacing, composition, emotional arc and technical polish, not shot copies, exact characters, copyrighted music or dialogue. | Reference board annotated with ‘principle to learn’ and ‘do not copy’. |
| 06 | **Logline, theme, and emotional curve** | Lock the statement: THE LIGHT BROKE. WE DID NOT. Chart wonder->loss->dread->defiance->wonder. Do not disclose Avatar secret. | Approved logline and emotional beat chart. |
| 07 | **Treatment and narrative synopsis** | Summarize entire short-film logic, beginning, revelation, transformation, showdown and unanswered choice. | 1-3 page treatment. |
| 08 | **Timed screenplay and voiceover script** | Write exact voice/dialogue beats with target time, pausing, SFX priority, alternate copy and legal review. | Timecoded script including VO text and subtitles. |
| 09 | **12-beat board and storytelling logic** | Map scene duration, intent and emotional response onto the established 12-shot plan. Ensure 2-minute total. | Approved beat sheet and beat ownership. |
| 10 | **Shot list and unique IDs** | Make one row per shot, including shots 01–12 and inserts; camera/lens/movement, environment, subjects, action, lighting, resolution, transitions and target duration. | Master SHOT_LIST.csv with permanent ShotIDs. |
| 11 | **Camera plan and cinematic grammar** | Define visual vocabulary, focal lengths, shallow vs deep focus, motion speeds, frame rates, screen direction, 180-degree continuity, safe title composition. | Camera/lens rulebook and camera move thumbnail frames. |
| 12 | **Storyboard pass A: thumbnails** | Draw all 12 frames plus sub-beats and action continuity; show silhouette and visual information hierarchy. | Storyboard PDF and JPEG boards. |
| 13 | **Storyboard pass B: colored keyframes** | One approved lighting/color keyframe per scene and inserts for hero reveal, wing energy, steel impact and title endcard. | Color script and high-fidelity key art reference. |
| 14 | **Scene-by-scene asset breakdown** | List every character, costume, weapon, prop, environmental mesh, set dressing, foliage, particle simulation, crowd plate and audio source. | Shot-to-asset matrix with build/buy/license flags. |
| 15 | **Art-direction lock and negative examples** | Approve original realistic/weathered dark fantasy vs cartoon glow; reference skin/hair, realistic scale, ancient damage, strong silhouette, cast-light VFX. | Look bible, color palette and rejection gallery. |
| 16 | **Aetherborn hero model and continuity lock** | Approve face, deep-brown skin shader, natural locs, proportion, blacksteel armor, fracture markings, violet-indigo energy wings and matching patterns. | Character turnaround, neutral-light skin test, texture/rig/wing bible. |
| 17 | **Supporting race/character casting sheets** | Select which of 13 ancestries appear in shots, with distinctive morphology, gender range, melanin, hair styles and cultural material language. | Casting/costume continuity charts, hero/NPC lookup sheets. |
| 18 | **Costume, armor and weapon design approvals** | Detailed front/back close-ups, wear, engraved lore symbols, attachment geometry, weapon grips, safe animation clearance and rarity look. | Armory turnarounds and shader/texture specifications. |
| 19 | **Environment geography and continuity** | Locate city, forge, forest, floating bridge, shadow boundary, Foundry and Cathedral consistent with V6; map sightlines and travel. | Environment layout plans, visual landmarks, matte-painting strategy. |
| 20 | **Visual-effects design and simulation plan** | Define Aethel fracture, wing topology, magic light transport, molten rock, ice vapor, lightning water conduction, Fractured face morph and physics. | VFX bid sheet, simulation dependency list, performance/risk grade. |
| 21 | **Creature design and combat choreography** | Plan Fractured mimic anatomy and deformation; sword/shield hits, dodge timings, guard counter, reaction forces, magic targets, environmental damage. | Stunt/combat diagrams and collision/rig requirements. |
| 22 | **Motion design, rigging and cloth/hair tests** | Plan rigs, face shapes, lip sync if any, long loc strand groups, armor clipping, cloth physics, hands/props and creature bones. | Rig test report and clip/penetration failure checklist. |
| 23 | **Lighting and color pipeline** | Approve scene exposure, physical units where relevant, highlights on dark skin, shadow readability, color management, HDR/SDR approach, shot-to-shot consistency. | Color script, LUT/OCIO/ACES plan and look-dev reference renders. |
| 24 | **Editorial rhythm and transition design** | Place camera cuts, action eyelines, flash frames, hard silences, VO holds, transitions and teaser end-card. | Editorial decision list and locked transition notes. |
| 25 | **Animatic / previs** | Assemble storyboard panels with timed VO temp and a rights-cleared scratch cue. Check plot comprehension without polished CG. | Full 120s animatic exported at low resolution. |
| 26 | **Music spotting session** | Mark precise entrances/exits for motif, percussion, silence, wing crescendo, fight drive and title. Assign soundtrack cue IDs. | Music spotting map and cue sheet keyed to shot IDs. |
| 27 | **Original score and leitmotif demo** | Produce three original Aethel motif candidates. Select, notate, license, and test transformations for menu, city, boss and trailer. | Approved motif WAV/MIDI and approved scratch arrangement. |
| 28 | **Sound-effect, foley and ambience plan** | Detail diegetic city bells, distant crowds, forging, insects, wind, wing electricity, metal impacts, damaged stone, broken time and silence. | SFX event matrix with channel priorities and sound rights. |
| 29 | **Voice casting/performance brief** | Choose trained/authorized voice talent or appropriately licensed synthetic voice, pacing, performance and recording specs. | Casting/consent records, VO guide and pickup list. |
| 30 | **Audio and picture sync planning** | Set timecode, framerate, ADR/slate convention, music stem alignment, cue handoff, VO priority and preview mix targets. | Audio session template and sound post workflow. |
| 31 | **Technology capability audit** | Identify actual Unreal/Blender/renderer/video tools installed and authorized, versions, plugins, GPU memory, disk space, render farm and upload restrictions. | Capability matrix: verified / available with cost / unavailable. |
| 32 | **Resolution, frame and color deliverables** | Lock 3840x2160 24fps 16:9, 2.39:1 composition-safe area if desired, delivery codec, master, captions, 60/30/15s cuts and truthful upscaling disclosure. | Technical delivery spec. |
| 33 | **Project topology, version control and backups** | Create naming conventions, folder schema, shot IDs, checkpoints, source control, backup policy, asset provenance and access boundaries. | Approved repository and directory structure. |
| 34 | **Shot complexity, resource and schedule estimates** | Estimate asset creation, animation, sims, retake allowance, render minutes/frame, VRAM, storage, manual quality review and human dependencies. | Shot bid sheet, schedule with critical path, risk register. |
| 35 | **Preflight and tool-to-tool interchange** | Test FBX/USD/glTF/Alembic materials as needed, skeletal animation, hair caches, texture sets, alpha, VFX comps, audio stem import and export. | Approved interchange recipes and tool compatibility checks. |
| 36 | **First quality proof shot** | Build only 5–8 seconds of wing reveal at representative native quality. Check face consistency, skin, loc physics, armor, fracture VFX, lighting, temporal stability and sound. | High-quality proof clip and scored QC report. |
| 37 | **Render and post-production tests** | Test small outputs from whole pipeline, sample render speed, offline/path tracer vs real-time labeling, grading, edit conform, dialogue, titles and video encoding. | Render settings, cost/performance profile and QC plan. |
| 38 | **Legal, privacy, and distribution review** | Verify every asset and musical source. Clear voice rights, image/reference use, Suno account tier at generation and download, project confidentiality, publicly claimable gameplay and platform labels. | Rights ledger, terms notes and marketing disclosure approval. |
| 39 | **Final creative and production approvals** | Review story, shots, casting, music, animatic, hero test, costs, schedule, asset sources and technical format. Freeze versions. | Signed GO/NO-GO checklist and locked preproduction package. |


### Four mandatory preproduction approval gates

**Gate A: Canon + treatment.** The lore, 12 story beats, target audience, trailer claims and script have no unresolved critical contradictions.

**Gate B: Visual + sound identity.** The Aetherborn character is consistent, at least one frame per scene is approved, the original Aethel motif has a fixed reference and music spotting exists.

**Gate C: Animatic + feasibility.** The entire 120 seconds plays as a coherent low-cost animatic, every shot has its assets and dependencies, and the rights/tool/resource ledger is viable.

**Gate D: Quality proof.** A representative 5-8s Aetherborn wing-reveal test passes face, hair, animation, physically credible VFX, composition, sound, time consistency, original-art and actual output-resolution checks. **Do not begin all expensive final shots if Gate D fails.**

# 9. Required shot-list record for Hermes

Each 12-shot row must have these fields:

```
ShotID,SeqID,StartTime,EndTime,Duration,StoryPurpose,Location,TimeOfDay,
FeaturedAncestries,FeaturedClasses,LeadCharacterID,HeroAssetVersion,
EnvironmentAssetID,PropsAndWeapons,ConceptBoardID,CameraID,LensMM,
CameraMove,FrameComposition,ScreenDirection,LightingRigID,ColorKeyID,
AnimationRequirements,RigRequirements,HairClothRequirements,VFXEvents,
SimulationRequirements,FootContactChecks,VOStart,VOEnd,VOText,
MusicCueID,MusicIn,MusicOut,MusicIntensity,SFXIDs,FoleyIDs,
DifficultyGrade,RenderMethod,SourceResolution,TargetResolution,FrameRate,
LookDevStatus,StoryboardStatus,PrevisStatus,LicenseStatus,
QAOwner,ApprovalOwner,Revision,Dependencies,EstimatedRenderMinutes,
LastReview,Notes
```

**First pass** should include the existing 12 shots and any necessary detail inserts, not a completely rewritten trailer. Assign deterministic IDs, for example `SQ01_SH010`, and only change plans through a documented change request.

# 10. Example preproduction directory tree

```
ECLIPSE_TRAILER_4K/
  00_ADMIN/CHARTER,RIGHTS,DECISIONS,APPROVALS,RISK
  01_SOURCES/V6_BIBLE,TRAILER_V1,APPROVED_IMAGES,LINKS
  02_SCRIPT/TREATMENT,BEAT_SHEET,VOICEOVER,CANON_MATRIX
  03_VISUAL_DESIGN/MOODBOARDS,COLOR_SCRIPT,CHARACTERS,WEAPONS,LOCATIONS
  04_BOARDS/STORYBOARD_V1,SHOT_LIST,SHOT_BREAKDOWNS
  05_PREVIS/ANIMATIC,TECHVIS,CAMERA_TESTS
  06_ASSETS/MODELS,TEXTURES,RIGS,ANIMATIONS,VFX,SIM_CACHES
  07_AUDIO/MOTIF,SONG_CANDIDATES,APPROVED_WAV,STEMS,VOICEOVER,SFX
  08_SHOTS/SQ01_SH010,SQ01_SH020,...
  09_EDITORIAL/SEQUENCER,EDIT_PROJECT,TEMP_MIX
  10_RENDER_TESTS/LOOKDEV,QUALITY_PROOF,TIMING,PERFORMANCE
  11_FINAL_DELIVERABLES/4K_MASTER,WEB_MP4,CUTDOWNS,CAPTIONS,POSTERS
  12_QA/SHOT_AUDITS,TECH_QA,CANON_QA,RIGHTS_QA
```

# 11. Master Hermes directive: full preproduction FIRST

> **ROLE:** Act as the Executive Producer, Cinematic Director, Technical Director, Art Director, Music Supervisor, Sound Designer, Shot Production Coordinator and QA Lead for the ECLIPSE: REALMS OF THE FRACTURED cinematic and game soundtrack.
>
> **SOURCE OF TRUTH:** Read, index and respect `ECLIPSE_MASTER_BIBLE_V6.pdf` and the 12-shot `ECLIPSE_Hermes_4K_Cinematic_Reveal_Trailer_Prompt_V1.pdf`. The Aetherborn is muscular, a dark-skinned Black male with authentic long locs and violet-indigo luminous fracture patterns, whose matching wings are celestial electricity, never feathers. Preserve originality, believable materials, and race/class diversity. The mystery behind the Aethel fracture remains unresolved. Do not spoil the Eclipse Avatar revelation.
>
> **MANDATE:** Build an original soundtrack with a documented five-note Aethel leitmotif, authentic acoustic and orchestral identity, culturally specific well-researched percussion, dark Fracture soundscapes, and separate cue families for 13 proposed ancestries, zone exploration, 5-person dungeons, 10-person raids, ranked 1v1/2v2/3v3/5v5 arenas, battlegrounds, boss phases, character creation, crafting, housing and the 120s trailer. Use Suno only if entitled and authorized. Track rights and commercial download status for each output. Never use another game's recordings, music or melodies as source input. Do not treat raw song outputs as adaptive game-ready layers.
>
> **STOP BEFORE FINAL PRODUCTION:** Complete stages 01–39 of this packet, with actual files for the charter, lore audit, approved 12-shot breakdown, timed screenplay, storyboards, look-dev, art direction, character continuity, scene assets, camera designs, combat choreography, music cue map, voice/SFX plans, animatic, audio and 4K pipeline specification, budget, rights ledger, risk register, QA checklist and 5–8 second proof shot. Report missing tools and permissions candidly.
>
> **ALLOWED PREPRO TEST:** Produce one polished 5–8s Aetherborn wing-reveal shot and short instrumental demos as quality-validation assets. These are not a finished trailer.
>
> **RENDER GATE:** Full-shot rendering is prohibited until approved Gate A–D deliverables are assembled and a human creative director has reviewed the final package. If approvals cannot be obtained, present the complete review packet and mark it `AWAITING APPROVAL`; do not claim completion.
>
> **TECH TARGET:** Actual 3840x2160, 24fps, cinematic 120s, 16:9, quality-master plus web export, captions, original music/stems, shot-by-shot QA and accurate disclosure of any upscaled shots or concept-only footage. Do not label cinematic images as playable footage or claim PC/console certification.
>
> **INTEGRATION:** For game audio, evaluate Unreal 5.8 MetaSounds + Quartz + Audio Modulation; demonstrate at least one calm-to-combat-to-boss transition with no clicks or musical drift in a playable prototype before promising adaptive music.
>
> **FIRST RESPONSE:** Report (a) exact discovered files and compatible tools, (b) source and canonical conflicts, (c) preproduction task owners, (d) the 12-shot planning table, (e) 3 Aethel motif directions, (f) list of essential missing high-res references, and (g) what can be verified now. Begin creating the approved preproduction artifacts without deleting earlier project work.

# 12. Quality scorecard and rejection conditions

| Category | Requirement | Failure conditions |
|---|---|---|
| Narrative | Emotionally legible in a mute animatic | Incomprehensible world stakes or spoiler |
| Canon | V6-aligned people, locations, Aetherborn traits | Feather wings, wrong lore, accidental final twist |
| Character continuity | Same facial geometry, skin, locs and armor | Face drift, hair swaps, inconsistent markings |
| Visual fidelity | Detailed original materials and believable lighting | Cartoon glare, oversized plastic weapons, muddy faces |
| Music originality | Identifiable original motif, coherent arrangements | Too similar to an existing recording, generic wall-to-wall rises |
| Sound balance | Dialogue, Foley, spell/weapon SFX audible | Music masks sword contacts or VO |
| Runtime and format | 120s target, 4K 24fps spec documented | False ‘native 4K’ claim |
| Production handoff | Versioned shot IDs and live tracker | Mismatched frame versions; unrecoverable assets |
| Rights | Provable ownership/licenses/plan authorization | Free-tier track used in commercial trailer; missing provenance |
| Audio implementation | Loops and transitions without clicks | Drifting stems, discontinuities, constant restarts |

# 13. Source register and verification

**Primary internal documents:** `ECLIPSE_MASTER_BIBLE_V6.pdf`, pp. 3-19 for lore and ancestries, p. 81 for sound design, pp. 82-84 for social context and settings; `ECLIPSE_Hermes_4K_Cinematic_Reveal_Trailer_Prompt_V1.pdf`, 12-shot breakdown and VO.

**Suno official (reviewed November 2026):**
- Commercial rights: https://help.suno.com/en/articles/9601665
- Terms effective Sept 3, 2026: https://suno.com/terms/
- Free vs commercial: https://help.suno.com/en/articles/9601601
- Downloads / WAV: https://help.suno.com/en/articles/13926081
- Stem separation: https://help.suno.com/en/articles/13925185
- Studio export: https://help.suno.com/en/articles/13925249
- New v6 models: https://help.suno.com/en/articles/13924801
- Copyright uncertainty: https://help.suno.com/en/articles/2746945
- Instrumental mode: https://help.suno.com/en/articles/3726721

**Epic Unreal official:**
- MetaSounds procedural music: https://dev.epicgames.com/documentation/unreal-engine/creating-procedural-music-with-metasounds
- Quartz timing: https://dev.epicgames.com/documentation/unreal-engine/overview-of-quartz-in-unreal-engine
- Audio Modulation: https://dev.epicgames.com/documentation/en-us/unreal-engine/audio-modulation-overview-in-unreal-engine
- Unreal 5.8 Sequencer audio changes: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes

**Confidentiality reminder:** Suno's current Terms include broad rights relating to content submissions. Before uploading the complete secret game bible, unpublished cinematic models or proprietary voice recordings to an outside service, have the project owner review relevant terms and approve the upload. For soundtrack generation, standalone musical descriptions should usually be sufficient.

---

**END OF V1 PRODUCTION PACKET.**  
**Status:** Production planning and ready-to-use prompt library. No audio, final trailer, or game runtime is represented as complete.

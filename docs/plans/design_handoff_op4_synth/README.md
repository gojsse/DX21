# Handoff: OP4 — 4-Operator FM Synth Plugin UI

## Overview
OP4 is a software instrument (VST/AU plugin + standalone app) that recreates and modernizes Yamaha's 1985–87 4-operator FM synths (DX21 / TX81Z), an RX21-style drum engine, a vintage effects section, a sysex patch librarian, and a themed patch-bank generator.

The single design goal: **make deep 4-op FM programming legible on one screen** — the thing the original hardware (a 2-line LCD + membrane buttons) failed at. Everything for a patch lives on one page with no paging.

This package covers **the primary application UI**: six views behind a top nav (Voice, Library, Drums, FX, Generator, MIDI/Settings), plus a runtime-switchable **theme token system** (three themes).

## About the Design Files
The files in this bundle are **design references authored in HTML** — prototypes showing the intended look and behavior. They are **not production code to copy directly**.

They are "Design Components" (`.dc.html`): a lightweight template + a `Component` logic class, assembled by the included `support.js` runtime. **Do not port `support.js` or the `.dc.html` format into production.** Read them as a precise spec.

The task is to **recreate these designs in the target codebase's environment** using its established patterns. For an audio plugin this is almost certainly a C++ framework — **JUCE** (with its `Component`/`LookAndFeel` system) or **iPlug2** — where each "view" is a component/panel and the theme token set becomes a `LookAndFeel` palette. If a web-based UI is chosen instead (e.g. a WebView-hosted React UI, common in modern plugins), the token system maps directly to CSS custom properties. Pick the environment that matches the DSP/host layer already in place.

Because the whole design is resizable vector UI, implement it with **relative/vector layout** (JUCE `FlexBox`/`Grid`, or CSS flin/grid) — never fixed bitmaps. Plugin windows are scaled constantly.

## Fidelity
**High-fidelity.** Final colors, typography, spacing, and component treatments. Recreate pixel-accurately at the reference scale (1200px design width), but drive everything from the token system so it scales and re-themes.

---

## Design Tokens (the heart of this handoff)

The entire visual identity is expressed as **one flat set of ~30 named tokens**, redefined per theme. In the HTML these are CSS custom properties on the root element; the `Component.renderVals()` `THEMES` map in `OP4 App.dc.html` is the source of truth. Reproduce this map verbatim in your codebase (a struct of colors per theme, or a CSS `:root[data-theme]` block).

### Token roles
| Token | Role |
|---|---|
| `--chassis` / `--chassis-b` | Outer body fill (gradient) + border |
| `--head` / `--head-b` | Header/nav band fill + divider |
| `--panel` / `--panel-b` | Section card fill + border |
| `--inset` | Recessed field/track background |
| `--dash` | Dashed drop-zone border |
| `--ink` / `--silk` / `--faint` / `--val` | Text: primary / label (silkscreen) / faint / value |
| `--disp` / `--disp-ink` / `--disp-dim` / `--disp-b` / `--disp-dot` / `--disp-track` / `--disp-inset` | LCD/LED display: field, ink, dim ink, border, envelope node fill, progress track, inner shadow |
| `--key` / `--key-b` / `--key-ink` | Membrane button (inactive): fill / border / label |
| `--keyA` / `--keyA-b` / `--keyA-ink` | Membrane button (active/accent): fill / border / label |
| `--knob` / `--knob-b` / `--ptr` | Knob body (radial gradient) / border / pointer |
| `--acc` / `--accg` | Accent color / accent glow (rgba) |
| `--szl` / `--szl-b` / `--szl-t` | Sizzler & highlighted-row zone: fill / border / title |
| `--nav` / `--nav-b` / `--nav-ink` | Nav tab (inactive) |

### The three themes (exact values)

**DX21 (default — committed identity).** Warm charcoal chassis, putty membrane keys, yellow-green LCD.
- chassis `linear-gradient(180deg,#37342f,#2b2825)`, border `#46423b`
- head `linear-gradient(180deg,#3d3a34,#312e29)`, head-b `#191713`
- panel `#262421`, panel-b `#3d3a35`, inset `#1e1c19`, dash `#57523f`
- ink `#d8d2c2`, silk `#96907f`, faint `#6e6959`, val `#c4beb0`
- disp `linear-gradient(180deg,#cdd964,#b6c74b)`, disp-ink `#26310f`, disp-dim `#5a6630`, disp-b `#8a9a35`, disp-dot `#c3d254`
- key `linear-gradient(180deg,#ddd6c6,#c8c0ad)`, key-b `#a89f8a`, key-ink `#33312e`
- keyA `linear-gradient(180deg,#a9c25c,#8fae4a)`, keyA-b `#77913a`, keyA-ink `#1f2912`
- knob `radial-gradient(circle at 35% 30%,#45423c,#2a2825 70%)`, knob-b `#524e46`, ptr `#e8e2d2`
- acc `#a9c25c`, accg `rgba(169,194,92,.8)`
- szl `#282b1e`, szl-b `#4d5433`, szl-t `#a9c25c`

**TX81Z.** Flat black 1U rack, red-orange LED segments.
- chassis `linear-gradient(180deg,#202124,#18191b)`, border `#303236`
- head `linear-gradient(180deg,#232427,#1b1c1e)`, head-b `#0a0a0b`
- panel `#141518`, panel-b `#2c2e32`, inset `#0e0f11`
- ink `#e4e2dd`, silk `#84878d`, faint `#55585e`, val `#c9ccd1`
- disp `#120705`, disp-ink `#ff6a3d`, disp-dim `#8a3a20`, disp-b `#38160c`, disp-dot `#17191c`
- key `linear-gradient(180deg,#2b2d31,#1f2124)`, key-b `#3a3c41`, key-ink `#c9ccd1`
- keyA `#170d09`, keyA-b `#58260f`, keyA-ink `#ff6a3d`
- knob `radial-gradient(circle at 35% 30%,#2f3135,#1a1b1d 70%)`, knob-b `#383a3f`, ptr `#e4e2dd`
- acc `#ff6a3d`, accg `rgba(255,106,61,.8)`
- szl `#1c0f08`, szl-b `#58260f`, szl-t `#ff6a3d`

**SK-1** (Casio SK-1 toy homage). Cream chassis, teal displays, pink membrane keys.
- chassis `linear-gradient(180deg,#ece8e0,#dad4c9)`, border `#b8b2a6`
- head `linear-gradient(180deg,#f2efe8,#e2ddd3)`, head-b `#c5bfb4`
- panel `#f7f5f0`, panel-b `#cfc9be`, inset `#e6e2d9`
- ink `#33312e`, silk `#8a8478`, faint `#a39d90`, val `#55514a`
- disp `linear-gradient(180deg,#8fdfd8,#6fd2c9)`, disp-ink `#0e3532`, disp-dim `#2f6b64`, disp-b `#3aa89e`, disp-dot `#7fd9d2`
- key `linear-gradient(180deg,#fbfaf7,#e9e5de)`, key-b `#c5bfb4`, key-ink `#4a463e`
- keyA `linear-gradient(180deg,#f48fb1,#ec6a99)`, keyA-b `#d84f7f`, keyA-ink `#4a1228`
- knob `radial-gradient(circle at 35% 30%,#4a4f55,#33373c 70%)`, knob-b `#2b2f34`, ptr `#f7f5f0`
- acc `#ec6a99`, accg `rgba(236,106,153,.8)`
- szl `#fbe7ef`, szl-b `#e9a8c2`, szl-t `#d84f7f`

Algorithm-glyph colors are a small parallel map (`ALGACC` in the logic) mirroring each theme's display ink/base for selected vs. unselected diagrams.

### Typography
- **Display / labels / buttons:** `Barlow Condensed` (condensed engineering sans, silkscreen feel). Weights 500/600/700. Labels use `letter-spacing` .10–.20em, uppercase.
- **All numeric values & readouts:** `IBM Plex Mono`. Weights 400/500/600.
- Common sizes: section labels 10px/600 Barlow (.16em); param labels 9px/600 Barlow (.14em); values 11–15px Plex Mono; big display name 14px Plex Mono; the SPX90 program number 26px Plex Mono.

### Spacing & shape
- 4px base grid. Panels: 10px radius (outer chassis), 6px (section cards), 3–5px (chips/buttons).
- Section card = `background:var(--panel); border:1px solid var(--panel-b); border-radius:6px; padding:9–13px`.
- LCD/LED display chip = `background:var(--disp); color:var(--disp-ink); border:1px solid var(--disp-b); box-shadow:var(--disp-inset)`.
- Membrane button = gradient `--key`/`--keyA`, 1px border, `box-shadow:0 2px 3px rgba(0,0,0,.3)`; active state uses the `--keyA*` set.
- Knob = 30–44px circle, radial-gradient body, `inset 0 2px 4px rgba(0,0,0,.5)`, a 2px pointer line rotated `value/99*270 − 135` degrees.

---

## Screens / Views

All views share: an outer chassis, a header (logo "OP4" + view title + 6 nav tabs + hardware-link indicator), and a footer (engine status + version/theme). Nav tab active state uses `--keyA`.

### 1. Voice panel (home) — the make-or-break screen
- **Purpose:** edit one patch completely, no paging.
- **Layout:** sub-header row (DX21⇄TX81Z mode toggle · patch nav ‹ LCD name ›, Compare). Body is a 2-col grid: left column `236px` (algorithm selector, feedback knob, carrier/modulator legend); right column fills with 4 stacked operator strips + a 4-cell bottom row.
- **Operator strip** (grid `148px 264px 1fr`): name + role tag (CARRIER/MOD) + activity dot; TX-only wave/fix chips (dimmed to 0.35 opacity in DX21 mode, full + accent in TX81Z mode); RATIO/LEVEL/DETUNE as LCD chips; a draggable ADSR **envelope graph** (SVG on the display field).
- **Algorithm selector (signature element):** 8 selectable glyphs, 2×4, each an SVG of the operator routing (carriers filled, modulators outlined, feedback bracket on OP4). The selected diagram's op blocks **pulse** (`opPulse` keyframe) — intended to animate with each operator's live envelope in production.
- **Bottom row:** LFO (waveform chip, sync, 4 knobs), Pitch EG (mini graph), Function (poly/mono, porta, transpose, single/dual/split), **Sizzler** (randomize/morph/drift — amount knob, mode segمنت, freeze).
- **Mode toggle behavior:** DX21 vs TX81Z *reveals/dims* TX-only controls (wave select, fixed freq) rather than hiding them — teaches the difference between the machines.

### 2. Library (Librarian)
- **Purpose:** manage banks; import/export sysex.
- **Layout:** 2-col grid `216px 1fr`. Left: search field, tag filter chips (ALL/EP/BASS/…), bank list (selected uses `--szl`), and a dashed **.syx drop zone** ("Drop a .syx file / or receive from your synth"). Right: a **32-slot bank grid** (8 cols; selected slot lit as a display; empty slots at 0.45 opacity), then a transfer strip — device-number stepper, "Send bank to synth" / "Receive bank" buttons, and an LCD progress readout styled after the hardware prompt ("MIDI TRANSMIT? ▸ SENDING VOICE 14/32 · 44%").

### 3. Drum rack
- **Purpose:** map/mix a 32-slot drum kit.
- **Layout:** sub-header (RX21 kit display, "Authentic '85" 12-bit toggle, group-bus chips with mutes). A slot table (grid `6px 30px 150px 52px 1fr 56px 56px 90px 50px 46px`): group color-bar, №, patch-name LCD chip, key, level meter, pan, tune, group, choke badge, mute/solo. Below: a **keyboard strip** (25 keys) whose top edge is colored by the group mapped to that key.

### 4. FX
- **Purpose:** two hardware-style rack units.
- **Layout:** top card = **FX500 simul-chain** — 5 blocks in a fixed row (COMP/DIST/EQ/PITCH/DELAY), each with a bypass toggle and 3–4 knobs; bypassed block dims to 0.5. Bottom card (grid `230px 1fr`) = **SPX90** — big amber-style program number, program list (selected uses `--szl`), and the selected algorithm's 5 parameter knobs. Note printed: the MEP4 echo lives in the MIDI tab (it processes notes before the synth).

### 5. Generator
- **Purpose:** generate themed patch banks.
- **Layout:** 2-col grid `230px 1fr`. Left: category list (PADS/SCI-FI/RPG/…), Count slider, Mutation slider, big GENERATE button, "Undo generate". Right: a **results tray** — 4-col grid of candidate cards, each with name, a mini envelope preview, and KEEP / DISCARD (kept cards use `--szl`). Header has "Export bank as .syx".

### 6. MIDI / Settings
- **Purpose:** CC/NRPN mapping, hardware, engine.
- **Layout:** 3-col grid `300px 1fr 250px`. Left: Hardware (device №, echo-to-hardware toggle with caption "Edit here, hear it on your DX21", MIDI channel), Engine (Vintage 8-voice / Modern, oversampling 1×/2×/4×), and the **MEP4 note-echo strip**. Center: **CC assignments** table (searchable, per-row Learn/Clear; the row in learn mode pulses "LISTENING…"). Right: read-only **NRPN reference** (MSB/LSB) + "Copy full NRPN map".

---

## Interactions & Behavior
- **Nav:** clicking a tab switches `state.view` (`voice|lib|drums|fx|gen|midi`). Active tab styled with `--keyA`.
- **Mode toggle (Voice):** `state.mode` `DX21|TX81Z`; flips TX-only control opacity/accent and the header link label.
- **Theme:** a prop (`theme`) selects the token set and re-renders everything. In production this is a global preference; changing it must restyle all views at once.
- **Every mouse-draggable control is also typeable and MIDI-learnable.** Right-click any control → assign CC / add to Sizzler targets (context menu — not yet drawn, but reserve for it).
- **Animation (functional, prioritize):** the selected algorithm's operator blocks pulse with live envelope activity; envelope & LFO graphs should redraw from live values. Keyframes in the prototype: `opPulse` (op-block breathing) and `blip` (link/learn indicators). Decorative motion kept minimal.
- **Copy register:** plain, hardware-manual confident. Buttons say what they do ("Send bank to synth", "Receive voice"). Errors speak in-voice and offer the next move ("Checksum failed — import anyway?"). Never apologize.
- **Undo everywhere**, including "undo generate" and "undo bank import".
- **Resizable:** the whole UI is vector; scale the 1200px reference layout to the plugin window.

## State Management
Minimum state observed in the prototype: `view` (active nav), `mode` (DX21/TX81Z), `theme`, `hardwareLink` (bool). Production adds: current patch + all ~120 FM params, 32-slot bank + multi-bank list, drum-kit slot array, FX chain state, generator settings + results tray, CC/NRPN maps, MIDI device/channel/echo, engine mode + oversampling, and an undo stack. Data fetching = MIDI sysex I/O (send/receive bank & voice) and `.syx` file import/export.

## Assets
No image assets — all UI is CSS/SVG (knobs, glyphs, envelope graphs are drawn). Fonts load from Google Fonts: **Barlow Condensed** and **IBM Plex Mono**; bundle these with the plugin rather than fetching at runtime. No Anthropic brand assets are used.

## Screenshots
`screenshots/01-voice.png` through `06-midi.png` — all six views, captured in the committed DX21 theme, for quick visual reference alongside the live `.dc.html` files.

## Files
- `OP4 App.dc.html` — the unified application: all six views + nav + the three-theme token system. **Primary reference.** The `THEMES` and `ALGACC` maps in its `<script data-dc-script>` logic are the canonical token source.
- `OP4 Voice Panel (explorations).dc.html` — the exploration canvas: two voice-panel directions (rackmount/console), four hardware-theme treatments (DX21 / graphite+LCD / TX81Z rack / SK-1 materials), and **three algorithm-selector explorations** (membrane grid, marquee strip, annotated blueprint plate) not yet folded into the app. Use for the signature-element treatment and alternate layouts.
- `support.js` — the prototype runtime. **Reference only — do not ship.**

## Known open items (not yet final)
1. **Algorithm topologies are plausible approximations**, not verified against the real Yamaha 4-op algorithm charts. Correct all 8 routings against the DX21/TX81Z manual before shipping.
2. The right-click "assign CC / add to Sizzler" context menu is specified but not drawn.
3. The algorithm-selector signature element has three explored treatments (in the explorations file) but the app currently uses the compact 2×4 grid; pick a final treatment.

# UMG Bloom / Emissive UI Plugin — Production Plan

> Companion to `ue-ui-bloom-plugin-roadmap.md` and `ue-ui-bloom-plugin-milestones.md`. This document holds time estimates, the week-by-week schedule, and rolling daily/session todo lists. Update the "Current Week" section as work progresses — treat everything beyond the current week as a living estimate, not a fixed commitment.

**Assumptions:** ~15 hrs/week available (part-time, 10–20 hrs/week range), solo effort, structured as a few longer sessions per week rather than a fixed daily split — adapt session placement to your actual schedule.

---

## Effort Estimates

### v1 track (Milestones 1–4) — solid estimates

| Milestone | Hours | Weeks (~15 hrs/wk) |
|---|---|---|
| 1 — v1 backend prototype | 45–60 | 3–4 |
| 2 — Blueprint API + editor UX | 55–75 | 4–5 |
| 3 — Docs + sample content | 28–38 | 2–2.5 |
| 4 — Fab listing + v1 (Lite) release | 18–28 | 1.5–2 |
| **v1 total** | **~150–200** | **~11–13 (~3 months)** |

### v2 track (Milestones 5–7) — provisional

| Milestone | Plan |
|---|---|
| 5 — RDG research spike | Timeboxed to ~2 weeks (~20–30 hrs). Deliverable: a real month-scale-vs-multi-month estimate for 6–7, not a fixed task list. |
| 6 — v2 implementation + version matrix | Provisional 80–150 hrs — re-estimate after M5 reports back |
| 7 — v2 Pro release | Provisional 20–30 hrs |

**Re-estimation checkpoint:** at the end of Milestone 5, revisit this table and replace the M6/M7 provisional ranges with real estimates before committing to a v2 timeline.

---

## Week-by-Week Schedule (v1 track)

| Week | Focus | Milestone tasks |
|---|---|---|
| 1 | Core visual mechanism spike | 1a |
| 2 | Geometry-sync spike + Slate reading | 1b, 1c |
| 3 | Subsystem assembly + isolation/blur | 1d, 1e, 1f |
| 4 | Format/resolution + cost validation; buffer | 1g, 1h |
| 5–6 | Widget classes + subsystem integration | 2a–2e |
| 7–8 | Editor tooling (details panel, auto-config, validation) | 2f–2h |
| 9 | Designer preview + PIE shortcut + presets + profiling | 2i–2l |
| 10 | Docs (quick-start, tuning guide, shared reference) | 3a–3c |
| 11 | Demo content + troubleshooting/FAQ + doc pipeline | 3d–3g |
| 12–13 | Fab listing prep, smoke test, publish v1 Lite | 4a–4i |

*(Weeks are planning units, not deadlines — expect drift, especially around Milestone 2's editor tooling, which has the widest task-count spread.)*

---

## Week 1 — Core Visual Mechanism Spike (1a) — ✅ Complete

**Goal:** prove the fundamental visual mechanism works end-to-end — hidden Widget Component → glow RT → PP material → pre-tonemap HDR composite — before any other investment. No dynamism, no Blueprint exposure yet.

- [x] **Session 1 (~5 hrs):** Widget-to-render-target capture
  1. Create an isolated test project/map (disposable, not the eventual plugin structure).
  2. Create `WBP_GlowTestContent` — a single Image/Color Block with a flat solid fill (stand-in glow source).
  3. Create `BP_GlowRig` Actor with a Widget Component; assign the widget class, set a Draw Size (e.g. 256×256).
  4. Keep the Widget Component's **Visible** flag ON (its internal redraw-to-texture appears to be gated on this — confirmed empirically: RT stops updating if Visible is off). Instead, turn off **Render in Main Pass** to exclude the primitive from the actual rendered scene while the RT keeps updating.
  5. Retrieve the RT via **Get Render Target** on the Widget Component.
  6. Sanity check: temporarily sample the RT on a visible debug plane with a basic unlit material to confirm the capture works before moving to post-process.
  - **Exit check:** widget content visibly rendered onto a texture via the debug plane.

- [x] **Session 2 (~5 hrs):** Post-process compositing at a hardcoded position
  1. Add an unbound (Infinite Extent) Post Process Volume to the level.
  2. New material: Material Domain → Post Process, Blendable Location → **Scene Color Before Bloom** (the slot that lets the engine's real bloom pass pick up your HDR emissive contribution — not just "before tonemapping" generically, since other pre-tonemap slots sit after bloom is already computed and would miss the point of Session 3).
  3. Texture Object parameter, default value = Session 1's RT — this is just a reference, not a color. Add a separate **Texture Sample** node, wire the Texture Object's output into its `Tex` input pin, and use the Sample node's `RGB` output for all downstream color math (the Texture Object alone has no color output).
  4. Compute hardcoded screen-space UV rectangle (`ScreenPosition` node or version-equivalent; remap + clamp to a fixed rect).
  5. Multiply sampled color into Emissive Color output — **correction:** don't wire the masked glow alone into Emissive Color, or full blend weight replaces the entire screen instead of adding to it. Add a **Scene Texture** node (Scene Texture Id = **PostProcessInput0**) to get the actual underlying scene color, then **Add** your masked glow contribution (texture × intensity × mask) to it, and wire that sum into Emissive Color. Leave blend weight at 1 once this is correct — weight isn't the on/off control for the final setup, just a diagnostic that caught this bug.
  6. **Correction:** the RT can't be hardcoded into the material (same reason as Session 1 — it's created dynamically, not a browsable asset), so the material must be assigned at runtime instead of added statically to a Post Process Volume:
     - Convert the material's Texture Object input to a parameter (e.g. `RTInput`) if not already done.
     - Add a **Post Process Component** to `BP_GlowRig` (not the Post Process Volume actor — `Add Or Update Blendable` isn't exposed there). Check **Unbound** on the component.
     - In the Level Blueprint, on BeginPlay: **Create Dynamic Material Instance** (Parent = the PP material) → **Set Texture Parameter Value** (`RTInput` = the RT from Get Render Target) → on the `BP_GlowRig` reference, **Get Component by Class** (Post Process Component) → **Add Or Update Blendable** (Blendable Object = the dynamic instance, Weight = 1.0).
     - The standalone Post Process Volume from step 1 can be left unused or deleted.
  7. Play — confirm content composites at the hardcoded position.
  - **Exit check:** glow-source content appears via the post-process pass at a fixed position (not yet glowing).

- [x] **Session 3 (~5 hrs):** Push into HDR range, validate real bloom, start reading
  1. Add an Intensity scalar parameter in the PP material, multiplied in before Emissive Color output (HDR headroom comes from this multiplier, not from UMG color values, which are clamped 0–1). **Correction:** wire Intensity so it only scales the glow term before the Add with Scene Color — multiplying it in after the Add scales the whole screen, not just the glow.
  2. Confirm Bloom enabled with sensible threshold; consider manual exposure metering for consistent testing — **confirmed necessary:** Auto Exposure was suppressing the effect until switched to Manual metering.
  3. Push Intensity through several values (2, 5, 10) — confirm genuine bloom characteristics (soft bleed growing with intensity), not just a brighter flat color.
  4. Note the intensity value that looks good — real data point for the §4 default `Glow Intensity` value.
  5. Remaining time (~1–2 hrs): start §3.3 reading — `FGeometry` absolute/local space handling, Retainer Box source skim — priming Week 2's geometry-sync spike.
  - **Exit check:** hardcoded glowing quad, intensity-driven, visibly blooming via the engine's real bloom pipeline. ✅ Achieved.

**Additional findings from Week 1:** Dynamic Material Instances snapshot parameter defaults at creation — editing the base material mid-PIE doesn't retroactively update a running instance; restart PIE or set the parameter explicitly at runtime. Post Process Component's own top-level Blend Weight (separate from the blendable's weight) must also be 1.0. Custom HLSL nodes adopted for material math going forward (see §3.1) — output type must be float4 (RGBA) to match Scene Texture/Emissive Color.

---

## Current Week — Week 2: Geometry-Sync Spike + Slate Reading (1b, 1c)

**Goal:** replace the hardcoded rectangle from Week 1 with a real UMG widget's actual screen position/size, tracked live — this is the piece flagged as riskiest in the whole v1 backend, so the aim is a working, verified mechanism, not a polished one.

- [ ] **Session 1 (~5 hrs):** Targeted Slate internals reading
  1. Read `FGeometry` (`Runtime/Slate/Public/Layout/Geometry.h`) — focus on `GetAbsolutePosition()`, `GetAbsoluteSize()`, and the local-to-absolute conversion functions. This is the type you'll be pulling widget position/size out of.
  2. Confirm `UWidget::GetCachedGeometry()` (available in Blueprint as **Get Cached Geometry**) is the actual entry point for reading a widget's current geometry each frame — this is the C++/BP bridge you'll use in Session 2.
  3. Look for Slate's viewport-space conversion helpers (`USlateBlueprintLibrary::AbsoluteToViewport` or version-equivalent — search in the editor if the exact name differs) — this is what turns a widget's absolute Slate-space geometry into actual pixel/viewport coordinates.
  4. Skim `SRetainerWidget` source (the Retainer Box implementation) specifically for how it captures a widget subtree to a render target and repositions the result — it's solving a closely related problem to what you're building.
  5. Skim the invalidation system (`SlateInvalidationRoot`) enough to know where invalidation hooks live — not needed yet (Refresh Mode defaults to Always per §6), but useful context for later.
  6. Write a short findings note — this becomes `docs/slate-internals-notes.md` per §3.3: which functions you'll actually call, and anything from Retainer Box worth reusing.

- [ ] **Session 2 (~5 hrs):** Wire a real widget's geometry into the material
  1. Build a small test HUD widget: a Canvas Panel containing one child (an Image or Border) placed at an arbitrary position — this stands in for a future `GlowContainer`/`GlowImage`.
  2. Add it to the viewport at runtime (**Create Widget** + **Add to Viewport**) so it has real, live screen geometry — not a Designer-time-only placement.
  3. Keep a reference to this widget instance somewhere you can tick against (Level Blueprint variable, or a simple Actor with an Event Tick).
  4. On Tick: call **Get Cached Geometry** on the widget, then convert to viewport pixel coordinates via the AbsoluteToViewport-style helper from Session 1.
  5. Normalize those pixel coordinates into [0,1] UV space using **Get Viewport Size** (`UWidgetLayoutLibrary`) — check empirically whether the conversion helper already accounts for DPI scale, or whether you need to divide by **Get Viewport Scale** separately.
  6. Each tick, call **Set Vector Parameter Value** on your existing dynamic material instance to push the computed `RectMin`/`RectMax`, replacing Week 1's hardcoded constants.
  - **Exit check:** the glow box now appears at the widget's actual on-screen position instead of a hardcoded one.

- [x] **Session 3 (~5 hrs):** Stress-test tracking robustness
  0. **Cleanup first:** replace the Session 2 Delay-based initialization workaround with a proper guard — IsValid check on `GlowDMI` at the top of Tick, skip the update that frame if not yet valid, rather than relying on a fixed delay (removes a frame-rate/load-dependent race).
  1. Add real movement/resizing via a UMG **Widget Animation** — **use an Offsets (LayoutData) track, not Render Transform**. Found during testing: Render Transform-driven Translation/Scale desyncs from the geometry-sync tracking (cached geometry reflects layout, not the separate visual paint transform) — logged as an open item in §3.1/milestone 1i, not something to solve in this session. Offsets-driven animation is a valid, confirmed-working test of the core mechanism.
  2. Re-run the bordered-widget comparison from Session 2 with the animation playing — confirm smooth tracking through the whole movement, not just a static frame.
  3. Re-test at a non-default **UI Scale** (Project Settings → User Interface) and at least one more window resolution.
  4. Specifically check screen edges/corners during the animation — most likely place for the mask-clamping logic to reveal an off-by-one or clamping bug that a centered test wouldn't catch.
  5. Log any drift/misalignment found, where, and whether resolved — feeds directly into §3.1 before Week 3 builds `GlowSubsystem` on this mechanism.
  - **Exit check:** a moving/resizing widget with the glow box tracking it correctly across at least two different window resolutions/scales, including near screen edges. ✅ Achieved.

**Week 2 exit condition:** a real UMG widget, moved and resized live, with the glow rectangle correctly following it — validated across more than one resolution. This is the mechanism the whole `GlowContainer`/`GlowSubsystem` design in §4 depends on, so don't move to Week 3 until this is genuinely solid, not just "looked fine once."

---

## Change Log
- *Initial production plan created: effort estimates, week-by-week v1 schedule, Week 1 daily todo list.*
- *Week 1 sessions expanded into step-by-step instructions.*
- *Corrected Session 2 step 2: Blendable Location is "Scene Color Before Bloom" in the current engine dropdown, not the generic "Before Tonemapping" originally written.*
- *Corrected Session 1 step 4: Widget Component's RT redraw is gated on the Visible flag — must stay on. Use Render in Main Pass = false instead to hide the primitive without stopping RT updates. Confirmed empirically; also affects the real §3.1 capture strategy, updated there too.*
- *Corrected Session 2 step 3: Texture Object alone has no color output — needs a separate Texture Sample node wired to it to actually get RGB values.*
- *Corrected Session 2 step 5: material was replacing the whole screen at full weight instead of compositing — needed to Add the glow onto a PostProcessInput0 Scene Texture sample rather than outputting the glow alone.*
- *Corrected Session 2 step 6: RT can't be hardcoded into the PP Volume's material array (dynamic asset, same as Session 1). Add Or Update Blendable is exposed on Post Process Component, not the Post Process Volume actor — added component to BP_GlowRig instead.*
- *Week 1 marked complete — exit condition achieved (intensity-driven glowing quad with real bloom response). Additional findings logged: DMI parameter snapshotting, PP Component blend weight, Custom HLSL float4 requirement.*
- *Week 2 planned: geometry-sync spike (1b) + Slate reading (1c), broken into three sessions.*
- *Week 2 Session 3 fleshed out: Delay-hack cleanup, Widget Animation-driven movement/resize test, multi-resolution/edge-case validation.*
- *Found: geometry sync tracks layout-driven (Offsets) changes correctly but not Render Transform-driven Translation/Scale. Logged as known limitation in roadmap §3.1 and follow-up task 1i in milestones; Session 3 test switched to Offsets-based animation.*
- *Week 2 Session 3 completed: stress-test tracking robustness passed with movement/resizing animation, multiple window resolutions, and DPI-scale validation. Exit condition achieved: glow box tracks moving/resizing widget correctly across at least two different window resolutions/scales.*
# UMG Bloom / Emissive UI Plugin — Project Roadmap

> Living document. Sections are intentionally scoped as an outline first — each will be expanded in detail, and any section that grows large should be split into its own linked file (e.g. `docs/architecture.md`, `docs/ux-flows.md`) with a summary + link left here.

**Status legend:** 🟢 Detailed &nbsp;|&nbsp; 🟡 Outlined only &nbsp;|&nbsp; ⚪ Not started

---

## 1. Project Overview
- **Goal:** Let average UE users (primarily Blueprint-first) add real bloom/emissive effects to UMG HUD elements with minimal setup, sold on Fab / used as a portfolio piece.
- **Non-goals (for now):** diegetic/3D world-space widget glow (already trivially solvable via existing engine features — out of scope), non-UMG UI frameworks.
- **Success criteria:** works with near-zero manual PP/material setup, looks good with default settings, editor-preview-accurate, stable across supported engine versions.

Status: 🟡

---

## 2. Release Strategy
- **v1 — "Foundation":** Render-target + post-process material approach. Ships first. Establishes the Blueprint-facing API, editor UX, docs, and Fab listing.
- **v2 — "Pro":** Scene View Extension / RDG backend swapped in behind the same front-facing API. Compute-shader blur, correct pre-tonemap compositing, better TAA/dynamic-res behavior. Sold as an upgrade/higher tier.
- Architecture boundary between "how glow is computed" and "how it's configured" must be clean from v1 so the swap doesn't break user-facing API/content.

Status: 🟡

---

## 3. Technical Architecture
### 3.1 v1 Backend (Render Target + Post-Process Material)

**Capture strategy:** Hidden/detached `UWidgetComponent` per glow collection, not `DrawWidgetToRenderTarget`. Widget Component owns its own RT (`GetRenderTarget()`) and has built-in tick/invalidate-driven redraw — reuses engine update-throttling instead of reimplementing dirty-flag logic.

**Critical design decision — what goes into the glow RT:** Do NOT capture the widget's full visible pixels into the RT (causes double-rendering + tonemap-induced color shift vs. the crisp on-screen UI). Instead, each glow-enabled widget renders two things independently:
1. Normal visible appearance — ordinary Slate/UMG compositing, untouched.
2. A masked glow-color shape (widget geometry/alpha as mask, tinted by exposed `Glow Color` / `Glow Intensity` properties) into a shared glow-collection RT via the hidden Widget Component.

Only the isolated glow layer goes through blur + pre-tonemap composite. Avoids fragile luminance-threshold extraction; gives art-directable glow rather than "whatever was bright."

**Format & resolution:**
- RGBA16F for HDR headroom (values >1.0 needed for real bloom response). Lower-precision (RGBA8 + material-side intensity multiplier) as a perf-tier fallback for mobile.
- RT resolution derived from `UWidgetLayoutLibrary::GetViewportSize()` + current DPI scale — NOT hardcoded — to avoid Canvas Panel alignment bugs. Downsample relative to viewport (e.g. half-res) since it's blurred anyway.

**Blur/glow chain:** Mirror engine's own bloom structure — small downsample chain (2–4 mips), Kawase blur passes (cheaper per-pass than full separable Gaussian, visually sufficient, same technique the engine's built-in bloom already uses).

**Compositing / alignment (trickiest part of v1):** Glow RT is captured independently of the main viewport's Slate pass, so the PP material needs each glow widget's absolute on-screen geometry (position + size, DPI-corrected) to composite the blur back in the right place. Requires syncing widget screen geometry from game thread into the compositing material (dynamic material instance parameter or similar) every frame / on invalidation. Recommend prototyping this geometry-sync step in isolation before building the rest of the chain around it.

Status: 🟢

### 3.2 v2 Backend (Scene View Extension / RDG)

**What changes vs. v1:** the isolated glow-layer capture (hidden Widget Component → glow RT, §3.1) stays conceptually the same. What changes is the composite/injection mechanism — moves from a PP material blendable to an RDG pass registered via `FSceneViewExtensionBase::SubscribeToPostProcessingPass`, giving precise pass-ordering control relative to TAA/TSR/dynamic resolution and enabling compute-based blur.

**Resource management:** blur ping-pong buffers via RDG's transient allocator (pooled/released automatically within the frame graph) rather than persistent `UTextureRenderTarget2D` assets as in v1.

**Multi-configuration correctness (real test matrix, not assumed):**
- TAA/TSR — empirically determine (via source/RenderDoc) which pass slot avoids TAA ghosting while staying pre-tonemap.
- Dynamic resolution — geometry-to-UV mapping from §3.1 must additionally account for the DRS scale factor, not just viewport size/DPI.
- Forward/deferred/mobile — mobile forward renderer has a different PP pass set; likely needs a mobile-specific hook or a documented fallback to the v1 material approach on mobile.
- Split-screen/stereo — view extension invoked per-view in `FSceneViewFamily`; geometry sync must resolve each view's own viewport rect.

**Version-porting strategy:** isolate hook names/enums behind a small per-engine-version compatibility shim (one file per supported version) rather than scattered `#if` checks — maps directly onto the version support matrix in §8.

Status: 🟢

### 3.3 Slate/UMG Rendering Internals (research spike)

**Goal:** understand exactly where widget geometry/draw elements get finalized — underpins both the §3.1 geometry-sync step and the §5 Designer-preview approximation.

**Study targets:**
- `FGeometry` + absolute-vs-local space conversion — the mechanism for reading a widget's on-screen position/size each tick.
- Invalidation system (`FSlateInvalidationRoot`, invalidation panels) — needed to implement `Refresh Mode: On Invalidate` via events rather than polling.
- Slate render batching / `FSlateDrawElement` — bounds what a custom `OnPaint` override can actually draw (relevant to §5's Designer approximation).
- Retainer Box source, read directly — already solves "render subtree off-screen, composite back"; best reference implementation available for RT/format/refresh decisions.

**Deliverable:** findings note, worth splitting into its own linked file (`docs/slate-internals-notes.md`) once written — should concretely answer (a) how to read absolute screen geometry + DPI scale cheaply every frame, (b) how to hook invalidation rather than poll.

Status: 🟢

---

## 4. Blueprint-Facing API / Widget & Component Design

**Core widget types — additive, not replacement-first:**
- `UGlowContainer` (Border-like wrapper) — wraps arbitrary existing child content, derives mask from child alpha. Non-invasive default; doesn't force UMG hierarchy restructuring.
- `UGlowImage` / `UGlowTextBlock` — drop-in subclasses of standard `Image`/`TextBlock` with glow properties built in, for the common single-element case without extra wrapper overhead.
- Both register themselves + screen geometry with a central `UGlowSubsystem` (Game Instance Subsystem) on Construct/Tick — subsystem owns the hidden Widget Component, shared glow RT, and geometry-sync work (§3.1). Widgets stay thin; subsystem holds the real backend logic.

**Property surface (two tiers, matches §5 progressive disclosure):**
- *Always visible:* `Glow Color` (linear color), `Glow Intensity` (float, HDR multiplier), `Glow Enabled` (bool)
- *Advanced (collapsed category):* `Blur Quality` (enum Low/Med/High — not a raw radius float), `Refresh Mode` (Always / On Invalidate / Manual), `RT Resolution Scale` (relative to viewport, not absolute pixels — DPI-safe by construction), `Mask Source` (auto-from-alpha vs custom mask texture, mainly for `GlowContainer`)

**Naming convention:** reuse vocabulary UE users already know from Light components (`Color`, `Intensity`) rather than inventing new terms — should read as native to the engine.

**Backend-swap boundary (critical for v1→v2):** Public UProperties/Blueprint-callable functions must not reference RT/PP-material internals directly. Internal `IGlowBackend` interface sits between widgets/subsystem and the actual rendering implementation; v1 (RT+material) and v2 (RDG/SVE) both implement it, selected via project setting or auto-detected capability. Enforce strictly from day one — as long as v1's property set is a subset of v2's, content authored against v1 keeps working unchanged after a Pro backend upgrade.

Status: 🟢

---

## 5. Editor Tooling & First-Run UX

**Designer-view preview limitation (important, not just a nice-to-have caveat):** UMG Designer canvas is Slate-only and does not render through the post-process pipeline — a true HDR pre-tonemap bloom cannot appear there regardless of implementation quality. Two-part approach:
1. Approximate, non-HDR faked glow in Designer via custom `OnPaint` override (Slate brushes) — good enough for layout feedback.
2. Be explicit in UX/docs that true bloom is only accurate in PIE; provide a fast "Preview Glow" → PIE shortcut rather than promising full-fidelity Designer preview.

**Auto-configuration flow:** Editor-only module checks, on first `GlowContainer`/`GlowImage` placement, for (a) a Post Process Volume with the glow composite material in its blendables and (b) bloom/HDR enabled in project settings. Missing either → single actionable notification ("Glow setup incomplete — Configure Project") that fixes both in one click (adds unbound/infinite-extent PP Volume with material pre-assigned, flips project settings). Highest-value first-run UX item — directly removes the manual-PP-Volume friction identified as the core anti-pattern.

**Validation warnings** (`UEditorValidator` or save/compile-time check):
- Glow widget inside a `ListView`/`TileView` entry with `Refresh Mode: Always` → suggest `On Invalidate`.
- `RT Resolution Scale` above sane threshold → perf warning.
- No PP Volume detected in level → warn + link to auto-configure.
- Bloom disabled in project settings → warn + link to auto-configure.

**Details panel polish:** Custom `IDetailCustomization` — collapsed Advanced category (not alphabetical default), non-technical tooltips, inline warning icons next to the specific property causing a validation issue (discoverable at the point of the problem, not just in the Message Log).

Status: 🟢

---

## 6. Performance & Guardrails

**Reframe — collection-level cost, not per-widget:** because all glow widgets contribute masks into one shared glow-collection RT (§4's `GlowSubsystem`), blur/composite cost is largely fixed per frame regardless of glow widget count — only cheap mask-drawing scales with count. Guardrails should target RT resolution/blur quality settings, not widget count.

**Memory budget (concrete, given §3.1 format/resolution decisions):**
- RGBA16F half-viewport (e.g. 960×540 @ 1080p): ~4.1MB base RT + ~30% for downsample/blur mip chain ≈ 5.5MB total.
- RGBA16F full-viewport (no downsample): ~16.6MB — the reason half-res is the default, not just for bandwidth but memory.
- RGBA8 fallback tier (mobile): under 1MB at equivalent relative scale.

**Default refresh behavior:** `Refresh Mode` defaults to `Always`, not `On Invalidate` — correctness over optimization. Invalidation-based default risks visible stale/missing glow if a custom widget doesn't properly trigger invalidation. `On Invalidate` is the escape hatch surfaced via the §5 validation warning for widgets in lists, not the global default.

**Cost budget target:** ~0.5ms total glow subsystem cost (masks + blur + composite) at default settings on mid-range desktop — validate against this during the v1 prototype rather than deciding in the abstract.

**Platform presets:** bundle `RT Resolution Scale` + `Blur Quality` + `Refresh Mode` into Desktop/Console/Mobile presets rather than requiring per-setting manual tuning per platform.

**In-editor profiling:** custom stat group (`stat GlowUI`) breaking down mask-draw / blur / composite frame time — internal testing tool and self-serve diagnostic for users who hit §5's perf validation warnings.

Status: 🟢

---

## 7. Documentation & Sample Content

**Quick-start (5-minute path):** mirrors the §5 auto-config flow exactly — install, wrap an existing Image in `GlowContainer`, set Color/Intensity, click "Configure Project" fix-it notification, press Play. If the real product flow and this doc diverge, simplify the flow rather than padding the doc.

**Advanced tuning guide:** covers `Blur Quality`, `RT Resolution Scale`, `Refresh Mode`, `Mask Source`, platform presets (§6) — uses the concrete numbers from §6 (memory figures, ~0.5ms budget) so trade-offs are grounded, not abstract.

**Demo content:** `GlowContainer`-wrapped icon example, direct `GlowImage`/`GlowTextBlock` usage, a "kitchen sink" multi-element widget demonstrating the §6 collection-batching cost benefit. Demo level ships with PP Volume pre-configured — works immediately on open, and doubles as the §9 marketing video/gif capture source.

**Troubleshooting/FAQ:** derived directly from §5's validation warning catalog (one FAQ entry per warning) rather than written separately, plus a pre-emptive entry for "why doesn't glow show in Designer view" (§5 limitation) before it becomes a support ticket.

**Doc structure across tiers:** single shared reference/advanced doc set (not split per listing) — Pro-only features marked with clear badges/upgrade CTAs inline, turning the docs into a natural upsell surface for Lite users who hit a capability they need. Exception: quick-starts stay tier-specific and short, so a Lite user's first-five-minutes path never references Pro-only setup steps.

**Doc source of truth:** markdown (this roadmap's `docs/` split-outs) as the single source; PDF/hosted site generated from it at release, not maintained separately.

Status: 🟢

## 8. QA & Version Support Matrix

**Matrix structure:** `Engine Version | v1 Status | v2 Status | Renderer paths tested | Known issues` — maintained continuously, not written once.

**Per-version checklist** (from §3.2's identified dimensions): forward/deferred, mobile, TAA/TSR, dynamic resolution, split-screen/VR (v2 only). Run before claiming support for a new engine version, not reactively after a user report.

**Version-compat shim maintenance:** the §3.2 per-engine-version shim carries its own changelog of what changed per version bump — internal maintenance value plus a transparency artifact ("last verified on 5.x") worth surfacing in the Fab listing.

**Automated smoke test:** functional test level + lightweight automation spec confirming the glow subsystem loads/runs error-free across target platforms — cheap catch for silent platform breakage ahead of manual QA.

Status: 🟢

## 9. Fab Marketplace Preparation

**Listing content:** preview video/gifs from the §7 demo level; thumbnail shows the effect actively working (glowing icon, dark background). Feature bullets map directly to the differentiators from earlier market research: HDR-correct vs. the free alternative's fake bloom, TAA/dynamic-resolution correct, minimal setup vs. that alternative's C++-only requirement and unmaintained/buggy status.

**Pricing/tiering:** v1 free, v2 "Pro" fairly priced as a paid upgrade. Leverages the `IGlowBackend` module boundary (§4) — free build can ship without the v2 RDG module compiled in at all, not just feature-flagged. **Decided: two separate Fab listings** (free "Lite" + paid "Pro") rather than one listing with an in-app upgrade — better discoverability given free plugins get browsed/installed far more, and keeps free-tier reviews from diluting Pro-tier expectations. Support scope explicitly stated: free tier = docs/FAQ/community only, direct support reserved for Pro buyers. Decide "free forever including updates" framing upfront rather than revisiting later.

**License/support:** check current Fab publisher documentation for EULA options rather than deciding today. Support: §7 docs/FAQ as first line, Discord/forum fallback for edge cases — keeps ongoing burden manageable for a likely-solo effort.

Status: 🟢

---

## 10. Milestones

> Detailed task-level checklist for each milestone lives in [`ue-ui-bloom-plugin-milestones.md`](./ue-ui-bloom-plugin-milestones.md) — this section stays the high-level summary. Time estimates, week-by-week schedule, and the rolling current-week todo list live in [`ue-ui-bloom-plugin-production-plan.md`](./ue-ui-bloom-plugin-production-plan.md).

1. ⚪ v1 backend prototype (personal validation, no polish)
   - a. Core visual mechanism spike — hidden Widget Component + static glow RT + hardcoded quad → PP material, pre-tonemap composite. Validates the fundamental visual approach before any other investment.
   - b. Geometry-sync spike — same setup, tracking a real moving/resizing widget's screen position correctly (riskiest piece identified in §3.1).
   - c. Slate internals reading (§3.3), done in parallel with (b) — directly informs the geometry-sync work.
   - d. Assemble into real architecture — `GlowContainer`/`GlowImage`/`GlowSubsystem` (§4) built around the now-validated mechanism.
2. ⚪ Blueprint API + editor UX pass
3. ⚪ Docs + sample content
4. ⚪ Fab listing + v1 release
5. ⚪ v2 RDG backend research spike
6. ⚪ v2 backend implementation + version matrix testing
7. ⚪ v2 "Pro" release

Status: 🟡

---

## Change Log
- *Initial roadmap created.*
- *§3.1 (v1 backend) detailed: capture strategy, glow-layer isolation design, format/resolution, blur chain, compositing/alignment.*
- *§4 (Blueprint API) detailed: GlowContainer/GlowImage/GlowTextBlock widgets, GlowSubsystem, two-tier property surface, IGlowBackend swap boundary.*
- *§5 (editor tooling) detailed: Designer-preview limitation + approximation strategy, auto-configuration flow, validation warnings, details panel customization.*
- *§3.2 (v2 RDG backend) and §3.3 (Slate internals spike) detailed — completes §3.*
- *§6 (performance/guardrails) detailed: collection-level cost reframe, memory budget, refresh-mode default rationale, cost target, platform presets, profiling tooling.*
- *§7 (docs/samples), §8 (QA/version matrix), §9 (Fab prep) detailed — all sections now complete except milestone execution (§10, tracked as work begins).*
- *§9 pricing updated: v1 free, v2 Pro paid upgrade (leverages IGlowBackend module boundary).*
- *§9 finalized: two separate Fab listings (free Lite + paid Pro) rather than single listing with upgrade.*
- *§7 updated: shared reference docs across tiers with Pro feature badges (upsell surface), tier-specific quick-starts only.*
- *§10 Milestone 1 broken into sequenced sub-steps: core visual spike → geometry-sync spike → Slate internals reading → architecture assembly.*
- *Split off `ue-ui-bloom-plugin-milestones.md` as the linked task-level checklist for all seven milestones.*
- *Split off `ue-ui-bloom-plugin-production-plan.md`: effort estimates, week-by-week v1 schedule, rolling current-week daily todo list.*

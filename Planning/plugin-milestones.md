# UMG Bloom / Emissive UI Plugin — Milestone Checklist

> Companion to `ue-ui-bloom-plugin-roadmap.md`. Each milestone below expands the corresponding entry in the roadmap's §10, with tasks pulled from the detailed section that covers them (referenced in brackets). Check items off as completed; this is the execution tracker, the roadmap stays the design/decisions record.

---

## Milestone 1 — v1 Backend Prototype
*(validation only, no polish — §3.1, §3.3)*

- [ ] 1a. Core visual mechanism spike: hidden Widget Component + static glow RT + hardcoded quad → PP material, pre-tonemap composite
- [ ] 1b. Geometry-sync spike: track a real moving/resizing widget's screen position + size (DPI-corrected) into the PP material
- [ ] 1c. Slate internals reading spike: `FGeometry`/absolute-local space conversion, invalidation system, `FSlateDrawElement`, Retainer Box source review
- [ ] 1d. Assemble minimal `GlowSubsystem`; prove multiple glow sources compositing into one shared RT (collection-based approach)
- [ ] 1e. Validate glow-layer isolation design (masked glow shape rendered separately from the widget's normal visible pixels)
- [ ] 1f. Implement blur/glow chain (downsample + Kawase passes, mirroring engine bloom structure)
- [ ] 1g. Validate RT format/resolution handling (RGBA16F, half-viewport, DPI-scale-derived sizing)
- [ ] 1h. Instrument and validate cost against the ~0.5ms budget target (§6)
- [ ] 1i. *(Follow-up, confirmed during Week 2)* Fix Render Transform Scale tracking gap — `Get Local Size` doesn't factor in Render Transform Scale, and naively multiplying size by scale ignores the Render Transform Pivot (scaling anchors at the widget's pivot, not local origin). Fix: compute `PivotLocal = RenderTransformPivot × LocalSize`, scale each corner relative to `PivotLocal` before Local-To-Absolute conversion. See §3.1.

## Milestone 2 — Blueprint API + Editor UX Pass
*(§4, §5, §6)*

- [ ] 2a. Implement `GlowContainer` widget (Border-like wrapper, alpha-derived mask)
- [ ] 2b. Implement `GlowImage` / `GlowTextBlock` drop-in widgets
- [ ] 2c. Wire widget registration + geometry sync into `GlowSubsystem`
- [ ] 2d. Implement two-tier property surface — basic: Color / Intensity / Enabled; advanced: Blur Quality, Refresh Mode, RT Resolution Scale, Mask Source
- [ ] 2e. Implement `IGlowBackend` interface boundary (v1/v2 swap-safe from the start)
- [ ] 2f. Details panel customization: collapsed Advanced category, non-technical tooltips, inline warning icons
- [ ] 2g. Auto-configuration flow: detect/create PP Volume + enable bloom project settings via one-click fix notification
- [ ] 2h. Validation warnings: list/refresh-mode, RT resolution threshold, missing PP Volume, bloom disabled
- [ ] 2i. Designer-view approximate preview (custom `OnPaint` override, non-HDR faked glow)
- [ ] 2j. "Preview Glow" → PIE shortcut
- [ ] 2k. Platform presets (Desktop / Console / Mobile bundles of Resolution Scale + Blur Quality + Refresh Mode)
- [ ] 2l. `stat GlowUI` profiling command

## Milestone 3 — Docs + Sample Content
*(§7)*

- [ ] 3a. Quick-start guide (Lite) mirroring the auto-config flow exactly
- [ ] 3b. Advanced tuning guide, grounded in §6's actual cost/memory numbers
- [ ] 3c. Shared reference docs across tiers with Pro feature badges / upgrade CTAs
- [ ] 3d. Demo widget blueprints: `GlowContainer` example, `GlowImage`/`GlowTextBlock` example, kitchen-sink multi-element demo
- [ ] 3e. Demo level with PP Volume pre-configured (doubles as §4/9 marketing capture source)
- [ ] 3f. Troubleshooting/FAQ derived from the validation warning catalog + Designer-preview limitation note
- [ ] 3g. Markdown source-of-truth structure in place; PDF/hosted site generation pipeline from it

## Milestone 4 — Fab Listing + v1 (Lite) Release
*(§8, §9)*

- [ ] 4a. Capture preview video/gifs from the demo level
- [ ] 4b. Design thumbnail (effect visibly working, dark background)
- [ ] 4c. Write feature bullets mapped to differentiators vs. the existing free/rough alternative
- [ ] 4d. Confirm current Fab publisher terms (pricing conventions, EULA/license options)
- [ ] 4e. Set up free "Lite" Fab listing (v1 only, v2 module not compiled in)
- [ ] 4f. Set up support channel (Discord/forum); confirm docs-first support scoping for free tier
- [ ] 4g. Initial version support matrix entry (first supported engine version(s))
- [ ] 4h. Automated smoke test passing (functional test level + automation spec)
- [ ] 4i. Publish v1 Lite listing

## Milestone 5 — v2 RDG Backend Research Spike
*(§3.2)*

- [ ] 5a. Prototype `FSceneViewExtensionBase` subclass + `SubscribeToPostProcessingPass` minimal hook
- [ ] 5b. Identify correct pass slot for pre-tonemap, TAA-safe compositing (verify via source/RenderDoc, not assumption)
- [ ] 5c. Prototype RDG-based compute blur pass; compare cost/quality vs. v1's pixel-shader chain
- [ ] 5d. Validate transient RDG resource usage for ping-pong blur buffers
- [ ] 5e. Test against dynamic resolution scaling (geometry/UV mapping incorporating DRS factor)
- [ ] 5f. Test against split-screen/stereo (per-view geometry resolution)
- [ ] 5g. Determine mobile forward-renderer strategy (native hook vs. fallback to v1 material approach)
- [ ] 5h. Draft version-compat shim structure (per-engine-version hook abstraction)

## Milestone 6 — v2 Backend Implementation + Version Matrix Testing
*(§3.2, §8)*

- [ ] 6a. Implement full v2 `IGlowBackend` using validated spike results
- [ ] 6b. Integrate with existing `GlowContainer`/`GlowImage`/`GlowSubsystem` (swap-safe per interface boundary)
- [ ] 6c. Build version support matrix table (Engine Version | v1 Status | v2 Status | Renderer paths tested | Known issues)
- [ ] 6d. Run per-version checklist (forward/deferred, mobile, TAA/TSR, dynamic res, split-screen/VR) across supported versions
- [ ] 6e. Extend automated smoke test coverage to v2 backend
- [ ] 6f. Update version-compat shim changelog per engine version tested

## Milestone 7 — v2 "Pro" Release
*(§9)*

- [ ] 7a. Extend shared docs with Pro-specific advanced content + badges (structure already in place from M3)
- [ ] 7b. Capture Pro-specific preview content (e.g. side-by-side v1 vs. v2 quality/perf comparison)
- [ ] 7c. Confirm Pro pricing per current Fab terms
- [ ] 7d. Set up separate paid "Pro" Fab listing
- [ ] 7e. Cross-link Lite listing → Pro listing
- [ ] 7f. Publish v2 Pro listing

---

## Change Log
- *Initial milestone checklist created, expanding roadmap §10 into component tasks.*

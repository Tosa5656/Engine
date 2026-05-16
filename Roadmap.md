# Roadmap - Singularity

---

## v0.3.0 - Foundation

### Textures & Materials
- [x] 2D texture loading
- [x] Texture descriptor sets and samplers
- [x] Mipmap generation
- [x] Texture filtering 
- [x] Texture array / atlas support
- [x] Normal mapping (tangent space normals)
- [x] Parallax occlusion mapping (POM)
- [x] Relief mapping
- [x] Copy texture folder to build

---

## v0.4.0 - Lighting & Shadows

### Light
- [x] Directional light (direction, color, intensity)
- [x] Point light (position, color, intensity, radius)
- [x] Spotlight (position, direction, cone angle, penumbra)
- [x] Light types enumeration and registry
- [x] Deferred shading G-buffer (position, normal, albedo, emissive)
- [x] Forward+ / tiled rendering fallback
- [x] Clustered Forward
- [x] HDR pipeline (RGB16F framebuffer)
- [x] Tonemapping (ACES filmic)
- [x] Exposure control (auto-exposure, manual)

### Shadows — Directional

#### Shadow mapping foundation
- [x] Shadow pass: render scene depth from light POV into dedicated depth attachment
- [x] Shadow sampler: sample depth attachment in fragment shader, compare against fragment depth in light space
- [x] Light space matrix: compute orthographic projection + view matrix from directional light direction
- [x] Frustum fit: tightly fit orthographic frustum to camera view frustum (stable cs)
- [x] Shadow acne mitigation: depth bias (slope-scaled, constant)
- [ ] Normal offset bias: shift shadow-receiving fragment along normal to reduce acne
- [ ] Peter Panning fix: bias balancing to avoid detached shadows
- [x] Shadow map resolution: configurable per-light (512–4096)
- [ ] Percentage-Close Filtering (PCF): 2×2, 3×3 bilinear hardware filter
- [x] Manual PCF: 3×3, 5×5 kernel with depth comparison loop in shader
- [ ] Poisson disk PCF: pre-rotated Poisson samples for smooth penumbra

### Light controls (ImGui)
- [x] Directional light yaw/pitch control
- [x] Light intensity control
- [x] Shadow bias sliders (constant, slope)
- [x] Light direction display

#### Cascade shadow maps (CSM)
- [ ] CSM — 3 cascade split scheme: near / mid / far
- [ ] Cascade partition: uniform split (linear)
- [ ] Cascade partition: logarithmic split (exponential)
- [ ] Cascade partition: practical split (pssm = lerp(log, uniform))
- [ ] Cascade selection: vertex shader computes cascade index from view-space depth
- [ ] Cascade blend: blend between adjacent cascade edges to hide transition seams
- [ ] Cascade blend width: configurable overlap percentage per cascade boundary
- [ ] Stable cascade: round cascade projection to texel-sized increments to prevent shimmer
- [ ] Cascade resolution: per-cascade independent resolution (higher near, lower far)
- [ ] Cascade debug visualization: color-coded cascade regions in ImGui overlay

#### PCF & soft shadows
- [ ] Rotated grid PCF: dither samples per pixel to reduce banding
- [ ] Stratified Poisson: distribute samples in disk, random rotation per pixel
- [ ] Percentage-Closer Soft Shadows (PCSS): blocker search → penumbra estimation → PCF
- [ ] PCSS blocker step: average blocker depth in search region
- [ ] PCSS penumbra: penumbra width ∝ (receiverDepth − blockerAvg) / blockerAvg
- [ ] PCSS filter: PCF with dynamic radius from penumbra estimate
- [ ] Contact Hardening: shadow hardness ∝ distance to contact (near contact = hard, far = soft)

### Shadows — Other light types
- [ ] Spot light shadow: perspective projection, 1D depth array, PCF
- [ ] Point light shadow: omnidirectional (cube map / dual-paraboloid), 6-face depth
- [ ] Point light PCF: sample cubemap faces with offset, hardware bilinear across faces
- [ ] Percentage-Closer Soft Shadows for spot / point: blocker search in light space

### Shadow map optimization
- [ ] Shadow map atlas: pack multiple shadow maps (dir, spot, point) into single texture array
- [ ] Atlas layout: static grid → dynamic rectangle packing (guillotine, shelf)
- [ ] Atlas tile invalidation: mark dirty tiles when light moves, re-render only moved lights
- [ ] Static shadow caching: render static geometry once, reuse while neither light nor occluder moves
- [ ] Static/dynamic split: cache static depth, re-render only dynamic objects each frame
- [ ] Frustum culling for shadow casters: skip objects outside light frustum in shadow pass
- [ ] Shadow LOD: reduce shadow map resolution for distant / small lights
- [ ] Shadow distance culling: lights beyond max shadow distance skip shadow pass entirely

### Shadow quality & debugging
- [ ] Shadow filtering quality presets (low=2×2 PCF, medium=4×4, high=8×8+Poisson, ultra=PCSS)
- [ ] Shadow fade-out: smoothly fade shadows at max cascade / max distance boundary
- [ ] Bias sliders in ImGui: constant bias, slope bias, normal offset per cascade
- [ ] Shadow map preview: render selected shadow map depth into ImGui window
- [ ] Cascade frustum wireframe: debug draw frustum of each cascade in 3D viewport
- [ ] Shadow caster highlight: highlight objects rendered into shadow map

### Post-processing
- [ ] Post-processing pass framework
- [ ] Bloom (threshold, intensity, radius)
- [ ] Gaussian blur ( separable, dual-pass)
- [ ] SSAO (Screen Space Ambient Occlusion)
- [ ] SSAO blur and bilateral filter
- [ ] Depth of Field (bokeh, circle of confusion)
- [ ] Motion blur (velocity buffer)
- [ ] Chromatic aberration
- [ ] Vignette
- [ ] Color grading / LUT

---

## v0.4.5 - Scripting basic

### Core Lua integration
- [ ] Lua VM initialization (lua_newstate)
- [ ] LuaJIT integration (JIT compilation, FFI)
- [ ] Lua state lifecycle (per-world, shared)
- [ ] Engine API binding (sol2 or manual)
- [ ] Lua module registration system
- [ ] C++ call Lua and Lua call C++ bridge
- [ ] Global script environment

### Script components
- [ ] ScriptComponent (attach Lua file to entity)
- [ ] Script path resolution (relative to project)
- [ ] onInit() callback
- [ ] onUpdate(dt) callback
- [ ] onDestroy() callback
- [ ] onCollisionEnter/Exit callbacks
- [ ] Entity reference from Lua (entity:move_to(x,y,z))
- [ ] Component access from Lua (entity:GetTransform())

### Exposed engine APIs
- [ ] Transform: position, rotation, scale, look_at
- [ ] Input: is_key_down, is_mouse_down, get_mouse_delta
- [ ] Time: delta_time, fixed_delta_time, time_since_start
- [ ] Debug: log, warn, error

### Development tools
- [ ] Script compilation error messages
- [ ] Runtime error handling (stack trace)
- [ ] In-game Lua console (REPL)
- [ ] Console history and autocomplete

---

## v0.5.0 - Physics

### Collision Detection
- [ ] AABB vs AABB (overlap test, penetration depth)
- [ ] Bounding Sphere vs Bounding Sphere
- [ ] OBB vs OBB (SAT)
- [ ] OBB vs AABB
- [ ] Capsule vs Capsule
- [ ] Capsule vs AABB
- [ ] Capsule vs OBB
- [ ] Triangle mesh vs shape (mesh collider)
- [ ] BVH (Bounding Volume Hierarchy) construction
- [ ] BVH traversal for queries
- [ ] SAT implementation (15 axes for OBB)
- [ ] Contact point generation
- [ ] Contact manifold (multiple points)

### Rigid Body
- [ ] RigidBodyComponent (dynamic / kinematic / static)
- [ ] Mass and inverse mass
- [ ] Velocity and angular velocity
- [ ] Inertia tensor (solid box, sphere, custom)
- [ ] Force and torque accumulation
- [ ] Impulse resolution
- [ ] Position correction (baumgarte stabilization)
- [ ] Distance constraint
- [ ] Hinge constraint (point-to-point, angular limits)
- [ ] Ball-socket constraint
- [ ] Spring constraint
- [ ] Gravity (world gravity, per-body override)
- [ ] Friction (static, dynamic)
- [ ] Restitution (bounciness)
- [ ] Sleeping (velocity threshold)

### Character Controller
- [ ] Capsule collider shape
- [ ] Move (with collision response)
- [ ] Ground detection (ray cast down)
- [ ] Slope limit (max climbable angle)
- [ ] Step climbing (small steps)
- [ ] Sweep test for movement
- [ ] Controller jump
- [ ] Controller velocity (air control)

### Optimization
- [ ] Octree spatial partitioning
- [ ] Uniform grid partitioning
- [ ] Broad phase culling (AABB overlap)
- [ ] Narrow phase culling (accurate shape test)
- [ ] Physics step (fixed timestep)
- [ ] Interpolation for rendering
- [ ] Job system for physics (parallel island detection)
- [ ] SIMD collision (SSE/NEON)

---

## v0.6.0 - Audio

### Core audio
- [ ] OpenAL context creation
- [ ] Device enumeration
- [ ] Buffer management (ALuint)
- [ ] Audio source management
- [ ] Listener (position, orientation, velocity)
- [ ] WAVE file loading (PCM)
- [ ] OGG/Vorbis streaming
- [ ] Audio thread (separate from main)

### Spatial audio
- [ ] 3D positioning (alPosition3f)
- [ ] Distance model (inverse, linear, exponent)
- [ ] Rolloff factor
- [ ] Reference distance
- [ ] Max distance
- [ ] Sound occlusion (environment filter)
- [ ] Reverb effect (EFX extension)
- [ ] Reverb zones in world
- [ ] Doppler shift (AL_DOPPLER_FACTOR)
- [ ] HRTF mode (AL_HRTF_SOFT)

### Sound system
- [ ] Sound bank / audio bank editor
- [ ] Sound playback events
- [ ] Priority queue (max polyphony)
- [ ] Object pooling (recycle sources)
- [ ] Crossfade between states
- [ ] Trigger on collision / trigger
- [ ] Trigger on animation event
- [ ] Footstep system (surface-based)

---

## v0.6.5 - Profiling & Diagnostics

### Profiling
- [ ] Tracy integration (tracy.h, ZoneScoped)
- [ ] Tracy server connection
- [ ] Frame capture and visualization
- [ ] Custom zones (Render, Physics, Scripts)
- [ ] Tracy Markers (frame boundaries)
- [ ] Tracy Plot (frame time, FPS)
- [ ] Microbenchmark framework (measure isolated functions)
- [ ] Benchmark results logging
- [ ] Automated per-commit benchmarks

### Editor profiler
- [ ] CPU profiler panel (per-system breakdown)
- [ ] GPU profiler (VK_EXT_calibrated_timestamps)
- [ ] Frame timeline visualization
- [ ] Flame graph view
- [ ] Memory pool allocator (stack, pool, heap)
- [ ] Allocator stats (bytes allocated, peak usage)
- [ ] Memory leak detection

---

## v0.7.0 - Scripting advanced

### Exposed engine APIs (advanced)
- [ ] Physics: apply_force, apply_impulse, set_velocity, get_velocity, get_collisions
- [ ] Audio: play_sound, stop_sound, set_volume, set_pitch, set_3d
- [ ] Rendering: set_material_param, enable_post_process, set_camera
- [ ] Scene: spawn_entity, destroy_entity, find_by_tag, find_by_name
- [ ] Math: vec2, vec3, vec4, quat, mat4 operations in Lua

### Development tools
- [ ] Lua script hot reload (detect file change, reload)
- [ ] Script reload without scene reset
- [ ] Live script patching (update functions)

### Script system
- [ ] Script inheritance (Script:subclass())
- [ ] Mixin / composition (attach multiple scripts)
- [ ] Coroutines (yield, resume in onUpdate)
- [ ] Async tasks (wait, delay)
- [ ] Global events (onCollisionEnter, onTriggerEnter, onGameStart, onGamePause)
- [ ] Event broadcasting to scripts
- [ ] Script execution order (priority)

### Script debugging
- [ ] Breakpoints (line breakpoint, conditional)
- [ ] Step over, step into, step out
- [ ] Watch variables (local, upvalue, global)
- [ ] Call stack view
- [ ] Lua stack trace to source mapping
- [ ] Debug adapter protocol (DAP) integration

---

## v0.7.5 - UI System

### UI Core
- [ ] UI layer rendering (overlay on top of 3D scene)
- [ ] Canvas system (auto-layout vs absolute positioning)
- [ ] UI elements: Button, Label, Image, Panel, Window
- [ ] UI element hierarchy (parent-child transform)
- [ ] Anchor and pivot system
- [ ] UI layout: Horizontal, Vertical, Grid, Flexbox
- [ ] RectTransform (position, size, anchors, offsets)
- [ ] Z-ordering and depth sorting
- [ ] UI draw call batching

### UI components
- [ ] Text (font loading, TTF/OTF, bitmap fonts)
- [ ] Text formatting (size, color, bold, italic)
- [ ] Text alignment (left, center, right, justify)
- [ ] Text overflow (ellipsis, word wrap)
- [ ] Button states (normal, hover, pressed, disabled)
- [ ] Input field (single line, multiline)
- [ ] Slider / Progress bar
- [ ] Checkbox / Toggle
- [ ] Dropdown / Combo box
- [ ] Scroll view / Scrollbar
- [ ] Tab view
- [ ] Tooltip

### UI interaction
- [ ] Mouse hover detection (ray from cursor)
- [ ] Click / double-click handling
- [ ] Keyboard navigation (tab, arrows)
- [ ] Drag and drop (UI elements, assets)
- [ ] Focus management (modal, popup)
- [ ] Event system (onClick, onValueChanged, etc.)
- [ ] UI animation (tween, fade, slide)

### UI theming
- [ ] Style sheet / theme system
- [ ] Style inheritance (base style → derived)
- [ ] Nine-patch / scaled sprites
- [ ] Color palette and typography presets

### In-game UI
- [ ] HUD elements (health bar, ammo, minimap)
- [ ] Pause menu
- [ ] Dialogue system (text boxes, choices)
- [ ] Inventory UI
- [ ] Mini-game UI (built with engine UI)
- [ ] Cursor management (custom cursor, locked)

### UI from Lua
- [ ] Create UI element from script
- [ ] Bind UI events to Lua callbacks
- [ ] Dynamic UI generation (lists, grids)
- [ ] UI layout from script

---

## v0.8.0 - Editor

### Editor core UI
- [ ] ImGui integration (docking, viewport)
- [ ] Dockable panels (drag, float, group)
- [ ] Main menu bar (File, Edit, View, Help)
- [ ] Toolbar (play, pause, step, save)
- [ ] Scene hierarchy panel (tree view, search)
- [ ] Inspector panel (component list, edit fields)
- [ ] Console / log panel (filter, clear, copy)
- [ ] Project browser panel
- [ ] Editor theming (dark, light)

### Viewport
- [ ] Viewport rendering (separate render pass)
- [ ] Orbit camera (mouse drag)
- [ ] Pan camera (middle mouse, RMB)
- [ ] Fly camera (WASD, QE, shift)
- [ ] Camera speed control
- [ ] Gizmo: Translate (move along axes/plane)
- [ ] Gizmo: Rotate (arcball, axis)
- [ ] Gizmo: Scale (uniform, per-axis)
- [ ] Gizmo: Bounding box (resize)
- [ ] Selection (click, box select, multi-select)
- [ ] Grid rendering (XZ, XY, YZ planes)
- [ ] Grid snapping (position, rotation)
- [ ] Selection highlight (wireframe overlay)
- [ ] Gizmo hotkeys (Q, W, E, R)

### Asset management
- [ ] Material editor (preview quad, sliders for params)
- [ ] Material graph (nodes, connections)
- [ ] Texture viewer (zoom, pan, mip view)
- [ ] Model / mesh preview (orbit, zoom)
- [ ] Audio preview (play button)
- [ ] Asset search and filters
- [ ] Drag-drop import

### Scene editing
- [ ] Runtime mode (game loop)
- [ ] Editor mode (simulation paused)
- [ ] Edit during play (changes persist)
- [ ] Undo / redo system
- [ ] Debug draw: AABB
- [ ] Debug draw: Bounding spheres
- [ ] Debug draw: light volumes
- [ ] Debug draw: physics colliders
- [ ] Debug draw: velocity vectors
- [ ] Scene serialization to JSON
- [ ] Scene deserialization from JSON
- [ ] Scene file format (.singscene)

---

## v0.9.0 - Advanced Graphics & PBR

### Materials and textures
- [ ] PBR workflow (metallic-roughness)
- [ ] Albedo map support
- [ ] Metallic map support
- [ ] Roughness map support
- [ ] AO map support
- [ ] Emissive map support
- [ ] Height map (for parallax)
- [ ] Material instances (shared params)
- [ ] Material editor (graph-based)
- [ ] Shader graph (visual node editor)
- [ ] Custom shader nodes (noise, math)

### Vulkan features
- [ ] Ray tracing extension detection
- [ ] Bottom-level acceleration structure (BLAS)
- [ ] Top-level acceleration structure (TLAS)
- [ ] Ray tracing pipeline
- [ ] Ray queries for reflections
- [ ] Denoising (NVIDIA Real-Time Denoiser)
- [ ] Mesh shaders (VK_NV_mesh_shader)
- [ ] Task shader (emit triangles)
- [ ] Amplification shader (generate meshlets)
- [ ] Variable Rate Shading (VRS) tiers
- [ ] VRS in different viewport regions
- [ ] Sampler feedback (texture streaming)

### GPU-driven rendering
- [ ] Indirect drawing (vkCmdDrawIndirect)
- [ ] Multi-draw indirect
- [ ] GPU frustum culling (compute shader)
- [ ] GPU occlusion culling (depth mip)
- [ ] GPU frustum vs meshlet culling
- [ ] Async compute queue setup
- [ ] Graphics queue sync
- [ ] Separate physics compute dispatch

---

## v1.0.0 - Projects System & Release

### Architecture
- [ ] Parallel system execution (thread pool)
- [ ] System dependencies graph
- [ ] System ordering (before/after tags)
- [ ] Async asset loading (thread pool)
- [ ] Asset loading cancellation
- [ ] Resource cache (LRU eviction)
- [ ] Hot reload: shaders
- [ ] Hot reload: textures
- [ ] Hot reload: scripts
- [ ] Hot reload: scenes

### Projects system

#### Project management
- [ ] Project creation wizard
- [ ] Project settings (name, version, company)
- [ ] Application icon
- [ ] Window configuration (size, fullscreen, vsync)
- [ ] External dependency manifest (.json, .toml)
- [ ] Project file format (.singproj - JSON)
- [ ] Project explorer (folder tree, show/hide filters)
- [ ] Recent projects list (persisted)
- [ ] Project migration (versioning)

#### Workspace
- [ ] Multi-project workspaces (.singworkspace)
- [ ] Solution build (build all projects)
- [ ] Engine source linking (embed in project)
- [ ] Engine prebuilt linking (DLL/SO)
- [ ] User preferences file (theme, shortcuts, layout)
- [ ] Keyboard shortcuts customization
- [ ] Editor theme (dark, light, custom colors)

#### Build system
- [ ] Project template system (empty, with physics, etc.)
- [ ] Template files and scripts
- [ ] In-engine Lua compilation (luac)
- [ ] Lua bytecode validation
- [ ] Asset bundler (collect, compress, package)
- [ ] .sgpkg format (bundled scene/materials/scripts)
- [ ] Build profile: dev (fast compile, debug symbols)
- [ ] Build profile: release (optimizations, strip)
- [ ] Build profile: shipping (min size, encrypted)
- [ ] Live coding (hot reload DLL/SO)
- [ ] Build progress bar
- [ ] Build errors panel (navigate to source)
- [ ] Build warnings panel
- [ ] Prebuilt engine distribution (ZIP, installer)
- [ ] Build artifact cleanup
# Moon Rover — Code Analysis & Improvement Report

> Scope: static review of the current source (C++/OpenGL/GLUT). The goal is to
> keep the project's essence (a small, educational third‑person rover simulator)
> while making it safer, cleaner and easier to maintain. Most suggestions are
> **organizational**; only a handful require touching real logic.

---

## 1. What the project is

A single‑window OpenGL demo that renders:

- A procedurally generated **terrain** (Perlin noise + cosine ridges) tiled 3×3 around the rover.
- A multi‑part **rover** model (chassis, rockers, bogies, drives, wheels) loaded from `.obj` files, with wheel spin / steering / suspension‑style tilt animations.
- A textured **Earth** and a **skybox** sphere.
- A **third‑person camera** driven by mouse + `WASD`.

Two executables are built: `Main` (the simulator) and `ShapeTest` (an isolated rover viewer).

### Strengths (keep these)
- Clear separation of *geometry generators* (`sphere`, `terrain`) behind opaque handle types (`typedef struct strX *X`) — a tidy C‑style "class".
- The math layer (`mat4`, `transforms`) is small and self‑contained.
- Assets (shaders/meshes/textures) are cleanly separated from code on disk.
- The rover draw code uses a proper matrix stack (`pushMatrix`/`popMatrix`) for hierarchical transforms.

---

## 2. Bugs

Ordered by severity. Each item: **what**, **why it's wrong**, **fix**.

### 2.1 🔴 Critical — Terrain index buffer writes past the end of the array

**Where:** `src/terrain.cpp` (struct `strTerrain`, `terrain_bind`, `terrain_draw`).

```cpp
struct strTerrain {
    GLuint terrainBuffer[4];   // indices 0..3 only
    ...
};

glGenBuffers(4, terrain->terrainBuffer);           // creates 4 buffers -> [0..3]
...
glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[4]); // ❌ index 4 is out of bounds
```

**Why it's wrong:** `terrainBuffer` has 4 elements, so valid indices are `0..3`.
`terrainBuffer[4]` reads/writes one element past the array (undefined behavior),
and `glGenBuffers(4, …)` never actually creates a name for the index buffer, so
the element‑array buffer used by `terrain_draw` is an *uninitialized* `GLuint`.
It only "works" today by luck of memory layout. Compare with `sphere.cpp`, which
does it correctly (`GLuint sphereBuffer[5]; glGenBuffers(5, …)` and uses `[4]`).

**Fix:**
```cpp
struct strTerrain {
    GLuint terrainBuffer[5];   // 4 attribute buffers + 1 index buffer
    ...
};
...
glGenBuffers(5, terrain->terrainBuffer);   // in terrain_bind
```

---

### 2.2 🔴 Critical — `glDrawElements` is given a byte count, not an element count

**Where:** `src/terrain.cpp` (`terrain_draw`) and `src/sphere.cpp` (`sphere_draw`).

```cpp
int totalIndexes = (terrain->numVertexX - 1) * (terrain->numVertexZ * 2 + 1);
glDrawElements(GL_TRIANGLE_STRIP, totalIndexes * sizeof(GLuint), GL_UNSIGNED_INT, 0);
//                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ❌ 4× too large
```

**Why it's wrong:** The 2nd argument of `glDrawElements` is the **number of
indices** to read, not the number of bytes. `sizeof(GLuint) == 4`, so the code
asks the driver to read `4 × totalIndexes` indices and runs 4× off the end of
the element buffer. This is undefined behavior; the primitive‑restart index and
driver leniency usually hide the corruption, but it can produce stray triangles
or crashes on other GPUs/drivers.

**Fix:**
```cpp
glDrawElements(GL_TRIANGLE_STRIP, totalIndexes, GL_UNSIGNED_INT, 0);
```

---

### 2.3 🟠 Medium — Unchecked `fopen` → crash on a missing asset

**Where:** `src/utils.cpp` (`loadShader`, `loadBMP`).

```cpp
FILE *shaderFile = fopen(filename, "r");
// no null check
while (!feof(shaderFile)) { ... }          // ❌ segfault if the file is missing
```

**Why it's wrong:** If the working directory is wrong or an asset is missing,
`fopen` returns `NULL` and the very next `feof`/`fread` dereferences it, crashing
with no useful message. This is the most common "it doesn't run for me" failure
mode for this kind of project.

**Fix:**
```cpp
FILE *shaderFile = fopen(filename, "r");
if (!shaderFile) {
    fprintf(stderr, "Could not open shader '%s'\n", filename);
    return nullptr; // and have callers handle nullptr
}
```
Apply the same guard in `loadBMP`.

---

### 2.4 🟠 Medium — Shader source is leaked every time it is compiled

**Where:** `src/utils.cpp` (`compileShader` + `loadShader`).

```cpp
GLuint compileShader(const char *filename, GLuint shaderType) {
    char const *source = loadShader(filename);   // malloc'd inside loadShader
    ...
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);
    return shaderId;                             // ❌ 'source' never freed
}
```

**Why it's wrong:** `loadShader` returns a heap buffer that is never released.
Small and one‑time, but it's a genuine leak and a bad pattern to copy.

**Fix:** `free((void*)source);` before returning (or use `std::string`).

---

### 2.5 🟡 Low — `setupTextures` binds sampler uniforms to texture *object ids*, not texture *units*

**Where:** `src/main.cpp` (`setupTextures`).

```cpp
glUniform1i(texturesLocs[0], textures[0]); // ❌ textures[0] is a texture NAME, not a unit
```

**Why it's wrong:** A `sampler2D` uniform must be set to the **texture unit
index** (0,1,2…), not the GL texture object name. This code is effectively
incorrect, but it is masked because `display()` re‑sets the correct unit right
before each draw (`glUniform1i(texturesLocs[0], 0)` + `glActiveTexture`). So
`setupTextures` is dead/misleading code today.

**Fix:** either delete `setupTextures`, or set units explicitly (`…, 0`, `…, 1`, …).

---

### 2.6 🟡 Low — Dead / unused code

- `src/camera.cpp` + `src/player.cpp` (and their headers) are compiled by
  `ShapeTest` conceptually but **not used**: in `main.cpp` the `Player`/`Camera`
  are commented out and a hand‑rolled `calculateCameraPosition()` is used instead.
- `Rover::setWheelsY(...)` — empty body.
- `Rover::get*WheelXZPosition(...)` — never called.
- `hueToRgb(...)` in `terrain.cpp` — only used by commented‑out debug code.
- `srand(time(NULL))` in `sphere_create` / `terrain_create` — `rand()` is never called.
- `printPlayer(...)` — empty.

**Why it matters:** dead code inflates the surface area a maintainer must read
and reason about, and hides which paths are actually live.

**Fix:** remove, or move behind a clearly named debug flag.

### 2.7 🟡 Low — Copy‑paste header guard comments

`include/camera.h` and `include/player.h` both end with `#endif  // PERLIN_H`
even though their guards are `CAMERA_H` / `PLAYER_H`. Cosmetic, but a symptom of
copy‑paste that can bite later.

### 2.8 🟡 Low — `GL_BLEND` enable/disable around Earth clouds is a no‑op

In `display()` the clouds path does `glEnable(GL_BLEND); …; glDisable(GL_BLEND);`
*before* `sphere_draw(earth)`, and the actual cloud compositing is done inside
`earth.fsh` via `mix(...)`. The blend toggling therefore does nothing and only
confuses the reader.

---

## 3. Maintainability improvements (essence preserved)

These are **organizational**; behavior stays the same.

1. **Kill the 3× duplicated shader setup in `main.cpp`.**
   `initShaders()` repeats ~40 lines three times (program1/2/3), and the copies
   have already drifted (program3 never sets the light/material uniforms;
   `reshapeFunc` updates only `projMatrixLoc1/2`). Introduce a tiny
   `ShaderProgram` struct that owns `id` + attribute/uniform locations and a
   `load(vsh, fsh)` helper:

   ```cpp
   struct ShaderProgram {
       GLuint id = 0;
       GLuint vPos, vCol, vTex, vNorm;
       GLuint model, view, proj;
       void load(const char* vsh, const char* fsh);
       void setMVP(const Mat4& m, const Mat4& v, const Mat4& p) const;
   };
   ```

2. **Group the ~30 file‑scope globals** in `main.cpp` (camera state, light
   arrays, matrices, program handles) into a few structs (`Camera`, `Lighting`,
   `Scene`). This makes the update/draw flow explicit and testable.

3. **Unify the geometry API.** `sphere_*` and `terrain_*` share ~90% of their
   bind/draw code (4–5 VBOs, primitive restart, `glDrawElements`). Extract a
   common `Mesh` (indexed, primitive‑restart triangle strip) and let sphere and
   terrain be *generators* that fill a `Mesh`. This also fixes 2.1/2.2 in one place.

4. **Pick one language style per module.** The codebase mixes C (`malloc`,
   opaque handles) and C++ (`std::vector`, classes). That's fine, but keeping
   `render/` consistently one way reduces cognitive load.

5. **Free generator scratch buffers after `*_bind`.** `sphere`/`terrain` keep
   their CPU‑side `vertices/normals/…` arrays alive forever even though only the
   VBOs are needed after binding (terrain does still need `vertices`/`normals`
   for `vertexFromXZPosition`; sphere does not).

6. **Modernize CMake.** `cmake_minimum_required(VERSION 3.0)` is deprecated;
   there is no `CMAKE_CXX_STANDARD`, and every `.cpp` becomes its own static
   library for no benefit. See the redesign below.

---

## 4. Proposed project structure

Rationale: separate **assets** from **code**, namespace the headers, and group
source by responsibility (core math ▸ rendering ▸ scene/app). This makes the
build graph obvious and lets `Main` and `ShapeTest` share the same libraries.

```
moon-rover/
├── CMakeLists.txt
├── README.md
├── cmake/                     # optional: FindGLEW.cmake, toolchain files
├── assets/                    # everything loaded at runtime (was top-level)
│   ├── meshes/                #   body.obj, wheel.obj, ...
│   ├── shaders/               #   *.vsh / *.fsh
│   └── textures/              #   *.bmp
├── include/moonrover/         # public headers, namespaced by folder
│   ├── core/                  #   mat4.h, transforms.h, utils.h, perlin.h
│   ├── render/                #   mesh.h, shape.hpp, sphere.h, terrain.h, rover.hpp, shader_program.h
│   └── scene/                 #   camera.h, player.h
├── src/
│   ├── core/                  #   mat4.cpp, transforms.cpp, utils.cpp, perlin.cpp
│   ├── render/                #   mesh.cpp, Shape.cpp, sphere.cpp, terrain.cpp, Rover.cpp, shader_program.cpp
│   ├── scene/                 #   camera.cpp, player.cpp
│   └── app/
│       ├── main.cpp           #   thin: init + GLUT callbacks only
│       ├── renderer.cpp/.h    #   owns ShaderPrograms + draw passes (from display())
│       └── input.cpp/.h       #   keyboard/mouse state (from move()/mouseFunc)
├── tools/
│   └── shape_test/            #   ShapeTest.cpp (its own executable)
├── screenshots/
└── docs/
    ├── ANALYSIS.en.md
    └── ANALYSIS.es.md
```

### Corresponding CMake sketch

```cmake
cmake_minimum_required(VERSION 3.16)
project(moon_rover LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)  # or keep the vendored lib/*.dll approach

# One library for shared engine code instead of one lib per .cpp
add_library(moonrover_core
    src/core/mat4.cpp src/core/transforms.cpp src/core/utils.cpp src/core/perlin.cpp
    src/render/mesh.cpp src/render/Shape.cpp src/render/sphere.cpp
    src/render/terrain.cpp src/render/Rover.cpp src/render/shader_program.cpp)
target_include_directories(moonrover_core PUBLIC include)
target_link_libraries(moonrover_core PUBLIC glew32 freeglut opengl32)

add_executable(Main src/app/main.cpp src/app/renderer.cpp src/app/input.cpp)
target_link_libraries(Main PRIVATE moonrover_core)

add_executable(ShapeTest tools/shape_test/ShapeTest.cpp)
target_link_libraries(ShapeTest PRIVATE moonrover_core)

# Copy runtime DLLs next to the binaries
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
```

> Note: asset paths are currently relative to the CWD (`"shaders/…"`,
> `"meshes/…"`, `"textures/…"`). If assets move under `assets/`, either update
> those literals to `"assets/shaders/…"` or add a small `assetPath()` helper and
> a post‑build `file(COPY assets DESTINATION .)` step so the binary still finds
> them.

---

## 5. Suggested order of work

1. Fix **2.1** and **2.2** (correctness, tiny diffs).
2. Add **2.3** null‑checks (stops silent crashes).
3. Remove dead code (**2.6/2.7**) and the no‑op blend (**2.8**).
4. Extract `ShaderProgram` + collapse the geometry API (**3.1, 3.3**).
5. Reorganize folders + modernize CMake (**§4**).

None of these change what the user sees on screen — they make the rover easier
to keep flying.

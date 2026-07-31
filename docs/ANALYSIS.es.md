# Moon Rover — Reporte de análisis y mejoras

> Alcance: revisión estática del código actual (C++/OpenGL/GLUT). El objetivo es
> **conservar la esencia** del proyecto (un simulador educativo de un rover en
> tercera persona) y a la vez hacerlo más seguro, limpio y fácil de mantener. La
> mayoría de las sugerencias son **de organización**; solo unas pocas tocan
> lógica real.

---

## 1. Qué es el proyecto

Una demo de OpenGL en una sola ventana que dibuja:

- Un **terreno** generado proceduralmente (ruido Perlin + crestas con cosenos), repetido en mosaico 3×3 alrededor del rover.
- Un **rover** de varias piezas (chasis, rockers, bogies, drives, ruedas) cargadas desde archivos `.obj`, con animaciones de giro de ruedas, dirección e inclinación tipo suspensión.
- Una **Tierra** texturizada y un **skybox** esférico.
- Una **cámara en tercera persona** controlada con mouse + `WASD`.

Se construyen dos ejecutables: `Main` (el simulador) y `ShapeTest` (un visor aislado del rover).

### Fortalezas (conservarlas)
- Buena separación de los *generadores de geometría* (`sphere`, `terrain`) detrás de tipos handle opacos (`typedef struct strX *X`) — una "clase" ordenada al estilo C.
- La capa matemática (`mat4`, `transforms`) es pequeña y autocontenida.
- Los recursos (shaders/meshes/texturas) están separados del código en disco.
- El dibujado del rover usa una pila de matrices real (`pushMatrix`/`popMatrix`) para las transformaciones jerárquicas.

---

## 2. Bugs

Ordenados por severidad. Cada punto: **qué**, **por qué está mal**, **cómo se arregla**.

### 2.1 🔴 Crítico — El index buffer del terreno escribe fuera del arreglo

**Dónde:** `src/terrain.cpp` (struct `strTerrain`, `terrain_bind`, `terrain_draw`).

```cpp
struct strTerrain {
    GLuint terrainBuffer[4];   // solo índices 0..3
    ...
};

glGenBuffers(4, terrain->terrainBuffer);           // crea 4 buffers -> [0..3]
...
glBindBuffer(GL_ARRAY_BUFFER, terrain->terrainBuffer[4]); // ❌ el índice 4 está fuera de rango
```

**Por qué está mal:** `terrainBuffer` tiene 4 elementos, así que los índices
válidos son `0..3`. `terrainBuffer[4]` lee/escribe un elemento más allá del
arreglo (comportamiento indefinido), y `glGenBuffers(4, …)` nunca crea el nombre
del buffer de índices, por lo que el *element array buffer* que usa
`terrain_draw` es un `GLuint` **sin inicializar**. Hoy "funciona" solo por suerte
del layout de memoria. Compáralo con `sphere.cpp`, que sí lo hace bien
(`GLuint sphereBuffer[5]; glGenBuffers(5, …)` y usa `[4]`).

**Arreglo:**
```cpp
struct strTerrain {
    GLuint terrainBuffer[5];   // 4 buffers de atributos + 1 de índices
    ...
};
...
glGenBuffers(5, terrain->terrainBuffer);   // en terrain_bind
```

---

### 2.2 🔴 Crítico — A `glDrawElements` se le pasa un conteo en bytes, no en elementos

**Dónde:** `src/terrain.cpp` (`terrain_draw`) y `src/sphere.cpp` (`sphere_draw`).

```cpp
int totalIndexes = (terrain->numVertexX - 1) * (terrain->numVertexZ * 2 + 1);
glDrawElements(GL_TRIANGLE_STRIP, totalIndexes * sizeof(GLuint), GL_UNSIGNED_INT, 0);
//                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ❌ 4× de más
```

**Por qué está mal:** El 2.º argumento de `glDrawElements` es el **número de
índices** a leer, no el número de bytes. Como `sizeof(GLuint) == 4`, el código
le pide al driver leer `4 × totalIndexes` índices y se sale 4× del final del
element buffer. Es comportamiento indefinido; el índice de *primitive restart* y
la tolerancia del driver suelen esconder la corrupción, pero puede producir
triángulos sueltos o crashes en otras GPUs/drivers.

**Arreglo:**
```cpp
glDrawElements(GL_TRIANGLE_STRIP, totalIndexes, GL_UNSIGNED_INT, 0);
```

---

### 2.3 🟠 Medio — `fopen` sin verificar → crash si falta un recurso

**Dónde:** `src/utils.cpp` (`loadShader`, `loadBMP`).

```cpp
FILE *shaderFile = fopen(filename, "r");
// sin verificación de null
while (!feof(shaderFile)) { ... }          // ❌ segfault si el archivo no existe
```

**Por qué está mal:** Si el directorio de trabajo es incorrecto o falta un
recurso, `fopen` devuelve `NULL` y el siguiente `feof`/`fread` lo desreferencia,
provocando un crash sin mensaje útil. Es la causa más común del típico "a mí no
me corre" en este tipo de proyectos.

**Arreglo:**
```cpp
FILE *shaderFile = fopen(filename, "r");
if (!shaderFile) {
    fprintf(stderr, "No se pudo abrir el shader '%s'\n", filename);
    return nullptr; // y que quien llama maneje el nullptr
}
```
Aplica la misma guarda en `loadBMP`.

---

### 2.4 🟠 Medio — El código fuente del shader se fuga en cada compilación

**Dónde:** `src/utils.cpp` (`compileShader` + `loadShader`).

```cpp
GLuint compileShader(const char *filename, GLuint shaderType) {
    char const *source = loadShader(filename);   // malloc dentro de loadShader
    ...
    glShaderSource(shaderId, 1, &source, NULL);
    glCompileShader(shaderId);
    return shaderId;                             // ❌ 'source' nunca se libera
}
```

**Por qué está mal:** `loadShader` devuelve un buffer en el heap que nunca se
libera. Es pequeño y ocurre una sola vez, pero es una fuga real y un mal patrón
para copiar.

**Arreglo:** `free((void*)source);` antes de retornar (o usar `std::string`).

---

### 2.5 🟡 Bajo — `setupTextures` asigna a los samplers el *id de textura*, no la *unidad*

**Dónde:** `src/main.cpp` (`setupTextures`).

```cpp
glUniform1i(texturesLocs[0], textures[0]); // ❌ textures[0] es un NOMBRE de textura, no una unidad
```

**Por qué está mal:** Un uniform `sampler2D` debe recibir el **índice de la
unidad de textura** (0,1,2…), no el nombre del objeto textura de GL. Este código
es incorrecto, pero queda enmascarado porque `display()` vuelve a fijar la unidad
correcta justo antes de cada dibujo (`glUniform1i(texturesLocs[0], 0)` +
`glActiveTexture`). Es decir, `setupTextures` hoy es código muerto/engañoso.

**Arreglo:** borrar `setupTextures`, o fijar las unidades explícitamente (`…, 0`, `…, 1`, …).

---

### 2.6 🟡 Bajo — Código muerto / sin usar

- `src/camera.cpp` + `src/player.cpp` (y sus headers): **no se usan**; en
  `main.cpp` el `Player`/`Camera` están comentados y en su lugar se usa un
  `calculateCameraPosition()` hecho a mano.
- `Rover::setWheelsY(...)` — cuerpo vacío.
- `Rover::get*WheelXZPosition(...)` — nunca se llaman.
- `hueToRgb(...)` en `terrain.cpp` — solo lo usaba código de depuración comentado.
- `srand(time(NULL))` en `sphere_create` / `terrain_create` — nunca se llama a `rand()`.
- `printPlayer(...)` — vacío.

**Por qué importa:** el código muerto aumenta la superficie que un mantenedor
debe leer y entender, y oculta qué rutas están realmente activas.

**Arreglo:** eliminarlo, o moverlo tras un flag de depuración con nombre claro.

### 2.7 🟡 Bajo — Comentarios de include guard copiados

`include/camera.h` e `include/player.h` terminan ambos con `#endif  // PERLIN_H`
aunque sus guardas son `CAMERA_H` / `PLAYER_H`. Es cosmético, pero es síntoma de
copy‑paste que puede confundir más adelante.

### 2.8 🟡 Bajo — El `GL_BLEND` alrededor de las nubes de la Tierra no hace nada

En `display()` la ruta de nubes hace `glEnable(GL_BLEND); …; glDisable(GL_BLEND);`
*antes* de `sphere_draw(earth)`, y la composición real de las nubes se hace
dentro de `earth.fsh` con `mix(...)`. Por lo tanto ese toggle de blend no tiene
efecto y solo confunde a quien lee.

---

## 3. Mejoras de mantenibilidad (sin perder la esencia)

Son **organizativas**; el comportamiento se mantiene igual.

1. **Eliminar la triplicación de la configuración de shaders en `main.cpp`.**
   `initShaders()` repite ~40 líneas tres veces (program1/2/3), y las copias ya
   divergieron (program3 nunca fija los uniforms de luz/material; `reshapeFunc`
   solo actualiza `projMatrixLoc1/2`). Introduce un pequeño struct
   `ShaderProgram` que sea dueño de `id` + las localizaciones de atributos/uniforms
   y un helper `load(vsh, fsh)`:

   ```cpp
   struct ShaderProgram {
       GLuint id = 0;
       GLuint vPos, vCol, vTex, vNorm;
       GLuint model, view, proj;
       void load(const char* vsh, const char* fsh);
       void setMVP(const Mat4& m, const Mat4& v, const Mat4& p) const;
   };
   ```

2. **Agrupar los ~30 globales de archivo** de `main.cpp` (estado de cámara,
   arreglos de luz, matrices, handles de programa) en unos pocos structs
   (`Camera`, `Lighting`, `Scene`). Esto hace explícito y testeable el flujo de
   actualización/dibujo.

3. **Unificar la API de geometría.** `sphere_*` y `terrain_*` comparten ~90% del
   código de bind/draw (4–5 VBOs, primitive restart, `glDrawElements`). Extrae un
   `Mesh` común (indexado, triangle strip con primitive‑restart) y deja que sphere
   y terrain sean *generadores* que llenan un `Mesh`. Esto además arregla 2.1/2.2
   en un solo lugar.

4. **Elegir un estilo de lenguaje por módulo.** El código mezcla C (`malloc`,
   handles opacos) y C++ (`std::vector`, clases). Está bien, pero mantener
   `render/` consistentemente en un estilo reduce la carga cognitiva.

5. **Liberar los buffers temporales del generador tras `*_bind`.**
   `sphere`/`terrain` mantienen vivos para siempre sus arreglos de CPU
   (`vertices/normals/…`) aunque tras el bind solo se necesitan los VBOs (ojo:
   el terreno sí sigue necesitando `vertices`/`normals` para
   `vertexFromXZPosition`; la esfera no).

6. **Modernizar CMake.** `cmake_minimum_required(VERSION 3.0)` está obsoleto; no
   hay `CMAKE_CXX_STANDARD`, y cada `.cpp` se vuelve su propia librería estática
   sin beneficio. Ver el rediseño abajo.

---

## 4. Propuesta de estructura del proyecto

Razón: separar **recursos** del **código**, poner los headers bajo un namespace
de carpetas, y agrupar el fuente por responsabilidad (matemática núcleo ▸
renderizado ▸ escena/app). Así el grafo de compilación queda obvio y `Main` y
`ShapeTest` pueden compartir las mismas librerías.

```
moon-rover/
├── CMakeLists.txt
├── README.md
├── cmake/                     # opcional: FindGLEW.cmake, toolchains
├── assets/                    # todo lo que se carga en runtime (antes en la raíz)
│   ├── meshes/                #   body.obj, wheel.obj, ...
│   ├── shaders/               #   *.vsh / *.fsh
│   └── textures/              #   *.bmp
├── include/moonrover/         # headers públicos, con namespace por carpeta
│   ├── core/                  #   mat4.h, transforms.h, utils.h, perlin.h
│   ├── render/                #   mesh.h, shape.hpp, sphere.h, terrain.h, rover.hpp, shader_program.h
│   └── scene/                 #   camera.h, player.h
├── src/
│   ├── core/                  #   mat4.cpp, transforms.cpp, utils.cpp, perlin.cpp
│   ├── render/                #   mesh.cpp, Shape.cpp, sphere.cpp, terrain.cpp, Rover.cpp, shader_program.cpp
│   ├── scene/                 #   camera.cpp, player.cpp
│   └── app/
│       ├── main.cpp           #   delgado: init + callbacks de GLUT
│       ├── renderer.cpp/.h    #   dueño de los ShaderProgram + pasadas de dibujo (de display())
│       └── input.cpp/.h       #   estado de teclado/mouse (de move()/mouseFunc)
├── tools/
│   └── shape_test/            #   ShapeTest.cpp (su propio ejecutable)
├── screenshots/
└── docs/
    ├── ANALYSIS.en.md
    └── ANALYSIS.es.md
```

### Esbozo de CMake correspondiente

```cmake
cmake_minimum_required(VERSION 3.16)
project(moon_rover LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
# find_package(GLEW REQUIRED)  # o mantener el enfoque de lib/*.dll versionadas

# Una librería para el código de motor compartido, en vez de una lib por .cpp
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

# Copiar las DLLs de runtime junto a los binarios
file(GLOB LIBRARIES "lib/*.dll")
file(COPY ${LIBRARIES} DESTINATION .)
```

> Nota: hoy las rutas de recursos son relativas al CWD (`"shaders/…"`,
> `"meshes/…"`, `"textures/…"`). Si los recursos se mueven bajo `assets/`, o bien
> actualizas esos literales a `"assets/shaders/…"`, o agregas un pequeño helper
> `assetPath()` y un paso post‑build `file(COPY assets DESTINATION .)` para que el
> binario los siga encontrando.

---

## 5. Orden de trabajo sugerido

1. Arreglar **2.1** y **2.2** (correctitud, diffs mínimos).
2. Agregar las verificaciones de null de **2.3** (evita crashes silenciosos).
3. Eliminar código muerto (**2.6/2.7**) y el blend sin efecto (**2.8**).
4. Extraer `ShaderProgram` + colapsar la API de geometría (**3.1, 3.3**).
5. Reorganizar carpetas + modernizar CMake (**§4**).

Ninguno de estos cambios altera lo que el usuario ve en pantalla — solo hacen que
el rover sea más fácil de mantener en órbita.

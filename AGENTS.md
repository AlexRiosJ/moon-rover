# AGENTS.md

Instrucciones para agentes de IA que trabajan en **Moon Rover**.

## Regla numero uno: `main` es la rama de desarrollo

Este repositorio tiene una situacion inusual y hay que respetarla:

| Rama | Rol |
|---|---|
| `master` | Rama por defecto en GitHub. Estado final del proyecto original. **Congelada: no se toca.** |
| `main` | Rama de desarrollo activo. Todo el trabajo nuevo nace aqui y vuelve aqui. |

Por tanto, siempre:

```bash
git fetch origin
git switch -c feat/<slug> origin/main
gh pr create --base main ...
```

Nunca abras un PR contra `master` ni le hagas commits. Que `master` siga siendo la
rama por defecto es intencional; no la cambies.

## Que es este proyecto

Simulador de un rover lunar en C++ con OpenGL y GLSL. Incluye generacion procedural
de terreno con ruido Perlin, texturas, iluminacion Phong, carga de modelos OBJ y una
camara en tercera persona. La ventana y la entrada se manejan con freeglut.

## Estructura

| Ruta | Contenido |
|---|---|
| `src/` | Implementaciones `.cpp`. `main.cpp` tiene el bucle de render y la entrada |
| `include/` | Cabeceras. `shapes/` contiene las clases `Shape` y `Rover` |
| `shaders/` | Shaders GLSL: `.vsh` de vertices, `.fsh` de fragmentos |
| `meshes/` | Modelos OBJ de las piezas del rover |
| `textures/` | Texturas en BMP |
| `lib/` | DLLs de Windows para freeglut, GLEW y GLFW |
| `CMakeLists.txt` | Build. Todo modulo nuevo se registra aqui |

## Compilar

En Windows con MinGW, que es el entorno de referencia:

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
cd .. && build/Main.exe
```

En Linux hace falta tener OpenGL, GLEW y freeglut instalados, y se usa el generador
por defecto en vez de `MinGW Makefiles`.

El ejecutable **debe lanzarse desde la raiz del repositorio**: las rutas a shaders,
texturas y modelos son relativas al directorio de trabajo.

Verificar que el proyecto compila antes de abrir un PR no es opcional.

## Convenciones de codigo

- Tabs para indentar y llaves en su propia linea, siguiendo el estilo ya existente.
- El estado del render vive en variables globales de `main.cpp`
  (`thirdPersonObj`, `objectYaw`, `cameraPosition`, `keys[]`). Encajate en ese
  patron en vez de reescribir la arquitectura, salvo que la tarea pida refactorizar.
- Codigo, nombres y mensajes de commit en ingles.
- Comenta solo lo que no se explica solo. Nada de comentarios obvios.

## Trampas conocidas de OpenGL

- Si desactivas estado global (`GL_DEPTH_TEST`, `GL_BLEND`, `glUseProgram`), **restauralo**
  antes de salir de la funcion. `display()` corre en cada frame y el estado se arrastra.
- Casa siempre la unidad de `glActiveTexture` con el valor del uniform correspondiente.
- Libera lo que crees: texturas, VAOs, VBOs y programas de shader.
- Nada de asignar memoria, leer archivos ni recompilar shaders dentro de `display()`.

## Que nunca commitear

- La carpeta `build/` ni ningun artefacto de compilacion.
- Binarios, ejecutables ni secretos.

## Ciclo de PRs automatizado

Los PRs contra `main` con las etiquetas `ai-generated` y `auto-merge` pasan por
revision automatica de Copilot, aprobacion y merge automatico en cuanto CI esta en
verde. Sin esas etiquetas, el PR espera a un humano. Los PRs del agente en la nube
reciben esas etiquetas solos, porque el workflow detecta al autor.

Escribe siempre `Closes #N` en el cuerpo del PR. GitHub no cierra el issue por su
cuenta, porque solo lo hace cuando el merge ocurre en la default branch y aqui la
default es `master`. De eso se encarga el workflow `close-linked-issues`.

Si una tarea resulta estar mal especificada o ser inviable, **para y dilo** en el
issue con la etiqueta `needs-human`. No improvises un diseno alternativo.

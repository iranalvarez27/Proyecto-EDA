# Proyecto-EDA

## Vecinos de igual tamaño en tiempo constante en octrees lineales: una aplicación a Marching Cubes

CS3014 Estructuras de Datos Avanzadas (UTEC). Integrantes: Irán María Alvarez Flores, Harold Canto Vidal, Luis Fernando Izaguirre Zavaleta.

Se compara el cálculo del vecino de igual tamaño con enteros dilatados (Schrack, 1992) frente a la reflexión de dígitos (Samet, 1989) sobre el mismo octree lineal, y su efecto en una variante de Marching Cubes que comparte vértices entre celdas.

## Avance

| Objetivo | Responsable | Hecho | Pendiente |
| --- | --- | --- | --- |
| 1. Cálculo de la dirección del vecino | Harold | Dilatación, contracción, suma dilatada, `schrack`, `samet` y `oracle`. Tests con los ejemplos del paper. Benchmark de peor caso y azar, caras y aristas (`bench_vecinos`). | Correr el benchmark, subir `results/obj1.csv` y hacer las figuras. |
| 2. Octree lineal y verificación de existencia | Luis | Octree lineal, búsqueda binaria y tabla hash, tests, benchmark con Schrack, Samet y oráculo para r = 4…10 en esfera y toro, figuras y sección del reporte (`docs/seccion_obj2.tex`). | Volver a correr el benchmark en la máquina común de la integración final. |
| 3. Marching Cubes con vértices compartidos | Irán | Celdas de superficie, baseline por celda, malla compartida, consultas de vecinos y benchmark (`main_mc`). | Pasar `schrack` y `samet` a `mcShared` (hoy `main_mc` usa su vecino de grilla), baseline de 3 vértices por triángulo, tests de malla y figuras. |
| 4. Verificación | todos | Schrack = Samet = oráculo con códigos aleatorios (`test_vecinos`) y en todas las consultas reales de Marching Cubes para r = 4…7 (`test_octree`). | Prueba conjunta: la malla compartida es idéntica con Schrack, Samet y el oráculo. |

Resultados del objetivo 2 con r = 10 (detalle en `docs/seccion_obj2.tex` y `results/obj2_resumen.csv`):

- La verificación de existencia es alrededor del 85 % del costo de dirección más existencia con búsqueda binaria.
- En las consultas de Marching Cubes, Schrack y Samet cuestan lo mismo (28–35 ns por dirección); el oráculo, 108–118 ns.
- La tabla hash hace la búsqueda completa casi el doble de rápido que la binaria (119–154 ns frente a 233–281 ns), pero construirla tarda 3.0 s en la esfera y 12.2 s en el toro, más de lo que ahorra en una sola pasada de Marching Cubes.

## Estructura

| Archivo | Contenido | Objetivo |
| --- | --- | --- |
| `include/octree.h` | `Code`, `Hoja`, direcciones, `encode`/`decode` y firmas comunes | todos |
| `include/superficies.h` | esfera, toro y test conservador `canCross` | todos |
| `include/vecinos.h`, `src/vecinos.cpp` | enteros dilatados, `schrack`, `samet`, `oracle` | 1 y 4 |
| `src/test_vecinos.cpp`, `src/bench_vecinos.cpp` | tests con los ejemplos del paper y benchmark | 1 y 4 |
| `include/octree_lineal.h`, `src/octree_lineal.cpp` | `construirOctree`, `hasLeaf` (binaria) y `hasLeafHash` | 2 |
| `src/test_octree.cpp`, `src/bench_octree.cpp` | tests del octree y benchmark | 2 |
| `scripts/graficar_obj2.py` | resumen y figuras del objetivo 2 | 2 |
| `include/marching_cubes.h`, `include/tablas.h` | tipos y tablas de Marching Cubes | 3 |
| `src/celdas_superficie.cpp`, `src/mc_baseline.cpp`, `src/mc_compartido.cpp`, `src/main_mc.cpp` | celdas de superficie, baseline, malla compartida y benchmark | 3 |

## Requisitos

- g++ con C++17 (probado con g++ 16.1 de MSYS2 en Windows 11).
- `make` (en Windows, `mingw32-make`).
- Python 3 con `matplotlib` para las figuras (`pip install matplotlib`).

## Compilar

```bash
make            # en Windows: mingw32-make
```

Los ejecutables quedan en `bin/`. Todo se compila con `-O2` y corre en un solo hilo. En Windows el Makefile enlaza con `-static`, para no depender de las DLL de MinGW que haya en el `PATH`.

Sin `make`, por ejemplo para el objetivo 2:

```bash
g++ -O2 -std=c++17 -Iinclude src/bench_octree.cpp src/octree_lineal.cpp src/vecinos.cpp src/celdas_superficie.cpp src/mc_compartido.cpp -o bin/bench_octree
```

## Tests

```bash
make test       # en Windows: mingw32-make test
```

Corre `bin/test_vecinos` (ejemplos del paper y Schrack = Samet = oráculo con códigos aleatorios) y `bin/test_octree`. Este último comprueba, para la esfera y el toro con r = 4…7:

1. el arreglo de hojas está ordenado y sin duplicados;
2. el volumen de todas las hojas suma 1 (en celdas de la grilla: la suma de 8^(r−l) es 8^r);
3. las celdas de superficie de Marching Cubes son hojas de nivel r;
4. las 4 celdas que comparten cada arista cortada existen;
5. `hasLeaf` y `hasLeafHash` coinciden con un `std::set` de referencia, en consultas positivas y negativas;
6. en las consultas reales de Marching Cubes, Schrack, Samet y el oráculo dan el mismo código y la vecina existe.

## Regenerar resultados y figuras

Se corren desde la raíz del repositorio. El argumento opcional es la profundidad máxima (por defecto 10).

| Objetivo | Comando | Salida |
| --- | --- | --- |
| 1 | `bin/bench_vecinos 10` | `results/obj1.csv` |
| 2 | `bin/bench_octree 10` y luego `python scripts/graficar_obj2.py` | `results/obj2.csv`, `results/obj2_resumen.csv`, `results/obj2_total.png`, `results/obj2_componentes.png` |
| 3 | `bin/mc 10` | `obj3.csv`, `esfera_r7.ply`, `toro_r7.ply` |

En PowerShell se escribe `bin\bench_octree.exe 10`. Con r = 10, el objetivo 2 usa unos 2 GB de RAM (el toro tiene 32.5 millones de hojas y la tabla hash las guarda todas). Para que los tiempos sean comparables, conviene correrlo con la laptop conectada a la corriente y sin otros programas pesados abiertos.

`bin/bench_octree 10 --consultas` además guarda en `results/consultas_<superficie>_r<r>.csv` la lista de consultas reales de Marching Cubes (celda, dirección y código de la vecina), para reutilizarla en los otros objetivos. Estos archivos no se suben al repositorio porque pesan varios MB y se regeneran con ese comando.

## Octree lineal y verificación de existencia (objetivo 2)

- **Construcción.** `construirOctree(superficie, r)` parte del cubo [0, 1]³ y visita las 8 hijas en orden Morton (`x + 2y + 4z`). Una celda que no pasa `canCross` (|F(centro)| > L · semidiagonal) no puede contener la superficie y se guarda como hoja grande; el resto se subdivide hasta el nivel r. Es el mismo test que usa `findCells`, por eso toda celda de superficie de Marching Cubes es una hoja de nivel r. El resultado es un `vector<Hoja>` ordenado por código, sin duplicados, que cubre todo el cubo.
- **Códigos.** Todas las hojas guardan su código con 3r bits; una hoja de nivel l < r tiene en cero sus 3(r − l) bits inferiores.
- **Existencia.** `hasLeaf` busca el par exacto (código, nivel) con `lower_bound`, en O(log N). `hasLeafHash` hace la misma consulta en un `unordered_set` con clave (código, nivel), en O(1) esperado, a cambio de memoria y del tiempo de construir la tabla. Una hoja de otro nivel no cuenta como vecina de igual tamaño.
- **Consultas.** El benchmark usa las mismas consultas que Marching Cubes (`buildQueries`): las vecinas por cara y por arista que necesita cada celda de superficie para compartir vértices. Antes de medir, verifica que Schrack, Samet y el oráculo devuelvan el mismo código y que la vecina exista en ambas estructuras.
- **Medición.** Para cada superficie, r = 4…10, método (Schrack, Samet, oráculo) y estructura (binaria, hash) se hacen 5 corridas. Cada corrida mide por separado el cálculo de la dirección, la verificación de existencia (sobre las vecinas ya calculadas) y la búsqueda completa. Si hay pocas consultas, el lote se repite hasta llegar a 2^20 consultas por medición; los tiempos se reportan en ns por consulta.

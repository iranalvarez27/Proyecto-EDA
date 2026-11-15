import csv
import statistics
from collections import defaultdict
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

CARPETA = Path("results")
SUPERFICIES = ["esfera", "toro"]
METODOS = ["schrack", "samet", "oraculo"]
COLOR = {"schrack": "#2a78d6", "samet": "#eb6834", "oraculo": "#1baf7a"}
GRIS = "#52514e"

TIEMPOS = ["t_por_celda", "t_sopa", "t_compartido", "t_direccion", "t_busqueda"]
CONTEOS = ["celdas", "v_sopa", "v_por_celda", "v_compartido", "triangulos"]

def cuartiles(valores):
    if len(valores) == 1:
        return valores[0], valores[0], valores[0]
    q1, mediana, q3 = statistics.quantiles(valores, n=4, method="inclusive")
    return q1, mediana, q3

def leer():
    grupos = defaultdict(list)
    with open(CARPETA / "obj3.csv", encoding="utf-8", newline="") as f:
        for fila in csv.DictReader(f):
            grupos[(fila["superficie"], int(fila["r"]), fila["metodo"])].append(fila)

    filas = []
    for (superficie, r, metodo), datos in sorted(grupos.items()):
        fila = {"superficie": superficie, "r": r, "metodo": metodo, "corridas": len(datos)}
        for columna in CONTEOS:
            fila[columna] = float(datos[0][columna])
        for columna in TIEMPOS:
            q1, mediana, q3 = cuartiles([float(x[columna]) for x in datos])
            fila[columna] = mediana
            fila[columna + "_q1"] = q1
            fila[columna + "_q3"] = q3
        fila["fraccion_direccion"] = fila["t_direccion"] / fila["t_compartido"]
        fila["fraccion_busqueda"] = fila["t_busqueda"] / fila["t_compartido"]
        filas.append(fila)
    return filas

def serie(filas, superficie, columna, metodo=None):
    puntos = sorted((f["r"], f[columna]) for f in filas
                    if f["superficie"] == superficie and (metodo is None or f["metodo"] == metodo))
    return [p[0] for p in puntos], [p[1] for p in puntos]

def estilo(ax, titulo, eje_x=True):
    ax.set_title(titulo, fontsize=11)
    if eje_x:
        ax.set_xlabel("Profundidad r")
    ax.grid(True, color="#e4e3df", linewidth=0.8)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

def guardar_resumen(filas):
    campos = ["superficie", "r", "celdas", "metodo", "corridas", "v_sopa", "v_por_celda", "v_compartido",
              "reduccion_vs_sopa", "reduccion_vs_por_celda", "t_sopa_s", "t_por_celda_s",
              "t_compartido_s", "t_compartido_iqr_s", "t_direccion_s", "t_busqueda_s",
              "fraccion_direccion", "fraccion_busqueda"]
    with open(CARPETA / "obj3_resumen.csv", "w", encoding="utf-8", newline="") as f:
        escritor = csv.DictWriter(f, fieldnames=campos)
        escritor.writeheader()
        for fila in sorted(filas, key=lambda x: (x["superficie"], x["r"], x["metodo"])):
            escritor.writerow({
                "superficie": fila["superficie"], "r": fila["r"], "celdas": int(fila["celdas"]),
                "metodo": fila["metodo"], "corridas": fila["corridas"],
                "v_sopa": int(fila["v_sopa"]), "v_por_celda": int(fila["v_por_celda"]),
                "v_compartido": int(fila["v_compartido"]),
                "reduccion_vs_sopa": round(fila["v_sopa"] / fila["v_compartido"], 3),
                "reduccion_vs_por_celda": round(fila["v_por_celda"] / fila["v_compartido"], 3),
                "t_sopa_s": f"{fila['t_sopa']:.6g}", "t_por_celda_s": f"{fila['t_por_celda']:.6g}",
                "t_compartido_s": f"{fila['t_compartido']:.6g}",
                "t_compartido_iqr_s": f"{fila['t_compartido_q3'] - fila['t_compartido_q1']:.6g}",
                "t_direccion_s": f"{fila['t_direccion']:.6g}", "t_busqueda_s": f"{fila['t_busqueda']:.6g}",
                "fraccion_direccion": round(fila["fraccion_direccion"], 4),
                "fraccion_busqueda": round(fila["fraccion_busqueda"], 4),
            })

def figura_vertices(filas):
    fig, ejes = plt.subplots(1, 2, figsize=(11, 4.2), sharey=True)
    for ax, superficie in zip(ejes, SUPERFICIES):
        for columna, etiqueta, color, marca in [("v_sopa", "3 por triángulo", "#c24a4a", "o"),
                                                ("v_por_celda", "compartidos dentro de la celda", GRIS, "s"),
                                                ("v_compartido", "compartidos entre celdas", COLOR["schrack"], "^")]:
            rs, vs = serie(filas, superficie, columna, "schrack")
            ax.plot(rs, vs, marker=marca, markersize=6, linewidth=2, color=color, label=etiqueta)
        ax.set_yscale("log")
        estilo(ax, superficie)
    ejes[0].set_ylabel("Vértices de la malla (escala log)")
    ejes[1].legend(frameon=False, fontsize=9, loc="upper left", bbox_to_anchor=(1.0, 1.0))
    fig.tight_layout()
    fig.savefig(CARPETA / "obj3_vertices.png", dpi=160)
    plt.close(fig)

def figura_tiempo(filas):
    fig, ejes = plt.subplots(1, 2, figsize=(11, 4.2), sharey=True)
    for ax, superficie in zip(ejes, SUPERFICIES):
        for metodo in METODOS:
            rs, ts = serie(filas, superficie, "t_compartido", metodo)
            ax.plot(rs, [t * 1000 for t in ts], marker="o", markersize=6, linewidth=2,
                    color=COLOR[metodo], label=f"compartida ({metodo})")
            _, q1 = serie(filas, superficie, "t_compartido_q1", metodo)
            _, q3 = serie(filas, superficie, "t_compartido_q3", metodo)
            ax.fill_between(rs, [t * 1000 for t in q1], [t * 1000 for t in q3],
                            color=COLOR[metodo], alpha=0.15, linewidth=0)
        rs, ts = serie(filas, superficie, "t_por_celda", "schrack")
        ax.plot(rs, [t * 1000 for t in ts], linestyle="--", marker="s", markersize=5, linewidth=1.5,
                color=GRIS, label="por celda")
        rs, ts = serie(filas, superficie, "t_sopa", "schrack")
        ax.plot(rs, [t * 1000 for t in ts], linestyle=":", marker="^", markersize=5, linewidth=1.5,
                color="#c24a4a", label="3 vértices por triángulo")
        ax.set_yscale("log")
        estilo(ax, superficie)
    ejes[0].set_ylabel("Tiempo total (ms, mediana, escala log)")
    ejes[1].legend(frameon=False, fontsize=9, loc="upper left", bbox_to_anchor=(1.0, 1.0))
    fig.tight_layout()
    fig.savefig(CARPETA / "obj3_tiempo.png", dpi=160)
    plt.close(fig)

def figura_fraccion(filas):
    fig, ejes = plt.subplots(1, 2, figsize=(11, 4.2), sharey=True)
    for ax, superficie in zip(ejes, SUPERFICIES):
        for metodo in METODOS:
            rs, fs = serie(filas, superficie, "fraccion_direccion", metodo)
            ax.plot(rs, [f * 100 for f in fs], marker="o", markersize=6, linewidth=2,
                    color=COLOR[metodo], label=f"{metodo}: solo dirección")
            rs, fs = serie(filas, superficie, "fraccion_busqueda", metodo)
            ax.plot(rs, [f * 100 for f in fs], linestyle="--", marker="s", markersize=5, linewidth=1.5,
                    color=COLOR[metodo], alpha=0.7, label=f"{metodo}: dirección + existencia")
        estilo(ax, superficie)
        ax.set_ylim(bottom=0)
    ejes[0].set_ylabel("% del tiempo de Marching Cubes compartido")
    ejes[1].legend(frameon=False, fontsize=8, loc="upper left", bbox_to_anchor=(1.0, 1.0))
    fig.tight_layout()
    fig.savefig(CARPETA / "obj3_fraccion.png", dpi=160)
    plt.close(fig)

def buscar(filas, superficie, r, metodo):
    for f in filas:
        if f["superficie"] == superficie and f["r"] == r and f["metodo"] == metodo:
            return f
    return None

def guardar_tablas(filas):
    todos = sorted(set(f["r"] for f in filas))
    rmin, rmax = todos[0], todos[-1]
    elegidos = sorted(set([rmin, (rmin + rmax) // 2, rmax]))
    lineas = ["# Objetivo 3: tablas generadas desde obj3.csv", "",
              "Los tiempos son la mediana de las corridas.", ""]

    for superficie in SUPERFICIES:
        lineas += [f"## Vértices de la malla ({superficie})", "",
                   "| r | Celdas | 3 por triángulo | Compartidos en la celda | Compartidos entre celdas |",
                   "|---|---|---|---|---|"]
        for r in elegidos:
            f = buscar(filas, superficie, r, "schrack")
            if f is None:
                continue
            lineas.append(f"| {r} | {int(f['celdas']):,} | {int(f['v_sopa']):,} | "
                          f"{int(f['v_por_celda']):,} | {int(f['v_compartido']):,} |")
        lineas.append("")

    lineas += [f"## Tiempo total con r = {rmax} (segundos, Schrack)", "",
               "| Superficie | 3 por triángulo | Compartidos en la celda | Compartidos entre celdas |",
               "|---|---|---|---|"]
    for superficie in SUPERFICIES:
        f = buscar(filas, superficie, rmax, "schrack")
        if f is None:
            continue
        lineas.append(f"| {superficie} | {f['t_sopa']:.3g} | {f['t_por_celda']:.3g} | {f['t_compartido']:.3g} |")

    lineas += ["", f"## Parte del tiempo de la malla compartida que se va en buscar vecinos (r = {rmax})", "",
               "| Superficie | Método | Solo la dirección | Dirección + existencia |",
               "|---|---|---|---|"]
    for superficie in SUPERFICIES:
        for metodo in METODOS:
            f = buscar(filas, superficie, rmax, metodo)
            if f is None:
                continue
            lineas.append(f"| {superficie} | {metodo} | {100 * f['fraccion_direccion']:.0f} % | "
                          f"{100 * f['fraccion_busqueda']:.0f} % |")
    lineas.append("")
    (CARPETA / "obj3_tablas.md").write_text("\n".join(lineas), encoding="utf-8")

def main():
    filas = leer()
    guardar_resumen(filas)
    guardar_tablas(filas)
    figura_vertices(filas)
    figura_tiempo(filas)
    figura_fraccion(filas)
    print("Generados: results/obj3_resumen.csv, results/obj3_tablas.md, results/obj3_vertices.png, "
          "results/obj3_tiempo.png, results/obj3_fraccion.png")

if __name__ == "__main__":
    main()

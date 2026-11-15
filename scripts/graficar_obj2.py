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
ESTRUCTURAS = ["binaria", "hash"]
COLOR = {"schrack": "#2a78d6", "samet": "#eb6834", "oraculo": "#1baf7a"}
LINEA = {"binaria": ("-", "o"), "hash": ("--", "s")}
COLUMNAS = ["ns_direccion", "ns_existencia", "ns_total"]

def cuartiles(valores):
    if len(valores) == 1:
        return valores[0], valores[0], valores[0]
    q1, mediana, q3 = statistics.quantiles(valores, n=4, method="inclusive")
    return q1, mediana, q3

def leer():
    grupos = defaultdict(list)
    with open(CARPETA / "obj2.csv", encoding="utf-8", newline="") as f:
        for fila in csv.DictReader(f):
            clave = (fila["superficie"], int(fila["r"]), fila["metodo"], fila["estructura"])
            grupos[clave].append(fila)

    resumen = []
    for (superficie, r, metodo, estructura), filas in sorted(grupos.items()):
        fila = {
            "superficie": superficie,
            "r": r,
            "hojas": int(filas[0]["hojas"]),
            "celdas": int(filas[0]["celdas"]),
            "consultas": int(filas[0]["consultas"]),
            "ms_octree": float(filas[0]["ms_octree"]),
            "ms_tabla": float(filas[0]["ms_tabla"]),
            "metodo": metodo,
            "estructura": estructura,
            "corridas": len(filas),
        }
        for columna in COLUMNAS:
            q1, mediana, q3 = cuartiles([float(x[columna]) for x in filas])
            fila[columna] = round(mediana, 2)
            fila[columna + "_iqr"] = round(q3 - q1, 2)
        resumen.append(fila)
    return resumen

def guardar(resumen):
    with open(CARPETA / "obj2_resumen.csv", "w", encoding="utf-8", newline="") as f:
        escritor = csv.DictWriter(f, fieldnames=list(resumen[0].keys()))
        escritor.writeheader()
        escritor.writerows(resumen)

def serie(resumen, superficie, columna, metodo=None, estructura=None):
    por_r = defaultdict(list)
    for fila in resumen:
        if fila["superficie"] != superficie:
            continue
        if metodo and fila["metodo"] != metodo:
            continue
        if estructura and fila["estructura"] != estructura:
            continue
        por_r[fila["r"]].append(fila[columna])
    rs = sorted(por_r)
    return rs, [statistics.mean(por_r[r]) for r in rs]

def estilo(ax, titulo, eje_x=True):
    ax.set_title(titulo, fontsize=11)
    if eje_x:
        ax.set_xlabel("Profundidad r")
    ax.grid(True, color="#e4e3df", linewidth=0.8)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

def figura_total(resumen):
    fig, ejes = plt.subplots(1, 2, figsize=(11, 4.2), sharey=True)
    for ax, superficie in zip(ejes, SUPERFICIES):
        for metodo in METODOS:
            for estructura in ESTRUCTURAS:
                rs, ns = serie(resumen, superficie, "ns_total", metodo, estructura)
                linea, marca = LINEA[estructura]
                ax.plot(rs, ns, linestyle=linea, marker=marca, markersize=6, linewidth=2,
                        color=COLOR[metodo], label=f"{metodo} + {estructura}")
        estilo(ax, superficie)
    ejes[0].set_ylim(bottom=0)
    ejes[0].set_ylabel("Búsqueda completa (ns por consulta, mediana)")
    ejes[1].legend(frameon=False, fontsize=9, loc="upper left", bbox_to_anchor=(1.0, 1.0))
    fig.tight_layout()
    fig.savefig(CARPETA / "obj2_total.png", dpi=160)
    plt.close(fig)

def figura_componentes(resumen):
    fig, ejes = plt.subplots(2, 2, figsize=(11, 7), sharex=True)
    for j, superficie in enumerate(SUPERFICIES):
        ax = ejes[0][j]
        for metodo in METODOS:
            rs, ns = serie(resumen, superficie, "ns_direccion", metodo=metodo)
            ax.plot(rs, ns, marker="o", markersize=6, linewidth=2, color=COLOR[metodo], label=metodo)
        estilo(ax, f"{superficie}: cálculo de la dirección", eje_x=False)
        ax.set_ylim(bottom=0)

        ax = ejes[1][j]
        for estructura in ESTRUCTURAS:
            rs, ns = serie(resumen, superficie, "ns_existencia", estructura=estructura)
            linea, marca = LINEA[estructura]
            ax.plot(rs, ns, linestyle=linea, marker=marca, markersize=6, linewidth=2,
                    color="#52514e", label=estructura)
        estilo(ax, f"{superficie}: verificación de existencia")
        ax.set_ylim(bottom=0)

    ejes[0][0].set_ylabel("ns por consulta (mediana)")
    ejes[1][0].set_ylabel("ns por consulta (mediana)")
    ejes[0][1].legend(frameon=False, fontsize=9)
    ejes[1][1].legend(frameon=False, fontsize=9)
    fig.tight_layout()
    fig.savefig(CARPETA / "obj2_componentes.png", dpi=160)
    plt.close(fig)

def buscar(resumen, superficie, r, metodo, estructura):
    for fila in resumen:
        if (fila["superficie"] == superficie and fila["r"] == r and
                fila["metodo"] == metodo and fila["estructura"] == estructura):
            return fila
    return None

def guardar_tablas(resumen):
    rmax = max(fila["r"] for fila in resumen)
    lineas = ["# Objetivo 2: tablas generadas desde obj2.csv", "",
              "Mediana de las corridas en ns por consulta; entre paréntesis, el rango intercuartílico.", "",
              f"## Búsqueda completa con r = {rmax}", "",
              "| Superficie | Hojas | Método | Estructura | Dirección | Existencia | Total | Existencia / total |",
              "|---|---|---|---|---|---|---|---|"]
    for superficie in SUPERFICIES:
        for metodo in METODOS:
            for estructura in ESTRUCTURAS:
                f = buscar(resumen, superficie, rmax, metodo, estructura)
                if f is None:
                    continue
                lineas.append(
                    f"| {superficie} | {f['hojas']:,} | {metodo} | {estructura} | "
                    f"{f['ns_direccion']:.0f} ({f['ns_direccion_iqr']:.0f}) | "
                    f"{f['ns_existencia']:.0f} ({f['ns_existencia_iqr']:.0f}) | "
                    f"{f['ns_total']:.0f} ({f['ns_total_iqr']:.0f}) | "
                    f"{100 * f['ns_existencia'] / f['ns_total']:.0f} % |")

    lineas += ["", "## Tamaño de cada octree", "",
               "| Superficie | r | Hojas N | Celdas de superficie M | Consultas | Construir octree (ms) | Construir tabla hash (ms) |",
               "|---|---|---|---|---|---|---|"]
    vistos = set()
    for fila in sorted(resumen, key=lambda x: (x["superficie"], x["r"])):
        clave = (fila["superficie"], fila["r"])
        if clave in vistos:
            continue
        vistos.add(clave)
        lineas.append(f"| {fila['superficie']} | {fila['r']} | {fila['hojas']:,} | {fila['celdas']:,} | "
                      f"{fila['consultas']:,} | {fila['ms_octree']:.1f} | {fila['ms_tabla']:.1f} |")
    lineas.append("")
    (CARPETA / "obj2_tablas.md").write_text("\n".join(lineas), encoding="utf-8")

def main():
    resumen = leer()
    guardar(resumen)
    guardar_tablas(resumen)
    figura_total(resumen)
    figura_componentes(resumen)
    print("Generados: results/obj2_resumen.csv, results/obj2_tablas.md, results/obj2_total.png, results/obj2_componentes.png")

if __name__ == "__main__":
    main()

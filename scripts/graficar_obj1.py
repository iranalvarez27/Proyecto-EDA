import csv
import statistics
import sys
from collections import defaultdict
from pathlib import Path


def cuartiles(valores):
    if len(valores) == 1:
        return valores[0], valores[0], valores[0]
    q1, mediana, q3 = statistics.quantiles(valores, n=4, method="inclusive")
    return q1, mediana, q3


def main():
    ruta = Path(sys.argv[1] if len(sys.argv) > 1 else "results/obj1.csv")
    if not ruta.exists():
        raise SystemExit(f"No existe {ruta}")
    carpeta = ruta.parent
    filas = list(csv.DictReader(ruta.open(encoding="utf-8", newline="")))
    grupos = defaultdict(list)
    for fila in filas:
        clave = int(fila["r"]), fila["caso"], fila["tipo"], fila["metodo"]
        grupos[clave].append(fila)

    resumen = []
    for (r, caso, tipo, metodo), datos in sorted(grupos.items()):
        q1, mediana, q3 = cuartiles([float(f["ns_por_consulta"]) for f in datos])
        resumen.append({
            "r": r,
            "caso": caso,
            "tipo": tipo,
            "metodo": metodo,
            "corridas": len(datos),
            "ns_por_consulta": round(mediana, 2),
            "ns_q1": round(q1, 2),
            "ns_q3": round(q3, 2),
            "ns_iqr": round(q3 - q1, 2),
            "digitos_reflejados": float(datos[0]["digitos_reflejados"]),
        })

    salida = carpeta / "obj1_resumen.csv"
    with salida.open("w", encoding="utf-8", newline="") as f:
        escritor = csv.DictWriter(f, fieldnames=list(resumen[0].keys()))
        escritor.writeheader()
        escritor.writerows(resumen)
    print("Resumen:", salida)

    guardar_tablas(resumen, carpeta / "obj1_tablas.md")

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("Matplotlib no instalado: resumen CSV guardado, sin figuras.")
        return

    for caso in sorted(set(f["caso"] for f in resumen)):
        for tipo in sorted(set(f["tipo"] for f in resumen)):
            fig, ax = plt.subplots(figsize=(8, 5))
            for metodo in sorted(set(f["metodo"] for f in resumen)):
                serie = sorted((f for f in resumen if f["caso"] == caso and
                                f["tipo"] == tipo and f["metodo"] == metodo),
                               key=lambda f: f["r"])
                rs = [f["r"] for f in serie]
                linea, = ax.plot(rs, [f["ns_por_consulta"] for f in serie], "o-", label=metodo)
                ax.fill_between(rs, [f["ns_q1"] for f in serie], [f["ns_q3"] for f in serie],
                                color=linea.get_color(), alpha=0.18, linewidth=0)
            ax.set_xlabel("Profundidad r")
            ax.set_ylabel("Tiempo por consulta (ns, mediana y rango intercuartílico)")
            ax.set_title(f"Objetivo 1: {tipo} / caso {caso}")
            ax.set_ylim(bottom=0)
            ax.grid(True, alpha=0.25)
            ax.legend()
            fig.tight_layout()
            png = carpeta / f"obj1_tiempo_{caso}_{tipo}.png"
            fig.savefig(png, dpi=160)
            plt.close(fig)
            print("Figura:", png)

    fig, ax = plt.subplots(figsize=(8, 5))
    for caso in sorted(set(f["caso"] for f in resumen)):
        for tipo in sorted(set(f["tipo"] for f in resumen)):
            serie = sorted((f for f in resumen if f["caso"] == caso and
                            f["tipo"] == tipo and f["metodo"] == "schrack"),
                           key=lambda f: f["r"])
            ax.plot([f["r"] for f in serie],
                    [f["digitos_reflejados"] for f in serie],
                    "o-" if tipo == "caras" else "s--", label=f"{caso} / {tipo}")
    ax.set_xlabel("Profundidad r")
    ax.set_ylabel("Dígitos reflejados por consulta (Samet)")
    ax.set_title("Objetivo 1: dígitos reflejados")
    ax.set_ylim(bottom=0)
    ax.grid(True, alpha=0.25)
    ax.legend()
    fig.tight_layout()
    png = carpeta / "obj1_digitos.png"
    fig.savefig(png, dpi=160)
    plt.close(fig)
    print("Figura:", png)


def buscar(resumen, r, caso, tipo, metodo):
    for f in resumen:
        if f["r"] == r and f["caso"] == caso and f["tipo"] == tipo and f["metodo"] == metodo:
            return f
    return None


def guardar_tablas(resumen, ruta):
    metodos = ["schrack", "samet", "oraculo"]
    todos = sorted(set(f["r"] for f in resumen))
    rmin, rmax = todos[0], todos[-1]
    elegidos = sorted(set([rmin, (rmin + rmax) // 2, rmax]))
    lineas = ["# Objetivo 1: tablas generadas desde obj1.csv", "",
              "Mediana de las corridas en ns por consulta; entre paréntesis, el rango intercuartílico.", ""]

    for tipo in ["caras", "aristas"]:
        lineas += [f"## Dirección del vecino por {tipo}", "",
                   "| Caso | r | Schrack | Samet | Oráculo | Dígitos que refleja Samet |",
                   "|---|---|---|---|---|---|"]
        for caso, nombre in [("peor", "Peor caso"), ("azar", "Azar")]:
            for r in elegidos:
                celdas = []
                digitos = ""
                for metodo in metodos:
                    f = buscar(resumen, r, caso, tipo, metodo)
                    if f is None:
                        celdas.append("-")
                        continue
                    celdas.append(f"{f['ns_por_consulta']:.0f} ({f['ns_iqr']:.0f})")
                    digitos = f"{f['digitos_reflejados']:.1f}"
                lineas.append(f"| {nombre} | {r} | " + " | ".join(celdas) + f" | {digitos} |")
        lineas.append("")

    lineas += ["## ¿Se observa el O(1)?", "",
               f"Cambio de la mediana entre r = {rmin} y r = {rmax} en el peor caso:", "",
               "| Tipo | Método | r mínimo (ns) | r máximo (ns) | Cociente |",
               "|---|---|---|---|---|"]
    for tipo in ["caras", "aristas"]:
        for metodo in metodos:
            a = buscar(resumen, rmin, "peor", tipo, metodo)
            b = buscar(resumen, rmax, "peor", tipo, metodo)
            if a is None or b is None:
                continue
            lineas.append(f"| {tipo} | {metodo} | {a['ns_por_consulta']:.0f} | {b['ns_por_consulta']:.0f} | "
                          f"{b['ns_por_consulta'] / a['ns_por_consulta']:.2f} |")
    lineas += ["", "Un cociente cercano a 1 indica tiempo constante en r; uno mayor indica que el costo crece con r.", ""]

    ruta.write_text("\n".join(lineas), encoding="utf-8")
    print("Tablas:", ruta)


if __name__ == "__main__":
    main()

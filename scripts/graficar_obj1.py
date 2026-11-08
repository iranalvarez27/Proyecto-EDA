#!/usr/bin/env python3
"""Promedia las 5 corridas del objetivo 1 y produce figuras PNG.

Uso: python3 scripts/graficar_obj1.py [results/obj1.csv]
Requiere matplotlib (pip install matplotlib).
"""
import csv
import statistics
import sys
from collections import defaultdict
from pathlib import Path


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
        resumen.append({
            "r": r,
            "caso": caso,
            "tipo": tipo,
            "metodo": metodo,
            "corridas": len(datos),
            "ns_por_consulta": statistics.mean(float(f["ns_por_consulta"]) for f in datos),
            "digitos_reflejados": float(datos[0]["digitos_reflejados"]),
        })

    salida = carpeta / "obj1_resumen.csv"
    with salida.open("w", encoding="utf-8", newline="") as f:
        escritor = csv.DictWriter(f, fieldnames=list(resumen[0].keys()))
        escritor.writeheader()
        escritor.writerows(resumen)
    print("Resumen:", salida)

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("Matplotlib no instalado: resumen CSV guardado, sin figuras.")
        return

    # Una figura por caso y tipo: tiempo por consulta de cada método.
    for caso in sorted(set(f["caso"] for f in resumen)):
        for tipo in sorted(set(f["tipo"] for f in resumen)):
            fig, ax = plt.subplots(figsize=(8, 5))
            for metodo in sorted(set(f["metodo"] for f in resumen)):
                serie = sorted((f for f in resumen if f["caso"] == caso and
                                f["tipo"] == tipo and f["metodo"] == metodo),
                               key=lambda f: f["r"])
                ax.plot([f["r"] for f in serie],
                        [f["ns_por_consulta"] for f in serie],
                        "o-", label=metodo)
            ax.set_xlabel("Profundidad r")
            ax.set_ylabel("Tiempo por consulta (ns)")
            ax.set_title(f"Objetivo 1: {tipo} / caso {caso}")
            ax.set_ylim(bottom=0)
            ax.grid(True, alpha=0.25)
            ax.legend()
            fig.tight_layout()
            png = carpeta / f"obj1_tiempo_{caso}_{tipo}.png"
            fig.savefig(png, dpi=160)
            plt.close(fig)
            print("Figura:", png)

    # Dígitos reflejados por Samet: no depende del método, se toma schrack.
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


if __name__ == "__main__":
    main()

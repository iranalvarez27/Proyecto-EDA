#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include "marching_cubes.h"

using namespace std;

const int REPETICIONES = 5;
const double TIEMPO_MINIMO = 0.05;

volatile long long sumidero = 0;

struct Metodo {
    const char* nombre;
    FuncionVecino funcion;
};

const Metodo metodos[] = {
    {"schrack", schrack},
    {"samet", samet},
    {"oraculo", oracle},
};
const int TOTAL_METODOS = sizeof(metodos) / sizeof(metodos[0]);

void savePly(const Malla& malla, const string& archivo) {
    ofstream ply(archivo);
    if (!ply.is_open()) {
        cerr << "Error: no se pudo escribir " << archivo << '\n';
        return;
    }

    ply << "ply\n";
    ply << "format ascii 1.0\n";
    ply << "element vertex " << malla.vertices.size() << "\n";
    ply << "property float x\n";
    ply << "property float y\n";
    ply << "property float z\n";
    ply << "element face " << malla.caras.size() << "\n";
    ply << "property list uchar int vertex_indices\n";
    ply << "end_header\n";

    for (int i = 0; i < (int)malla.vertices.size(); i++) {
        ply << malla.vertices[i].x << " " << malla.vertices[i].y << " " << malla.vertices[i].z << "\n";
    }
    for (int i = 0; i < (int)malla.caras.size(); i++) {
        ply << "3 " << malla.caras[i][0] << " " << malla.caras[i][1] << " " << malla.caras[i][2] << "\n";
    }
}

double elapsed(chrono::steady_clock::time_point inicio) {
    chrono::duration<double> transcurrido = chrono::steady_clock::now() - inicio;
    return transcurrido.count();
}

template <typename F>
double medir(F f) {
    int veces = 0;
    auto inicio = chrono::steady_clock::now();
    do {
        sumidero = sumidero + f();
        veces++;
    } while (elapsed(inicio) < TIEMPO_MINIMO);
    return elapsed(inicio) / veces;
}

template <typename F>
double medirPromedio(F f) {
    double mejor = medir(f);
    for (int rep = 1; rep < REPETICIONES; rep++) {
        mejor = min(mejor, medir(f));
    }
    return mejor;
}

int main(int argc, char* argv[]) {
    int rMaximo = 10;
    if (argc > 1) {
        rMaximo = atoi(argv[1]);
    }

    ofstream csv("results/obj3.csv");
    if (!csv.is_open()) {
        cerr << "Error: no se pudo escribir results/obj3.csv (correr desde la raiz del repositorio)\n";
        return 1;
    }
    csv << "superficie,r,celdas,metodo,t_por_celda,t_sopa,t_compartido,t_direccion,t_busqueda,"
        << "fraccion_direccion,fraccion_busqueda,v_sopa,v_por_celda,v_compartido,triangulos\n";

    Superficie superficies[2] = {ESFERA, TORO};

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= rMaximo; r++) {
            vector<Hoja> celdas = findCells(superficies[s], r);
            vector<Consulta> consultas = buildQueries(superficies[s], r, celdas);

            Malla sopa, porCelda;
            double tSopa = medirPromedio([&]() { mcSoup(superficies[s], r, celdas, sopa); return (long long)sopa.vertices.size(); });
            double tPorCelda = medirPromedio([&]() { mcPerCell(superficies[s], r, celdas, porCelda); return (long long)porCelda.vertices.size(); });

            for (int m = 0; m < TOTAL_METODOS; m++) {
                Malla compartida;
                int fallasVecino = 0;
                FuncionVecino f = metodos[m].funcion;

                double tCompartido = medirPromedio([&]() {
                    fallasVecino = mcShared(superficies[s], r, celdas, f, compartida);
                    return (long long)compartida.vertices.size();
                });
                double tDireccion = medirPromedio([&]() { return runDirecciones(r, celdas, consultas, f); });
                double tBusqueda = medirPromedio([&]() { return runQueries(r, celdas, consultas, f); });

                if (fallasVecino != 0) {
                    cout << "ADVERTENCIA: " << fallasVecino << " vecinas no encontradas\n";
                }

                double fDireccion = tDireccion / tCompartido;
                double fBusqueda = tBusqueda / tCompartido;

                csv << superficies[s].nombre << "," << r << "," << celdas.size() << ","
                    << metodos[m].nombre << ","
                    << tPorCelda << "," << tSopa << "," << tCompartido << ","
                    << tDireccion << "," << tBusqueda << ","
                    << fDireccion << "," << fBusqueda << ","
                    << sopa.vertices.size() << "," << porCelda.vertices.size() << ","
                    << compartida.vertices.size() << "," << compartida.caras.size() << "\n";

                cout << superficies[s].nombre << " r=" << r
                     << " celdas=" << celdas.size()
                     << " metodo=" << metodos[m].nombre
                     << " t_compartido=" << tCompartido << "s"
                     << " direccion=" << fDireccion * 100 << "%"
                     << " direccion+existencia=" << fBusqueda * 100 << "%"
                     << " vertices " << sopa.vertices.size()
                     << " / " << porCelda.vertices.size()
                     << " / " << compartida.vertices.size() << "\n";

                if (r == 7 && m == 0) {
                    savePly(compartida, "results/" + string(superficies[s].nombre) + "_r7.ply");
                }
            }
        }
    }

    cout << "Resultados en results/obj3.csv\n";
    return 0;
}

#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "marching_cubes.h"

using namespace std;

const int CORRIDAS = 11;
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
    csv << "superficie,r,celdas,metodo,corrida,t_por_celda,t_sopa,t_compartido,t_direccion,t_busqueda,"
        << "v_sopa,v_por_celda,v_compartido,triangulos\n";

    Superficie superficies[2] = {ESFERA, TORO};

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= rMaximo; r++) {
            vector<Hoja> celdas = findCells(superficies[s], r);
            vector<Consulta> consultas = buildQueries(superficies[s], r, celdas);

            Malla sopa, porCelda;
            vector<Malla> compartida(TOTAL_METODOS);
            int fallasVecino = 0;

            auto pasadaSopa = [&]() {
                mcSoup(superficies[s], r, celdas, sopa);
                return (long long)sopa.vertices.size();
            };
            auto pasadaPorCelda = [&]() {
                mcPerCell(superficies[s], r, celdas, porCelda);
                return (long long)porCelda.vertices.size();
            };

            sumidero = sumidero + pasadaSopa() + pasadaPorCelda();
            for (int m = 0; m < TOTAL_METODOS; m++) {
                fallasVecino = fallasVecino + mcShared(superficies[s], r, celdas, metodos[m].funcion, compartida[m]);
                sumidero = sumidero + runDirecciones(r, celdas, consultas, metodos[m].funcion);
                sumidero = sumidero + runQueries(r, celdas, consultas, metodos[m].funcion);
            }

            if (fallasVecino != 0) {
                cout << "ADVERTENCIA: " << fallasVecino << " vecinas no encontradas\n";
            }

            for (int corrida = 1; corrida <= CORRIDAS; corrida++) {
                double tSopa = medir(pasadaSopa);
                double tPorCelda = medir(pasadaPorCelda);

                for (int m = 0; m < TOTAL_METODOS; m++) {
                    FuncionVecino f = metodos[m].funcion;

                    double tCompartido = medir([&]() {
                        mcShared(superficies[s], r, celdas, f, compartida[m]);
                        return (long long)compartida[m].vertices.size();
                    });
                    double tDireccion = medir([&]() { return runDirecciones(r, celdas, consultas, f); });
                    double tBusqueda = medir([&]() { return runQueries(r, celdas, consultas, f); });

                    csv << superficies[s].nombre << "," << r << "," << celdas.size() << ","
                        << metodos[m].nombre << "," << corrida << ","
                        << tPorCelda << "," << tSopa << "," << tCompartido << ","
                        << tDireccion << "," << tBusqueda << ","
                        << sopa.vertices.size() << "," << porCelda.vertices.size() << ","
                        << compartida[m].vertices.size() << "," << compartida[m].caras.size() << "\n";
                }
            }

            cout << superficies[s].nombre << " r=" << r << " celdas=" << celdas.size()
                 << " vertices " << sopa.vertices.size() << " / " << porCelda.vertices.size()
                 << " / " << compartida[0].vertices.size() << "\n";

            if (r == 7) {
                savePly(compartida[0], "results/" + string(superficies[s].nombre) + "_r7.ply");
            }
        }
    }

    cout << "Resultados en results/obj3.csv\n";
    return 0;
}

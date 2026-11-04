#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "marching_cubes.h"

using namespace std;

const int REPETICIONES = 5;

struct Metodo {
    const char* nombre;
    FuncionVecino funcion;
};

const Metodo metodos[] = {
    {"oraculo", oracle},
};
const int TOTAL_METODOS = sizeof(metodos) / sizeof(metodos[0]);

double elapsed(chrono::steady_clock::time_point inicio) {
    chrono::duration<double> transcurrido = chrono::steady_clock::now() - inicio;
    return transcurrido.count();
}

int main(int argc, char* argv[]) {
    int rMaximo = 10;
    if (argc > 1) {
        rMaximo = atoi(argv[1]);
    }

    ofstream csv("obj3.csv");
    csv << "superficie,r,celdas,metodo,t_por_celda,t_compartido,t_vecinos,fraccion_vecinos,"
        << "v_por_celda,v_compartido,triangulos\n";

    Superficie superficies[2] = {ESFERA, TORO};

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= rMaximo; r++) {
            vector<Hoja> celdas = findCells(superficies[s], r);
            vector<Consulta> consultas = buildQueries(superficies[s], r, celdas);

            Malla porCelda;
            double tPorCelda = 0;
            for (int rep = 0; rep < REPETICIONES; rep++) {
                auto inicio = chrono::steady_clock::now();
                mcPerCell(superficies[s], r, celdas, porCelda);
                tPorCelda = tPorCelda + elapsed(inicio);
            }
            tPorCelda = tPorCelda / REPETICIONES;

            for (int m = 0; m < TOTAL_METODOS; m++) {
                Malla compartida;
                double tCompartido = 0;
                double tVecinos = 0;
                long long checksum = 0;
                int fallasVecino = 0;

                for (int rep = 0; rep < REPETICIONES; rep++) {
                    auto inicio = chrono::steady_clock::now();
                    fallasVecino = mcShared(superficies[s], r, celdas, metodos[m].funcion, compartida);
                    tCompartido = tCompartido + elapsed(inicio);

                    inicio = chrono::steady_clock::now();
                    checksum = checksum + runQueries(r, celdas, consultas, metodos[m].funcion);
                    tVecinos = tVecinos + elapsed(inicio);
                }
                tCompartido = tCompartido / REPETICIONES;
                tVecinos = tVecinos / REPETICIONES;

                if (fallasVecino != 0) {
                    cout << "ADVERTENCIA: " << fallasVecino << " vecinas no encontradas\n";
                }

                double fraccion = tVecinos / tCompartido;

                csv << superficies[s].nombre << "," << r << "," << celdas.size() << ","
                    << metodos[m].nombre << ","
                    << tPorCelda << "," << tCompartido << ","
                    << tVecinos << "," << fraccion << ","
                    << porCelda.vertices.size() << ","
                    << compartida.vertices.size() << "," << compartida.caras.size() << "\n";

                cout << superficies[s].nombre << " r=" << r
                     << " celdas=" << celdas.size()
                     << " metodo=" << metodos[m].nombre
                     << " t_compartido=" << tCompartido << "s"
                     << " vecinos=" << fraccion * 100 << "%"
                     << " vertices " << porCelda.vertices.size()
                     << " / " << compartida.vertices.size()
                     << " (checksum " << checksum << ")\n";

                if (r == 7 && m == 0) {
                    savePly(compartida, string(superficies[s].nombre) + "_r7.ply");
                }
            }
        }
    }

    cout << "Resultados en obj3.csv\n";
    return 0;
}

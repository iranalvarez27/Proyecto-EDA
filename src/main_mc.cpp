#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include "marching_cubes.h"

using namespace std;

const int REPETICIONES = 5;

bool neighborGrid(Code c, int nivel, int direccion, int r, Code& salida) {
    uint32_t x, y, z;
    decode(c, r, x, y, z);

    long long paso = 1LL << (r - nivel);
    long long limite = 1LL << r;

    long long nuevoX = (long long)x + desplazamiento[direccion][0] * paso;
    long long nuevoY = (long long)y + desplazamiento[direccion][1] * paso;
    long long nuevoZ = (long long)z + desplazamiento[direccion][2] * paso;

    if (nuevoX < 0 || nuevoX >= limite) return false;
    if (nuevoY < 0 || nuevoY >= limite) return false;
    if (nuevoZ < 0 || nuevoZ >= limite) return false;

    salida = encode(nuevoX, nuevoY, nuevoZ, r);
    return true;
}

struct Metodo {
    const char* nombre;
    FuncionVecino funcion;
};

const Metodo metodos[] = {
    {"grilla", neighborGrid},
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

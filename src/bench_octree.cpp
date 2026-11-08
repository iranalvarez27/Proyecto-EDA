#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <filesystem>
#include <cstdlib>
#include "octree_lineal.h"
#include "marching_cubes.h"
#include "vecinos.h"

using namespace std;

const int CORRIDAS = 5;
const long long MINIMO_CONSULTAS = 1 << 20;

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

const char* estructuras[2] = {"binaria", "hash"};

struct ConsultaVecino {
    Code codigo;
    int direccion;
};

double elapsedNs(chrono::steady_clock::time_point inicio) {
    chrono::duration<double, nano> transcurrido = chrono::steady_clock::now() - inicio;
    return transcurrido.count();
}

bool exists(bool usarHash, const vector<Hoja>& hojas, const TablaHojas& tabla, Code c, int r) {
    if (usarHash) {
        return hasLeafHash(tabla, c, r);
    }
    return hasLeaf(hojas, c, r);
}

Code runDirection(int r, const vector<ConsultaVecino>& consultas, int vueltas, FuncionVecino vecino) {
    Code checksum = 0;
    for (int v = 0; v < vueltas; v++) {
        for (int i = 0; i < (int)consultas.size(); i++) {
            Code salida = 0;
            if (vecino(consultas[i].codigo, r, consultas[i].direccion, r, salida)) {
                checksum = checksum ^ salida;
            }
        }
    }
    return checksum;
}

Code runExistence(int r, const vector<Code>& vecinas, int vueltas, bool usarHash,
                  const vector<Hoja>& hojas, const TablaHojas& tabla) {
    Code encontradas = 0;
    for (int v = 0; v < vueltas; v++) {
        for (int i = 0; i < (int)vecinas.size(); i++) {
            encontradas = encontradas + exists(usarHash, hojas, tabla, vecinas[i], r);
        }
    }
    return encontradas;
}

Code runFull(int r, const vector<ConsultaVecino>& consultas, int vueltas, FuncionVecino vecino,
             bool usarHash, const vector<Hoja>& hojas, const TablaHojas& tabla) {
    Code encontradas = 0;
    for (int v = 0; v < vueltas; v++) {
        for (int i = 0; i < (int)consultas.size(); i++) {
            Code salida = 0;
            if (vecino(consultas[i].codigo, r, consultas[i].direccion, r, salida)) {
                encontradas = encontradas + exists(usarHash, hojas, tabla, salida, r);
            }
        }
    }
    return encontradas;
}

int countErrors(int r, const vector<ConsultaVecino>& consultas, const vector<Hoja>& hojas,
                const TablaHojas& tabla, vector<Code>& vecinas) {
    int errores = 0;
    vecinas.assign(consultas.size(), 0);
    for (int i = 0; i < (int)consultas.size(); i++) {
        Code o = 0;
        bool dentro = oracle(consultas[i].codigo, r, consultas[i].direccion, r, o);
        if (!dentro || !hasLeaf(hojas, o, r) || !hasLeafHash(tabla, o, r)) {
            errores = errores + 1;
            continue;
        }
        for (int m = 0; m < TOTAL_METODOS; m++) {
            Code salida = 0;
            if (!metodos[m].funcion(consultas[i].codigo, r, consultas[i].direccion, r, salida) || salida != o) {
                errores = errores + 1;
            }
        }
        vecinas[i] = o;
    }
    return errores;
}

void saveQueries(const Superficie& s, int r, const vector<ConsultaVecino>& consultas,
                 const vector<Code>& vecinas) {
    string archivo = "results/consultas_" + string(s.nombre) + "_r" + to_string(r) + ".csv";
    ofstream salida(archivo);
    salida << "superficie,r,codigo,nivel,direccion,codigo_vecina\n";
    for (int i = 0; i < (int)consultas.size(); i++) {
        salida << s.nombre << "," << r << "," << consultas[i].codigo << "," << r << ","
               << consultas[i].direccion << "," << vecinas[i] << "\n";
    }
}

int main(int argc, char* argv[]) {
    int rMaximo = 10;
    bool exportar = false;
    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "--consultas") {
            exportar = true;
        } else {
            rMaximo = atoi(argv[i]);
        }
    }

    filesystem::create_directories("results");
    ofstream csv("results/obj2.csv");
    csv << "superficie,r,hojas,celdas,consultas,vueltas,ms_octree,ms_tabla,"
        << "metodo,estructura,corrida,ns_direccion,ns_existencia,ns_total\n";

    Superficie superficies[2] = {ESFERA, TORO};
    Code checksum = 0;

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= rMaximo; r++) {
            auto inicio = chrono::steady_clock::now();
            vector<Hoja> hojas = construirOctree(superficies[s], r);
            double msOctree = elapsedNs(inicio) / 1e6;

            inicio = chrono::steady_clock::now();
            TablaHojas tabla = construirTabla(hojas);
            double msTabla = elapsedNs(inicio) / 1e6;

            vector<Hoja> celdas = findCells(superficies[s], r);
            vector<Consulta> base = buildQueries(superficies[s], r, celdas);
            vector<ConsultaVecino> consultas(base.size());
            for (int i = 0; i < (int)base.size(); i++) {
                consultas[i].codigo = celdas[base[i].celda].codigo;
                consultas[i].direccion = base[i].direccion;
            }

            vector<Code> vecinas;
            int errores = countErrors(r, consultas, hojas, tabla, vecinas);
            if (errores != 0) {
                cout << "ERROR: " << errores << " consultas fallaron en " << superficies[s].nombre
                     << " r=" << r << "\n";
                return 1;
            }
            if (exportar) {
                saveQueries(superficies[s], r, consultas, vecinas);
            }

            int vueltas = 1;
            if ((long long)consultas.size() < MINIMO_CONSULTAS) {
                vueltas = (MINIMO_CONSULTAS + consultas.size() - 1) / consultas.size();
            }
            double total = (double)consultas.size() * vueltas;

            cout << superficies[s].nombre << " r=" << r << " hojas=" << hojas.size()
                 << " celdas=" << celdas.size() << " consultas=" << consultas.size()
                 << " octree=" << msOctree << "ms tabla=" << msTabla << "ms\n";

            for (int m = 0; m < TOTAL_METODOS; m++) {
                for (int e = 0; e < 2; e++) {
                    bool usarHash = (e == 1);
                    double sumaTotal = 0;

                    for (int corrida = 1; corrida <= CORRIDAS; corrida++) {
                        inicio = chrono::steady_clock::now();
                        checksum = checksum ^ runDirection(r, consultas, vueltas, metodos[m].funcion);
                        double nsDireccion = elapsedNs(inicio) / total;

                        inicio = chrono::steady_clock::now();
                        checksum = checksum + runExistence(r, vecinas, vueltas, usarHash, hojas, tabla);
                        double nsExistencia = elapsedNs(inicio) / total;

                        inicio = chrono::steady_clock::now();
                        checksum = checksum + runFull(r, consultas, vueltas, metodos[m].funcion, usarHash, hojas, tabla);
                        double nsTotal = elapsedNs(inicio) / total;
                        sumaTotal = sumaTotal + nsTotal;

                        csv << superficies[s].nombre << "," << r << "," << hojas.size() << ","
                            << celdas.size() << "," << consultas.size() << "," << vueltas << ","
                            << msOctree << "," << msTabla << "," << metodos[m].nombre << ","
                            << estructuras[e] << "," << corrida << "," << nsDireccion << ","
                            << nsExistencia << "," << nsTotal << "\n";
                    }

                    cout << "  " << metodos[m].nombre << " + " << estructuras[e] << ": "
                         << sumaTotal / CORRIDAS << " ns/consulta\n";
                }
            }
        }
    }

    cout << "Resultados en results/obj2.csv (checksum " << checksum << ")\n";
    return 0;
}

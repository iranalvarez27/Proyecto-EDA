#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <filesystem>
#include <cstdlib>
#include "vecinos.h"

using namespace std;

const int CORRIDAS = 5;
const int TOTAL_CONSULTAS = 1 << 20;

struct ConsultaVecino {
    Code codigo;
    int direccion;
};

typedef bool (*FuncionVecino)(Code c, int nivel, int direccion, int r, Code& salida);

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

uint32_t coordenada(int d, bool peor, int r, mt19937_64& generador) {
    if (peor && d == 1) return (1u << (r - 1)) - 1;
    if (peor && d == -1) return 1u << (r - 1);
    return generador() % (1u << r);
}

vector<ConsultaVecino> buildNeighborQueries(int r, bool peor, bool caras, mt19937_64& generador) {
    vector<ConsultaVecino> consultas(TOTAL_CONSULTAS);
    for (int i = 0; i < TOTAL_CONSULTAS; i++) {
        int direccion;
        if (caras) {
            direccion = generador() % TOTAL_CARAS;
        } else {
            direccion = TOTAL_CARAS + generador() % (TOTAL_DIRECCIONES - TOTAL_CARAS);
        }

        uint32_t x = coordenada(desplazamiento[direccion][0], peor, r, generador);
        uint32_t y = coordenada(desplazamiento[direccion][1], peor, r, generador);
        uint32_t z = coordenada(desplazamiento[direccion][2], peor, r, generador);

        consultas[i].codigo = encode(x, y, z, r);
        consultas[i].direccion = direccion;
    }
    return consultas;
}

Code runNeighborQueries(int r, const vector<ConsultaVecino>& consultas, FuncionVecino vecino) {
    Code checksum = 0;
    for (int i = 0; i < (int)consultas.size(); i++) {
        Code salida = 0;
        if (vecino(consultas[i].codigo, r, consultas[i].direccion, r, salida)) {
            checksum = checksum ^ salida;
        }
    }
    return checksum;
}

int main(int argc, char* argv[]) {
    int rMaximo = 10;
    if (argc > 1) {
        rMaximo = atoi(argv[1]);
    }

    filesystem::create_directories("results");
    ofstream csv("results/obj1.csv");
    csv << "r,caso,tipo,metodo,corrida,ns_por_consulta,digitos_reflejados\n";

    mt19937_64 generador(2026);
    Code checksum = 0;

    for (int r = 4; r <= rMaximo; r++) {
        for (int p = 0; p < 2; p++) {
            for (int t = 0; t < 2; t++) {
                bool peor = (p == 1);
                bool caras = (t == 0);
                const char* caso = peor ? "peor" : "azar";
                const char* tipo = caras ? "caras" : "aristas";

                vector<ConsultaVecino> consultas = buildNeighborQueries(r, peor, caras, generador);

                digitosReflejados = 0;
                runNeighborQueries(r, consultas, samet);
                double digitos = (double)digitosReflejados / TOTAL_CONSULTAS;

                for (int m = 0; m < TOTAL_METODOS; m++) {
                    for (int corrida = 1; corrida <= CORRIDAS; corrida++) {
                        auto inicio = chrono::steady_clock::now();
                        checksum = checksum ^ runNeighborQueries(r, consultas, metodos[m].funcion);
                        chrono::duration<double, nano> transcurrido = chrono::steady_clock::now() - inicio;
                        double ns = transcurrido.count() / TOTAL_CONSULTAS;

                        csv << r << "," << caso << "," << tipo << "," << metodos[m].nombre << ","
                            << corrida << "," << ns << "," << digitos << "\n";
                    }
                }

                cout << "r=" << r << " " << caso << " " << tipo
                     << " digitos=" << digitos << "\n";
            }
        }
    }

    cout << "Resultados en results/obj1.csv (checksum " << checksum << ")\n";
    return 0;
}

#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <algorithm>
#include "marching_cubes.h"
#include "vecinos.h"

using namespace std;

int fallas = 0;

void check(bool condicion, const string& nombre) {
    if (condicion) {
        cout << "OK     " << nombre << "\n";
    } else {
        cout << "FALLA  " << nombre << "\n";
        fallas = fallas + 1;
    }
}

typedef tuple<long long, long long, long long> Coord;

long long cuantizar(double v) {
    return llround(v * 1e9);
}
typedef array<Coord, 3> Triangulo;

bool mallasIguales(const Malla& a, const Malla& b) {
    if (a.vertices.size() != b.vertices.size()) return false;
    if (a.caras.size() != b.caras.size()) return false;
    for (int i = 0; i < (int)a.vertices.size(); i++) {
        if (a.vertices[i].x != b.vertices[i].x) return false;
        if (a.vertices[i].y != b.vertices[i].y) return false;
        if (a.vertices[i].z != b.vertices[i].z) return false;
    }
    return a.caras == b.caras;
}

bool indicesValidos(const Malla& m) {
    int n = m.vertices.size();
    for (int i = 0; i < (int)m.caras.size(); i++) {
        for (int k = 0; k < 3; k++) {
            if (m.caras[i][k] < 0 || m.caras[i][k] >= n) return false;
        }
    }
    return true;
}

bool sinVerticesHuerfanos(const Malla& m) {
    vector<bool> usado(m.vertices.size(), false);
    for (int i = 0; i < (int)m.caras.size(); i++) {
        for (int k = 0; k < 3; k++) usado[m.caras[i][k]] = true;
    }
    for (int i = 0; i < (int)usado.size(); i++) {
        if (!usado[i]) return false;
    }
    return true;
}

int aristasNoCerradas(const Malla& m) {
    map<pair<int, int>, int> cuenta;
    for (int i = 0; i < (int)m.caras.size(); i++) {
        for (int k = 0; k < 3; k++) {
            int a = m.caras[i][k];
            int b = m.caras[i][(k + 1) % 3];
            cuenta[make_pair(min(a, b), max(a, b))]++;
        }
    }
    int malas = 0;
    for (auto it = cuenta.begin(); it != cuenta.end(); ++it) {
        if (it->second != 2) malas++;
    }
    return malas;
}

vector<Triangulo> triangulosGeometricos(const Malla& m) {
    vector<Triangulo> resultado;
    for (int i = 0; i < (int)m.caras.size(); i++) {
        Triangulo t;
        for (int k = 0; k < 3; k++) {
            const Punto& p = m.vertices[m.caras[i][k]];
            t[k] = Coord(cuantizar(p.x), cuantizar(p.y), cuantizar(p.z));
        }
        sort(t.begin(), t.end());
        resultado.push_back(t);
    }
    sort(resultado.begin(), resultado.end());
    return resultado;
}

struct Metodo {
    const char* nombre;
    FuncionVecino funcion;
};

const Metodo metodos[] = {
    {"schrack", schrack},
    {"samet", samet},
    {"oraculo", oracle},
};

int main() {
    Superficie superficies[2] = {ESFERA, TORO};

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= 7; r++) {
            string pref = string(superficies[s].nombre) + " r=" + to_string(r) + ": ";

            vector<Hoja> celdas = findCells(superficies[s], r);

            Malla sopa, porCelda;
            mcSoup(superficies[s], r, celdas, sopa);
            mcPerCell(superficies[s], r, celdas, porCelda);

            Malla compartida[3];
            int sinVecina[3];
            for (int m = 0; m < 3; m++) {
                sinVecina[m] = mcShared(superficies[s], r, celdas, metodos[m].funcion, compartida[m]);
            }

            check(sinVecina[0] == 0 && sinVecina[1] == 0 && sinVecina[2] == 0,
                  pref + "ninguna arista queda sin vértice (todas las vecinas se encuentran)");

            check(mallasIguales(compartida[0], compartida[1]) && mallasIguales(compartida[0], compartida[2]),
                  pref + "malla compartida idéntica con Schrack, Samet y el oráculo");

            check(indicesValidos(compartida[0]), pref + "índices de caras válidos");
            check(sinVerticesHuerfanos(compartida[0]), pref + "no hay vértices sin usar");

            check(aristasNoCerradas(compartida[0]) == 0, pref + "malla cerrada: cada arista está en 2 triángulos");

            check(compartida[0].caras.size() == sopa.caras.size() && compartida[0].caras.size() == porCelda.caras.size(),
                  pref + "misma cantidad de triángulos en las tres versiones");

            check(triangulosGeometricos(compartida[0]) == triangulosGeometricos(sopa),
                  pref + "los triángulos compartidos coinciden con la sopa (coordenadas a 1e-9)");

            check(sopa.vertices.size() == 3 * sopa.caras.size(), pref + "la sopa tiene 3 vértices por triángulo");
            check(compartida[0].vertices.size() < porCelda.vertices.size()
                  && porCelda.vertices.size() < sopa.vertices.size(),
                  pref + "vértices: compartida < por celda < sopa");
        }
    }

    if (fallas == 0) {
        cout << "Todos los tests de malla pasaron\n";
        return 0;
    }
    cout << fallas << " tests de malla fallaron\n";
    return 1;
}

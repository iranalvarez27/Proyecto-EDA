#include <cmath>
#include "marching_cubes.h"
#include "tablas.h"

using namespace std;

void getCorners(const Superficie& s, uint32_t x, uint32_t y, uint32_t z, int r,
                     Punto posiciones[8], double valores[8]) {
    double lado = 1.0 / (1 << r);

    for (int i = 0; i < 8; i++) {
        posiciones[i].x = (x + vertexOffset[i][0]) * lado;
        posiciones[i].y = (y + vertexOffset[i][1]) * lado;
        posiciones[i].z = (z + vertexOffset[i][2]) * lado;
        valores[i] = s.funcion(posiciones[i].x, posiciones[i].y, posiciones[i].z);
    }
}

int cubeIndex(const double valores[8]) {
    int indice = 0;
    for (int i = 0; i < 8; i++) {
        if (valores[i] < 0) {
            indice = indice | (1 << i);
        }
    }
    return indice;
}

Punto interpolate(const Punto& p1, const Punto& p2, double v1, double v2) {
    double t = v1 / (v1 - v2);

    Punto p;
    p.x = p1.x + (p2.x - p1.x) * t;
    p.y = p1.y + (p2.y - p1.y) * t;
    p.z = p1.z + (p2.z - p1.z) * t;
    return p;
}

int findCell(const vector<Hoja>& celdas, Code codigo, int nivel) {
    int izq = 0;
    int der = (int)celdas.size() - 1;

    while (izq <= der) {
        int medio = izq + (der - izq) / 2;

        if (celdas[medio].codigo == codigo && celdas[medio].nivel == nivel) {
            return medio;
        }

        Hoja buscada;
        buscada.codigo = codigo;
        buscada.nivel = nivel;

        if (lessLeaf(celdas[medio], buscada)) {
            izq = medio + 1;
        } else {
            der = medio - 1;
        }
    }
    return -1;
}

void traverse(const Superficie& s, uint32_t x, uint32_t y, uint32_t z, int nivel, int r,
              vector<Hoja>& salida) {
    double lado = 1.0 / (1 << nivel);
    double centroX = (x + 0.5) * lado;
    double centroY = (y + 0.5) * lado;
    double centroZ = (z + 0.5) * lado;

    if (!canCross(s, centroX, centroY, centroZ, lado)) {
        return;
    }

    if (nivel == r) {
        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);

        int indice = cubeIndex(valores);
        if (indice != 0 && indice != 255) {
            Hoja celda;
            celda.codigo = encode(x, y, z, r);
            celda.nivel = r;
            salida.push_back(celda);
        }
        return;
    }

    for (int hija = 0; hija < 8; hija++) {
        uint32_t dx = hija & 1;
        uint32_t dy = (hija >> 1) & 1;
        uint32_t dz = (hija >> 2) & 1;
        traverse(s, 2 * x + dx, 2 * y + dy, 2 * z + dz, nivel + 1, r, salida);
    }
}

vector<Hoja> findCells(const Superficie& s, int r) {
    vector<Hoja> celdas;
    traverse(s, 0, 0, 0, 0, r, celdas);
    return celdas;
}

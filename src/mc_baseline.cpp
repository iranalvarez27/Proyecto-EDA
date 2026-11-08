#include "marching_cubes.h"
#include "tablas.h"

using namespace std;

void mcSoup(const Superficie& s, int r, const vector<Hoja>& celdas, Malla& malla) {
    malla.vertices.clear();
    malla.caras.clear();

    for (int i = 0; i < (int)celdas.size(); i++) {
        uint32_t x, y, z;
        decode(celdas[i].codigo, r, x, y, z);

        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);

        int indice = cubeIndex(valores);

        for (int t = 0; triTable[indice][t] != -1; t = t + 3) {
            int base = malla.vertices.size();
            for (int k = 0; k < 3; k++) {
                int arista = triTable[indice][t + k];
                int a = edgeIndex[arista][0];
                int b = edgeIndex[arista][1];
                malla.vertices.push_back(interpolate(posiciones[a], posiciones[b], valores[a], valores[b]));
            }
            malla.caras.push_back({base, base + 2, base + 1});
        }
    }
}

void mcPerCell(const Superficie& s, int r, const vector<Hoja>& celdas, Malla& malla) {
    malla.vertices.clear();
    malla.caras.clear();

    for (int i = 0; i < (int)celdas.size(); i++) {
        uint32_t x, y, z;
        decode(celdas[i].codigo, r, x, y, z);

        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);

        int indice = cubeIndex(valores);

        int indiceArista[12];
        for (int arista = 0; arista < 12; arista++) {
            indiceArista[arista] = -1;
            if (edgeTable[indice] & (1 << arista)) {
                int a = edgeIndex[arista][0];
                int b = edgeIndex[arista][1];
                indiceArista[arista] = malla.vertices.size();
                malla.vertices.push_back(interpolate(posiciones[a], posiciones[b], valores[a], valores[b]));
            }
        }

        for (int t = 0; triTable[indice][t] != -1; t = t + 3) {
            int v0 = indiceArista[triTable[indice][t]];
            int v1 = indiceArista[triTable[indice][t + 1]];
            int v2 = indiceArista[triTable[indice][t + 2]];
            malla.caras.push_back({v0, v2, v1});
        }
    }
}

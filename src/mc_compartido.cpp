#include "marching_cubes.h"
#include "tablas.h"

using namespace std;

const int aristasPropias[3] = {0, 3, 8};

const int dirArista[12] = {
    -1,X_MAS,Y_MAS,-1,Z_MAS,XZ_MAS_MAS,
    YZ_MAS_MAS,Z_MAS,-1,X_MAS,XY_MAS_MAS,Y_MAS
};

const int slotArista[12] = {
    0, 1, 0, 1,
    0, 1, 0, 1,
    2, 2, 2, 2
};

const int dirsVecinas[6] = {X_MAS, Y_MAS, Z_MAS, XY_MAS_MAS, XZ_MAS_MAS, YZ_MAS_MAS};

int mcShared(const Superficie& s, int r, const vector<Hoja>& celdas, FuncionVecino vecino, Malla& malla) {
    malla.vertices.clear();
    malla.caras.clear();

    int total = celdas.size();
    vector<int> indiceCubo(total);
    vector<array<int, 3>> verticePropio(total);

    for (int i = 0; i < total; i++) {
        uint32_t x, y, z;
        decode(celdas[i].codigo, r, x, y, z);

        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);

        indiceCubo[i] = cubeIndex(valores);

        for (int k = 0; k < 3; k++) {
            int arista = aristasPropias[k];
            verticePropio[i][k] = -1;

            if (edgeTable[indiceCubo[i]] & (1 << arista)) {
                int a = edgeIndex[arista][0];
                int b = edgeIndex[arista][1];
                verticePropio[i][k] = malla.vertices.size();
                malla.vertices.push_back(interpolate(posiciones[a], posiciones[b], valores[a], valores[b]));
            }
        }
    }

    int fallas = 0;

    for (int i = 0; i < total; i++) {
        int indice = indiceCubo[i];

        int posicionVecina[TOTAL_DIRECCIONES];
        bool yaBuscada[TOTAL_DIRECCIONES];
        for (int d = 0; d < TOTAL_DIRECCIONES; d++) {
            posicionVecina[d] = -1;
            yaBuscada[d] = false;
        }

        int indiceArista[12];
        for (int arista = 0; arista < 12; arista++) {
            indiceArista[arista] = -1;

            if ((edgeTable[indice] & (1 << arista)) == 0) {
                continue;
            }

            int direccion = dirArista[arista];
            int k = slotArista[arista];

            if (direccion == -1) {
                indiceArista[arista] = verticePropio[i][k];
            } else {
                if (!yaBuscada[direccion]) {
                    yaBuscada[direccion] = true;

                    Code codigoVecina;
                    bool dentro = vecino(celdas[i].codigo, r, direccion, r, codigoVecina);
                    if (dentro) {
                        posicionVecina[direccion] = findCell(celdas, codigoVecina, r);
                    }
                }

                int posicion = posicionVecina[direccion];
                if (posicion != -1) {
                    indiceArista[arista] = verticePropio[posicion][k];
                }
            }

            if (indiceArista[arista] == -1) {
                fallas = fallas + 1;
            }
        }

        for (int t = 0; triTable[indice][t] != -1; t = t + 3) {
            int v0 = indiceArista[triTable[indice][t]];
            int v1 = indiceArista[triTable[indice][t + 1]];
            int v2 = indiceArista[triTable[indice][t + 2]];
            malla.caras.push_back({v0, v2, v1});
        }
    }

    return fallas;
}

vector<Consulta> buildQueries(const Superficie& s, int r, const vector<Hoja>& celdas) {
    vector<Consulta> consultas;

    for (int i = 0; i < (int)celdas.size(); i++) {
        uint32_t x, y, z;
        decode(celdas[i].codigo, r, x, y, z);

        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);
        int indice = cubeIndex(valores);

        bool necesaria[TOTAL_DIRECCIONES];
        for (int d = 0; d < TOTAL_DIRECCIONES; d++) {
            necesaria[d] = false;
        }
        for (int arista = 0; arista < 12; arista++) {
            if ((edgeTable[indice] & (1 << arista)) && dirArista[arista] != -1) {
                necesaria[dirArista[arista]] = true;
            }
        }
        for (int j = 0; j < 6; j++) {
            int direccion = dirsVecinas[j];
            if (necesaria[direccion]) {
                Consulta c;
                c.celda = i;
                c.direccion = direccion;
                consultas.push_back(c);
            }
        }
    }
    return consultas;
}

long long runQueries(int r, const vector<Hoja>& celdas,const vector<Consulta>& consultas, FuncionVecino vecino) {
    long long checksum = 0;

    for (int i = 0; i < (int)consultas.size(); i++) {
        Code codigoVecina;
        bool dentro = vecino(celdas[consultas[i].celda].codigo, r, consultas[i].direccion, r, codigoVecina);
        if (dentro) {
            checksum = checksum + findCell(celdas, codigoVecina, r);
        }
    }
    return checksum;
}

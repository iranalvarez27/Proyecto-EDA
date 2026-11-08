#include <iostream>
#include <random>
#include <set>
#include <string>
#include "octree_lineal.h"
#include "marching_cubes.h"
#include "vecinos.h"
#include "tablas.h"

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

bool sortedUnique(const vector<Hoja>& hojas) {
    for (int i = 1; i < (int)hojas.size(); i++) {
        if (!lessLeaf(hojas[i - 1], hojas[i])) return false;
    }
    return true;
}

bool fullVolume(const vector<Hoja>& hojas, int r) {
    Code volumen = 0;
    for (int i = 0; i < (int)hojas.size(); i++) {
        int corrimiento = 3 * (r - hojas[i].nivel);
        Code bajos = ((Code)1 << corrimiento) - 1;
        if ((hojas[i].codigo & bajos) != 0) return false;
        volumen = volumen + ((Code)1 << corrimiento);
    }
    return volumen == ((Code)1 << (3 * r));
}

bool cellsAtLevelR(const vector<Hoja>& hojas, const vector<Hoja>& celdas, int r) {
    for (int i = 0; i < (int)celdas.size(); i++) {
        if (!hasLeaf(hojas, celdas[i].codigo, r)) return false;
    }
    return true;
}

bool edgesHaveFourCells(const Superficie& s, int r, const vector<Hoja>& hojas,
                        const vector<Hoja>& celdas) {
    int limite = 1 << r;
    for (int i = 0; i < (int)celdas.size(); i++) {
        uint32_t x, y, z;
        decode(celdas[i].codigo, r, x, y, z);

        Punto posiciones[8];
        double valores[8];
        getCorners(s, x, y, z, r, posiciones, valores);
        int indice = cubeIndex(valores);

        for (int arista = 0; arista < 12; arista++) {
            if ((edgeTable[indice] & (1 << arista)) == 0) continue;

            int a = edgeIndex[arista][0];
            int b = edgeIndex[arista][1];
            int base[3] = {(int)x + vertexOffset[a][0], (int)y + vertexOffset[a][1],
                           (int)z + vertexOffset[a][2]};

            int ejes[2];
            int k = 0;
            for (int d = 0; d < 3; d++) {
                if (vertexOffset[a][d] == vertexOffset[b][d]) {
                    ejes[k] = d;
                    k = k + 1;
                }
            }

            for (int m = 0; m < 4; m++) {
                int p[3] = {base[0], base[1], base[2]};
                p[ejes[0]] = p[ejes[0]] - (m & 1);
                p[ejes[1]] = p[ejes[1]] - ((m >> 1) & 1);

                bool dentro = true;
                for (int d = 0; d < 3; d++) {
                    if (p[d] < 0 || p[d] >= limite) dentro = false;
                }
                if (dentro && !hasLeaf(hojas, encode(p[0], p[1], p[2], r), r)) return false;
            }
        }
    }
    return true;
}

bool matchesSet(const vector<Hoja>& hojas, const TablaHojas& tabla, int r) {
    set<Hoja, bool (*)(const Hoja&, const Hoja&)> referencia(lessLeaf);
    for (int i = 0; i < (int)hojas.size(); i++) {
        referencia.insert(hojas[i]);
    }
    if (referencia.size() != hojas.size() || tabla.size() != hojas.size()) return false;

    mt19937_64 generador(2026);
    for (int prueba = 0; prueba < 20000; prueba++) {
        Hoja h;
        if (prueba % 2 == 0) {
            h = hojas[generador() % hojas.size()];
        } else {
            uint32_t x = generador() % (1u << r);
            uint32_t y = generador() % (1u << r);
            uint32_t z = generador() % (1u << r);
            h.codigo = encode(x, y, z, r);
            h.nivel = generador() % (r + 1);
        }
        bool esperado = referencia.count(h) > 0;
        if (hasLeaf(hojas, h.codigo, h.nivel) != esperado) return false;
        if (hasLeafHash(tabla, h.codigo, h.nivel) != esperado) return false;
    }
    return true;
}

bool queriesFound(const Superficie& s, int r, const vector<Hoja>& hojas, const TablaHojas& tabla,
                  const vector<Hoja>& celdas) {
    vector<Consulta> consultas = buildQueries(s, r, celdas);
    if (consultas.empty()) return false;

    for (int i = 0; i < (int)consultas.size(); i++) {
        Code c = celdas[consultas[i].celda].codigo;
        int direccion = consultas[i].direccion;

        Code a = 0, b = 0, o = 0;
        bool dentroA = schrack(c, r, direccion, r, a);
        bool dentroB = samet(c, r, direccion, r, b);
        bool dentroO = oracle(c, r, direccion, r, o);

        if (!dentroO || !dentroA || !dentroB) return false;
        if (a != o || b != o) return false;
        if (!hasLeaf(hojas, o, r) || !hasLeafHash(tabla, o, r)) return false;
    }
    return true;
}

double farAway(double, double, double) {
    return 100.0;
}

int main() {
    Superficie superficies[2] = {ESFERA, TORO};

    for (int s = 0; s < 2; s++) {
        for (int r = 4; r <= 7; r++) {
            vector<Hoja> hojas = construirOctree(superficies[s], r);
            vector<Hoja> celdas = findCells(superficies[s], r);
            TablaHojas tabla = construirTabla(hojas);

            string prefijo = string(superficies[s].nombre) + " r=" + to_string(r) + ": ";
            check(sortedUnique(hojas), prefijo + "arreglo ordenado y sin duplicados");
            check(fullVolume(hojas, r), prefijo + "el volumen de las hojas suma 1");
            check(cellsAtLevelR(hojas, celdas, r), prefijo + "las celdas de superficie son hojas de nivel r");
            check(edgesHaveFourCells(superficies[s], r, hojas, celdas), prefijo + "las 4 celdas de cada arista cortada existen");
            check(matchesSet(hojas, tabla, r), prefijo + "binaria y hash coinciden con std::set");
            check(queriesFound(superficies[s], r, hojas, tabla, celdas), prefijo + "consultas de MC: Schrack = Samet = oraculo y la vecina existe");
        }
    }

    Superficie vacia = {"vacia", farAway, 1.0};
    vector<Hoja> raiz = construirOctree(vacia, 5);
    check(raiz.size() == 1 && raiz[0].nivel == 0, "superficie fuera del cubo: una sola hoja");
    check(hasLeaf(raiz, 0, 0) && !hasLeaf(raiz, 0, 5), "una hoja de otro nivel no cuenta como vecina");

    cout << (fallas == 0 ? "Todos los tests pasaron\n" : "Hay tests que fallaron\n");
    return fallas;
}

#include <algorithm>
#include "octree_lineal.h"

using namespace std;

void subdividir(const Superficie& s, uint32_t x, uint32_t y, uint32_t z, int nivel, int r,
                vector<Hoja>& hojas) {
    double lado = 1.0 / (1 << nivel);
    double centroX = (x + 0.5) * lado;
    double centroY = (y + 0.5) * lado;
    double centroZ = (z + 0.5) * lado;

    if (nivel == r || !canCross(s, centroX, centroY, centroZ, lado)) {
        int corrimiento = r - nivel;
        Hoja hoja;
        hoja.codigo = encode(x << corrimiento, y << corrimiento, z << corrimiento, r);
        hoja.nivel = nivel;
        hojas.push_back(hoja);
        return;
    }

    for (int hija = 0; hija < 8; hija++) {
        uint32_t dx = hija & 1;
        uint32_t dy = (hija >> 1) & 1;
        uint32_t dz = (hija >> 2) & 1;
        subdividir(s, 2 * x + dx, 2 * y + dy, 2 * z + dz, nivel + 1, r, hojas);
    }
}

vector<Hoja> construirOctree(const Superficie& s, int r) {
    vector<Hoja> hojas;
    subdividir(s, 0, 0, 0, 0, r, hojas);
    return hojas;
}

bool hasLeaf(const vector<Hoja>& hojas, Code c, int nivel) {
    Hoja buscada;
    buscada.codigo = c;
    buscada.nivel = nivel;

    vector<Hoja>::const_iterator it = lower_bound(hojas.begin(), hojas.end(), buscada, lessLeaf);
    return it != hojas.end() && it->codigo == c && it->nivel == nivel;
}

size_t HashHoja::operator()(const Hoja& hoja) const {
    return hash<Code>()(hoja.codigo * 32 + hoja.nivel);
}

bool IgualHoja::operator()(const Hoja& a, const Hoja& b) const {
    return a.codigo == b.codigo && a.nivel == b.nivel;
}

TablaHojas construirTabla(const vector<Hoja>& hojas) {
    TablaHojas tabla;
    tabla.reserve(hojas.size());
    for (int i = 0; i < (int)hojas.size(); i++) {
        tabla.insert(hojas[i]);
    }
    return tabla;
}

bool hasLeafHash(const TablaHojas& tabla, Code c, int nivel) {
    Hoja buscada;
    buscada.codigo = c;
    buscada.nivel = nivel;
    return tabla.find(buscada) != tabla.end();
}

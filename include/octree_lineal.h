#ifndef OCTREE_LINEAL_H
#define OCTREE_LINEAL_H
#include <vector>
#include <unordered_set>
#include "octree.h"
#include "superficies.h"

std::vector<Hoja> construirOctree(const Superficie& s, int r);

struct HashHoja {
    size_t operator()(const Hoja& hoja) const;
};

struct IgualHoja {
    bool operator()(const Hoja& a, const Hoja& b) const;
};

typedef std::unordered_set<Hoja, HashHoja, IgualHoja> TablaHojas;

TablaHojas construirTabla(const std::vector<Hoja>& hojas);

bool hasLeafHash(const TablaHojas& tabla, Code c, int nivel);

#endif

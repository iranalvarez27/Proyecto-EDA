#ifndef VECINOS_H
#define VECINOS_H
#include "octree.h"

extern long long digitosReflejados;

Code mascaraGrupos(int D, int r);

Code mascaraX(int D, int r);

Code dilatar(long long valor, int D, int r);

uint32_t contraer(Code dilatado, int D, int r);

Code sumaDilatada(Code a, Code b, int D, int r);

Code incremento(const int desp[], int D, int r);

#endif

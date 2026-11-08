#ifndef OCTREE_H
#define OCTREE_H
#include <cstdint>
#include <vector>

typedef uint64_t Code;

struct Hoja {
    Code codigo = 0;
    int nivel = 0;
};

const int R_MAXIMO = 20;

inline bool lessLeaf(const Hoja& a, const Hoja& b) {
    if (a.codigo != b.codigo) {
        return a.codigo < b.codigo;
    }
    return a.nivel < b.nivel;
}

enum Direccion {
    X_MAS, X_MENOS,
    Y_MAS, Y_MENOS,
    Z_MAS, Z_MENOS,

    XY_MAS_MAS, XY_MAS_MENOS, XY_MENOS_MAS, XY_MENOS_MENOS,
    XZ_MAS_MAS, XZ_MAS_MENOS, XZ_MENOS_MAS, XZ_MENOS_MENOS,
    YZ_MAS_MAS, YZ_MAS_MENOS, YZ_MENOS_MAS, YZ_MENOS_MENOS,
    TOTAL_DIRECCIONES
};

const int TOTAL_CARAS = 6;

const int desplazamiento[TOTAL_DIRECCIONES][3] = {
    { 1,  0,  0}, {-1,  0,  0},
    { 0,  1,  0}, { 0, -1,  0},
    { 0,  0,  1}, { 0,  0, -1},

    { 1,  1,  0}, { 1, -1,  0}, {-1,  1,  0}, {-1, -1,  0},
    { 1,  0,  1}, { 1,  0, -1}, {-1,  0,  1}, {-1,  0, -1},
    { 0,  1,  1}, { 0,  1, -1}, { 0, -1,  1}, { 0, -1, -1}
};

inline Code encode(uint32_t x, uint32_t y, uint32_t z, int r) {
    Code codigo = 0;
    for (int i = 0; i < r; i++) {
        Code bitX = (x >> i) & 1;
        Code bitY = (y >> i) & 1;
        Code bitZ = (z >> i) & 1;

        codigo = codigo | (bitX << (3 * i));
        codigo = codigo | (bitY << (3 * i + 1));
        codigo = codigo | (bitZ << (3 * i + 2));
    }
    return codigo;
}

inline void decode(Code codigo, int r, uint32_t& x, uint32_t& y, uint32_t& z) {
    x = 0;
    y = 0;
    z = 0;
    for (int i = 0; i < r; i++) {
        uint32_t bitX = (codigo >> (3 * i)) & 1;
        uint32_t bitY = (codigo >> (3 * i + 1)) & 1;
        uint32_t bitZ = (codigo >> (3 * i + 2)) & 1;

        x = x | (bitX << i);
        y = y | (bitY << i);
        z = z | (bitZ << i);
    }
}


bool schrack(Code c, int nivel, int direccion, int r, Code& salida);
bool samet(Code c, int nivel, int direccion, int r, Code& salida);
bool oracle(Code c, int nivel, int direccion, int r, Code& salida);
bool hasLeaf(const std::vector<Hoja>& hojas, Code c, int nivel);

#endif

#include "vecinos.h"

using namespace std;

long long digitosReflejados = 0;

Code mascaraGrupos(int D, int r) {
    if (D * r >= 64) {
        return ~(Code)0;
    }
    return ((Code)1 << (D * r)) - 1;
}

Code mascaraX(int D, int r) {
    Code patron = 0x1249249249249249ULL;
    if (D == 2) {
        patron = 0x5555555555555555ULL;
    }
    return patron & mascaraGrupos(D, r);
}

Code dilatar(long long valor, int D, int r) {
    Code resultado = 0;
    for (int i = 0; i < r; i++) {
        Code bit = (valor >> i) & 1;
        resultado = resultado | (bit << (D * i));
    }
    return resultado;
}

uint32_t contraer(Code dilatado, int D, int r) {
    uint32_t resultado = 0;
    for (int i = 0; i < r; i++) {
        uint32_t bit = (dilatado >> (D * i)) & 1;
        resultado = resultado | (bit << i);
    }
    return resultado;
}

// Suma de cada eje por separado: los bits de los otros ejes se rellenan con 1
// para que el acarreo los cruce (en 3D: t1 = 6tx, t2 = 5tx, t3 = 3tx).
Code sumaDilatada(Code a, Code b, int D, int r) {
    Code grupos = mascaraGrupos(D, r);
    Code tx = mascaraX(D, r);
    Code resultado = 0;
    for (int k = 0; k < D; k++) {
        Code tk = tx << k;
        Code relleno = grupos & ~tk;
        resultado = resultado | (((a | relleno) + (b & tk)) & tk);
    }
    return resultado;
}

// Desplazamientos unitarios: dilatar(1) = 1 y dilatar(-1) = tx.
Code incremento(const int desp[], int D, int r) {
    Code resultado = 0;
    for (int k = 0; k < D; k++) {
        if (desp[k] == 1) {
            resultado = resultado | ((Code)1 << k);
        }
        if (desp[k] == -1) {
            resultado = resultado | (mascaraX(D, r) << k);
        }
    }
    return resultado;
}

bool schrack(Code c, int nivel, int direccion, int r, Code& salida) {
    Code tx = mascaraX(3, r) & ~mascaraGrupos(3, r - nivel);

    for (int k = 0; k < 3; k++) {
        Code tk = tx << k;
        int d = desplazamiento[direccion][k];
        if (d == 1 && (c & tk) == tk) return false;
        if (d == -1 && (c & tk) == 0) return false;
    }

    Code inc = incremento(desplazamiento[direccion], 3, nivel) << (3 * (r - nivel));
    salida = sumaDilatada(c, inc, 3, r);
    return true;
}

bool samet(Code c, int nivel, int direccion, int r, Code& salida) {
    salida = c;
    int primero = r - nivel;
    int reflejados = 0;

    for (int k = 0; k < 3; k++) {
        int d = desplazamiento[direccion][k];
        if (d == 0) {
            continue;
        }

        int j = primero;
        while (j < r) {
            Code bit = (Code)1 << (3 * j + k);
            bool estaba = (salida & bit) != 0;
            salida = salida ^ bit;
            if ((d == 1 && !estaba) || (d == -1 && estaba)) {
                break;
            }
            j++;
        }
        if (j == r) {
            return false;
        }

        if (j - primero + 1 > reflejados) {
            reflejados = j - primero + 1;
        }
    }

    digitosReflejados = digitosReflejados + reflejados;
    return true;
}

bool oracle(Code c, int nivel, int direccion, int r, Code& salida) {
    uint32_t x, y, z;
    decode(c, r, x, y, z);

    long long paso = 1LL << (r - nivel);
    long long limite = 1LL << r;

    long long nuevoX = (long long)x + desplazamiento[direccion][0] * paso;
    long long nuevoY = (long long)y + desplazamiento[direccion][1] * paso;
    long long nuevoZ = (long long)z + desplazamiento[direccion][2] * paso;

    if (nuevoX < 0 || nuevoX >= limite) return false;
    if (nuevoY < 0 || nuevoY >= limite) return false;
    if (nuevoZ < 0 || nuevoZ >= limite) return false;

    salida = encode(nuevoX, nuevoY, nuevoZ, r);
    return true;
}

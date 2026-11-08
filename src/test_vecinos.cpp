#include <iostream>
#include <random>
#include "vecinos.h"

using namespace std;

int fallas = 0;

void check(bool condicion, const char* nombre) {
    if (condicion) {
        cout << "OK     " << nombre << "\n";
    } else {
        cout << "FALLA  " << nombre << "\n";
        fallas = fallas + 1;
    }
}

int main() {
    // Figura 3: dilataciones 3D para r = 3
    Code figura3[8] = {0, 1, 8, 9, 64, 65, 72, 73};
    bool bien = true;
    for (int v = 0; v < 8; v++) {
        if (dilatar(v, 3, 3) != figura3[v]) bien = false;
    }
    check(bien, "Figura 3: dilataciones 3D para r = 3");
    check(dilatar(5, 3, 3) == 65, "3D: 5 -> 65");

    check(sumaDilatada(dilatar(7, 2, 4), dilatar(6, 2, 4), 2, 4) == dilatar(13, 2, 4), "2D: 7 + 6 -> 13");

    // -5 necesita un bit de signo: r = 3 usa 4 bits y r = 4 usa 5 bits
    check(dilatar(-5, 2, 4) == 69, "2D: -5 -> 69 (r = 3)");
    check(dilatar(-5, 2, 5) == 325, "2D: -5 -> 325 (r = 4)");
    check((int)contraer(69, 2, 4) - 16 == -5, "2D: contraer(69) = -5");

    // 3013_4 = 199, 2032_4 = 142, 1301_4 = 113
    check(sumaDilatada(199, 142, 2, 4) == 113, "2D: 3013_4 + 2032_4 = 1301_4");

    int desp2D[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
    Code esperado[8] = {1, 3, 2, 87, 85, 255, 170, 171};
    bien = true;
    for (int i = 0; i < 8; i++) {
        if (incremento(desp2D[i], 2, 4) != esperado[i]) bien = false;
    }
    check(bien, "2D: incrementos para r = 4");

    bien = true;
    for (int D = 2; D <= 3; D++) {
        for (int r = 1; r <= 5; r++) {
            for (uint32_t v = 0; v < (1u << r); v++) {
                if (contraer(dilatar(v, D, r), D, r) != v) bien = false;
            }
        }
    }
    check(bien, "Ida y vuelta dilatar/contraer para r <= 5");

    mt19937_64 generador(12345);
    bien = true;
    for (int prueba = 0; prueba < 200000; prueba++) {
        int r = 1 + generador() % R_MAXIMO;
        int nivel = generador() % (r + 1);
        int direccion = generador() % TOTAL_DIRECCIONES;

        int corrimiento = r - nivel;
        uint32_t x = (generador() % (1u << nivel)) << corrimiento;
        uint32_t y = (generador() % (1u << nivel)) << corrimiento;
        uint32_t z = (generador() % (1u << nivel)) << corrimiento;
        Code c = encode(x, y, z, r);

        Code a = 0, b = 0, o = 0;
        bool dentroA = schrack(c, nivel, direccion, r, a);
        bool dentroB = samet(c, nivel, direccion, r, b);
        bool dentroO = oracle(c, nivel, direccion, r, o);

        if (dentroA != dentroO || dentroB != dentroO) bien = false;
        if (dentroO && (a != o || b != o)) bien = false;
    }
    check(bien, "Schrack = Samet = oraculo con codigos aleatorios");

    cout << (fallas == 0 ? "Todos los tests pasaron\n" : "Hay tests que fallaron\n");
    return fallas;
}

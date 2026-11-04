#ifndef SUPERFICIES_H
#define SUPERFICIES_H
#include <cmath>

const double ESFERA_CENTRO = 0.5;
const double ESFERA_RADIO = 0.35;

inline double sphere(double x, double y, double z) {
    double dx = x - ESFERA_CENTRO;
    double dy = y - ESFERA_CENTRO;
    double dz = z - ESFERA_CENTRO;
    return dx * dx + dy * dy + dz * dz - ESFERA_RADIO * ESFERA_RADIO;
}

const double ESFERA_LIPSCHITZ = 1.74;

const double TORO_CENTRO = 0.5;
const double TORO_R = 0.3;
const double TORO_RHO = 0.1;

inline double torus(double x, double y, double z) {
    double dx = x - TORO_CENTRO;
    double dy = y - TORO_CENTRO;
    double dz = z - TORO_CENTRO;

    double q = std::sqrt(dx * dx + dy * dy);
    double a = q - TORO_R;
    return a * a + dz * dz - TORO_RHO * TORO_RHO;
}


const double TORO_LIPSCHITZ = 1.30;
typedef double (*FuncionImplicita)(double, double, double);

struct Superficie {
    const char* nombre;
    FuncionImplicita funcion;
    double lipschitz;
};

const Superficie ESFERA = {"esfera", sphere, ESFERA_LIPSCHITZ};
const Superficie TORO = {"toro", torus, TORO_LIPSCHITZ};

inline bool canCross(const Superficie& s, double cx, double cy, double cz, double lado) {
    double semidiagonal = lado * std::sqrt(3.0) / 2.0;
    double valor = s.funcion(cx, cy, cz);
    if (std::fabs(valor) <= s.lipschitz * semidiagonal) {
        return true;
    }
    return false;
}

#endif

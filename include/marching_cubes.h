#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include <vector>
#include <array>
#include <string>
#include "octree.h"
#include "superficies.h"

struct Punto {
    double x = 0, y = 0, z = 0;
};

struct Malla {
    std::vector<Punto> vertices;
    std::vector<std::array<int, 3>> caras;
};

std::vector<Hoja> findCells(const Superficie& s, int r);

void getCorners(const Superficie& s, uint32_t x, uint32_t y, uint32_t z, int r,
                     Punto posiciones[8], double valores[8]);

int cubeIndex(const double valores[8]);

Punto interpolate(const Punto& p1, const Punto& p2, double v1, double v2);

int findCell(const std::vector<Hoja>& celdas, Code codigo, int nivel);

void mcPerCell(const Superficie& s, int r, const std::vector<Hoja>& celdas, Malla& malla);

typedef bool (*FuncionVecino)(Code c, int nivel, int direccion, int r, Code& salida);

int mcShared(const Superficie& s, int r, const std::vector<Hoja>& celdas,
                            FuncionVecino vecino, Malla& malla);

struct Consulta {
    int celda;
    int direccion;
};

std::vector<Consulta> buildQueries(const Superficie& s, int r, const std::vector<Hoja>& celdas);

long long runQueries(int r, const std::vector<Hoja>& celdas,
                                 const std::vector<Consulta>& consultas, FuncionVecino vecino);

struct ReporteMalla {
    int vertices = 0;
    int verticesUsados = 0;
    int caras = 0;
    int aristas = 0;
    int verticesDuplicados = 0;
    int aristasAbiertas = 0;
    int aristasMalOrientadas = 0;
    int euler = 0;
    int carasHaciaAfuera = 0;
};

ReporteMalla checkMesh(const Malla& malla, const Superficie& s);

int countUnique(const Malla& malla);

void savePly(const Malla& malla, const std::string& archivo);

#endif

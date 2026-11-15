# Objetivo 2: tablas generadas desde obj2.csv

Mediana de las corridas en ns por consulta; entre paréntesis, el rango intercuartílico.

## Búsqueda completa con r = 10

| Superficie | Hojas | Método | Estructura | Dirección | Existencia | Total | Existencia / total |
|---|---|---|---|---|---|---|---|
| esfera | 16,202,600 | schrack | binaria | 28 (2) | 139 (6) | 189 (8) | 74 % |
| esfera | 16,202,600 | schrack | hash | 28 (2) | 66 (4) | 119 (5) | 56 % |
| esfera | 16,202,600 | samet | binaria | 28 (2) | 139 (6) | 205 (12) | 68 % |
| esfera | 16,202,600 | samet | hash | 28 (2) | 66 (4) | 115 (6) | 57 % |
| esfera | 16,202,600 | oraculo | binaria | 73 (4) | 139 (6) | 196 (5) | 71 % |
| esfera | 16,202,600 | oraculo | hash | 73 (4) | 66 (4) | 165 (5) | 40 % |
| toro | 32,507,168 | schrack | binaria | 28 (2) | 144 (13) | 198 (7) | 72 % |
| toro | 32,507,168 | schrack | hash | 28 (2) | 63 (5) | 110 (9) | 58 % |
| toro | 32,507,168 | samet | binaria | 29 (1) | 144 (13) | 212 (8) | 68 % |
| toro | 32,507,168 | samet | hash | 29 (1) | 63 (5) | 104 (3) | 61 % |
| toro | 32,507,168 | oraculo | binaria | 73 (4) | 144 (13) | 206 (7) | 70 % |
| toro | 32,507,168 | oraculo | hash | 73 (4) | 63 (5) | 158 (3) | 40 % |

## Tamaño de cada octree

| Superficie | r | Hojas N | Celdas de superficie M | Consultas | Construir octree (ms) | Construir tabla hash (ms) |
|---|---|---|---|---|---|---|
| esfera | 4 | 3,032 | 584 | 1,575 | 0.2 | 0.4 |
| esfera | 5 | 14,568 | 2,408 | 6,495 | 0.8 | 2.4 |
| esfera | 6 | 61,440 | 9,488 | 25,641 | 4.2 | 18.7 |
| esfera | 7 | 249,208 | 37,856 | 102,465 | 11.8 | 37.9 |
| esfera | 8 | 1,010,192 | 151,400 | 409,845 | 51.4 | 173.8 |
| esfera | 9 | 4,053,680 | 605,264 | 1,638,567 | 223.2 | 843.3 |
| esfera | 10 | 16,202,600 | 2,421,200 | 6,554,427 | 920.9 | 3010.6 |
| toro | 4 | 3,032 | 424 | 1,166 | 0.7 | 0.3 |
| toro | 5 | 15,744 | 1,840 | 4,998 | 0.8 | 1.7 |
| toro | 6 | 77,680 | 7,104 | 19,402 | 3.9 | 10.2 |
| toro | 7 | 378,288 | 27,720 | 75,910 | 21.7 | 56.2 |
| toro | 8 | 1,910,112 | 112,040 | 306,278 | 97.9 | 268.9 |
| toro | 9 | 8,032,592 | 450,760 | 1,231,236 | 444.6 | 1166.6 |
| toro | 10 | 32,507,168 | 1,798,288 | 4,914,134 | 1949.4 | 5339.6 |

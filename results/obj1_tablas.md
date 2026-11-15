# Objetivo 1: tablas generadas desde obj1.csv

Mediana de las corridas en ns por consulta; entre paréntesis, el rango intercuartílico.

## Dirección del vecino por caras

| Caso | r | Schrack | Samet | Oráculo | Dígitos que refleja Samet |
|---|---|---|---|---|---|
| Peor caso | 4 | 44 (4) | 44 (4) | 36 (4) | 4.0 |
| Peor caso | 7 | 44 (1) | 52 (3) | 55 (3) | 7.0 |
| Peor caso | 10 | 44 (1) | 58 (6) | 72 (2) | 10.0 |
| Azar | 4 | 43 (4) | 52 (3) | 36 (5) | 1.6 |
| Azar | 7 | 44 (1) | 54 (3) | 56 (1) | 1.9 |
| Azar | 10 | 46 (5) | 57 (9) | 77 (6) | 2.0 |

## Dirección del vecino por aristas

| Caso | r | Schrack | Samet | Oráculo | Dígitos que refleja Samet |
|---|---|---|---|---|---|
| Peor caso | 4 | 56 (3) | 62 (2) | 35 (1) | 4.0 |
| Peor caso | 7 | 58 (2) | 78 (3) | 55 (1) | 7.0 |
| Peor caso | 10 | 56 (4) | 93 (4) | 74 (5) | 10.0 |
| Azar | 4 | 58 (4) | 78 (2) | 37 (2) | 1.9 |
| Azar | 7 | 57 (3) | 84 (3) | 54 (3) | 2.5 |
| Azar | 10 | 57 (3) | 83 (7) | 73 (4) | 2.6 |

## ¿Se observa el O(1)?

Cambio de la mediana entre r = 4 y r = 10 en el peor caso:

| Tipo | Método | r mínimo (ns) | r máximo (ns) | Cociente |
|---|---|---|---|---|
| caras | schrack | 44 | 44 | 1.01 |
| caras | samet | 44 | 58 | 1.32 |
| caras | oraculo | 36 | 72 | 2.02 |
| aristas | schrack | 56 | 56 | 1.01 |
| aristas | samet | 62 | 93 | 1.51 |
| aristas | oraculo | 35 | 74 | 2.11 |

Un cociente cercano a 1 indica tiempo constante en r; uno mayor indica que el costo crece con r.

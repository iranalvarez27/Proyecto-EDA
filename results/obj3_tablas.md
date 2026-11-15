# Objetivo 3: tablas generadas desde obj3.csv

Los tiempos son la mediana de las corridas.

## Vértices de la malla (esfera)

| r | Celdas | 3 por triángulo | Compartidos en la celda | Compartidos entre celdas |
|---|---|---|---|---|
| 4 | 584 | 3,480 | 2,328 | 582 |
| 7 | 37,856 | 227,112 | 151,416 | 37,854 |
| 10 | 2,421,200 | 14,527,176 | 9,684,792 | 2,421,198 |

## Vértices de la malla (toro)

| r | Celdas | 3 por triángulo | Compartidos en la celda | Compartidos entre celdas |
|---|---|---|---|---|
| 4 | 424 | 2,544 | 1,696 | 424 |
| 7 | 27,720 | 166,320 | 110,880 | 27,720 |
| 10 | 1,798,288 | 10,789,728 | 7,193,152 | 1,798,288 |

## Tiempo total con r = 10 (segundos, Schrack)

| Superficie | 3 por triángulo | Compartidos en la celda | Compartidos entre celdas |
|---|---|---|---|
| esfera | 0.479 | 0.464 | 1.67 |
| toro | 0.407 | 0.392 | 1.26 |

## Parte del tiempo de la malla compartida que se va en buscar vecinos (r = 10)

| Superficie | Método | Solo la dirección | Dirección + existencia |
|---|---|---|---|
| esfera | schrack | 11 % | 66 % |
| esfera | samet | 11 % | 68 % |
| esfera | oraculo | 25 % | 65 % |
| toro | schrack | 10 % | 63 % |
| toro | samet | 11 % | 65 % |
| toro | oraculo | 25 % | 62 % |

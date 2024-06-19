# TP3-Algoritmos

Este programa toma un archivo .txt, el cual solo puede contener 0, 1 y x's y que se supone que es una imagen satelital de un campo. Los 0s representan campo limpio, los 1s maleza y las X barreras. A lo largo del programa, se construyen matrices y se realizan operaciones geometricas de 2 dimensiones. Se hace uso de grafos no dirigidos y matrices de adyacencia para simplificar la resolucion de problemas 2D.

El objetivo del programa es identificar el recorrido más corto en linea recta que puede hacer un dron fumigador entre varias manchas de 3x3 de maleza, pasando por los parches una sola vez y empezando y terminando en las coordenadas 0,0 (esquina superior izquierda del txt), es decir, se busca identificar el ciclo hamiltoneano mas corto de todos los posibles entre los vertices de la matriz de adyacencia

CANTIDADVERTICES = cantidad de manchas (se tiene que configurar segun lo que uno mismo vea en el txt). Se recomienda que no tenga mas de 10 manchas porque el algoritmo del ciclo hamiltoneano es complejo

FILAS y COLUMNAS hacen referencia a la cantidad de filas y columnas que tiene el txt a analizar 
El txt se encuentra en docs y se adjuntan archivos que pueden ser utiles para entender el codigo

TODO: falta hacer que sume la distancia entre el ultimo vertice (aka ultima mancha) que se visita y el inicio

/*
 * Esta clase busca y verifica la existencia de ciclos hamiltoneanos en un grafo representado por una matriz de adyacencia
 */

#ifndef HAMILTONEANO_H
#define HAMILTONEANO_H

#include <vector>
#include <iostream>

// !aka cantidad de parches entre los que se puede viajar (incluyendo el inicio 2 veces)
#define CANTIDADVERTICES 9

using namespace std;

class Hamilton {
private:
    // Flag que indica si se ha encontrado al menos un ciclo hamiltoneano
    bool hasCycle;

    /*
    * Este metodo se usa en hamCycle()
    * Obtiene recursivamente todos los ciclos hamiltoneanos posibles
    */ 
    void FindHamCycle(int graph[CANTIDADVERTICES][CANTIDADVERTICES], int pos, vector<int>& path, bool visited[]);
    
    /*
    * Este metodo se usa en FindHamCycle()
    * Verifica si un vertice v puede ser agregado al ciclo hamiltoneano actual en la posicion pos
    */ 
    bool isSafe(int v, int graph[CANTIDADVERTICES][CANTIDADVERTICES], const vector<int>& path, int pos);

public:
    Hamilton() : hasCycle(false), cantCaminos(0) {}

    // Contador de la cantidad de ciclos hamiltoneanos o caminos encontrados
    int cantCaminos;
    // Almacena los caminos encontrados
    int MatrizConCaminos[1000][1000];

    /*
    * Este metodo se usa en main()
    * Inicia la busqueda de todos los ciclos hamiltoneanos posibles dada una matriz de adyacencia
    */ 
    void hamCycle(int graph[CANTIDADVERTICES][CANTIDADVERTICES]);

};

void Hamilton::hamCycle(int graph[CANTIDADVERTICES][CANTIDADVERTICES]) {
    hasCycle = false;
    // Prepara el vector para armar un camino a seguir, agregando al vertice 0 como posicion inicial
    vector<int> path;
    path.push_back(0);

    // Mantiene el seguimiento de los vertices visitados
    bool visited[CANTIDADVERTICES] = {false};
    // Marca al vertice 0 como visitado
    visited[0] = true;

    // Llamada al metodo encargado de buscar los ciclos hamiltoneanos (metodo recursivo)
    FindHamCycle(graph, 1, path, visited);

    // Para el caso de que no haya ciclo hamiltoneano posible para el grafo dado
    if (!hasCycle) {
        cout << "\tNo Hamiltonian Cycle possible\n";
    }
}

bool Hamilton::isSafe(int v, int graph[CANTIDADVERTICES][CANTIDADVERTICES], const vector<int>& path, int pos) {
    // Si el vertice NO es adyacente al ultimo vertice agregado al camino
    if (graph[path[pos - 1]][v] == 0) {
        return false;
    }
    // Si el vertice ya está en el camino
    for (int i = 0; i < pos; i++) {
        if (path[i] == v) {
            return false;
        }
    }
    // Si no es ninguno de los anteriores, se puede agregar el vertice
    return true;
}

void Hamilton::FindHamCycle(int graph[CANTIDADVERTICES][CANTIDADVERTICES], int pos, vector<int>& path, bool visited[]) {
    // Si todos los vertices estan incluidos en el ciclo, verifica si el camino forma un ciclo
    if (pos == CANTIDADVERTICES) {
        // Si se forma un ciclo (que es cuando hay una arista del ultimo vertice agregado hacia el vertice inicial), lo imprime y almacena
        if (graph[path[path.size() - 1]][path[0]] != 0) {
            // Se incluye al vertice inicial en el camino y se lo imprime
            path.push_back(0);
            //*cout << "\t\t";
            // Imprime al resto del ciclo y lo guarda en MatrizConCaminos
            //*for (int i = 0; i < path.size(); i++) {
                //*cout << " " << path[i];
                for (int j = 0; j < path.size(); ++j) {
                    MatrizConCaminos[cantCaminos][j] = path[j];
                }
            //*}
            // Se incrementa la cantidad de caminos cada vez que se encuentra un camino
            cantCaminos++;
            //*cout << "\n";
            // Quita el vertice inicial agregado
            path.pop_back();
            // Actualiza la flag
            hasCycle = true;
        }
        return;
    }

    // Prueba todos los vertices posibles como siguientes en el camino/ciclo
    for (int v = 0; v < CANTIDADVERTICES; v++) {
        // Si el vertice es seguro y no se ha visitado, lo agrega al camino y lo marca como visitado
        // luego llama recursivamente para el siguiente vertice
        if (isSafe(v, graph, path, pos) && !visited[v]) {
            // Agrega el vertice al camino y lo marca como visitado
            path.push_back(v);
            visited[v] = true;
            // Llamada recursiva para el resto del camino
            FindHamCycle(graph, pos + 1, path, visited);
            // Desmarca al vertice actual y lo elimina del camino para probar otras posibilidades
            visited[v] = false;
            path.pop_back();
        }
    }
}

#endif // HAMILTONEANO_H

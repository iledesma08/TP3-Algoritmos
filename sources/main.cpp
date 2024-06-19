/*
 * Este programa busca la ruta optima de vuelo de un dron fumigador.
 * Recibe una imagen satelital en formato .txt la cual identifica parches de maleza con 1s, campo sin maleza con 0s y barreras con X
 * El dron solamente se detiene a fumigar parches de maleza significativas (parches de 3x3)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "bitset"
#include "queue"
#include "vector"
#include "Mancha.h"
#include "math.h"
#include "Hamilton.h"

#define INFI 9999
#define FILAS 100
#define COLUMNAS 100

using namespace std;

struct Barrera{
    Posicion p1;
    Posicion p2;
};

// !Variables Globales:
// * Matriz que guarda el arreglo de bit<b1, b0> b1 indica si hay barrera || b0: indica si hay maleza
// Va a tener la misma estructura que el .txt
bitset<2> matrizDeBits[FILAS][COLUMNAS]; 
// *Matriz que guarda las distancias entre 2 manchas, un subindice es una mancha y el otro, otra
int matrizPesos[CANTIDADVERTICES][CANTIDADVERTICES];
// * Matriz que indica si el dron puede ir de una mancha a otra (si no hay una barrera en el medio)
// Un 1 indica que si puede, un 0 indica que no puede
int matrizAdyacencia[CANTIDADVERTICES][CANTIDADVERTICES];
// *Cola que guarda las diferentes barreras que se van identificando
vector<Barrera*> vectorBarreras;
// *Cola que guarda las diferentes manchas de 3x3 que se van identificando
vector<Mancha*> arrayManchas;

// !Prototipo de funciones:
//Funciones para completar matrices
bool llenarMatrizDeBits();                                                      
void completarMatrizAdyacencia();
int calcularDistancia(Posicion, Posicion);
//Funciones para filtrar donde hay manchas y barreras
void filtrar();                                                  
void revisarManchas(int, int);
void revisarBarreras(int, int );
// Funciones para controlar intersecciones
bool doIntersect(Posicion p1, Posicion q1, Posicion p2, Posicion q2);       
int orientation(Posicion p, Posicion q, Posicion r);
bool onSegment(Posicion p, Posicion q, Posicion r);

void printMatrizAdyacencia();

int main(){
    // Se llena la matriz de bits con el contenido del .txt, verificando si se pudo abrir el .txt
    if (!llenarMatrizDeBits()) {
        cout << "\nNo se pudo abrir el archivo" << endl;
        return 0;
    };
    // Identifico parches y barreras (que es lo que me importa para armar la matriz de adyacencia)
    filtrar();
    // Armo la matriz de adyacencia teniendo en cuenta si hay barreras entre 2 parches
    completarMatrizAdyacencia();

    // Empiezo a buscar el camino mas corto
    Hamilton *hamiltonian = new Hamilton();
    //*cout<<"\n\nCaminos posibles: "<<endl;
    hamiltonian->hamCycle(matrizAdyacencia);

    // Inicio una matriz cuadrada de enteros con tamaño igual a la cantidad de caminos encontrados al hacer hamCycle
    int matriz[hamiltonian->cantCaminos][hamiltonian->cantCaminos];

    // Recorro la matriz de enteros y en cada fila copio un camino obtenido
    for (int i = 0; i < hamiltonian->cantCaminos; ++i) {
        for (int j = 0; j < hamiltonian->cantCaminos - 1; ++j) {
            matriz[i][j]=hamiltonian->MatrizConCaminos[i][j];
        }
    }

    // Identifica el camino mas corto
    vector<int> caminoActual, caminoMasCorto;
    int distanciaMenor = INFI;
    // Analizo camino por camino (fila por fila)
    for (int i = 0; i < hamiltonian->cantCaminos - 1; ++i) {
        // Reinicio la distancia y el camino actual para cada iteracion
        int distancia=0;
        caminoActual.erase(caminoActual.begin(), caminoActual.end());
        // Voy vertice por vertice de la matriz (columna por columna)
        for (int j = 0; j < CANTIDADVERTICES; ++j) {
            // Va cargando al vector camino y calcula la distancia equivalente de ese vector
            caminoActual.push_back(matriz[i][j]);
            int aux1 = matriz[i][j], aux2 = matriz[i][j+1];
            distancia += matrizPesos[aux1][aux2];
        }
        int actual = distancia;
        // Compara si la distancia equivalente del camino actual es la mas chica de todas
        if (actual < distanciaMenor ){
            distanciaMenor = actual;
            caminoMasCorto = caminoActual;
        }
    }
    cout<<"\nEl camino mas corto es:  ";
    caminoMasCorto.erase(caminoMasCorto.begin() + CANTIDADVERTICES + 1, caminoMasCorto.end());
    cout<<"0";
    for (int i = 1; i < caminoMasCorto.size()-1; ++i) {
        cout<<" -> "<<caminoMasCorto[i];
    }
    cout<<" -> 0";
    // !Se supone que la distancia entre cada "pixel" son 10 metros
    cout << "\nCon una distancia aproximada de " << distanciaMenor*10 << " metros" << "\n" << endl;
    return 0;
}

/*
 * Este metodo construye una matriz que refleja lo observado en el .txt, 
 * estableciendo donde hay campo (00), maleza (01) o barrera (10)
 */ 
bool llenarMatrizDeBits() {
    char dato;
    int i = 0, j = 0;
    fstream myFile;
    string line, direccion = "../docs/text100.txt";
    
    // Abrimos el archivo
    myFile.open(direccion, ios::in);
    if (myFile.is_open()) {
        // Leemos fila por fila
        while (getline(myFile, line)) {
            istringstream isstream(line);
            // Leemos columna por columna
            while (!isstream.eof() && j < COLUMNAS) {
                isstream >> dato;
                if (dato == '1') {
                    matrizDeBits[i][j].set(0);  // Seteamos en 1 a b0 si hay maleza (01)
                } else if (dato == 'x' || dato == 'X') {
                    matrizDeBits[i][j].set(1);  // Seteamos en 1 a b1 si hay barrera (10)
                }
                j++;
            }
            j = 0;
            i++;
        }
        myFile.close();
        // El archivo se pudo abrir
        return true;
    } 
    // El archivo no se pudo abrir
    return false;
}

/*
 * Este metodo identifica parches de 3x3 de maleza o barreras prolongadas 
 * Guarda dichos parches y barreras en vectores destinados a ello y, para el caso de las barreras,
 * guarda su longitud y el espacio que ocupa tambien
 */ 
void filtrar() {
    // !Guardo al inicio como mancha para que el dron empiece y vuelva a ese lugar
    Posicion posIn;  
    posIn.x = 0; 
    posIn.y = 0;
    Mancha* Inicio = new Mancha(posIn);
    arrayManchas.push_back(Inicio);  

    // Recorro la matriz de bits fila por fila
    for (int filas = 0; filas < FILAS - 1; ++filas) {
        // Recorro columna por columna
        for (int columnas = 0; columnas < COLUMNAS - 1; ++columnas) {
            // Si b0 es 1 (osea hay maleza), comprueba que haya sea un parche de 3x3 (2 para la derecha y 2 para abajo)
            if (matrizDeBits[filas][columnas].test(0)) {
                revisarManchas(filas, columnas);
            }
            // Si b1 es 1 (osea hay barrera), identifica hasta donde se extiende
            if (matrizDeBits[filas][columnas].test(1)) {
                revisarBarreras(filas, columnas);
            }
        }
    }
}

/*
 * Este metodo se utiliza en filtrar()
 * Identifica y guarda todos los parches de maleza de 3x3 en la imagen satelital, junto con su posicion
 */
void revisarManchas(int y, int x) {
    // Prueba que efectivamente sea una mancha de al menos 3x3 (tener en cuenta que x,y es la esquina sup izq)
    bool prueba = matrizDeBits[y][x + 1].test(0) && matrizDeBits[y][x + 2].test(0) &&
                  matrizDeBits[y + 1][x].test(0) && matrizDeBits[y + 1][x + 1].test(0) && matrizDeBits[y + 1][x + 2].test(0) &&
                  matrizDeBits[y + 2][x].test(0) && matrizDeBits[y + 2][x + 1].test(0) && matrizDeBits[y + 2][x + 2].test(0);
    if (prueba) {
        // Creo una posición en el centro de la mancha
        Posicion posicion;  
        posicion.x = x + 1; 
        posicion.y = y + 1;
        Mancha* mancha = new Mancha(posicion);
        // Guardo la mancha
        arrayManchas.push_back(mancha);  

        // Una vez marcada la mancha, borro dos esquinas para no confundir el centro.
        matrizDeBits[y][x + 2].reset(0);  
        matrizDeBits[y + 2][x].reset(0);
    }
}

/*
 * Este metodo se utiliza en filtrar()
 * Identifica y guarda todas las barreras, junto con sus longitudes y posiciones
 */
void revisarBarreras(int y, int x) {
    // Guardo la posicion inicial de la barrera, que está modelada como una recta
    Barrera* barrera = new Barrera;
    barrera->p1.x = x;
    barrera->p1.y = y;

    // Para ir guardando la longitud de la barrera
    int contadorAuxiliar = 1;
    // Flag que marca que la barrera se sigue extendiendo
    bool flag = true;

    // Como se recorre al txt de izq a der y de arriba a abajo, se analizan los siguientes casos:
    // Si la barrera se extiende hacia la derecha
    if (matrizDeBits[y][x + 1].test(1)) {  
        contadorAuxiliar++;
        while (flag) {
            // Si la barrera se sigue extendiendo, voy guardando su longitud
            if (matrizDeBits[y][x + contadorAuxiliar].test(1)) {
                contadorAuxiliar++;
            } 
            // Una vez que se deja de extender, guardo hasta donde llegó
            else {
                // Guardo la posicion2
                barrera->p2.x = x + contadorAuxiliar;
                barrera->p2.y = y;
                flag = false;
                // Una vez identificada la barrera la saco del mapa para no tomarla de nuevo
                for (int i = barrera->p1.x; i < barrera->p2.x; ++i) {
                    matrizDeBits[barrera->p1.y][i].reset(1);
                }
            }
        }
    } 
    // Si la barrera se extiende hacia abajo
    if (matrizDeBits[y + 1][x].test(1)) { 
        contadorAuxiliar++;
        while (flag) {
            // Si la barrera se sigue extendiendo, voy guardando su longitud
            if (matrizDeBits[y + contadorAuxiliar][x].test(1)) {
                contadorAuxiliar++;
            } 
            // Una vez que se deja de extender, guardo hasta donde llegó
            else {
                // Guardo la posicion2
                barrera->p2.x = x;
                barrera->p2.y = y + contadorAuxiliar;
                flag = false;
                // Una vez identificada la barrera la saco del mapa para no tomarla de nuevo
                for (int i = barrera->p1.y; i < barrera->p2.y; ++i) {
                    matrizDeBits[i][barrera->p1.x].reset(1);
                }
            }
        }
    }
    // Guardo la barrera
    vectorBarreras.push_back(barrera);  
}

/*
 * Este metodo se utiliza en filtrar()
 * Identifica y guarda todas las barreras, junto con sus longitudes y posiciones
 */
void completarMatrizAdyacencia(){
    //Inicializo la matriz como si en ningun lugar hubiese paso.
    for (int i = 0; i < arrayManchas.size(); ++i){
        for (int j = 0; j < arrayManchas.size(); ++j){
            matrizAdyacencia[i][j] = 0;
            matrizPesos[i][j] = 0;
        }
    }

    //A medida que encuentro camino, lo asigno con la distancia euclideana.
    Posicion p1, p2, q1, q2;
    // Recorro todo el arreglo de manchas que fui guardando como si fuera una matriz de manchas
    for (int i = 0; i < arrayManchas.size(); ++i) {
        // Tomo la posicion de una mancha
        p1 = arrayManchas[i]->getPosicion();                         
        for (int j = i+1; j < arrayManchas.size(); ++j) {
            // Tomo la posicion de otra mancha
            q1 = arrayManchas[j]->getPosicion();                   

            // Creo un puntero auxiliar al vector barreras
            vector<Barrera*> aux=vectorBarreras;

            // Comparo hasta quedarme sin barreras en el vector
            while(!aux.empty()){
                // Tomo la posicion inicial de una barrera
                p2 = aux.back()->p1;   
                // Tomo la posicion final de esa misma barrera                             
                q2 = aux.back()->p2;  
                // Saco del vector a la barrera que estoy analizando                     
                aux.pop_back();
                
                /*
                * Si no hay interseccion entre la recta que forma la barrera y la recta que forman ambas manchas,
                * agrego a la matriz de adyacencia y establezco la distancia entre los parches en matrizPesos.
                */
                if (!doIntersect(p1, q1, p2, q2)){
                    matrizPesos[i][j] = calcularDistancia(p1, q1);
                    matrizAdyacencia[i][j] = matrizAdyacencia[j][i] = 1;
                } else {
                    matrizPesos[i][j] = INFI;
                    matrizAdyacencia[i][j] = matrizAdyacencia[j][i] = 0;
                    aux.erase(aux.begin(),aux.end()); //Borro el vector entero
                }
            }
        }
    }
    //Imprime a ver como quedo la matriz
    printMatrizAdyacencia();
}

/*
 * Esta funcion se utiliza en completarMatrizAdyacencia()
 * Retorna verdadero si existe un interseccion entre las dos rectas.
 * VER CARPETA DOCS, intersecciones.png para entender mejor el metodo.
 */
bool doIntersect(Posicion p1, Posicion q1, Posicion p2, Posicion q2) {
    // Encuentra  las 4 orientaciones fundamentales para los casos generales y especiales
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // !Caso general
    if (o1 != o2 && o3 != o4) return true;

    // !Casos especiales (colineales Y en el mismo segmento)
    // p1, q1 y p2 son colineales y p2 se encuentra en el segmento p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
    // p1, q1 y q2 son colineales y q2 se encuentra en el segmento p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
    // p2, q2 y p1 son colineales y p1 se encuentra en el segmento p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
    // p2, q2 y q1 son colineales y q1 se encuentra en el segmento p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false;  // !Ninguno de los casos anteriores (no se interseca)
}

/*
 * Este metodo se usa en doIntersect()
 * Para encontrar la orientacion de la 3-tupla (p, q, r).
 * La funcion retorna los siguientes valores:
 * 0 --> p, q y r son colineales
 * 1 --> Sentido horario
 * 2 --> Sentido anti-horario
 */
int orientation(Posicion p, Posicion q, Posicion r) {
    int val = (q.y - p.y)*(r.x - q.x) - (q.x - p.x)*(r.y - q.y);

    // colineal
    if (val == 0) return 0;  
    // sentido horario o anti-horario   
    return (val > 0) ? 1 : 2;  
}

/*
 * Este metodo se usa en doIntersect()
 * Dados tres puntos colineales [p,q,r] la funcion verifica si el punto q esta en el segmento pr
 */
bool onSegment(Posicion p, Posicion q, Posicion r) {
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
        return true;
    return false;
}

/*
 * Esta funcion se utiliza en completarMatrizAdyacencia()
 * Calcula la distancia euclidiana entre dos puntos de una superficie de dos dimensiones.
 */
int calcularDistancia(Posicion p1, Posicion q1) {
    return sqrt(pow(q1.x - p1.x, 2) + pow(q1.y - p1.y, 2));
}

void printMatrizAdyacencia() {
    cout << endl;
    cout << "Matriz de adyacencia: " << "\n" << endl;
    for (size_t x = 0; x <= arrayManchas.size(); ++x) {
        for (size_t y = 0; y <= arrayManchas.size(); ++y) {
            // Para imprimir los vertices
            if (x==0) {
                if (x!=y) {
                    cout << "\t" << y-1;
                } else {
                    cout << "\t" << ' ';
                }
            } else if (y==0) {
                cout << "\t" << x-1; 
            } 
            // Para imprimir la matriz en si
            else {
                cout << "\t" << matrizAdyacencia[x-1][y-1];
            }
        }
        cout << endl;
    }
}
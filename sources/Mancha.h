#ifndef MANCHA_H
#define MANCHA_H

// Se define una posicion en un plano 2D con dos variables enteras
struct Posicion {
    int x;
    int y;
};

class Mancha {
private:
    Posicion posicion;

public:
    Mancha(const Posicion &paux) : posicion(paux) {}

    const Posicion& getPosicion() const { return posicion; }
    void setPosicion(const Posicion &paux) { posicion = paux; }
};

#endif // MANCHA_H

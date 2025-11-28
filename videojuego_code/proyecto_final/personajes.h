#ifndef PERSONAJES_H
#define PERSONAJES_H

#include "ob.h"
// 2. CLASE  Personaje

class Personaje : public ObjetoJuego {
public:
    float velocidad;
    Personaje(float x, float y, int w, int h, float v);
    void mover(float dx, float dy);
    void dibujar()  override ;
};


// CLASE JUGADOR (Sabio Maya)
class Fragmento;
class Jugador : public Personaje {
public:
    int fragmentosSalvados;
    qint64 tiempoContactoFragMs;
    bool estaIntentandoRetener;
    Fragmento *fragmentoEnContactoActual;
    Jugador(float x, float y, float w, float h, float vel);
    void dibujar() ;

};

#endif // PERSONAJES_H

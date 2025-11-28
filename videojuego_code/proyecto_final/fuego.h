#ifndef FUEGO_H
#define FUEGO_H


#include "personajes.h"


// CLASE AGENTE FUEGO
class AgenteFuego : public Personaje {
public:

    AgenteFuego(float x, float y);
    void dibujar() ;
};

#endif // FUEGO_H

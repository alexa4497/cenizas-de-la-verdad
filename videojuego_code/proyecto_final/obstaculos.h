#ifndef OBSTACULOS_H
#define OBSTACULOS_H

#include "ob.h"

// CLASE MURO
class Muro : public ObjetoJuego {
public:
    Muro(float x, float y, int w, int h);
    void dibujar()  override ;

};

#endif // OB_H

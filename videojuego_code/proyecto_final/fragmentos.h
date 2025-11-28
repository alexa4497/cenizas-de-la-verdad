#ifndef FRAGMENTOS_H
#define FRAGMENTOS_H

#include "ob.h"
#include <QPixmap>


// CLASE FRAGMENTO

class Fragmento : public ObjetoJuego {
public:
    bool estaSalvado;
    bool estaQuemado;
    QPixmap imagenFragmento;

    Fragmento(float x, float y);
    void dibujar()  override ;
};

#endif // FRAGMENTOS_H

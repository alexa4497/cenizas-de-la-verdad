#include "fragmentos.h"

// Constructor
Fragmento::Fragmento(float x, float y)
    : ObjetoJuego(x, y, 20, 20), estaSalvado(false), estaQuemado(false) {
    imagenFragmento.load(":/imagenes/multimedia/imagenes/fragmento.png");
    imagenFragmento = imagenFragmento.scaled(ancho, alto, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


void Fragmento::dibujar() {

}

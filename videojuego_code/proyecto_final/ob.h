#ifndef OB_H
#define OB_H

#include <QRect>

class ObjetoJuego {
public:
    float pos_x, pos_y;
    int ancho, alto;

    ObjetoJuego(float x, float y, int w, int h) : pos_x(x), pos_y(y), ancho(w), alto(h) {}

    virtual ~ObjetoJuego() {}

    virtual void dibujar() = 0;

    QRect getRectanguloColision() const {
        return QRect(pos_x, pos_y, ancho, alto);
    }

    // Getters de Posición y Tamaño
    float getPos_x() const { return pos_x; }
    float getPos_y() const { return pos_y; }
    float getAncho() const { return ancho; }
    float getAlto() const { return alto; }

    // Setters de Posicion
    void setPos_x(float x) { pos_x = x; }
    void setPos_y(float y) { pos_y = y; }

};
#endif // OBJETOSJUEGO_H

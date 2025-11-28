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
};
#endif // OBJETOSJUEGO_H

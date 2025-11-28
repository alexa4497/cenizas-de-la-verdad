#ifndef PERSONAJE_H
#define PERSONAJE_H


// 2. CLASE  Personaje

class Personaje : public ObjetoJuego {
public:
    float velocidad;

    Personaje(float x, float y, int w, int h, float v)
        : ObjetoJuego(x, y, w, h), velocidad(v) {}

    void mover(float dx, float dy) {
        float magnitud = std::sqrt(dx * dx + dy * dy);
        if (magnitud > 0) {
            pos_x += (dx / magnitud) * velocidad;
            pos_y += (dy / magnitud) * velocidad;
        }
    }

};


#endif // PERSONAJE_H

#ifndef NIVELUNO_H
#define NIVELUNO_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include "objetosjuego.h"

namespace Ui {
class Niveluno;
}

class Niveluno : public QWidget
{
    Q_OBJECT

public:
    explicit Niveluno(QWidget *parent = nullptr);
    ~Niveluno();

private slots:
    void actualizarJuego();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::Niveluno *ui;

    // Atributos de la Lógica del Juego
    QTimer *timerJuego;
    QElapsedTimer timerElapsedJuego; // ¡RENOMBRADO! Usaremos este nombre en CPP

    Jugador sabioMaya; // Ya no es abstracta
    AgenteFuego agenteFuego; // Ya no es abstracta
    QVector<Fragmento> fragmentos;
    QVector<Muro> muros;

    // Constantes
    const int FRAGMENTOS_REQUERIDOS;
    const int TIEMPO_LIMITE_SEGUNDOS;
    const qint64 TIEMPO_RETENCION_MS;

    // Métodos para la lógica del juego
    void moverJugador(float dx, float dy);
    void inicializarFragmentos();
    void inicializarMuros();
    void verificarColisiones();
    void verificarEstadoJuego();
};

#endif // NIVELUNO_H

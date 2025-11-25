#ifndef NIVELUNO_H
#define NIVELUNO_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include "objetosjuego.h" // Asumo que contiene Muro, Fragmento, SabioMaya, etc.

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
    QElapsedTimer timerElapsedJuego;
    QTimer *timerReinicio;

    // Clases de juego
    Jugador sabioMaya;
    AgenteFuego agenteFuego;
    QVector<Fragmento> fragmentos;
    QVector<Muro> muros;
    int juegoEstado;


    // Constantes y Zonas de Fuego
    QList<Muro> zonasFuego; // OK: Declarada
    const int FRAGMENTOS_REQUERIDOS;
    const int TIEMPO_LIMITE_SEGUNDOS;
    const qint64 TIEMPO_RETENCION_MS;

    // Métodos de lógica del juego
    void moverJugador(float dx, float dy);
    void verificarColisiones();
    void verificarEstadoJuego();

    // Métodos de inicialización
    void inicializarFragmentos();
    void inicializarMuros();
    void inicializarZonasFuego();
    void reiniciarNivel();


};

#endif // NIVELUNO_H

#ifndef NIVELUNO_H
#define NIVELUNO_H

#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector>
#include <QPixmap> // ¡IMPORTANTE! Necesario para QPixmap
#include <QKeyEvent> // Necesario para keyPressEvent (aunque ya estaba en .cpp, mejor explicitarlo)
#include "objetosjuego.h" // Contiene Jugador, Muro, Fragmento, AgenteFuego, etc.

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
    void reiniciarNivel();
    void actualizarAnimacionFuego();
    void actualizarAnimacionMaya(); // <--- ¡NUEVO SLOT REQUERIDO!
    // -------------------------------------------------------------

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
    QVector<Muro> zonasFuego;

    int juegoEstado;

    // Constantes
    const int FRAGMENTOS_REQUERIDOS;
    const int TIEMPO_LIMITE_SEGUNDOS;
    const qint64 TIEMPO_RETENCION_MS;

    // ------------------------------------------------------------------
    // VARIABLES PARA LA ANIMACIÓN DEL FUEGO
    // ------------------------------------------------------------------
    QVector<QPixmap> framesFuego;
    int currentFrameFuegoIndex;
    QTimer *timerAnimacionFuego;

    // ------------------------------------------------------------------
    // NUEVAS VARIABLES PARA LA ANIMACIÓN DEL SABIO MAYA (¡SOLUCIÓN DE ERRORES!)
    // ------------------------------------------------------------------
    QPixmap fullSpriteSheetMaya;    // Hoja completa (240x240)
    QVector<QPixmap> framesMaya;    // Frames individuales (60x60)
    int currentFrameMayaIndex;      // Índice actual del frame (0-15)

    QTimer *timerAnimacionMaya;     // Timer para controlar la velocidad de la caminata
    int animRowOffset;              // Desplazamiento de fila (0, 4, 8, 12)
    bool isMoving;                  // Estado de movimiento (para animar solo al caminar)
    // ------------------------------------------------------------------

    // Métodos de lógica del juego
    void moverJugador(float dx, float dy);
    void verificarColisiones();
    void verificarEstadoJuego();

    // Métodos de inicialización
    void inicializarFragmentos();
    void inicializarMuros();
    void inicializarZonasFuego();
};

#endif // NIVELUNO_H

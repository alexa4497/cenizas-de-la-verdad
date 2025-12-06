#include "nivel_dos.h"
#include <QDebug>
#include <QMessageBox>
#include <cmath>
#include <algorithm>


Niveldos::Niveldos(QWidget *parent)
    : QWidget(parent)
{
    // Ventana de visualización más pequeña que el mapa
    this->resize(600, 600);
    this->setWindowTitle("Nivel 2: La Persecución del Laberinto");

    // Inicialización de Agentes y Entorno
    generarLaberinto();

    float playerStartX = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    float playerStartY = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    float inquisitorStartX = TILE_SIZE * 13 + TILE_SIZE / 2.0f;
    float inquisitorStartY = TILE_SIZE * 13 + TILE_SIZE / 2.0f;

    jugador = new Jugador(playerStartX, playerStartY);
    inquisidor = new Inquisidor(inquisitorStartX, inquisitorStartY, 40, 40, 1.5f);

    // Fragmentos (Objetivos)
    fragmentos.append(new Fragmento(TILE_SIZE * 5 + TILE_SIZE / 2.0f, TILE_SIZE * 3 + TILE_SIZE / 2.0f));
    fragmentos.append(new Fragmento(TILE_SIZE * 12 + TILE_SIZE / 2.0f, TILE_SIZE * 5 + TILE_SIZE / 2.0f));
    fragmentos.append(new Fragmento(TILE_SIZE * 2 + TILE_SIZE / 2.0f, TILE_SIZE * 11 + TILE_SIZE / 2.0f));

    // Game Loop (60 FPS)
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveldos::gameLoop);
    timerJuego->start(1000 / 60);

    // Timer para Pathfinding (Razonamiento y Percepción: 5 veces por segundo)
    timerPathfinding = new QTimer(this);
    connect(timerPathfinding, &QTimer::timeout, this, &Niveldos::actualizarRutaInquisidor);
    timerPathfinding->start(200);
}

Niveldos::~Niveldos() {
    delete jugador;
    delete inquisidor;
    qDeleteAll(fragmentos);
}

void Niveldos::generarLaberinto() {
    laberinto = {
        // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1}, // 1 (Jugador)
        {1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1}, // 2
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1}, // 3
        {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1}, // 4
        {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1}, // 5
        {1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1}, // 6
        {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1}, // 7
        {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1}, // 8
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 9
        {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1}, // 10
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1}, // 11
        {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1}, // 12
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 13 (Inquisidor)
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}  // 14
    };
    if (laberinto.size() != ROWS || laberinto[0].size() != COLS) {
        qWarning() << "El laberinto tiene dimensiones incorrectas.";
    }
}

void Niveldos::gameLoop() {
    // Acción: Mover el Inquisidor usando la ruta calculada
    inquisidor->ejecutarAccion(inquisidor->obtenerRutaActual(), (float)TILE_SIZE);

    actualizarScroll();
    checkColisiones();
    update();
}

void Niveldos::actualizarRutaInquisidor() {
    // Percepción: Obtener la celda del jugador
    QPoint celdaJugador = inquisidor->percibirObjetivo(jugador->getPos_x(), jugador->getPos_y());

    // Percepción: Obtener la celda del Inquisidor
    QPoint celdaInquisidor = obtenerCelda(inquisidor->getPos_x(), inquisidor->getPos_y());

    // Razonamiento: Calcular y almacenar la nueva ruta óptima (A*)
    // Esta función debe haber sido modificada para almacenar la ruta internamente
    inquisidor->calcularRuta(laberinto, celdaInquisidor, celdaJugador);
}

void Niveldos::checkColisiones() {
    // 1. Detección de Colisiones Jugador-Fragmento
    for (int i = 0; i < fragmentos.size(); ++i) {
        Fragmento *f = fragmentos[i];
        if (!f->estaSalvado &&
            std::abs(jugador->getPos_x() - f->getPos_x()) < (jugador->getAncho() / 2 + f->getAncho() / 2) &&
            std::abs(jugador->getPos_y() - f->getPos_y()) < (jugador->getAlto() / 2 + f->getAlto() / 2))
        {
            f->estaSalvado = true;
            qDebug() << "Fragmento salvado!";
        }
    }

    // 2. Detección de Colisiones Jugador-Inquisidor (FIN DEL JUEGO)
    if (std::abs(jugador->getPos_x() - inquisidor->getPos_x()) < (jugador->getAncho() / 2 + inquisidor->getAncho() / 2) &&
        std::abs(jugador->getPos_y() - inquisidor->getPos_y()) < (jugador->getAlto() / 2 + inquisidor->getAlto() / 2))
    {
        timerJuego->stop();
        timerPathfinding->stop();
        QMessageBox::information(this, "Fin del Juego", "¡Has sido capturado por el Inquisidor!");
        this->close();
    }
}

void Niveldos::keyPressEvent(QKeyEvent *event) {
    float newX = jugador->getPos_x();
    float newY = jugador->getPos_y();
    float velocidad = jugador->getVelocidad();

    switch (event->key()) {
    case Qt::Key_W: newY -= velocidad; break;
    case Qt::Key_S: newY += velocidad; break;
    case Qt::Key_A: newX -= velocidad; break;
    case Qt::Key_D: newX += velocidad; break;
    default: QWidget::keyPressEvent(event); return;
    }

    // Comprobar colisiones con Muros
    if (!esMuro(newX - jugador->getAncho() / 2, newY - jugador->getAlto() / 2) &&
        !esMuro(newX + jugador->getAncho() / 2, newY - jugador->getAlto() / 2) &&
        !esMuro(newX - jugador->getAncho() / 2, newY + jugador->getAlto() / 2) &&
        !esMuro(newX + jugador->getAncho() / 2, newY + jugador->getAlto() / 2))
    {
        jugador->setPos_x(newX);
        jugador->setPos_y(newY);
    }
}


void Niveldos::actualizarScroll() {
    const float SCROLL_ZONE = 150; // Zona de margen donde se activa el scroll
    const float WINDOW_W = 600;
    const float WINDOW_H = 600;

    float playerX_in_window = jugador->getPos_x() - offsetX;
    float playerY_in_window = jugador->getPos_y() - offsetY;

    // Actualizar offsetX
    if (playerX_in_window > WINDOW_W - SCROLL_ZONE) {
        offsetX += playerX_in_window - (WINDOW_W - SCROLL_ZONE);
    } else if (playerX_in_window < SCROLL_ZONE) {
        offsetX += playerX_in_window - SCROLL_ZONE;
    }

    // Actualizar offsetY
    if (playerY_in_window > WINDOW_H - SCROLL_ZONE) {
        offsetY += playerY_in_window - (WINDOW_H - SCROLL_ZONE);
    } else if (playerY_in_window < SCROLL_ZONE) {
        offsetY += playerY_in_window - SCROLL_ZONE;
    }

    // Limitar el Scroll
    offsetX = std::max(0.0f, std::min(offsetX, (float)MAX_SCROLL_X));
    offsetY = std::max(0.0f, std::min(offsetY, (float)MAX_SCROLL_Y));
}

void Niveldos::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Aplicar el scroll a la vista
    painter.translate(-offsetX, -offsetY);

    // 1. Dibujar el Laberinto
    QColor wallColor(100, 100, 100);
    QColor pathColor(200, 200, 200);

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            QRect rect(j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE);
            painter.fillRect(rect, laberinto[i][j] == 1 ? wallColor : pathColor);
        }
    }

    // 2. Dibujar Fragmentos
    for (Fragmento *f : fragmentos) {
        if (!f->estaSalvado) {
            painter.drawPixmap(f->getPos_x() - f->getAncho() / 2, f->getPos_y() - f->getAlto() / 2,
                               f->getAncho(), f->getAlto(), f->imagenFragmento);
        }
    }

    // 3. Dibujar Agentes (Jugador e Inquisidor)
    painter.setPen(Qt::NoPen);

    // Jugador (círculo verde)
    painter.setBrush(Qt::green);
    painter.drawEllipse(QPointF(jugador->getPos_x(), jugador->getPos_y()),
                        jugador->getAncho() / 2, jugador->getAlto() / 2);

    // Inquisidor (círculo rojo)
    painter.setBrush(Qt::red);
    painter.drawEllipse(QPointF(inquisidor->getPos_x(), inquisidor->getPos_y()),
                        inquisidor->getAncho() / 2, inquisidor->getAlto() / 2);
}

bool Niveldos::esMuro(float x, float y) const {
    QPoint celda = obtenerCelda(x, y);
    if (celda.x() < 0 || celda.x() >= COLS || celda.y() < 0 || celda.y() >= ROWS) {
        return true;
    }
    return laberinto[celda.y()][celda.x()] == 1;
}

QPoint Niveldos::obtenerCelda(float x, float y) const {
    return QPoint(static_cast<int>(x / TILE_SIZE), static_cast<int>(y / TILE_SIZE));
}

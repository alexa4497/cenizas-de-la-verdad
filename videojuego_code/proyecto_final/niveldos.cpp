#include "niveldos.h"
#include <QDebug>
#include <QMessageBox>
#include <cmath>
#include <algorithm>
#include <QPushButton>
#include <QPainter>
#include <QCloseEvent>
Niveldos::Niveldos(QWidget *parent)
    : QWidget(parent), timerJuego(nullptr), timerPathfinding(nullptr), timerAnimacion(nullptr),
    musicaFondo(nullptr), audioOutput(nullptr),
    jugador(nullptr), inquisidor(nullptr),
    frameIndexJugador(0), frameIndexInquisidor(0),
    direccionJugador(2), direccionInquisidor(2)
{
    // Ventana de visualizacion
    this->resize(600, 600);
    this->setWindowTitle("Nivel 2: El Laberinto del Conocimiento");
    setFocusPolicy(Qt::StrongFocus);

    // Cargar audio
    cargarAudio();

    // Cargar texturas y sprites
    if (!cargarTexturas() || !cargarSprites()) {
        QMessageBox::warning(this, "Error", "No se pudieron cargar algunas texturas o sprites.");
    }

    // Inicializacion del laberinto
    generarLaberinto();

    // Posiciones iniciales
    float playerStartX = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    float playerStartY = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    float inquisitorStartX = TILE_SIZE * 28 + TILE_SIZE / 2.0f;
    float inquisitorStartY = TILE_SIZE * 19 + TILE_SIZE / 2.0f;

    // Crear jugador
    jugador = new Jugador(playerStartX, playerStartY, 40, 40, 2.8f);

    // Crear inquisidor
    inquisidor = new Inquisidor(inquisitorStartX, inquisitorStartY, 40, 40, 1.0f);

    // ========== FRAGMENTOS ==========

    fragmentos.append(new Fragmento(TILE_SIZE * 27 + TILE_SIZE / 2.0f, TILE_SIZE * 1 + TILE_SIZE / 2.0f));   // (27,1)
    fragmentos.append(new Fragmento(TILE_SIZE * 21 + TILE_SIZE / 2.0f, TILE_SIZE * 3 + TILE_SIZE / 2.0f));   // (21,3)
    fragmentos.append(new Fragmento(TILE_SIZE * 7 + TILE_SIZE / 2.0f,  TILE_SIZE * 5 + TILE_SIZE / 2.0f));   // (7,5)
    fragmentos.append(new Fragmento(TILE_SIZE * 13 + TILE_SIZE / 2.0f, TILE_SIZE * 7 + TILE_SIZE / 2.0f));   // (13,7)
    fragmentos.append(new Fragmento(TILE_SIZE * 25 + TILE_SIZE / 2.0f, TILE_SIZE * 9 + TILE_SIZE / 2.0f));   // (25,9)
    fragmentos.append(new Fragmento(TILE_SIZE * 5 + TILE_SIZE / 2.0f,  TILE_SIZE * 11 + TILE_SIZE / 2.0f));  // (5,11)
    fragmentos.append(new Fragmento(TILE_SIZE * 15 + TILE_SIZE / 2.0f, TILE_SIZE * 13 + TILE_SIZE / 2.0f));  // (15,13)
    fragmentos.append(new Fragmento(TILE_SIZE * 28 + TILE_SIZE / 2.0f, TILE_SIZE * 17 + TILE_SIZE / 2.0f));  // (28,17)

    // ========== VERIFICAR Y FORZAR estado inicial ==========
    qDebug() << "=== INICIALIZANDO FRAGMENTOS ===";
    for (int i = 0; i < fragmentos.size(); i++) {
        Fragmento* f = fragmentos[i];

        // FORZAR que empiece no salvado
        f->estaSalvado = false;

        // Verificar posición
        int celdaX = static_cast<int>(f->getPos_x() / TILE_SIZE);
        int celdaY = static_cast<int>(f->getPos_y() / TILE_SIZE);

        if (celdaY >= 0 && celdaY < ROWS && celdaX >= 0 && celdaX < COLS) {
            if (laberinto[celdaY][celdaX] == 0) {
                qDebug() << "Fragmento " << i+1 << ": (" << celdaX << "," << celdaY
                         << ") - PASILLO - estaSalvado: " << f->estaSalvado;
            } else {
                qDebug() << "¡ERROR! Fragmento " << i+1 << " en PARED ("
                         << celdaX << "," << celdaY << ")";
            }
        }
    }
    qDebug() << "Total fragmentos: " << fragmentos.size();


    // Game Loop (60 FPS)
    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveldos::gameLoop);
    timerJuego->start(1000 / 60);

    // Timer para Pathfinding
    timerPathfinding = new QTimer(this);
    connect(timerPathfinding, &QTimer::timeout, this, &Niveldos::actualizarRutaInquisidor);
    timerPathfinding->start(500);

    // Timer para animaciones (10 FPS)
    timerAnimacion = new QTimer(this);
    connect(timerAnimacion, &QTimer::timeout, this, &Niveldos::actualizarAnimaciones);
    timerAnimacion->start(100);

    inicializarUI();
}
void Niveldos::inicializarUI() {
    // Crear y configurar el botón de salir
    btnSalir = new QPushButton("Salir", this);
    btnSalir->setGeometry(width() - 100, 10, 80, 30);
    btnSalir->setStyleSheet(
        "QPushButton {"
        "    background-color: #d32f2f;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #b71c1c;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #7f0000;"
        "}"
        );

    connect(btnSalir, &QPushButton::clicked, this, &Niveldos::onSalirClicked);
}

void Niveldos::onSalirClicked() {
    if (timerJuego && timerJuego->isActive()) timerJuego->stop();
    if (timerPathfinding && timerPathfinding->isActive()) timerPathfinding->stop();
    if (timerAnimacion && timerAnimacion->isActive()) timerAnimacion->stop();

    if (musicaFondo) {
        musicaFondo->stop();
    }

    emit regresarMenuPrincipal();
    this->close();
}


void Niveldos::reiniciarNivel() {

    // 1. DETENER TIMERS
    if (timerJuego && timerJuego->isActive()) timerJuego->stop();
    if (timerPathfinding && timerPathfinding->isActive()) timerPathfinding->stop();
    if (timerAnimacion && timerAnimacion->isActive()) timerAnimacion->stop();

    // 2. ELIMINAR FRAGMENTOS EXISTENTES
    for (Fragmento* f : fragmentos) {
        if (f) delete f;
    }
    fragmentos.clear();

    // 3. ELIMINAR JUGADOR E INQUISIDOR
    delete jugador;
    delete inquisidor;

    // 4. RECREAR JUGADOR
    float playerStartX = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    float playerStartY = TILE_SIZE * 1 + TILE_SIZE / 2.0f;
    jugador = new Jugador(playerStartX, playerStartY, 40, 40, 2.8f);

    // 5. RECREAR INQUISIDOR
    float inquisitorStartX = TILE_SIZE * 28 + TILE_SIZE / 2.0f;
    float inquisitorStartY = TILE_SIZE * 19 + TILE_SIZE / 2.0f;
    inquisidor = new Inquisidor(inquisitorStartX, inquisitorStartY, 40, 40, 1.0f);


    // 6. RECREAR FRAGMENTOS CON LAS MISMAS POSICIONES
    fragmentos.append(new Fragmento(TILE_SIZE * 27 + TILE_SIZE / 2.0f, TILE_SIZE * 1 + TILE_SIZE / 2.0f));   // (27,1)
    fragmentos.append(new Fragmento(TILE_SIZE * 21 + TILE_SIZE / 2.0f, TILE_SIZE * 3 + TILE_SIZE / 2.0f));   // (21,3)
    fragmentos.append(new Fragmento(TILE_SIZE * 7 + TILE_SIZE / 2.0f,  TILE_SIZE * 5 + TILE_SIZE / 2.0f));   // (7,5)
    fragmentos.append(new Fragmento(TILE_SIZE * 13 + TILE_SIZE / 2.0f, TILE_SIZE * 7 + TILE_SIZE / 2.0f));   // (13,7)
    fragmentos.append(new Fragmento(TILE_SIZE * 25 + TILE_SIZE / 2.0f, TILE_SIZE * 9 + TILE_SIZE / 2.0f));   // (25,9)
    fragmentos.append(new Fragmento(TILE_SIZE * 5 + TILE_SIZE / 2.0f,  TILE_SIZE * 11 + TILE_SIZE / 2.0f));  // (5,11)
    fragmentos.append(new Fragmento(TILE_SIZE * 15 + TILE_SIZE / 2.0f, TILE_SIZE * 13 + TILE_SIZE / 2.0f));  // (15,13)
    fragmentos.append(new Fragmento(TILE_SIZE * 28 + TILE_SIZE / 2.0f, TILE_SIZE * 17 + TILE_SIZE / 2.0f));  // (28,17)

    // 7. FORZAR que NINGUN fragmento empiece como salvado
    for (int i = 0; i < fragmentos.size(); i++) {
        Fragmento* f = fragmentos[i];
        f->estaSalvado = false;  // ¡ESTO ES CRÍTICO!

        qDebug() << "Fragmento " << i+1 << " - estaSalvado: " << f->estaSalvado;
    }

    // 8. RESETEAR VARIABLES
    offsetX = 0.0f;
    offsetY = 0.0f;
    frameIndexJugador = 0;
    frameIndexInquisidor = 0;
    direccionJugador = 2;
    direccionInquisidor = 2;

    // 9. REINICIAR TIMERS
    if (timerJuego) timerJuego->start(1000 / 60);
    if (timerPathfinding) timerPathfinding->start(500);
    if (timerAnimacion) timerAnimacion->start(100);

    update();
}


void Niveldos::mostrarMenuFinJuego(const QString& titulo, const QString& mensaje) {
    if (timerJuego && timerJuego->isActive()) timerJuego->stop();

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(titulo);
    msgBox.setText(mensaje);
    msgBox.setIcon(QMessageBox::Information);

    QPushButton *btnReintentar = msgBox.addButton(tr("Volver a Jugar"), QMessageBox::ActionRole);
    QPushButton *btnSalir = msgBox.addButton(tr("Salir al Menú"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(btnReintentar);

    msgBox.exec();

    QAbstractButton *clickedButton = msgBox.clickedButton();


}

void Niveldos::cargarAudio() {
    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(0.4f);

    musicaFondo = new QMediaPlayer(this);
    musicaFondo->setAudioOutput(audioOutput);
    musicaFondo->setSource(QUrl("qrc:/sonido/multimedia/audio/audio_nivel2.ogg"));
    musicaFondo->setLoops(QMediaPlayer::Infinite);
    musicaFondo->play();
}

bool Niveldos::cargarTexturas() {
    texturaPared.load(":/imagenes/multimedia/imagenes/paredes_nivel2.png");
    if (texturaPared.isNull()) {
        texturaPared = QPixmap(50, 50);
        texturaPared.fill(QColor(100, 70, 50));
    } else {
        texturaPared = texturaPared.scaled(50, 50, Qt::KeepAspectRatioByExpanding);
    }

    texturaSuelo.load(":/imagenes/multimedia/imagenes/textura_piso_nivel2.png");
    if (texturaSuelo.isNull()) {
        texturaSuelo = QPixmap(50, 50);
        texturaSuelo.fill(QColor(200, 180, 150));
    } else {
        texturaSuelo = texturaSuelo.scaled(50, 50, Qt::KeepAspectRatioByExpanding);
    }

    texturaFragmento.load(":/imagenes/multimedia/imagenes/pergamino_pequeno.png");
    if (texturaFragmento.isNull()) {
        texturaFragmento = QPixmap(40, 40);
        texturaFragmento.fill(QColor(200, 200, 50));
    } else {
        texturaFragmento = texturaFragmento.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return true;
}

bool Niveldos::cargarSprites() {
    spriteJugador.load(":/imagenes/multimedia/imagenes/jugadornivel2final.png");
    if (spriteJugador.isNull()) {
        qDebug() << "No se pudo cargar sprite sheet del jugador";
        return false;
    }

    framesJugador.clear();
    int frameWidth = 50;
    int frameHeight = 50;

    for (int fila = 0; fila < 4; ++fila) {
        for (int col = 0; col < 6; ++col) {
            QPixmap frame = spriteJugador.copy(col * frameWidth,
                                               fila * frameHeight,
                                               frameWidth,
                                               frameHeight);
            framesJugador.append(frame.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    spriteInquisidor.load(":/imagenes/multimedia/imagenes/inquisidorsprite.png");
    if (spriteInquisidor.isNull()) {
        qDebug() << "No se pudo cargar sprite sheet del inquisidor";
        return false;
    }

    framesInquisidor.clear();
    for (int fila = 0; fila < 4; ++fila) {
        for (int col = 0; col < 6; ++col) {
            QPixmap frame = spriteInquisidor.copy(col * frameWidth,
                                                  fila * frameHeight,
                                                  frameWidth,
                                                  frameHeight);
            framesInquisidor.append(frame.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    return true;
}

QPixmap Niveldos::obtenerFrameJugador() {
    if (framesJugador.isEmpty()) return QPixmap();

    int baseFrame = direccionJugador * 6;
    int frameActual = baseFrame + (frameIndexJugador % 6);

    if (frameActual >= framesJugador.size()) {
        return framesJugador[0];
    }
    return framesJugador[frameActual];
}

QPixmap Niveldos::obtenerFrameInquisidor() {
    if (framesInquisidor.isEmpty()) return QPixmap();

    int baseFrame = direccionInquisidor * 6;
    int frameActual = baseFrame + (frameIndexInquisidor % 6);

    if (frameActual >= framesInquisidor.size()) {
        return framesInquisidor[0];
    }
    return framesInquisidor[frameActual];
}

void Niveldos::actualizarAnimaciones() {
    frameIndexJugador = (frameIndexJugador + 1) % 6;
    frameIndexInquisidor = (frameIndexInquisidor + 1) % 6;
    update();
}

void Niveldos::generarLaberinto() {
    laberinto = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,0,1},
        {1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,0,0,1,0,1,1,1,0,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };
}

void Niveldos::actualizarRutaInquisidor() {
    QPoint celdaJugador = obtenerCelda(jugador->getPos_x(), jugador->getPos_y());
    QPoint celdaInquisidor = obtenerCelda(inquisidor->getPos_x(), inquisidor->getPos_y());

    QVector<QPoint> nuevaRuta = inquisidor->calcularRuta(laberinto, celdaInquisidor, celdaJugador);

    if (!nuevaRuta.isEmpty()) {
        inquisidor->establecerRuta(nuevaRuta);
    }
}

QPoint Niveldos::obtenerCelda(float x, float y) const {
    return QPoint(static_cast<int>(x / TILE_SIZE), static_cast<int>(y / TILE_SIZE));
}

void Niveldos::actualizarScroll() {
    const float SCROLL_ZONE = 150;
    const float WINDOW_W = 600;
    const float WINDOW_H = 600;

    float playerX_in_window = jugador->getPos_x() - offsetX;
    float playerY_in_window = jugador->getPos_y() - offsetY;

    if (playerX_in_window > WINDOW_W - SCROLL_ZONE) {
        offsetX += playerX_in_window - (WINDOW_W - SCROLL_ZONE);
    } else if (playerX_in_window < SCROLL_ZONE) {
        offsetX += playerX_in_window - SCROLL_ZONE;
    }

    if (playerY_in_window > WINDOW_H - SCROLL_ZONE) {
        offsetY += playerY_in_window - (WINDOW_H - SCROLL_ZONE);
    } else if (playerY_in_window < SCROLL_ZONE) {
        offsetY += playerY_in_window - SCROLL_ZONE;
    }

    offsetX = std::max(0.0f, std::min(offsetX, (float)MAX_SCROLL_X));
    offsetY = std::max(0.0f, std::min(offsetY, (float)MAX_SCROLL_Y));
}

void Niveldos::checkColisiones() {
    QRect rectJugador = jugador->getRectanguloColision();
    int fragmentosRecogidos = 0;

    for (Fragmento *f : fragmentos) {
        if (!f->estaSalvado) {
            QRect rectFragmento = f->getRectanguloColision();

            // Área de colisión más grande
            QRect rectFragmentoAmpliado = rectFragmento.adjusted(-15, -15, 15, 15);

            if (rectJugador.intersects(rectFragmentoAmpliado)) {
                f->estaSalvado = true;
                qDebug() << "Fragmento recogido!";
            }
        }

        if (f->estaSalvado) {
            fragmentosRecogidos++;
        }
    }

    QRect rectInquisidor = inquisidor->getRectanguloColision();
    if (rectJugador.intersects(rectInquisidor)) {
        qDebug() << "¡Capturado por el inquisidor!";
        mostrarMenuFinJuego("Derrota", "¡Has sido capturado por el Inquisidor!");
        return;
    }

    if (fragmentosRecogidos == fragmentos.size()) {
        qDebug() << "¡VICTORIA! Todos los fragmentos recogidos";
        mostrarMenuFinJuego("Victoria", "¡VICTORIA! Has recogido todos los fragmentos.");
    }
}

bool Niveldos::esMuro(float x, float y) const {
    QPoint celda = obtenerCelda(x, y);
    if (celda.x() < 0 || celda.x() >= COLS || celda.y() < 0 || celda.y() >= ROWS) {
        return true;
    }
    return laberinto[celda.y()][celda.x()] == 1;
}

void Niveldos::gameLoop() {
    inquisidor->ejecutarAccion(inquisidor->obtenerRutaActual(), (float)TILE_SIZE);

    QPoint celdaActual = obtenerCelda(inquisidor->getPos_x(), inquisidor->getPos_y());
    static QPoint ultimaCelda = celdaActual;

    if (celdaActual != ultimaCelda) {
        int deltaX = celdaActual.x() - ultimaCelda.x();
        int deltaY = celdaActual.y() - ultimaCelda.y();

        if (deltaY < 0) direccionInquisidor = 0;
        else if (deltaY > 0) direccionInquisidor = 2;
        else if (deltaX > 0) direccionInquisidor = 3;
        else if (deltaX < 0) direccionInquisidor = 1;

        ultimaCelda = celdaActual;
    }

    inquisidor->avanzarRuta(TILE_SIZE);
    actualizarScroll();
    checkColisiones();
}

void Niveldos::keyPressEvent(QKeyEvent *event)
{
    float newX = jugador->getPos_x();
    float newY = jugador->getPos_y();
    float velocidad = jugador->getVelocidad();
    bool moved = false;

    switch (event->key()) {
    case Qt::Key_W:
        newY -= velocidad;
        direccionJugador = 0;
        moved = true;
        break;
    case Qt::Key_S:
        newY += velocidad;
        direccionJugador = 2;
        moved = true;
        break;
    case Qt::Key_A:
        newX -= velocidad;
        direccionJugador = 1;
        moved = true;
        break;
    case Qt::Key_D:
        newX += velocidad;
        direccionJugador = 3;
        moved = true;
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    if (!esMuro(newX - jugador->getAncho() / 2, newY - jugador->getAlto() / 2) &&
        !esMuro(newX + jugador->getAncho() / 2, newY - jugador->getAlto() / 2) &&
        !esMuro(newX - jugador->getAncho() / 2, newY + jugador->getAlto() / 2) &&
        !esMuro(newX + jugador->getAncho() / 2, newY + jugador->getAlto() / 2))
    {
        jugador->setPos_x(newX);
        jugador->setPos_y(newY);
    }

    if (moved) {
        update();
    }
}

void Niveldos::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.translate(-offsetX, -offsetY);

    // Dibujar laberinto
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            QRect rect(j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE);

            if (laberinto[i][j] == 1) {
                if (!texturaPared.isNull()) {
                    painter.drawPixmap(rect, texturaPared);
                } else {
                    painter.fillRect(rect, QColor(100, 70, 50));
                }
            } else {
                if (!texturaSuelo.isNull()) {
                    painter.drawPixmap(rect, texturaSuelo);
                } else {
                    painter.fillRect(rect, QColor(200, 180, 150));
                }
            }
        }
    }

    // Dibujar fragmentos NO RECOGIDOS
    for (Fragmento *f : fragmentos) {
        if (!f->estaSalvado) {
            // Sombra
            painter.setBrush(QColor(0, 0, 0, 80));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QPointF(f->getPos_x(), f->getPos_y() + 3),
                                f->getAncho() / 2, f->getAlto() / 4);

            // Fragmento
            if (!texturaFragmento.isNull()) {
                painter.drawPixmap(f->getPos_x() - f->getAncho() / 2,
                                   f->getPos_y() - f->getAlto() / 2,
                                   texturaFragmento.width(),
                                   texturaFragmento.height(),
                                   texturaFragmento);
            } else {
                painter.setBrush(QColor(255, 215, 0));
                painter.setPen(Qt::black);
                painter.drawEllipse(QPointF(f->getPos_x(), f->getPos_y()),
                                    f->getAncho() / 2, f->getAlto() / 2);
            }
        }
    }

    // Dibujar sombras
    painter.setBrush(QColor(0, 0, 0, 60));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(jugador->getPos_x(), jugador->getPos_y() + 20),
                        jugador->getAncho() / 2, jugador->getAlto() / 4);

    painter.drawEllipse(QPointF(inquisidor->getPos_x(), inquisidor->getPos_y() + 20),
                        inquisidor->getAncho() / 2, inquisidor->getAlto() / 4);

    // Dibujar jugador
    QPixmap frameJugador = obtenerFrameJugador();
    if (!frameJugador.isNull()) {
        painter.drawPixmap(jugador->getPos_x() - jugador->getAncho() / 2,
                           jugador->getPos_y() - jugador->getAlto() / 2,
                           jugador->getAncho(),
                           jugador->getAlto(),
                           frameJugador);
    } else {
        painter.setBrush(QColor(50, 200, 50));
        painter.setPen(Qt::white);
        painter.drawEllipse(QPointF(jugador->getPos_x(), jugador->getPos_y()),
                            jugador->getAncho() / 2, jugador->getAlto() / 2);
    }

    // Dibujar inquisidor
    QPixmap frameInquisidor = obtenerFrameInquisidor();
    if (!frameInquisidor.isNull()) {
        painter.drawPixmap(inquisidor->getPos_x() - inquisidor->getAncho() / 2,
                           inquisidor->getPos_y() - inquisidor->getAlto() / 2,
                           inquisidor->getAncho(),
                           inquisidor->getAlto(),
                           frameInquisidor);
    } else {
        painter.setBrush(QColor(200, 50, 50));
        painter.setPen(Qt::white);
        painter.drawEllipse(QPointF(inquisidor->getPos_x(), inquisidor->getPos_y()),
                            inquisidor->getAncho() / 2, inquisidor->getAlto() / 2);
    }

    // HUD
    painter.resetTransform();
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.setPen(Qt::NoPen);
    painter.drawRect(10, height() - 80, 200, 70);

    painter.setPen(Qt::white);
    painter.setFont(QFont("Times New Roman", 12, QFont::Bold));

    int fragmentosRestantes = 0;
    for (Fragmento *f : fragmentos) {
        if (!f->estaSalvado) fragmentosRestantes++;
    }

    painter.drawText(20, height() - 50, QString("Fragmentos: %1/8").arg(8 - fragmentosRestantes));

    // Dibujar barra de progreso
    float progreso = (8.0f - fragmentosRestantes) / 8.0f;
    painter.setBrush(QColor(50, 150, 50, 180));
    painter.drawRect(20, height() - 30, 180 * progreso, 15);

    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(20, height() - 30, 180, 15);
}

void Niveldos::closeEvent(QCloseEvent *event) {
    if (timerJuego && timerJuego->isActive()) timerJuego->stop();
    if (timerPathfinding && timerPathfinding->isActive()) timerPathfinding->stop();
    if (timerAnimacion && timerAnimacion->isActive()) timerAnimacion->stop();

    if (musicaFondo) {
        musicaFondo->stop();
    }

    event->accept();
}

Niveldos::~Niveldos() {
    if (musicaFondo) {
        musicaFondo->stop();
        delete musicaFondo;
    }

    if (audioOutput) {
        delete audioOutput;
    }

    if (timerJuego) {
        timerJuego->stop();
        delete timerJuego;
    }
    if (timerPathfinding) {
        timerPathfinding->stop();
        delete timerPathfinding;
    }
    if (timerAnimacion) {
        timerAnimacion->stop();
        delete timerAnimacion;
    }

    delete jugador;
    delete inquisidor;
    qDeleteAll(fragmentos);
}

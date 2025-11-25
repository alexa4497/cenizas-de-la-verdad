#include "niveluno.h"
#include "ui_niveluno.h"
#include <QDebug>
#include <QPainter>
#include <QKeyEvent>
#include <algorithm>

// Definición de constantes de tamaño del nivel
const int NIVEL_WIDTH = 1250;
const int NIVEL_HEIGHT = 650;

// Constructor
Niveluno::Niveluno(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Niveluno)
    // Inicialización del Jugador y Agente
    , sabioMaya(100.0f, 100.0f)
    , agenteFuego(600.0f, 600.0f)
    // Inicialización de Constantes
    , FRAGMENTOS_REQUERIDOS(30)
    , TIEMPO_LIMITE_SEGUNDOS(60)
    , TIEMPO_RETENCION_MS(3000)
{
    ui->setupUi(this);
    this->resize(NIVEL_WIDTH, NIVEL_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);

    timerJuego = new QTimer(this);
    connect(timerJuego, &QTimer::timeout, this, &Niveluno::actualizarJuego);

    inicializarFragmentos();
    inicializarMuros();

    // Inicio del Juego
    timerElapsedJuego.start();
    timerJuego->start(16); // ~60 FPS
    qDebug() << "Nivel 1 Iniciado. Tiempo límite:" << TIEMPO_LIMITE_SEGUNDOS << "segundos.";
}



Niveluno::~Niveluno()
{
    delete ui;
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE INICIALIZACIÓN
// ----------------------------------------------------

void Niveluno::inicializarFragmentos() {
    fragmentos.clear();

    fragmentos.append(Fragmento(50, 60));
    fragmentos.append(Fragmento(650, 60));
    fragmentos.append(Fragmento(150, 450));
    fragmentos.append(Fragmento(550, 300));
    fragmentos.append(Fragmento(450, 500));

    for (int i = fragmentos.size(); i < FRAGMENTOS_REQUERIDOS; ++i) {
        float x = 50 + (i % 6) * 80;
        float y = 50 + (i / 5) * 80;
        if (x > 300 && x < 400) continue;
        fragmentos.append(Fragmento(x, y));
    }
}

void Niveluno::inicializarMuros() {
    muros.clear();
    const int muro_grosor = 30; // <-- Cambia este valor (ej: a 30) para muros más gruesos.

    // ----------------------------------------------------
    // 1. Muros del Borde Exterior (Garantizan la cobertura del 100%)
    // ----------------------------------------------------

    // Muro Superior (desde x=0 hasta el final, en y=0)
    muros.append(Muro(0, 0, NIVEL_WIDTH, muro_grosor));

    // Muro Inferior (desde x=0 hasta el final, en la base de la pantalla)
    muros.append(Muro(0, NIVEL_HEIGHT - muro_grosor, NIVEL_WIDTH, muro_grosor));

    // Muro Izquierdo (Desde el fin del Superior (y=muro_grosor) hasta el inicio del Inferior)
    muros.append(Muro(0, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));

    // Muro Derecho (Desde el fin del Superior (y=muro_grosor) hasta el inicio del Inferior)
    muros.append(Muro(NIVEL_WIDTH - muro_grosor, muro_grosor, muro_grosor, NIVEL_HEIGHT - 2 * muro_grosor));

    // ----------------------------------------------------
    // 2. Muros Interiores (Modifica y añade aquí tus propios diseños)
    // ----------------------------------------------------

    // Ejemplo de cómo añadir un muro horizontal (si quieres que encaje con el borde):
    // muros.append(Muro(muro_grosor, 150, 200, muro_grosor));

    // Ejemplo de cómo añadir un muro vertical:
    // muros.append(Muro(250, muro_grosor, muro_grosor, 150));

    // *** Espacio para que modifiques tus muros interiores ***

    // Puedes usar la lógica de los muros anteriores como guía para rellenar el laberinto:

    // Muro horizontal cerca de la parte superior
    muros.append(Muro(muro_grosor, 120, 300 - muro_grosor, muro_grosor));
    // Muro vertical central
    muros.append(Muro(400, 120, muro_grosor, 300 - muro_grosor));
    // Otro muro horizontal
    muros.append(Muro(muro_grosor, 400, 380 - muro_grosor, muro_grosor));
    // Muro horizontal en el lado derecho
    muros.append(Muro(420 + muro_grosor, 250, NIVEL_WIDTH - (420 + muro_grosor) - muro_grosor, muro_grosor));
}
// ----------------------------------------------------
// IMPLEMENTACIÓN DE MÉTODOS DE LÓGICA Y JUEGO
// ----------------------------------------------------

void Niveluno::moverJugador(float dx, float dy) {
    sabioMaya.mover(dx, dy);

    int tamJugador = sabioMaya.ancho;
    sabioMaya.pos_x = std::min(sabioMaya.pos_x, (float)NIVEL_WIDTH - tamJugador);
    sabioMaya.pos_x = std::max(sabioMaya.pos_x, 0.0f);
    sabioMaya.pos_y = std::min(sabioMaya.pos_y, (float)NIVEL_HEIGHT - tamJugador);
    sabioMaya.pos_y = std::max(sabioMaya.pos_y, 0.0f);
}

void Niveluno::actualizarJuego() {
    agenteFuego.decidirPropagacion(fragmentos);
    verificarColisiones();
    verificarEstadoJuego();

    update();
}

void Niveluno::verificarColisiones() {
    QRect rectJugador = sabioMaya.getRectanguloColision();
    qint64 tiempoActual = timerElapsedJuego.elapsed();

    // 1. Encontrar el fragmento en contacto (y actualizar el puntero de la clase)
    sabioMaya.fragmentoEnContactoActual = nullptr;

    for (Fragmento &f : fragmentos) {
        if (f.estaSalvado) continue;

        const int BUFFER = 10;
        QRect rectFragmentoAmpliado = f.getRectanguloColision().adjusted(-BUFFER, -BUFFER, BUFFER, BUFFER);

        if (rectJugador.intersects(rectFragmentoAmpliado)) {
            sabioMaya.fragmentoEnContactoActual = &f;
            break;
        }
    }

    // --------------------------------------------------------------------------
    // ⚠️ LÓGICA DE RESETEO CRÍTICO POR FALTA DE CONTACTO
    // --------------------------------------------------------------------------
    // Si la tecla 'E' está presionada pero NO hay fragmento en contacto,
    // ¡RESETEAMOS EL TIEMPO INMEDIATAMENTE!
    if (sabioMaya.estaIntentandoRetener && sabioMaya.fragmentoEnContactoActual == nullptr) {
        if (sabioMaya.tiempoContactoFragMs != 0) {
            qDebug() << "RESETEO CRÍTICO: Perdimos contacto con el fragmento mientras 'E' estaba pulsada.";
            sabioMaya.tiempoContactoFragMs = 0;
        }
    }

    // --------------------------------------------------------------------------
    // LÓGICA DE TIEMPO DE RETENCIÓN
    // --------------------------------------------------------------------------

    // La cuenta SOLO debe progresar si hay contacto Y la tecla 'E' está presionada.
    bool debeContar = sabioMaya.estaIntentandoRetener && (sabioMaya.fragmentoEnContactoActual != nullptr);

    qint64 tiempoTranscurrido = 0;

    if (debeContar) {

        // 1. INICIO DEL CONTEO
        if (sabioMaya.tiempoContactoFragMs == 0) {
            sabioMaya.tiempoContactoFragMs = tiempoActual;
            qDebug() << "INICIO CONTEO OK en fragmento: " << tiempoActual << "ms";
        }

        // 2. CÁLCULO DEL TIEMPO TRANSCURRIDO
        if (sabioMaya.tiempoContactoFragMs != 0) {
            tiempoTranscurrido = tiempoActual - sabioMaya.tiempoContactoFragMs;
        }

        // 3. VERIFICACIÓN DE ÉXITO
        if (tiempoTranscurrido >= TIEMPO_RETENCION_MS) {

            if (sabioMaya.fragmentoEnContactoActual != nullptr) {
                sabioMaya.fragmentoEnContactoActual->estaSalvado = true;
                sabioMaya.fragmentosSalvados++;
                qDebug() << "C. ¡FRAGMENTO RECOGIDO CON ÉXITO! Total:" << sabioMaya.fragmentosSalvados;
            } else {
                qDebug() << "ERROR FINAL: Intento de recolección sin puntero, reseteo.";
            }

            // Resetear el estado de la clase inmediatamente
            sabioMaya.tiempoContactoFragMs = 0;
            sabioMaya.estaIntentandoRetener = false;
            sabioMaya.fragmentoEnContactoActual = nullptr;
            return;

        } else {
            // DIAGNÓSTICO DE PROGRESO
            if (sabioMaya.tiempoContactoFragMs != 0) {
                qDebug() << "B. PROGRESO: " << tiempoTranscurrido << "ms /" << TIEMPO_RETENCION_MS << "ms";
            }
        }

    } else {
        // RESETEO DEL CONTEO: Si suelta 'E' (estaIntentandoRetener=false) y no está contando, se resetea.
        if (sabioMaya.tiempoContactoFragMs != 0) {
            qDebug() << "RESETEO A 0. Causa: 'E' suelta o pérdida de contacto con fragmento.";
            sabioMaya.tiempoContactoFragMs = 0;
        }
    }

    // --------------------------------------------------------------------------
    // 2. Colisiones con Muros
    // --------------------------------------------------------------------------
    if (!sabioMaya.estaIntentandoRetener) {
        for (const Muro &m : muros) {
            if (rectJugador.intersects(m.getRectanguloColision())) {
                float factorRebote = 0.5f;
                sabioMaya.pos_x -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_x < m.pos_x ? 1 : -1);
                sabioMaya.pos_y -= sabioMaya.velocidad * factorRebote * (sabioMaya.pos_y < m.pos_y ? 1 : -1);
            }
        }
    }
}







void Niveluno::verificarEstadoJuego() {
    int tiempoRestante = TIEMPO_LIMITE_SEGUNDOS - (timerElapsedJuego.elapsed() / 1000); // ¡USANDO EL NUEVO NOMBRE!

    if (sabioMaya.fragmentosSalvados >= FRAGMENTOS_REQUERIDOS) {
        timerJuego->stop();
        qDebug() << "¡VICTORIA! Nivel completado.";
        return;
    }

    if (tiempoRestante <= 0) {
        timerJuego->stop();
        qDebug() << "DERROTA. Tiempo agotado.";
        return;
    }
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE EVENTOS (TECLADO Y PINTADO)
// ----------------------------------------------------
void Niveluno::keyPressEvent(QKeyEvent *event) {
    float dx = 0.0f;
    float dy = 0.0f;

    // 1. Manejo del intento de retención ('E')
    // 1. Manejo del intento de retención ('E')
    if (event->key() == Qt::Key_E) {
        // Ignoramos la repetición del evento (Auto-Repeat) para que solo se active una vez
        if (event->isAutoRepeat()) return;

        // Activamos la bandera.
        sabioMaya.estaIntentandoRetener = true;
    }

    // 2. Manejo del movimiento
    if (!sabioMaya.estaIntentandoRetener) {
        if (event->key() == Qt::Key_W) dy = -1.0f;
        if (event->key() == Qt::Key_S) dy = 1.0f;
        if (event->key() == Qt::Key_A) dx = -1.0f;
        if (event->key() == Qt::Key_D) dx = 1.0f;

        if (dx != 0.0f || dy != 0.0f) {
            moverJugador(dx, dy);
        }
    }

    update();
}


void Niveluno::keyReleaseEvent(QKeyEvent *event) {
    // 1. Ignoramos la repetición del evento (Auto-Repeat)
    if (event->isAutoRepeat()) return;

    if (event->key() == Qt::Key_E) {
        // Desactivamos la bandera de retención. ¡CRÍTICO!
        sabioMaya.estaIntentandoRetener = false;

        // No es necesario resetear tiempoContactoFragMs o el puntero aquí,
        // ya que verificarColisiones() lo hace al detectar que estaIntentandoRetener es false.

        qDebug() << "TECLA 'E' LIBERADA. Bandera de retención desactivada.";
    }

    // Es CRÍTICO llamar al evento base al final.
    QWidget::keyReleaseEvent(event);
}


void Niveluno::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.fillRect(rect(), QColor(50, 40, 40));

    painter.setBrush(Qt::black);
    painter.setPen(Qt::NoPen);
    for (const Muro &m : muros) {
        painter.drawRect(m.getRectanguloColision());
    }

    for (const Fragmento &f : fragmentos) {
        if (!f.estaSalvado && !f.estaQuemado) {
            painter.setBrush(Qt::yellow);
            painter.setPen(Qt::NoPen);
            painter.drawRect(f.getRectanguloColision());
        }
    }

    painter.setBrush(Qt::darkGreen);
    painter.setPen(Qt::white);
    painter.drawEllipse(sabioMaya.getRectanguloColision());

    painter.setBrush(Qt::red);
    painter.drawRect(agenteFuego.getRectanguloColision());

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(20, 20, QString("Fragmentos: %1 / %2").arg(sabioMaya.fragmentosSalvados).arg(FRAGMENTOS_REQUERIDOS));

    int tiempoRestante = TIEMPO_LIMITE_SEGUNDOS - (timerElapsedJuego.elapsed() / 1000); // ¡USANDO EL NUEVO NOMBRE!
    painter.drawText(20, 40, QString("Tiempo: %1 s").arg(tiempoRestante));

    if (sabioMaya.tiempoContactoFragMs > 0) {
        qreal progreso = (timerElapsedJuego.elapsed() - sabioMaya.tiempoContactoFragMs) / (qreal)TIEMPO_RETENCION_MS; // ¡USANDO EL NUEVO NOMBRE!
        if (progreso > 1.0) progreso = 1.0;
        painter.setPen(Qt::green);
        painter.setFont(QFont("Arial", 12, QFont::Bold));
        painter.drawText(width() / 2 - 50, height() - 50,
                         QString("RESCATANDO... %1%").arg(static_cast<int>(progreso * 100)));
    }
}

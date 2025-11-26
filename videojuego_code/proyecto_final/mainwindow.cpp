// mainwindow.cpp (Nuevo Constructor MainWindow::MainWindow)

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QFont>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>      // <--- ¡Necesario para QLabel!
#include <QPixmap>     // <--- ¡Necesario para QPixmap!
#include "niveluno.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Las constantes Niveluno son: 1250x650
    const int NIVEL_WIDTH = 1250;
    const int NIVEL_HEIGHT = 650;

    // --- CONSTANTES DE BOTÓN Y POSICIONAMIENTO ---
    const int BUTTON_WIDTH = 300;
    const int BUTTON_HEIGHT = 60;
    // const int PADDING_TOP = 300;   // <-- Ya no usaremos este padding directo al layout
    // ------------------------------------

    // --- NUEVAS CONSTANTES PARA LA IMAGEN ---
    const int IMAGE_WIDTH = 500;
    const int IMAGE_HEIGHT = 300;
    const int IMAGE_TOP_MARGIN = 50; // Margen desde la parte superior de la ventana hasta la imagen
    // ------------------------------------------


    // 1. Inicialización y Diseño
    ui->setupUi(this);
    this->resize(NIVEL_WIDTH, NIVEL_HEIGHT);


    // ------------------------------------------------------------------
    // ** FONDOS Y ESTILO DE VENTANA **
    // ------------------------------------------------------------------

    QString imagePath = "C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/img_inicio.png";

    this->setStyleSheet(QString(
                            "QMainWindow {"
                            "background-image: url(%1);"
                            "background-repeat: no-repeat;"
                            "background-position: center;"
                            "}"
                            ).arg(imagePath));


    // ------------------------------------------------------------------
    // ** MODIFICACIÓN CRÍTICA: AÑADIR IMAGEN Y CENTRAR TODO CON LAYOUT **
    // ------------------------------------------------------------------

    // 1. Crear el contenedor central para los elementos
    QWidget *centerWidget = new QWidget(this);

    // 2. Crear el Layout Vertical que gestionará la posición
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);

    // 3. Definir la Tipografía (Times New Roman, más grande y negrita)
    QFont font("Times New Roman", 18, QFont::Bold);

    // ------------------------------------------------------------------
    // A. AÑADIR Y CONFIGURAR LA IMAGEN
    // ------------------------------------------------------------------
    QLabel *imageLabel = new QLabel(centerWidget); // El label contendrá la imagen
    QPixmap imagePixmap("C:/Users/alexa/Desktop/proyecto_final/videojuego_code/multimedia/imagenes/titulo.jpg"); // <--- ¡CAMBIA ESTA RUTA A TU IMAGEN!

    if (!imagePixmap.isNull()) {
        // Escalar la imagen al tamaño deseado
        imagePixmap = imagePixmap.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(imagePixmap);
        // Centrar la imagen dentro del QLabel
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT); // Asegurar el tamaño fijo del label

        // Añadir un espaciador superior para posicionar la imagen desde arriba
        layout->addSpacing(IMAGE_TOP_MARGIN); // Margen desde el borde superior de la ventana
        layout->addWidget(imageLabel, 0, Qt::AlignCenter); // Añadir la imagen al layout, centrada
        layout->addSpacing(30); // Pequeño espacio entre la imagen y los botones

        qDebug() << "Imagen de título cargada y añadida.";
    } else {
        qDebug() << "ERROR: No se pudo cargar la imagen de título desde la ruta especificada.";
    }
    // ------------------------------------------------------------------


    // --- Configurar Botones y Añadirlos al Layout ---
    QSize buttonSize(BUTTON_WIDTH, BUTTON_HEIGHT);

    // 4.1. Botón Nivel 1 (btnNivel1)
    ui->btnNivel1->setFont(font);
    ui->btnNivel1->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel1, 0, Qt::AlignCenter);

    // 4.2. Botón Nivel 2 (btnNivel2)
    ui->btnNivel2->setFont(font);
    ui->btnNivel2->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel2, 0, Qt::AlignCenter);

    // 4.3. Botón Nivel 3 (btnNivel3)
    ui->btnNivel3->setFont(font);
    ui->btnNivel3->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel3, 0, Qt::AlignCenter);


    // 5. Añadir Stretch para empujar los elementos hacia arriba y llenar el espacio restante
    layout->addStretch();

    // 6. ¡IMPORTANTE! Asignar el widget contenedor como el widget central de la ventana
    this->setCentralWidget(centerWidget);

    // ------------------------------------------------------------------
    // ** FIN DE LAYOUT **
    // ------------------------------------------------------------------


    // 3. Conexiones (Se mantienen sin cambios)
    connect(ui->btnNivel1, &QPushButton::clicked, this, &MainWindow::abrirNivelUno);
    connect(ui->btnNivel2, &QPushButton::clicked, this, &MainWindow::abrirNivelDos);
    connect(ui->btnNivel3, &QPushButton::clicked, this, &MainWindow::abrirNivelTres);
}

// ... (Resto de la clase MainWindow no cambia) ...

MainWindow::~MainWindow()
{
    delete ui;
}

// ----------------------------------------------------
// IMPLEMENTACIÓN DE SLOTS
// ----------------------------------------------------

void MainWindow::abrirNivelUno() {
    // 1. Ocultar la ventana del menú principal
    this->hide();

    // 2. Crear una nueva instancia del Nivel 1
    Niveluno *nivel = new Niveluno(nullptr);

    // 3. Mostrar la ventana del Nivel 1
    nivel->show();

    // 4. Asegurar que el objeto Niveluno se destruya automáticamente cuando se cierre su ventana.
    nivel->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::abrirNivelDos()
{
    qDebug() << "--- Nivel 2 CONECTADO ---";
}

void MainWindow::abrirNivelTres()
{
    qDebug() << "--- Nivel 3 CONECTADO ---";
}

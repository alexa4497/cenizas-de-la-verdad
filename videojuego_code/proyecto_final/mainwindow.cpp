#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QFont>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
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

    const int IMAGE_WIDTH = 500;
    const int IMAGE_HEIGHT = 300;
    const int IMAGE_TOP_MARGIN = 50;

    ui->setupUi(this);
    this->resize(NIVEL_WIDTH, NIVEL_HEIGHT);

    // ** FONDOS Y ESTILO DE VENTANA **

    QString imagePath = ":/imagenes/multimedia/imagenes/img_inicio.png";

    this->setStyleSheet(QString(
                            "QMainWindow {"
                            "background-image: url(%1);"
                            "background-repeat: no-repeat;"
                            "background-position: center;"
                            "}"
                            ).arg(imagePath));

    //titulo

    QWidget *centerWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centerWidget);
    QFont font("Times New Roman", 18, QFont::Bold);
    QLabel *imageLabel = new QLabel(centerWidget);
    QPixmap imagePixmap(":/imagenes/multimedia/imagenes/titulo.jpg");

    if (!imagePixmap.isNull()) {
        // Escalar la imagen al tamaño
        imagePixmap = imagePixmap.scaled(IMAGE_WIDTH, IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(imagePixmap);
        // Centrar
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setFixedSize(IMAGE_WIDTH, IMAGE_HEIGHT);

        layout->addSpacing(IMAGE_TOP_MARGIN);
        layout->addWidget(imageLabel, 0, Qt::AlignCenter);
        layout->addSpacing(30);
    }

    // --- Configurar Botones y Anadirlos al Layout ---
    QSize buttonSize(BUTTON_WIDTH, BUTTON_HEIGHT);

    ui->btnNivel1->setFont(font);
    ui->btnNivel1->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel1, 0, Qt::AlignCenter);

    ui->btnNivel2->setFont(font);
    ui->btnNivel2->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel2, 0, Qt::AlignCenter);

    ui->btnNivel3->setFont(font);
    ui->btnNivel3->setFixedSize(buttonSize);
    layout->addWidget(ui->btnNivel3, 0, Qt::AlignCenter);


    layout->addStretch();

    this->setCentralWidget(centerWidget);

    // 3. Conexiones (Se mantienen sin cambios)
    connect(ui->btnNivel1, &QPushButton::clicked, this, &MainWindow::abrirNivelUno);
    connect(ui->btnNivel2, &QPushButton::clicked, this, &MainWindow::abrirNivelDos);
    connect(ui->btnNivel3, &QPushButton::clicked, this, &MainWindow::abrirNivelTres);
}


MainWindow::~MainWindow()
{
    delete ui;
}

// IMPLEMENTACIÓN DE SLOTS

void MainWindow::abrirNivelUno() {

    this->hide();
    Niveluno *nivel = new Niveluno(nullptr);
    nivel->show();
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

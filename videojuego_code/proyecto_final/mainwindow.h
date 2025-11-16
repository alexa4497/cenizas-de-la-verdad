#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "niveluno.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots: // <-- ¡Agrega esta sección si no existe!
    void abrirNivelUno();   // <--- ¡Tu nombre deseado!
    void abrirNivelDos();
    void abrirNivelTres();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

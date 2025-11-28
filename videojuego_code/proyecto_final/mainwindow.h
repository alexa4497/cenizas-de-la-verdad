#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "niveluno.h"
#include "ob.h"

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
    Ui::MainWindow *ui;

private slots:
    void abrirNivelUno();
    void abrirNivelDos();
    void abrirNivelTres();



};
#endif // MAINWINDOW_H

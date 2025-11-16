#ifndef NIVELUNO_H
#define NIVELUNO_H

#include <QWidget>

namespace Ui {
class Niveluno;
}

class Niveluno : public QWidget
{
    Q_OBJECT

public:
    explicit Niveluno(QWidget *parent = nullptr);
    ~Niveluno();

private:
    Ui::Niveluno *ui;
};

#endif // NIVELUNO_H

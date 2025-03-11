#ifndef TYPES_H
#define TYPES_H

#include <QWidget>

namespace Ui {
class typy;
}

class typy : public QWidget
{
    Q_OBJECT

public:
    explicit typy(QWidget *parent = nullptr);
    ~typy();

private:
    Ui::typy *ui;
};

#endif // TYPES_H

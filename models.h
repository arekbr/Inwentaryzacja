#ifndef MODELS_H
#define MODELS_H

#include <QWidget>

namespace Ui {
class models;
}

class models : public QWidget
{
    Q_OBJECT

public:
    explicit models(QWidget *parent = nullptr);
    ~models();

private:
    Ui::models *ui;
};

#endif // MODELS_H

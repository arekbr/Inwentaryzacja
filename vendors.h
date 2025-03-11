#ifndef VENDORS_H
#define VENDORS_H

#include <QWidget>

namespace Ui {
class vendors;
}

class vendors : public QWidget
{
    Q_OBJECT

public:
    explicit vendors(QWidget *parent = nullptr);
    ~vendors();

private:
    Ui::vendors *ui;
};

#endif // VENDORS_H

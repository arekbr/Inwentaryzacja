#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include "photoitem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setEditMode(bool edit, int recordId = -1);

signals:
    void recordSaved(int recordId);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void loadRecord(int recordId);
    void loadPhotos(int recordId);
    void onRemovePhotoClicked();
    void onPhotoClicked(PhotoItem *item);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool m_editMode;
    int m_recordId;
    int m_selectedPhotoIndex;
    QGraphicsScene *m_photoScene; // Dodajemy scenę dla graphicsView
};

#endif // MAINWINDOW_H

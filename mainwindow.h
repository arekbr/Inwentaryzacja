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

    // Ustawienie trybu edycji: edit = true – edycja, false – dodawanie nowego rekordu
    // recordId – identyfikator rekordu do edycji (dla trybu edycji)
    void setEditMode(bool edit, int recordId = -1);

signals:
    void recordSaved(int recordId); // Sygnał informujący o zapisaniu rekordu z ID

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void loadRecord(int recordId);
    void loadPhotos(int recordId); // Ładowanie zdjęć
    void onRemovePhotoClicked();   // Nowy slot do usuwania zdjęcia
    void onPhotoClicked(PhotoItem *item); // Poprawiony slot do obsługi kliknięcia na miniaturę

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool m_editMode;
    int m_recordId;
    int m_selectedPhotoIndex; // Indeks wybranego zdjęcia w siatce
};

#endif // MAINWINDOW_H

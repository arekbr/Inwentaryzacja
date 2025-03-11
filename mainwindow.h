#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

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
    void recordSaved(); // Sygnał informujący o zapisaniu rekordu

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked(); // Nowy slot dla przycisku "Dodaj zdjęcia"
    void loadRecord(int recordId);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool m_editMode;
    int m_recordId;
};

#endif // MAINWINDOW_H

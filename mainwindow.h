#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QComboBox>
#include <QList>

namespace Ui {
class MainWindow;
}

// Forward-deklaracja klasy PhotoItem (używana w slotach)
class PhotoItem;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Metoda do ładowania danych z tabeli 'tableName' do QComboBox
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);


    bool eventFilter(QObject *obj, QEvent *event) override;

    // Ustawienie trybu edycji (edit) lub dodawania
    void setEditMode(bool edit, const QString &recordId = QString());

    // Tryb klonowania
    void setCloneMode(const QString &recordId);

    // Metody pozwalające innym klasom pobrać wskaźnik do ComboBoxów
    QComboBox* getNewItemVendorComboBox() const;
    QComboBox* getNewItemModelComboBox() const;
    QComboBox* getNewItemStoragePlaceComboBox() const;
    QComboBox* getNewItemTypeComboBox() const;
    QComboBox* getNewItemStatusComboBox() const;

signals:
    // Sygnał, gdy zapiszesz nowy / edytowany rekord
    void recordSaved(const QString &recordId);

private slots:
    // Sloty, do których łączysz connect(..., this, &MainWindow::onSaveClicked);
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void onRemovePhotoClicked();

    // Kliknięcie w PhotoItem
    void onPhotoClicked(PhotoItem *item);

    // Otwieranie okien słownikowych
    void onAddTypeClicked();
    void onAddVendorClicked();
    void onAddModelClicked();
    void onAddStatusClicked();
    void onAddStoragePlaceClicked();

private:
    // Metoda wczytania rekordu o podanym ID do formularza
    void loadRecord(const QString &recordId);

    // Wczytanie zdjęć powiązanych z danym rekordem
    void loadPhotos(const QString &recordId);

    // Jeśli rekord nie zapisany w bazie, trzymamy zdjęcia w buforze w RAM
    void loadPhotosFromBuffer();

    // Pole do trzymania połączenia z bazą
    QSqlDatabase db;

    // Wskaźnik do wygenerowanego UI (z .ui)
    Ui::MainWindow *ui;

    // Flaga, czy edytujemy istniejący rekord
    bool m_editMode;

    // ID (np. UUID) aktualnie edytowanego rekordu
    QString m_recordId;

    // Która miniaturka zdjęcia jest aktualnie wybrana
    int m_selectedPhotoIndex;

    // Bufor zdjęć (gdy rekord jeszcze nie zapisany do bazy)
    QList<QByteArray> m_photoBuffer;

    // Bufor ścieżka do zapamiętania jakie zdjęcie dodało się do bazy i przeniesienie go do GOTOWE
    QStringList    m_photoPathsBuffer;

};

#endif // MAINWINDOW_H

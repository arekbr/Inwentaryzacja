/**
 * @file mainwindow.h
 * @brief Deklaracja klasy MainWindow do zarządzania formularzem eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera deklarację klasy MainWindow, która reprezentuje główne okno formularza
 * do dodawania, edycji i klonowania eksponatów w aplikacji inwentaryzacyjnej. Klasa obsługuje
 * ładowanie danych do combo boxów, zarządzanie zdjęciami eksponatów, tryby edycji i klonowania,
 * a także interakcje z bazą danych MySQL. Emituje sygnały po zapisaniu rekordu i zawiera sloty
 * do obsługi akcji użytkownika (np. zapis, anulowanie, dodawanie zdjęć).
 */

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

/**
 * @class MainWindow
 * @brief Główne okno formularza do zarządzania eksponatami.
 *
 * Klasa MainWindow dziedziczy po QMainWindow i odpowiada za interfejs użytkownika
 * do dodawania, edycji i klonowania rekordów eksponatów. Obsługuje ładowanie danych
 * do combo boxów, zarządzanie zdjęciami (w bazie i buforze), tryby edycji/klonowania
 * oraz komunikację z bazą danych. Emituje sygnał recordSaved po zapisaniu rekordu.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje okno formularza, ustanawia połączenie z bazą danych i konfiguruje
     * interfejs użytkownika.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     *
     * Zwalnia zasoby interfejsu użytkownika i zamyka połączenie z bazą danych.
     */
    ~MainWindow();

    /**
     * @brief Ładuje dane z tabeli do combo boxa.
     * @param tableName Nazwa tabeli w bazie danych (np. "types", "vendors").
     * @param comboBox Wskaźnik na QComboBox, do którego ładowane są dane.
     *
     * Pobiera unikalne wartości z kolumny "name" podanej tabeli i wypełnia nimi combo box.
     */
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);

    /**
     * @brief Obsługuje zdarzenia filtrowania dla obiektów interfejsu.
     * @param obj Obiekt, dla którego przetwarzane jest zdarzenie.
     * @param event Wskaźnik na zdarzenie.
     * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
     *
     * Przesłania metodę QMainWindow do obsługi specyficznych zdarzeń interfejsu.
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief Ustawia tryb edycji lub dodawania rekordu.
     * @param edit true dla trybu edycji, false dla trybu dodawania.
     * @param recordId ID rekordu do edycji (puste dla nowego rekordu).
     *
     * Konfiguruje formularz w trybie edycji istniejącego rekordu lub dodawania nowego.
     */
    void setEditMode(bool edit, const QString &recordId = QString());

    /**
     * @brief Ustawia tryb klonowania rekordu.
     * @param recordId ID rekordu do sklonowania.
     *
     * Wypełnia formularz danymi istniejącego rekordu, przygotowując go do zapisu jako nowy rekord.
     */
    void setCloneMode(const QString &recordId);

    /**
     * @brief Zwraca wskaźnik na combo box dla producentów.
     * @return Wskaźnik na QComboBox dla producentów.
     */
    QComboBox* getNewItemVendorComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla modeli.
     * @return Wskaźnik na QComboBox dla modeli.
     */
    QComboBox* getNewItemModelComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla miejsc przechowywania.
     * @return Wskaźnik na QComboBox dla miejsc przechowywania.
     */
    QComboBox* getNewItemStoragePlaceComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla typów.
     * @return Wskaźnik na QComboBox dla typów.
     */
    QComboBox* getNewItemTypeComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla statusów.
     * @return Wskaźnik na QComboBox dla statusów.
     */
    QComboBox* getNewItemStatusComboBox() const;

signals:
    /**
     * @brief Sygnał emitowany po zapisaniu nowego lub edytowanego rekordu.
     * @param recordId ID zapisanego rekordu (np. UUID).
     *
     * Informuje inne komponenty aplikacji, że rekord został zapisany w bazie danych.
     */
    void recordSaved(const QString &recordId);

private slots:
    /**
     * @brief Zapisuje dane z formularza do bazy danych.
     *
     * Waliduje dane, zapisuje rekord w tabeli eksponaty i powiązane zdjęcia w tabeli photos,
     * a następnie emituje sygnał recordSaved.
     */
    void onSaveClicked();

    /**
     * @brief Anuluje edycję/dodawanie i zamyka okno.
     *
     * Zamyka okno formularza bez zapisywania zmian.
     */
    void onCancelClicked();

    /**
     * @brief Dodaje nowe zdjęcie do rekordu.
     *
     * Otwiera okno wyboru pliku, ładuje zdjęcie do bufora i aktualizuje podgląd miniaturek.
     */
    void onAddPhotoClicked();

    /**
     * @brief Usuwa wybrane zdjęcie z rekordu.
     *
     * Usuwa zdjęcie z bufora lub bazy danych i aktualizuje podgląd miniaturek.
     */
    void onRemovePhotoClicked();

    /**
     * @brief Obsługuje kliknięcie w miniaturę zdjęcia.
     * @param item Wskaźnik na obiekt PhotoItem reprezentujący miniaturę.
     *
     * Ustawia wybraną miniaturę jako aktywną i umożliwia jej podgląd lub usunięcie.
     */
    void onPhotoClicked(PhotoItem *item);

    /**
     * @brief Otwiera okno dodawania nowego typu.
     *
     * Wyświetla okno dialogowe do dodania nowego typu eksponatu do tabeli types.
     */
    void onAddTypeClicked();

    /**
     * @brief Otwiera okno dodawania nowego producenta.
     *
     * Wyświetla okno dialogowe do dodania nowego producenta do tabeli vendors.
     */
    void onAddVendorClicked();

    /**
     * @brief Otwiera okno dodawania nowego modelu.
     *
     * Wyświetla okno dialogowe do dodania nowego modelu do tabeli models.
     */
    void onAddModelClicked();

    /**
     * @brief Otwiera okno dodawania nowego statusu.
     *
     * Wyświetla okno dialogowe do dodania nowego statusu do tabeli statuses.
     */
    void onAddStatusClicked();

    /**
     * @brief Otwiera okno dodawania nowego miejsca przechowywania.
     *
     * Wyświetla okno dialogowe do dodania nowego miejsca przechowywania do tabeli storage_places.
     */
    void onAddStoragePlaceClicked();

private:
    /**
     * @brief Wczytuje rekord o podanym ID do formularza.
     * @param recordId ID rekordu do wczytania.
     *
     * Pobiera dane rekordu z tabeli eksponaty i wypełnia nimi pola formularza.
     */
    void loadRecord(const QString &recordId);

    /**
     * @brief Wczytuje zdjęcia powiązane z danym rekordem.
     * @param recordId ID rekordu, którego zdjęcia są wczytywane.
     *
     * Pobiera zdjęcia z tabeli photos i wyświetla ich miniatury w formularzu.
     */
    void loadPhotos(const QString &recordId);

    /**
     * @brief Wczytuje zdjęcia z bufora pamięci.
     *
     * Wyświetla miniatury zdjęć przechowywanych w buforze (dla rekordów jeszcze niezapisanych).
     */
    void loadPhotosFromBuffer();

    /// Połączenie z bazą danych MySQL.
    QSqlDatabase db;

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::MainWindow *ui;

    /// Flaga wskazująca, czy formularz jest w trybie edycji (true) czy dodawania (false).
    bool m_editMode;

    /// ID (np. UUID) aktualnie edytowanego rekordu.
    QString m_recordId;

    /// Indeks aktualnie wybranej miniatury zdjęcia.
    int m_selectedPhotoIndex;

    /// Bufor przechowujący zdjęcia dla rekordów jeszcze niezapisanych w bazie.
    QList<QByteArray> m_photoBuffer;

    /// Bufor przechowujący ścieżki do zdjęć przed zapisaniem do bazy.
    QStringList m_photoPathsBuffer;
};

#endif // MAINWINDOW_H

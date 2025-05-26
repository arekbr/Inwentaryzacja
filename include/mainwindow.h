/**
 * @file mainwindow.h
 * @brief Deklaracja klasy MainWindow do zarządzania formularzem eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik zawiera deklarację klasy MainWindow, która reprezentuje główne okno formularza
 * do dodawania, edycji i klonowania eksponatów w aplikacji inwentaryzacyjnej. Klasa obsługuje
 * ładowanie danych do combo boxów, zarządzanie zdjęciami eksponatów, tryby edycji i klonowania,
 * a także interakcje z bazą danych MySQL. Emituje sygnały po zapisaniu rekordu i zawiera sloty
 * do obsługi akcji użytkownika (np. zapis, anulowanie, dodawanie zdjęć).
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy MainWindow** – dziedziczy po QMainWindow.
 * 2. **Metody publiczne** – konstruktor, destruktor, metody do ładowania danych i ustawiania trybów.
 * 3. **Sygnały** – emitowane po zapisaniu rekordu.
 * 4. **Sloty prywatne** – obsługują interakcje użytkownika (przyciski, zdjęcia, słowniki).
 * 5. **Metody prywatne** – wczytywanie rekordów i zdjęć.
 * 6. **Zmienne prywatne** – przechowują stan formularza, bufor zdjęć i połączenie z bazą danych.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QMainWindow, QSqlDatabase, QComboBox, QList.
 * - **Nagłówki aplikacji**: photoitem.h (forward-deklaracja).
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_mainwindow.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest kluczowym komponentem aplikacji, odpowiedzialnym za edycję i dodawanie rekordów eksponatów.
 * - Obsługuje zarówno SQLite, jak i MySQL, z konfiguracją zarządzaną przez DatabaseConfigDialog.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QCloseEvent>


namespace Ui {
class MainWindow;
}

// Forward-deklaracja klasy PhotoItem (używana w slotach)
class PhotoItem;

/**
 * @class MainWindow
 * @brief Główne okno formularza do zarządzania eksponatami.
 *
 * @section ClassOverview
 * Klasa MainWindow dziedziczy po QMainWindow i odpowiada za interfejs użytkownika
 * do dodawania, edycji i klonowania rekordów eksponatów. Obsługuje ładowanie danych
 * do combo boxów, zarządzanie zdjęciami (w bazie i buforze), tryby edycji/klonowania
 * oraz komunikację z bazą danych. Emituje sygnał recordSaved po zapisaniu rekordu.
 *
 * @section Responsibilities
 * - Wyświetlanie formularza do wprowadzania danych eksponatu.
 * - Ładowanie danych słownikowych (typy, producenci, modele, statusy, miejsca przechowywania) do combo boxów.
 * - Zarządzanie zdjęciami: dodawanie, usuwanie, podgląd miniatur.
 * - Obsługa trybów: dodawanie nowego rekordu, edycja istniejącego, klonowanie rekordu.
 * - Zapisywanie danych do bazy danych MySQL i emitowanie sygnału po zapisie.
 * - Otwieranie dialogów do dodawania nowych wartości słownikowych.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy MainWindow.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje okno formularza, ustanawia połączenie z bazą danych, konfiguruje
     * interfejs użytkownika, ładuje dane do combo boxów, ustawia sloty dla przycisków
     * i włącza automatyczne dodawanie nowych wartości do słowników.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow.
     *
     * @section DestructorOverview
     * Zwalnia zasoby interfejsu użytkownika, zamyka połączenie z bazą danych
     * i usuwa obiekty dynamiczne.
     */
    ~MainWindow();

    /**
     * @brief Ładuje dane z tabeli do combo boxa.
     * @param tableName Nazwa tabeli w bazie danych (np. "types", "vendors").
     * @param comboBox Wskaźnik na QComboBox, do którego ładowane są dane.
     *
     * @section MethodOverview
     * Pobiera unikalne wartości z kolumny "name" podanej tabeli i wypełnia nimi combo box,
     * ustawiając ID jako dane użytkownika dla każdej pozycji.
     */
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);

    /**
     * @brief Obsługuje zdarzenia filtrowania dla obiektów interfejsu.
     * @param obj Obiekt, dla którego przetwarzane jest zdarzenie.
     * @param event Wskaźnik na zdarzenie.
     * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
     *
     * @section MethodOverview
     * Przesłania metodę QMainWindow, aby otwierać listę combo boxa po kliknięciu myszą.
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief Ustawia tryb edycji lub dodawania rekordu.
     * @param edit true dla trybu edycji, false dla trybu dodawania.
     * @param recordId ID rekordu do edycji (puste dla nowego rekordu).
     *
     * @section MethodOverview
     * Konfiguruje formularz w trybie edycji istniejącego rekordu lub dodawania nowego,
     * czyszcząc pola i ustawiając domyślne wartości dla statusu i miejsca przechowywania.
     */
    void setEditMode(bool edit, const QString &recordId = QString());

    /**
     * @brief Ustawia tryb klonowania rekordu.
     * @param recordId ID rekordu do sklonowania.
     *
     * @section MethodOverview
     * Wypełnia formularz danymi istniejącego rekordu, przygotowując go do zapisu
     * jako nowy rekord, z wyczyszczeniem zdjęć i ID rekordu.
     */
    void setCloneMode(const QString &recordId);

    /**
     * @brief Zwraca wskaźnik na combo box dla producentów.
     * @return Wskaźnik na QComboBox dla producentów.
     *
     * @section MethodOverview
     * Umożliwia dostęp do combo boxa producentów z innych klas (np. dialogów słownikowych).
     */
    QComboBox *getNewItemVendorComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla modeli.
     * @return Wskaźnik na QComboBox dla modeli.
     *
     * @section MethodOverview
     * Umożliwia dostęp do combo boxa modeli z innych klas (np. dialogów słownikowych).
     */
    QComboBox *getNewItemModelComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla miejsc przechowywania.
     * @return Wskaźnik na QComboBox dla miejsc przechowywania.
     *
     * @section MethodOverview
     * Umożliwia dostęp do combo boxa miejsc przechowywania z innych klas.
     */
    QComboBox *getNewItemStoragePlaceComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla typów.
     * @return Wskaźnik na QComboBox dla typów.
     *
     * @section MethodOverview
     * Umożliwia dostęp do combo boxa typów z innych klas (np. dialogów słownikowych).
     */
    QComboBox *getNewItemTypeComboBox() const;

    /**
     * @brief Zwraca wskaźnik na combo box dla statusów.
     * @return Wskaźnik na QComboBox dla statusów.
     *
     * @section MethodOverview
     * Umożliwia dostęp do combo boxa statusów z innych klas (np. dialogów słownikowych).
     */
    QComboBox *getNewItemStatusComboBox() const;

signals:
    /**
     * @brief Sygnał emitowany po zapisaniu nowego lub edytowanego rekordu.
     * @param recordId ID zapisanego rekordu (np. UUID).
     *
     * @section SignalOverview
     * Informuje inne komponenty aplikacji (np. itemList), że rekord został zapisany
     * w bazie danych, umożliwiając odświeżenie widoku.
     */
    void recordSaved(const QString &recordId);

private slots:
    /**
     * @brief Zapisuje dane z formularza do bazy danych.
     *
     * @section SlotOverview
     * Waliduje dane, zapisuje rekord w tabeli eksponaty i powiązane zdjęcia w tabeli photos,
     * przenosi pliki zdjęć do katalogu "gotowe", emituje sygnał recordSaved i zamyka okno.
     */
    void onSaveClicked();

    /**
     * @brief Anuluje edycję/dodawanie i zamyka okno.
     *
     * @section SlotOverview
     * Zamyka okno formularza bez zapisywania zmian.
     */
    void onCancelClicked();

    /**
     * @brief Dodaje nowe zdjęcie do rekordu.
     *
     * @section SlotOverview
     * Otwiera okno wyboru pliku, ładuje zdjęcie do bufora (dla nowych rekordów) lub bazy danych
     * (dla edycji), przenosi plik do katalogu "gotowe" i aktualizuje podgląd miniaturek.
     */
    void onAddPhotoClicked();

    /**
     * @brief Usuwa wybrane zdjęcie z rekordu.
     *
     * @section SlotOverview
     * Usuwa zdjęcie z bufora lub bazy danych po potwierdzeniu użytkownika i aktualizuje
     * podgląd miniaturek.
     */
    void onRemovePhotoClicked();

    /**
     * @brief Obsługuje kliknięcie w miniaturę zdjęcia.
     * @param item Wskaźnik na obiekt PhotoItem reprezentujący miniaturę.
     *
     * @section SlotOverview
     * Ustawia wybraną miniaturę jako aktywną i aktualizuje indeks wybranego zdjęcia,
     * umożliwiając jego usunięcie lub podgląd.
     */
    void onPhotoClicked(PhotoItem *item);

    /**
     * @brief Otwiera okno dodawania nowego typu.
     *
     * @section SlotOverview
     * Wyświetla okno dialogowe (klasa types) do dodania nowego typu eksponatu
     * do tabeli types i odświeża combo box po zamknięciu.
     */
    void onAddTypeClicked();

    /**
     * @brief Otwiera okno dodawania nowego producenta.
     *
     * @section SlotOverview
     * Wyświetla okno dialogowe (klasa vendors) do dodania nowego producenta
     * do tabeli vendors i odświeża combo box po zamknięciu.
     */
    void onAddVendorClicked();

    /**
     * @brief Otwiera okno dodawania nowego modelu.
     *
     * @section SlotOverview
     * Wyświetla okno dialogowe (klasa models) do dodania nowego modelu
     * do tabeli models i odświeża combo box po zamknięciu.
     */
    void onAddModelClicked();

    /**
     * @brief Otwiera okno dodawania nowego statusu.
     *
     * @section SlotOverview
     * Wyświetla okno dialogowe (klasa status) do dodania nowego statusu
     * do tabeli statuses i odświeża combo box po zamknięciu.
     */
    void onAddStatusClicked();

    /**
     * @brief Otwiera okno dodawania nowego miejsca przechowywania.
     *
     * @section SlotOverview
     * Wyświetla okno dialogowe (klasa storage) do dodania nowego miejsca przechowywania
     * do tabeli storage_places i odświeża combo box po zamknięciu.
     */
    void onAddStoragePlaceClicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    /**
     * @brief Wczytuje rekord o podanym ID do formularza.
     * @param recordId ID rekordu do wczytania.
     *
     * @section MethodOverview
     * Pobiera dane rekordu z tabeli eksponaty, wypełnia pola formularza
     * i ładuje powiązane zdjęcia.
     */
    void loadRecord(const QString &recordId);

    /**
     * @brief Wczytuje zdjęcia powiązane z danym rekordem.
     * @param recordId ID rekordu, którego zdjęcia są wczytywane.
     *
     * @section MethodOverview
     * Pobiera zdjęcia z tabeli photos, tworzy miniatury i wyświetla je w QGraphicsView.
     */
    void loadPhotos(const QString &recordId);

    /**
     * @brief Wczytuje zdjęcia z bufora pamięci.
     *
     * @section MethodOverview
     * Wyświetla miniatury zdjęć przechowywanych w buforze (dla rekordów jeszcze niezapisanych)
     * w QGraphicsView.
     */
    void loadPhotosFromBuffer();

    QString validateUuid(const QString &uuid, const QString &defaultValue);

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

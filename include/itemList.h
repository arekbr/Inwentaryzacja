/**
 * @file itemList.h
 * @brief Deklaracja klasy itemList, odpowiedzialnej za zarządzanie listą eksponatów w aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.10
 * @date 2025-04-30
 *
 * Klasa itemList reprezentuje widget wyświetlający tabelę eksponatów z możliwością filtrowania, edycji,
 * dodawania, usuwania i klonowania rekordów. Obsługuje interakcję z miniaturami zdjęć, podgląd
 * pełnoekranowy, wybór skórki graficznej oraz dynamiczną zmianę czcionek w zależności od skórki.
 */

#ifndef ITEMLIST_H
#define ITEMLIST_H

#include <QComboBox>
#include <QItemSelection>
#include <QLabel>
#include <QSettings>
#include <QSqlRelationalTableModel>
#include <QWidget>
#include "ItemFilterProxyModel.h"
#include "photoitem.h"

namespace Ui {
class itemList;
}

/**
 * @class itemList
 * @brief Widget zarządzający listą eksponatów z interfejsem użytkownika.
 *
 * Klasa itemList odpowiada za wyświetlanie i zarządzanie tabelą eksponatów w aplikacji do inwentaryzacji
 * retro komputerów. Umożliwia filtrowanie danych za pomocą combo boxów, obsługę zdjęć eksponatów
 * (podgląd, powiększenie, pełny ekran), operacje CRUD (Create, Read, Update, Delete) na rekordach,
 * zmianę skórki graficznej oraz dynamiczną zmianę czcionek w zależności od wybranej skórki.
 */
class itemList : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy itemList.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * Inicjalizuje interfejs użytkownika, model danych, filtry, skórkę graficzną, czcionki oraz połączenia sygnałów i slotów.
     */
    explicit itemList(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy itemList.
     *
     * Zwalnia zasoby, w tym interfejs użytkownika i timery.
     */
    ~itemList();

private slots:
    /**
     * @brief Otwiera okno dodawania nowego eksponatu.
     */
    void onNewButtonClicked();

    /**
     * @brief Otwiera okno edycji wybranego eksponatu.
     */
    void onEditButtonClicked();

    /**
     * @brief Zamyka aplikację.
     */
    void onEndButtonClicked();

    /**
     * @brief Usuwa wybrany eksponat po potwierdzeniu.
     */
    void onDeleteButtonClicked();

    /**
     * @brief Wyświetla okno "O programie" z informacjami o aplikacji.
     */
    void onAboutClicked();

    /**
     * @brief Obsługuje zmianę zaznaczenia w tabeli eksponatów.
     * @param selected Zaznaczone indeksy.
     * @param deselected Odznaczone indeksy.
     *
     * Aktualizuje wyświetlane miniatury zdjęć dla wybranego eksponatu.
     */
    void onTableViewSelectionChanged(const QItemSelection &selected,
                                     const QItemSelection &deselected);

    /**
     * @brief Odświeża listę po zapisaniu rekordu.
     * @param recordId ID zapisanego rekordu.
     */
    void onRecordSaved(const QString &recordId);

    /**
     * @brief Odświeża listę eksponatów.
     * @param recordId Opcjonalne ID rekordu do wybrania po odświeżeniu.
     */
    void refreshList(const QString &recordId = QString());

    /**
     * @brief Wyświetla podgląd zdjęcia po najechaniu na miniaturę.
     * @param item Wskaźnik na element PhotoItem.
     */
    void onPhotoHovered(PhotoItem *item);

    /**
     * @brief Ukrywa podgląd zdjęcia po opuszczeniu miniatury.
     * @param item Wskaźnik na element PhotoItem.
     */
    void onPhotoUnhovered(PhotoItem *item);

    /**
     * @brief Otwiera zdjęcie w trybie pełnoekranowym po kliknięciu.
     * @param item Wskaźnik na element PhotoItem.
     */
    void onPhotoClicked(PhotoItem *item);

    /**
     * @brief Otwiera okno klonowania wybranego eksponatu.
     */
    void onCloneButtonClicked();

    /**
     * @brief Aktualizuje filtry po zmianie wartości w combo boxach.
     */
    void onFilterChanged();

    /**
     * @brief Obsługuje zmianę skórki graficznej.
     * @param skin Nazwa wybranej skórki (np. "Amiga", "ZX Spectrum", "Standard").
     *
     * Ładuje odpowiedni plik QSS, ustawia odpowiednią czcionkę i zapisuje wybór w QSettings.
     */
    void onSkinChanged(const QString &skin);

private:
    /**
     * @brief Weryfikuje schemat bazy danych.
     * @param db Referencja do obiektu bazy danych.
     * @return true, jeśli schemat jest poprawny; false w przeciwnym razie.
     */
    bool verifyDatabaseSchema(QSqlDatabase &db);

    /**
     * @brief Tworzy schemat bazy danych, jeśli nie istnieje.
     * @param db Referencja do obiektu bazy danych.
     */
    void createDatabaseSchema(QSqlDatabase &db);

    /**
     * @brief Wstawia przykładowe dane do bazy danych.
     * @param db Referencja do obiektu bazy danych.
     */
    void insertSampleData(QSqlDatabase &db);

    /**
     * @brief Inicjalizuje filtry combo boxów.
     * @param db Referencja do obiektu bazy danych.
     */
    void initFilters(QSqlDatabase &db);

    /**
     * @brief Odświeża zawartość filtrów combo boxów.
     */
    void refreshFilters();

    /**
     * @brief Odbudowuje listy w combo boxach filtrów.
     */
    void updateFilterComboBoxes();

    /**
     * @brief Ładuje arkusz stylów dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     */
    void loadStyleSheet(const QString &skin);

    /**
     * @brief Ładuje czcionkę dla wybranej skórki.
     * @param skin Nazwa skórki (np. "Amiga", "ZX Spectrum", "Standard").
     */
    void loadFont(const QString &skin);

    /**
     * @brief Obsługuje zdarzenia filtrowania (np. opuszczenie okna podglądu).
     * @param watched Obiekt, dla którego przetwarzane jest zdarzenie.
     * @param event Wskaźnik na zdarzenie.
     * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /// Wskaźnik na obiekt interfejsu użytkownika.
    Ui::itemList *ui;

    /// Model źródłowy danych (relacyjna tabela eksponatów).
    QSqlRelationalTableModel *m_sourceModel;

    /// Model proxy do filtrowania danych.
    ItemFilterProxyModel *m_proxyModel;

    /// Combo box dla filtru typu eksponatu.
    QComboBox *filterTypeComboBox;

    /// Combo box dla filtru producenta.
    QComboBox *filterVendorComboBox;

    /// Combo box dla filtru modelu.
    QComboBox *filterModelComboBox;

    /// Combo box dla filtru statusu.
    QComboBox *filterStatusComboBox;

    /// Combo box dla filtru miejsca przechowywania.
    QComboBox *filterStorageComboBox;

    /// ID aktualnie wybranego rekordu.
    QString m_currentRecordId;

    /// Okno podglądu powiększonego zdjęcia.
    QWidget *m_previewWindow;

    /// Flaga wskazująca, czy kursor znajduje się nad oknem podglądu.
    bool m_previewHovered;

    /// Aktualnie podświetlony element zdjęcia.
    PhotoItem *m_currentHoveredItem;

    /// Timer do sprawdzania pozycji kursora w podglądzie zdjęć.
    QTimer *m_hoverCheckTimer;

    /// Timer do utrzymywania aktywności połączenia z bazą danych.
    QTimer *m_keepAliveTimer;

    /// ID czcionki Topaz załadowanej z zasobów.
    int m_topazFontId;

    /// ID czcionki ZX Spectrum załadowanej z zasobów.
    int m_zxFontId;
};

#endif // ITEMLIST_H

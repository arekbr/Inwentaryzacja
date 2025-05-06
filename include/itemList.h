/**
 * @file itemList.h
 * @brief Deklaracja klasy itemList, odpowiedzialnej za zarządzanie listą eksponatów w aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Klasa itemList reprezentuje widget wyświetlający tabelę eksponatów z możliwością filtrowania, edycji,
 * dodawania, usuwania i klonowania rekordów. Obsługuje interakcję z miniaturami zdjęć, podgląd
 * pełnoekranowy oraz dynamiczne filtrowanie danych. Skórka graficzna i czcionki są zarządzane
 * przez klasę DatabaseConfigDialog.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy itemList** – dziedziczy po QWidget.
 * 2. **Metody publiczne** – konstruktor i destruktor.
 * 3. **Sloty prywatne** – obsługują interakcje użytkownika (przyciski, tabela, zdjęcia).
 * 4. **Metody prywatne** – zarządzają schematem bazy danych, filtrami i zdarzeniami.
 * 5. **Zmienne prywatne** – przechowują model danych, filtry, timery i stan interfejsu.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QWidget, QComboBox, QSqlRelationalTableModel, QSettings, QLabel, QItemSelection.
 * - **Nagłówki aplikacji**: ItemFilterProxyModel.h, photoitem.h.
 * - **Namespace Ui**: Zawiera definicję interfejsu użytkownika (ui_itemList.h).
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest centralnym komponentem aplikacji, integrującym interfejs użytkownika z danymi bazy danych.
 * - Obsługuje zarówno SQLite, jak i MySQL, z konfiguracją zarządzaną przez DatabaseConfigDialog.
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
 * @section ClassOverview
 * Klasa itemList odpowiada za wyświetlanie i zarządzanie tabelą eksponatów w aplikacji do inwentaryzacji
 * retro komputerów. Umożliwia filtrowanie danych za pomocą combo boxów, obsługę zdjęć eksponatów
 * (podgląd, powiększenie, pełny ekran) oraz operacje CRUD (Create, Read, Update, Delete) na rekordach.
 *
 * @section Responsibilities
 * - Wyświetlanie tabeli eksponatów z danymi z bazy danych.
 * - Filtrowanie kaskadowe za pomocą combo boxów.
 * - Obsługa miniatur zdjęć (podgląd, pełny ekran).
 * - Operacje na rekordach: dodawanie, edycja, usuwanie, klonowanie.
 * - Zarządzanie schematem bazy danych i przykładowymi danymi.
 */
class itemList : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy itemList.
     * @param parent Wskaźnik na nadrzędny widget. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje interfejs użytkownika, model danych (QSqlRelationalTableModel), model proxy
     * (ItemFilterProxyModel), filtry combo boxów oraz połączenia sygnałów i slotów dla interakcji
     * użytkownika.
     */
    explicit itemList(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy itemList.
     *
     * @section DestructorOverview
     * Zwalnia zasoby, w tym interfejs użytkownika, model danych, timery i inne obiekty dynamiczne.
     */
    ~itemList();

private slots:
    /**
     * @brief Otwiera okno dodawania nowego eksponatu.
     *
     * @section SlotOverview
     * Tworzy nowe okno MainWindow w trybie dodawania rekordu i podłącza sygnał recordSaved
     * do slotu onRecordSaved.
     */
    void onNewButtonClicked();

    /**
     * @brief Otwiera okno edycji wybranego eksponatu.
     *
     * @section SlotOverview
     * Sprawdza, czy wybrano rekord, a następnie otwiera okno MainWindow w trybie edycji
     * dla wybranego ID rekordu.
     */
    void onEditButtonClicked();

    /**
     * @brief Zamyka aplikację.
     *
     * @section SlotOverview
     * Wywołuje qApp->quit(), kończąc działanie aplikacji.
     */
    void onEndButtonClicked();

    /**
     * @brief Usuwa wybrany eksponat po potwierdzeniu.
     *
     * @section SlotOverview
     * Po wybraniu rekordu wyświetla pytanie o potwierdzenie, a następnie usuwa rekord
     * z bazy danych i odświeża widok.
     */
    void onDeleteButtonClicked();

    /**
     * @brief Wyświetla okno "O programie" z informacjami o aplikacji.
     *
     * @section SlotOverview
     * Pokazuje QMessageBox z informacjami o nazwie, wersji, autorach i opisie aplikacji.
     */
    void onAboutClicked();

    /**
     * @brief Obsługuje zmianę zaznaczenia w tabeli eksponatów.
     * @param selected Zaznaczone indeksy.
     * @param deselected Odznaczone indeksy.
     *
     * @section SlotOverview
     * Aktualizuje wyświetlane miniatury zdjęć dla wybranego eksponatu, pobierając zdjęcia
     * z tabeli photos i wyświetlając je w QGraphicsView.
     */
    void onTableViewSelectionChanged(const QItemSelection &selected,
                                     const QItemSelection &deselected);

    /**
     * @brief Odświeża listę po zapisaniu rekordu.
     * @param recordId ID zapisanego rekordu.
     *
     * @section SlotOverview
     * Wywołuje refreshList z podanym ID rekordu, aby odświeżyć widok i zaznaczyć zapisany rekord.
     */
    void onRecordSaved(const QString &recordId);

    /**
     * @brief Odświeża listę eksponatów.
     * @param recordId Opcjonalne ID rekordu do wybrania po odświeżeniu.
     *
     * @section SlotOverview
     * Odświeża model danych, tabelę i filtry, opcjonalnie zaznaczając rekord o podanym ID.
     */
    void refreshList(const QString &recordId = QString());

    /**
     * @brief Wyświetla podgląd zdjęcia po najechaniu na miniaturę.
     * @param item Wskaźnik na element PhotoItem.
     *
     * @section SlotOverview
     * Tworzy okno podglądu z powiększonym zdjęciem i pozycjonuje je względem miniatury.
     * Uruchamia timer do sprawdzania pozycji kursora.
     */
    void onPhotoHovered(PhotoItem *item);

    /**
     * @brief Ukrywa podgląd zdjęcia po opuszczeniu miniatury.
     * @param item Wskaźnik na element PhotoItem.
     *
     * @section SlotOverview
     * Zamyka okno podglądu po krótkim opóźnieniu, jeśli kursor nie znajduje się nad oknem
     * podglądu lub miniaturą.
     */
    void onPhotoUnhovered(PhotoItem *item);

    /**
     * @brief Otwiera zdjęcie w trybie pełnoekranowym po kliknięciu.
     * @param item Wskaźnik na element PhotoItem.
     *
     * @section SlotOverview
     * Tworzy nowe okno FullScreenPhotoViewer z oryginalnym zdjęciem.
     */
    void onPhotoClicked(PhotoItem *item);

    /**
     * @brief Otwiera okno klonowania wybranego eksponatu.
     *
     * @section SlotOverview
     * Sprawdza, czy wybrano rekord, a następnie otwiera okno MainWindow w trybie klonowania
     * dla wybranego ID rekordu.
     */
    void onCloneButtonClicked();

    /**
     * @brief Aktualizuje filtry po zmianie wartości w combo boxach.
     *
     * @section SlotOverview
     * Ustawia filtry w modelu proxy na podstawie wybranych wartości w combo boxach
     * i odbudowuje listy combo boxów dla filtrowania kaskadowego.
     */
    void onFilterChanged();

private:
    /**
     * @brief Weryfikuje schemat bazy danych.
     * @param db Referencja do obiektu bazy danych.
     * @return true, jeśli schemat jest poprawny; false w przeciwnym razie.
     *
     * @section MethodOverview
     * Sprawdza, czy wszystkie wymagane tabele istnieją w bazie danych.
     */
    bool verifyDatabaseSchema(QSqlDatabase &db);

    /**
     * @brief Tworzy schemat bazy danych, jeśli nie istnieje.
     * @param db Referencja do obiektu bazy danych.
     *
     * @section MethodOverview
     * Tworzy tabele dla eksponatów, typów, producentów, modeli, statusów, miejsc przechowywania
     * i zdjęć.
     */
    void createDatabaseSchema(QSqlDatabase &db);

    /**
     * @brief Wstawia przykładowe dane do bazy danych.
     * @param db Referencja do obiektu bazy danych.
     *
     * @section MethodOverview
     * Wstawia przykładowe rekordy dla typów, producentów, modeli, statusów, miejsc przechowywania
     * i eksponatów.
     */
    void insertSampleData(QSqlDatabase &db);

    /**
     * @brief Inicjalizuje filtry combo boxów.
     * @param db Referencja do obiektu bazy danych.
     *
     * @section MethodOverview
     * Wypełnia combo boxy danymi z tabel bazy danych i podłącza sloty do aktualizacji filtrów.
     */
    void initFilters(QSqlDatabase &db);

    /**
     * @brief Odświeża zawartość filtrów combo boxów.
     *
     * @section MethodOverview
     * Zachowuje wybrane wartości filtrów, odświeża dane w combo boxach i przywraca wartości,
     * jeśli nadal istnieją.
     */
    void refreshFilters();

    /**
     * @brief Odbudowuje listy w combo boxach filtrów.
     *
     * @section MethodOverview
     * Aktualizuje zawartość combo boxów z uwzględnieniem kaskadowego filtrowania,
     * zachowując wybrane wartości.
     */
    void updateFilterComboBoxes();

    /**
     * @brief Obsługuje zdarzenia filtrowania (np. opuszczenie okna podglądu).
     * @param watched Obiekt, dla którego przetwarzane jest zdarzenie.
     * @param event Wskaźnik na zdarzenie.
     * @return true, jeśli zdarzenie zostało obsłużone; false w przeciwnym razie.
     *
     * @section MethodOverview
     * Zamyka okno podglądu zdjęcia, jeśli kursor opuści jego obszar.
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
};

#endif // ITEMLIST_H

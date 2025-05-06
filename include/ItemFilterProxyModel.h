/**
 * @file ItemFilterProxyModel.h
 * @brief Deklaracja klasy ItemFilterProxyModel do filtrowania listy eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik ItemFilterProxyModel.h definiuje klasę ItemFilterProxyModel, która dziedziczy po 
 * QSortFilterProxyModel i służy do dynamicznego filtrowania listy eksponatów w aplikacji 
 * inwentaryzacyjnej. Klasa umożliwia ustawienie filtrów dla różnych atrybutów eksponatu 
 * (typ, producent, model, status, miejsce przechowywania) i kontroluje, które wiersze 
 * modelu źródłowego (np. QSqlRelationalTableModel) są wyświetlane w widoku.
 *
 * @section Structure
 * Plik nagłówkowy zawiera:
 * 1. **Deklarację klasy ItemFilterProxyModel** – dziedziczy po QSortFilterProxyModel.
 * 2. **Metody publiczne** – konstruktor i metody do ustawiania filtrów.
 * 3. **Metoda chroniona** – filterAcceptsRow, realizująca logikę filtrowania.
 * 4. **Zmienne prywatne** – przechowują wartości filtrów dla każdego atrybutu.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klasy QSortFilterProxyModel do filtrowania i sortowania danych.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Klasa jest zaprojektowana do współpracy z modelem źródłowym, takim jak QSqlRelationalTableModel, 
 *   w kontekście aplikacji inwentaryzacyjnej.
 * - Filtry są dynamiczne i mogą być ustawiane niezależnie dla każdego atrybutu.
 */

/*
 * === ItemFilterProxyModel.h ===
 * Model proxy dla filtrowania listy eksponatów
 */
#ifndef ITEMFILTERPROXYMODEL_H
#define ITEMFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

/**
 * @class ItemFilterProxyModel
 * @brief Model proxy do filtrowania listy eksponatów.
 *
 * @section ClassOverview
 * Klasa ItemFilterProxyModel dziedziczy po QSortFilterProxyModel i dostarcza mechanizm 
 * dynamicznego filtrowania danych eksponatów na podstawie atrybutów takich jak typ, 
 * producent, model, status i miejsce przechowywania. Umożliwia ustawienie filtrów 
 * poprzez dedykowane metody i implementuje logikę filtrowania w metodzie filterAcceptsRow.
 *
 * @section Responsibilities
 * - Ustawianie filtrów dla różnych atrybutów eksponatu.
 * - Decydowanie, które wiersze modelu źródłowego spełniają kryteria filtrów.
 * - Współpraca z widokami Qt (np. QTableView) do wyświetlania przefiltrowanych danych.
 */
class ItemFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ItemFilterProxyModel.
     * @param parent Wskaźnik na obiekt nadrzędny. Domyślnie nullptr.
     *
     * @section ConstructorOverview
     * Inicjalizuje model proxy, ustawiając go jako dziecko podanego rodzica. 
     * Przygotowuje model do filtrowania danych bez dodatkowych ustawień początkowych.
     */
    explicit ItemFilterProxyModel(QObject *parent = nullptr);

    /**
     * @brief Ustawia filtr dla typu eksponatu.
     * @param type QString zawierający nazwę typu eksponatu lub pusty ciąg dla braku filtru.
     *
     * @section MethodOverview
     * Zapisuje wartość filtru dla typu eksponatu i wywołuje invalidateFilter(), 
     * aby odświeżyć widok z uwzględnieniem nowego filtru. Pusty ciąg wyłącza filtr dla typu.
     */
    void setTypeFilter(const QString &type);

    /**
     * @brief Ustawia filtr dla producenta eksponatu.
     * @param vendor QString zawierający nazwę producenta lub pusty ciąg dla braku filtru.
     *
     * @section MethodOverview
     * Zapisuje wartość filtru dla producenta i wywołuje invalidateFilter(), 
     * aby odświeżyć widok. Pusty ciąg wyłącza filtr dla producenta.
     */
    void setVendorFilter(const QString &vendor);

    /**
     * @brief Ustawia filtr dla modelu eksponatu.
     * @param model QString zawierający nazwę modelu lub pusty ciąg dla braku filtru.
     *
     * @section MethodOverview
     * Zapisuje wartość filtru dla modelu i wywołuje invalidateFilter(), 
     * aby odświeżyć widok. Pusty ciąg wyłącza filtr dla modelu.
     */
    void setModelFilter(const QString &model);

    /**
     * @brief Ustawia filtr dla statusu eksponatu.
     * @param status QString zawierający nazwę statusu lub pusty ciąg dla braku filtru.
     *
     * @section MethodOverview
     * Zapisuje wartość filtru dla statusu i wywołuje invalidateFilter(), 
     * aby odświeżyć widok. Pusty ciąg wyłącza filtr dla statusu.
     */
    void setStatusFilter(const QString &status);

    /**
     * @brief Ustawia filtr dla miejsca przechowywania eksponatu.
     * @param storage QString zawierający nazwę miejsca przechowywania lub pusty ciąg dla braku filtru.
     *
     * @section MethodOverview
     * Zapisuje wartość filtru dla miejsca przechowywania i wywołuje invalidateFilter(), 
     * aby odświeżyć widok. Pusty ciąg wyłącza filtr dla miejsca przechowywania.
     */
    void setStorageFilter(const QString &storage);

protected:
    /**
     * @brief Decyduje, czy dany wiersz modelu źródłowego powinien być widoczny.
     * @param sourceRow Numer wiersza w modelu źródłowym.
     * @param sourceParent Indeks rodzica w modelu źródłowym.
     * @return true, jeśli wiersz spełnia wszystkie aktywne filtry; false w przeciwnym razie.
     *
     * @section MethodOverview
     * Przesłania metodę QSortFilterProxyModel, implementując logikę filtrowania. 
     * Sprawdza, czy wartości w kolumnach modelu źródłowego (typ, producent, model, 
     * status, miejsce przechowywania) odpowiadają ustawionym filtrom. Pusty filtr 
     * dla danego atrybutu oznacza, że wszystkie wartości dla tej kolumny są akceptowane.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    /// Filtr dla typu eksponatu (pusty oznacza brak filtru).
    QString m_type;
    /// Filtr dla producenta eksponatu (pusty oznacza brak filtru).
    QString m_vendor;
    /// Filtr dla modelu eksponatu (pusty oznacza brak filtru).
    QString m_model;
    /// Filtr dla statusu eksponatu (pusty oznacza brak filtru).
    QString m_status;
    /// Filtr dla miejsca przechowywania eksponatu (pusty oznacza brak filtru).
    QString m_storage;
};

#endif // ITEMFILTERPROXYMODEL_H

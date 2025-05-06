/**
 * @file ItemFilterProxyModel.cpp
 * @brief Implementacja klasy ItemFilterProxyModel do filtrowania listy eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version \projectnumber
 * @date 2025-05-03
 *
 * @section Overview
 * Plik ItemFilterProxyModel.cpp zawiera implementację metod klasy ItemFilterProxyModel, 
 * która umożliwia dynamiczne filtrowanie listy eksponatów na podstawie atrybutów takich 
 * jak typ, producent, model, status i miejsce przechowywania. Klasa korzysta z mechanizmu 
 * QSortFilterProxyModel, umożliwiając filtrowanie wierszy modelu źródłowego (np. 
 * QSqlRelationalTableModel) z ignorowaniem wielkości liter.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Konstruktor** – inicjalizuje model proxy i konfiguruje filtrowanie.
 * 2. **Metody ustawiania filtrów** – zapisują wartości filtrów i odświeżają widok.
 * 3. **Metoda filterAcceptsRow** – implementuje logikę filtrowania wierszy.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas QSortFilterProxyModel, QModelIndex, QAbstractItemModel.
 * - **Nagłówek**: ItemFilterProxyModel.h.
 *
 * @section Notes
 * - Kod nie został zmodyfikowany, zgodnie z wymaganiami użytkownika. Dodano jedynie komentarze i dokumentację.
 * - Filtrowanie jest niezależne od wielkości liter, co ułatwia dopasowanie wartości.
 * - Klasa zakłada, że model źródłowy ma określoną strukturę kolumn (np. typ w kolumnie 2).
 */

/*
 * === ItemFilterProxyModel.cpp ===
 * Implementacja filtrowania według wybranych wartości z QComboBox
 */
#include "ItemFilterProxyModel.h"
#include <QModelIndex>

/**
 * @brief Konstruktor klasy ItemFilterProxyModel.
 * @param parent Wskaźnik na obiekt nadrzędny. Domyślnie nullptr.
 *
 * @section ConstructorOverview
 * Inicjalizuje model proxy jako dziecko podanego rodzica. Ustawia filtrowanie 
 * bez uwzględnienia wielkości liter poprzez setFilterCaseSensitivity(Qt::CaseInsensitive).
 */
ItemFilterProxyModel::ItemFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Filtrowanie bez uwzględnienia wielkości liter
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

/**
 * @brief Ustawia filtr dla typu eksponatu.
 * @param type QString zawierający nazwę typu eksponatu lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla typu eksponatu w zmiennej m_type i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setTypeFilter(const QString &type)
{
    m_type = type;
    invalidateFilter(); // odświeżenie widoku
}

/**
 * @brief Ustawia filtr dla producenta eksponatu.
 * @param vendor QString zawierający nazwę producenta lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla producenta w zmiennej m_vendor i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setVendorFilter(const QString &vendor)
{
    m_vendor = vendor;
    invalidateFilter();
}

/**
 * @brief Ustawia filtr dla modelu eksponatu.
 * @param model QString zawierający nazwę modelu lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla modelu w zmiennej m_model i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setModelFilter(const QString &model)
{
    m_model = model;
    invalidateFilter();
}

/**
 * @brief Ustawia filtr dla statusu eksponatu.
 * @param status QString zawierający nazwę statusu lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla statusu w zmiennej m_status i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setStatusFilter(const QString &status)
{
    m_status = status;
    invalidateFilter();
}

/**
 * @brief Ustawia filtr dla miejsca przechowywania eksponatu.
 * @param storage QString zawierający nazwę miejsca przechowywania lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla miejsca przechowywania w zmiennej m_storage i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setStorageFilter(const QString &storage)
{
    m_storage = storage;
    invalidateFilter();
}

/**
 * @brief Decyduje, czy dany wiersz modelu źródłowego powinien być widoczny.
 * @param sourceRow Numer wiersza w modelu źródłowym.
 * @param sourceParent Indeks rodzica w modelu źródłowym.
 * @return true, jeśli wiersz spełnia wszystkie aktywne filtry; false w przeciwnym razie.
 *
 * @section MethodOverview
 * Implementuje logikę filtrowania, sprawdzając, czy wartości w kolumnach modelu 
 * źródłowego odpowiadają ustawionym filtrom. Używa lambdy pasuje do porównania wartości 
 * w kolumnach (2: typ, 3: producent, 4: model, 9: status, 10: miejsce przechowywania) 
 * z wartościami filtrów. Pusty filtr dla danej kolumny oznacza akceptację wszystkich wartości.
 */
bool ItemFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QAbstractItemModel *src = sourceModel();
    auto pasuje = [&](int col, const QString &wzor) {
        if (wzor.isEmpty())
            return true; // brak filtra => wszystko pasuje
        QModelIndex idx = src->index(sourceRow, col, sourceParent);
        return src->data(idx).toString() == wzor; // porównanie wartości
    };

    // Kolumny w QSqlRelationalTableModel:
    // 2 = Typ, 3 = Producent, 4 = Model, 9 = Status, 10 = Miejsce przechowywania
    return pasuje(2, m_type) && pasuje(3, m_vendor) && pasuje(4, m_model) && pasuje(9, m_status)
           && pasuje(10, m_storage);
}

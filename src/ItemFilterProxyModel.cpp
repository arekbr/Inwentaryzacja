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
 * jak typ, producent, model, status, miejsce przechowywania i nazwa. Klasa korzysta z 
 * mechanizmu QSortFilterProxyModel, umożliwiając filtrowanie wierszy modelu źródłowego 
 * (np. QSqlRelationalTableModel) z ignorowaniem wielkości liter.
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
 * - Kod obsługuje filtrowanie pełnotekstowe dla nazwy eksponatu oraz dokładne dopasowanie 
 *   dla pozostałych atrybutów.
 * - Filtrowanie jest niezależne od wielkości liter, co ułatwia dopasowanie wartości.
 * - Klasa zakłada, że model źródłowy ma określoną strukturę kolumn (np. nazwa w kolumnie 1, 
 *   typ w kolumnie 2).
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
 * @brief Ustawia filtr dla nazwy eksponatu.
 * @param filter QString zawierający fragment nazwy eksponatu lub pusty ciąg dla braku filtru.
 *
 * @section MethodOverview
 * Zapisuje wartość filtru dla nazwy w zmiennej m_nameFilter i wywołuje 
 * invalidateFilter(), aby odświeżyć widok z uwzględnieniem nowego filtru.
 */
void ItemFilterProxyModel::setNameFilter(const QString &filter)
{
    qDebug() << "ItemFilterProxyModel: Ustawiam nameFilter:" << filter;
    m_nameFilter = filter;
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
 * źródłowego odpowiadają ustawionym filtrom. Dla nazwy (kolumna 1) używa contains 
 * (wyszukiwanie pełnotekstowe), dla pozostałych kolumn (2: typ, 3: producent, 
 * 4: model, 9: status, 10: miejsce przechowywania) wymaga dokładnego dopasowania.
 */
bool ItemFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QAbstractItemModel *src = sourceModel();
    if (!src)
        return false;

    auto pasuje = [&](int col, const QString &wzor) {
        if (wzor.isEmpty())
            return true; // Brak filtra => wszystko pasuje
        QModelIndex idx = src->index(sourceRow, col, sourceParent);
        return src->data(idx).toString() == wzor; // Dokładne dopasowanie
    };

    // Sprawdź filtr nazwy (kolumna 1)
    if (!m_nameFilter.isEmpty()) {
        QModelIndex nameIndex = src->index(sourceRow, 1, sourceParent); // Kolumna 1 to 'name'
        QString name = src->data(nameIndex).toString();
        if (!name.contains(m_nameFilter, Qt::CaseInsensitive))
            return false; // Nazwa nie pasuje do filtra
    }

    // Sprawdź pozostałe filtry (typ, producent, model, status, miejsce przechowywania)
    return pasuje(2, m_type) && pasuje(3, m_vendor) && pasuje(4, m_model) && pasuje(9, m_status)
           && pasuje(10, m_storage);
}

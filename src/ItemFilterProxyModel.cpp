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
    , m_type()
    , m_vendor()
    , m_model()
    , m_status()
    , m_storage()
    , m_nameFilter()
    , m_showOriginalPackaging(false)
    , m_originalPackagingFilterEnabled(false)
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

void ItemFilterProxyModel::setOriginalPackagingFilter(bool show)
{
    m_showOriginalPackaging = show;
    m_originalPackagingFilterEnabled = show; // Filtr jest aktywny tylko gdy checkbox jest zaznaczony
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
    QModelIndex typeIndex = sourceModel()->index(sourceRow, 2, sourceParent);
    QModelIndex vendorIndex = sourceModel()->index(sourceRow, 3, sourceParent);
    QModelIndex modelIndex = sourceModel()->index(sourceRow, 4, sourceParent);
    QModelIndex statusIndex = sourceModel()->index(sourceRow, 9, sourceParent);
    QModelIndex storageIndex = sourceModel()->index(sourceRow, 10, sourceParent);
    QModelIndex nameIndex = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex packagingIndex = sourceModel()->index(sourceRow, 13, sourceParent);

    bool typeMatch = m_type.isEmpty() || sourceModel()->data(typeIndex).toString() == m_type;
    bool vendorMatch = m_vendor.isEmpty() || sourceModel()->data(vendorIndex).toString() == m_vendor;
    bool modelMatch = m_model.isEmpty() || sourceModel()->data(modelIndex).toString() == m_model;
    bool statusMatch = m_status.isEmpty() || sourceModel()->data(statusIndex).toString() == m_status;
    bool storageMatch = m_storage.isEmpty() || sourceModel()->data(storageIndex).toString() == m_storage;
    bool nameMatch = m_nameFilter.isEmpty() || 
                    sourceModel()->data(nameIndex).toString().contains(m_nameFilter, Qt::CaseInsensitive);
    
    // Zmiana logiki - filtrujemy tylko gdy checkbox jest zaznaczony
    bool packagingMatch = !m_originalPackagingFilterEnabled || 
                         (m_originalPackagingFilterEnabled && sourceModel()->data(packagingIndex).toBool());

    return typeMatch && vendorMatch && modelMatch && statusMatch && 
           storageMatch && nameMatch && packagingMatch;
}

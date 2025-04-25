/**
 * @file ItemFilterProxyModel.cpp
 * @brief Implementacja klasy ItemFilterProxyModel do filtrowania listy eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera implementację metod klasy ItemFilterProxyModel, odpowiedzialnej za filtrowanie
 * danych eksponatów na podstawie wartości wybranych w combo boxach (typ, producent, model,
 * status, miejsce przechowywania). Klasa korzysta z QSortFilterProxyModel, umożliwiając
 * dynamiczne filtrowanie wierszy modelu źródłowego (QSqlRelationalTableModel) bez
 * uwzględnienia wielkości liter.
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
 * Inicjalizuje model proxy, ustawiając go jako dziecko podanego rodzica i konfigurując
 * filtrowanie bez uwzględnienia wielkości liter.
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
 * Zapisuje wartość filtru dla typu i odświeża widok poprzez wywołanie invalidateFilter().
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
 * Zapisuje wartość filtru dla producenta i odświeża widok poprzez wywołanie invalidateFilter().
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
 * Zapisuje wartość filtru dla modelu i odświeża widok poprzez wywołanie invalidateFilter().
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
 * Zapisuje wartość filtru dla statusu i odświeża widok poprzez wywołanie invalidateFilter().
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
 * Zapisuje wartość filtru dla miejsca przechowywania i odświeża widok poprzez wywołanie invalidateFilter().
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
 * Sprawdza, czy wartości w kolumnach modelu źródłowego (typ, producent, model, status,
 * miejsce przechowywania) odpowiadają ustawionym filtrom. Jeśli filtr jest pusty, dana
 * kolumna jest uznawana za pasującą. Kolumny są mapowane na: 2 (typ), 3 (producent),
 * 4 (model), 9 (status), 10 (miejsce przechowywania).
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

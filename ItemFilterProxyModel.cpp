/*
 * === ItemFilterProxyModel.cpp ===
 * Implementacja filtrowania według wybranych wartości z QComboBox
 */
#include "ItemFilterProxyModel.h"
#include <QModelIndex>

ItemFilterProxyModel::ItemFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Filtrowanie bez uwzględnienia wielkości liter
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ItemFilterProxyModel::setTypeFilter(const QString &type)
{
    m_type = type;
    invalidateFilter(); // odświeżenie widoku
}

void ItemFilterProxyModel::setVendorFilter(const QString &vendor)
{
    m_vendor = vendor;
    invalidateFilter();
}

void ItemFilterProxyModel::setModelFilter(const QString &model)
{
    m_model = model;
    invalidateFilter();
}

void ItemFilterProxyModel::setStatusFilter(const QString &status)
{
    m_status = status;
    invalidateFilter();
}

void ItemFilterProxyModel::setStorageFilter(const QString &storage)
{
    m_storage = storage;
    invalidateFilter();
}

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

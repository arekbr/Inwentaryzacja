/*
 * === ItemFilterProxyModel.h ===
 * Model proxy dla filtrowania listy eksponatów
 */
#ifndef ITEMFILTERPROXYMODEL_H
#define ITEMFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class ItemFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ItemFilterProxyModel(QObject *parent = nullptr);

    // Ustawienia filtrów (puste = wszystkie)
    void setTypeFilter(const QString &type);
    void setVendorFilter(const QString &vendor);
    void setModelFilter(const QString &model);
    void setStatusFilter(const QString &status);
    void setStorageFilter(const QString &storage);

protected:
    // Metoda decydująca, czy dany wiersz powinien być widoczny
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_type;
    QString m_vendor;
    QString m_model;
    QString m_status;
    QString m_storage;
};

#endif // ITEMFILTERPROXYMODEL_H

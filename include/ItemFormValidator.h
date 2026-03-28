#ifndef ITEMFORMVALIDATOR_H
#define ITEMFORMVALIDATOR_H

#include <QSqlDatabase>
#include <QString>

enum class ItemValidationField
{
    None,
    Name,
    Value,
    Status,
    Type,
    Vendor,
    Model,
    StoragePlace,
    Database
};

struct ItemValidationResult
{
    bool isValid = true;
    QString title;
    QString message;
    ItemValidationField field = ItemValidationField::None;
    int parsedValue = 0;

    static ItemValidationResult ok(int value = 0);
    static ItemValidationResult error(const QString &title,
                                      const QString &message,
                                      ItemValidationField field);
};

class ItemFormValidator
{
public:
    static ItemValidationResult validateName(const QString &name);
    static ItemValidationResult validateSelection(const QString &selectedId,
                                                  const QString &fieldLabel,
                                                  ItemValidationField field);
    static ItemValidationResult validateNumericValue(const QString &rawValue);
    static ItemValidationResult validateModelVendorConsistency(QSqlDatabase db,
                                                               const QString &vendorId,
                                                               const QString &modelId);
};

#endif // ITEMFORMVALIDATOR_H

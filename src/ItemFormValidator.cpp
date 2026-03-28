#include "ItemFormValidator.h"

#include <QSqlError>
#include <QSqlQuery>

ItemValidationResult ItemValidationResult::ok(int value)
{
    ItemValidationResult result;
    result.parsedValue = value;
    return result;
}

ItemValidationResult ItemValidationResult::error(const QString &title,
                                                 const QString &message,
                                                 ItemValidationField field)
{
    ItemValidationResult result;
    result.isValid = false;
    result.title = title;
    result.message = message;
    result.field = field;
    return result;
}

ItemValidationResult ItemFormValidator::validateName(const QString &name)
{
    if (!name.trimmed().isEmpty())
        return ItemValidationResult::ok();

    return ItemValidationResult::error(QObject::tr("Brak danych"),
                                       QObject::tr("Nazwa eksponatu jest wymagana."),
                                       ItemValidationField::Name);
}

ItemValidationResult ItemFormValidator::validateSelection(const QString &selectedId,
                                                          const QString &fieldLabel,
                                                          ItemValidationField field)
{
    if (!selectedId.trimmed().isEmpty())
        return ItemValidationResult::ok();

    return ItemValidationResult::error(QObject::tr("Brak danych"),
                                       QObject::tr("Pole \"%1\" wymaga wyboru poprawnej wartości.")
                                           .arg(fieldLabel),
                                       field);
}

ItemValidationResult ItemFormValidator::validateNumericValue(const QString &rawValue)
{
    const QString trimmedValue = rawValue.trimmed();
    if (trimmedValue.isEmpty())
        return ItemValidationResult::ok(0);

    bool ok = false;
    const int parsedValue = trimmedValue.toInt(&ok);
    if (ok)
        return ItemValidationResult::ok(parsedValue);

    return ItemValidationResult::error(QObject::tr("Błędna wartość"),
                                       QObject::tr("Pole \"Wartość\" musi zawierać liczbę całkowitą."),
                                       ItemValidationField::Value);
}

ItemValidationResult ItemFormValidator::validateModelVendorConsistency(QSqlDatabase db,
                                                                       const QString &vendorId,
                                                                       const QString &modelId)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT 1 FROM models WHERE id = :model_id AND vendor_id = :vendor_id"));
    query.bindValue(QStringLiteral(":model_id"), modelId);
    query.bindValue(QStringLiteral(":vendor_id"), vendorId);

    if (!query.exec()) {
        return ItemValidationResult::error(QObject::tr("Błąd walidacji"),
                                           QObject::tr("Nie udało się sprawdzić zgodności modelu i producenta:\n%1")
                                               .arg(query.lastError().text()),
                                           ItemValidationField::Database);
    }

    if (query.next())
        return ItemValidationResult::ok();

    return ItemValidationResult::error(QObject::tr("Niespójne dane"),
                                       QObject::tr("Wybrany model nie należy do wskazanego producenta.\n"
                                                  "Wybierz zgodny model albo zmień producenta."),
                                       ItemValidationField::Model);
}

/**
 * @file ItemFilterProxyModel.h
 * @brief Deklaracja klasy ItemFilterProxyModel do filtrowania listy eksponatów.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera deklarację klasy ItemFilterProxyModel, która dziedziczy po QSortFilterProxyModel
 * i służy do dynamicznego filtrowania listy eksponatów w aplikacji inwentaryzacyjnej. Klasa
 * umożliwia ustawienie filtrów dla typu, producenta, modelu, statusu i miejsca przechowywania
 * eksponatu, a następnie decyduje, które wiersze modelu źródłowego powinny być wyświetlane
 * na podstawie zadanych kryteriów.
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
 * Klasa ItemFilterProxyModel dziedziczy po QSortFilterProxyModel i implementuje
 * mechanizm filtrowania danych eksponatów na podstawie wartości takich jak typ,
 * producent, model, status i miejsce przechowywania. Filtry są ustawiane za pomocą
 * dedykowanych metod, a logika filtrowania jest realizowana w metodzie filterAcceptsRow().
 */
class ItemFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ItemFilterProxyModel.
     * @param parent Wskaźnik na obiekt nadrzędny. Domyślnie nullptr.
     *
     * Inicjalizuje model proxy, ustawiając go jako dziecko podanego rodzica.
     */
    explicit ItemFilterProxyModel(QObject *parent = nullptr);

    /**
     * @brief Ustawia filtr dla typu eksponatu.
     * @param type QString zawierający nazwę typu eksponatu lub pusty ciąg dla braku filtru.
     *
     * Ustawia filtr, który ogranicza wyświetlane eksponaty do tych o określonym typie.
     * Pusty ciąg oznacza brak filtru (wszystkie typy).
     */
    void setTypeFilter(const QString &type);

    /**
     * @brief Ustawia filtr dla producenta eksponatu.
     * @param vendor QString zawierający nazwę producenta lub pusty ciąg dla braku filtru.
     *
     * Ustawia filtr, który ogranicza wyświetlane eksponaty do tych od określonego producenta.
     * Pusty ciąg oznacza brak filtru (wszyscy producenci).
     */
    void setVendorFilter(const QString &vendor);

    /**
     * @brief Ustawia filtr dla modelu eksponatu.
     * @param model QString zawierający nazwę modelu lub pusty ciąg dla braku filtru.
     *
     * Ustawia filtr, który ogranicza wyświetlane eksponaty do tych o określonym modelu.
     * Pusty ciąg oznacza brak filtru (wszystkie modele).
     */
    void setModelFilter(const QString &model);

    /**
     * @brief Ustawia filtr dla statusu eksponatu.
     * @param status QString zawierający nazwę statusu lub pusty ciąg dla braku filtru.
     *
     * Ustawia filtr, który ogranicza wyświetlane eksponaty do tych o określonym statusie.
     * Pusty ciąg oznacza brak filtru (wszystkie statusy).
     */
    void setStatusFilter(const QString &status);

    /**
     * @brief Ustawia filtr dla miejsca przechowywania eksponatu.
     * @param storage QString zawierający nazwę miejsca przechowywania lub pusty ciąg dla braku filtru.
     *
     * Ustawia filtr, który ogranicza wyświetlane eksponaty do tych przechowywanych w określonym miejscu.
     * Pusty ciąg oznacza brak filtru (wszystkie miejsca).
     */
    void setStorageFilter(const QString &storage);

protected:
    /**
     * @brief Decyduje, czy dany wiersz modelu źródłowego powinien być widoczny.
     * @param sourceRow Numer wiersza w modelu źródłowym.
     * @param sourceParent Indeks rodzica w modelu źródłowym.
     * @return true, jeśli wiersz spełnia wszystkie aktywne filtry; false w przeciwnym razie.
     *
     * Przesłania metodę QSortFilterProxyModel, sprawdzając, czy wartości w wierszu
     * modelu źródłowego zgadzają się z ustawionymi filtrami (typ, producent, model,
     * status, miejsce przechowywania).
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

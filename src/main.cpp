/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.2.2
 * @date 2025-05-03
 *
 * @section Overview
 * Plik main.cpp stanowi główny punkt wejścia aplikacji inwentaryzacyjnej opartej na bibliotece Qt.
 * Odpowiada za inicjalizację aplikacji, ustawienie jej podstawowych właściwości (nazwa, wersja, ikona),
 * ładowanie tłumaczeń językowych, wyświetlanie okna konfiguracji bazy danych oraz nawiązywanie
 * połączenia z wybraną bazą danych (SQLite lub MySQL). Po poprawnym skonfigurowaniu środowiska
 * uruchamia główne okno aplikacji reprezentowane przez klasę itemList.
 *
 * @section Structure
 * Kod jest podzielony na następujące sekcje:
 * 1. **Sekcja inicjalizacji aplikacji Qt** - konfiguracja podstawowa aplikacji, stylu graficznego i ikony.
 * 2. **Sekcja ładowania tłumaczeń** - dynamiczne ładowanie plików tłumaczeń na podstawie języka systemowego.
 * 3. **Sekcja konfiguracji bazy danych** - wyświetlanie okna dialogowego i pobieranie ustawień bazy danych.
 * 4. **Sekcja nawiązywania połączenia z bazą danych** - obsługa różnych typów baz danych (SQLite, MySQL).
 * 5. **Sekcja uruchamiania głównego okna** - inicjalizacja i wyświetlanie głównego interfejsu aplikacji.
 * 6. **Sekcja pętli zdarzeń Qt** - uruchomienie głównej pętli aplikacji.
 *
 * @section Dependencies
 * - **Qt Framework**: Używa klas takich jak QApplication, QTranslator, QIcon, QLocale do zarządzania aplikacją i tłumaczeniami.
 * - **Nagłówki aplikacji**: DatabaseConfigDialog (okno konfiguracji bazy), itemList (główne okno), utils (funkcje pomocnicze, np. setupDatabase).
 * - **Zasoby**: Ikona aplikacji (:/images/icon.png) oraz pliki tłumaczeń (:/i18n/Inwentaryzacja_*.qm).
 *
 * @section Notes
 * - Kod nie pozwala na modyfikację, zgodnie z wymaganiami użytkownika. Wszelkie komentarze i dokumentacja są dodawane bez ingerencji w oryginalny kod.
 * - Aplikacja obsługuje dwa typy baz danych: SQLite (lokalna baza plikowa) oraz MySQL (baza sieciowa).
 * - Styl graficzny jest ustawiony na "Fusion", aby zapewnić spójny wygląd niezależnie od systemu operacyjnego.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QTranslator>

// Nagłówki aplikacji
#include "DatabaseConfigDialog.h"
#include "itemList.h"
#include "utils.h"

/**
 * @brief Główna funkcja aplikacji inwentaryzacyjnej.
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return Kod zakończenia aplikacji (0 dla sukcesu, inne dla błędów).
 *
 * @section FunctionOverview
 * Funkcja main jest punktem wejścia aplikacji i odpowiada za jej inicjalizację oraz uruchomienie.
 * Wykonuje następujące kroki:
 * 1. Inicjalizuje aplikację Qt i ustawia jej styl graficzny.
 * 2. Konfiguruje nazwę, wersję i ikonę aplikacji.
 * 3. Ładuje tłumaczenia językowe na podstawie ustawień systemowych.
 * 4. Wyświetla okno konfiguracji bazy danych i pobiera ustawienia od użytkownika.
 * 5. Nawiązuje połączenie z bazą danych (SQLite lub MySQL) za pomocą funkcji setupDatabase.
 * 6. Uruchamia główne okno aplikacji (klasa itemList) i wchodzi w główną pętlę zdarzeń Qt - Qt.
 *
 * @section ReturnValues
 * - **0**: Aplikacja zakończyła się pomyślnie lub użytkownik anulował konfigurację.
 * - **Inne**: Błąd podczas nawiązywania połączenia z bazą danych lub nieznany typ bazy danych.
 */
int main(int argc, char *argv[])
{
    // Sekcja 1: Inicjalizacja aplikacji Qt
    // Tworzy instancję QApplication, która zarządza zasobami aplikacji i obsługą zdarzeń.
    // Ustawia styl "Fusion" dla spójnego wyglądu na różnych platformach.
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    // Ustawienie metadanych aplikacji: nazwa i wersja.
    QCoreApplication::setApplicationName(QStringLiteral("Inwentaryzacja"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.2.2"));

    // Ustawienie ikony aplikacji z zasobów Qt.
    QIcon icon(":/images/icon.png");
    a.setWindowIcon(icon);

    // Sekcja 2: Ładowanie tłumaczeń
    // Dynamiczne ładowanie tłumaczeń na podstawie języka systemowego użytkownika.
    // Pliki tłumaczeń znajdują się w zasobach Qt (:/i18n/Inwentaryzacja_*.qm).
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Inwentaryzacja_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // Sekcja 3: Konfiguracja bazy danych
    // Wyświetla okno dialogowe DatabaseConfigDialog, w którym użytkownik wybiera typ bazy danych
    // (SQLite lub MySQL) i wprowadza odpowiednie parametry połączenia.
    DatabaseConfigDialog configDlg;
    if (configDlg.exec() != QDialog::Accepted) {
        qDebug() << "Użytkownik anulował konfigurację bazy danych.";
        return 0;
    }

    // Sekcja 4: Nawiązywanie połączenia z bazą danych
    // Pobiera typ bazy danych wybrany przez użytkownika i inicjuje połączenie.
    // Obsługuje SQLite (lokalny plik) oraz MySQL (połączenie sieciowe).
    QString dbType = configDlg.selectedDatabaseType();
    bool connected = false;
    // Akceptujmy wszystkie warianty SQLite (np. "SQLite", "SQLite3", ...)
    if (dbType.startsWith("SQLite", Qt::CaseInsensitive)) {
        QString filePath = configDlg.sqliteFilePath();
        if (filePath.isEmpty()) {
            qDebug() << "Nie wybrano pliku bazy SQLite!";
            return 0;
        }
        connected = setupDatabase("SQLite3", filePath);
    } else if (dbType.compare("MySQL", Qt::CaseInsensitive) == 0) {
        connected = setupDatabase("MySQL",
                                  configDlg.mysqlDatabase(),
                                  configDlg.mysqlHost(),
                                  configDlg.mysqlUser(),
                                  configDlg.mysqlPassword(),
                                  configDlg.mysqlPort());
    } else {
        qDebug() << "Nieznany typ bazy danych:" << dbType;
        return 0;
    }

    // Sprawdzenie, czy połączenie z bazą danych zostało nawiązane.
    if (!connected) {
        qDebug() << "Nie udało się nawiązać połączenia z bazą danych.";
        return 0;
    }

    // Sekcja 5: Uruchamianie głównego okna
    // Tworzy i wyświetla główne okno aplikacji (klasa itemList), które zawiera główny interfejs użytkownika.
    itemList w;
    w.show();

    // Sekcja 6: Pętla zdarzeń Qt
    // Uruchamia główną pętlę zdarzeń Qt, która obsługuje interakcje użytkownika i zdarzenia systemowe.
    return a.exec();
}

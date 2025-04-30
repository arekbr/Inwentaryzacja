/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.10
 * @date 2025-04-30
 *
 * Plik zawiera funkcję main, która inicjalizuje aplikację Qt, ustawia nazwę i wersję aplikacji,
 * ładuje tłumaczenia, wyświetla okno konfiguracji bazy danych (SQLite lub MySQL), nawiązuje
 * połączenie z bazą danych za pomocą funkcji setupDatabase, ładuje skórkę graficzną z QSettings
 * i uruchamia główne okno aplikacji (klasa itemList). Czcionki są zarządzane przez klasę itemList.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QTranslator>

// Nagłówki aplikacji
#include "DatabaseConfigDialog.h"
#include "itemList.h"
#include "utils.h"

/**
 * @brief Ładuje arkusz stylów z podanego pliku QSS.
 * @param filePath Ścieżka do pliku QSS w systemie zasobów Qt (np. ":/styles/default.qss").
 *
 * Otwiera plik QSS, wczytuje jego zawartość i ustawia styl aplikacji za pomocą qApp->setStyleSheet.
 * Loguje sukces lub błąd ładowania.
 */
void loadStyleSheet(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        qDebug() << "Zawartość pliku stylu (" << filePath << "):\n" << style;
        qApp->setStyleSheet(style);
        file.close();
    } else {
        qWarning() << "Nie można załadować pliku stylu:" << filePath;
    }
}

/**
 * @brief Główna funkcja aplikacji inwentaryzacyjnej.
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return Kod zakończenia aplikacji (0 dla sukcesu, inne dla błędów).
 *
 * Inicjalizuje aplikację Qt, ustawia nazwę i wersję aplikacji, ładuje tłumaczenia na podstawie
 * języka systemowego, ładuje skórkę graficzną z QSettings, wyświetla okno dialogowe konfiguracji
 * bazy danych, nawiązuje połączenie z bazą danych (SQLite lub MySQL) i uruchamia główne okno aplikacji.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Wymuszenie stylu Qt, aby uniknąć nadpisywania przez styl systemowy
    a.setStyle("Fusion");

    // Ładowanie skórki z QSettings
    QSettings settings("SMOK", "Inwentaryzacja");
    QString skin = settings.value("skin", "Standard").toString();
    QString qssPath;
    if (skin == "Amiga") {
        qssPath = ":/styles/amiga.qss";
    } else if (skin == "ZX Spectrum") {
        qssPath = ":/styles/zxspectrum.qss";
    } else {
        qssPath = ":/styles/default.qss"; // Domyślnie Standard
    }
    loadStyleSheet(qssPath);

    QCoreApplication::setApplicationName(QStringLiteral("Inwentaryzacja"));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));

    QIcon icon(":/images/icon.png");
    a.setWindowIcon(icon);

    // Instalacja tłumaczeń
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Inwentaryzacja_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // Wyświetlenie okna dialogowego konfiguracji bazy danych
    DatabaseConfigDialog configDlg;
    if (configDlg.exec() != QDialog::Accepted) {
        qDebug() << "Użytkownik anulował konfigurację bazy danych.";
        return 0;
    }

    // Pobranie typu bazy danych wybranego przez użytkownika
    QString dbType = configDlg.selectedDatabaseType();
    bool connected = false;
    // zaakceptujmy wszystkie warianty SQLite (np. "SQLite", "SQLite3", ...)
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

    if (!connected) {
        qDebug() << "Nie udało się nawiązać połączenia z bazą danych.";
        return 0;
    }

    // Uruchomienie głównego okna aplikacji
    itemList w;
    w.show();

    return a.exec();
}

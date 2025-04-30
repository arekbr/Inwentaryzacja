/**
 * @file main.cpp
 * @brief Główny punkt wejścia aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK & ChatGPT & GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera funkcję main, która inicjalizuje aplikację Qt, ustawia nazwę i wersję aplikacji,
 * ładuje tłumaczenia, wyświetla okno konfiguracji bazy danych (SQLite lub MySQL), nawiązuje
 * połączenie z bazą danych za pomocą funkcji setupDatabase i uruchamia główne okno aplikacji
 * (klasa itemList). Funkcja main jest punktem startowym dla aplikacji inwentaryzacyjnej.
 */

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFontDatabase>
#include <QLocale>
#include <QTextStream>
#include <QTranslator>

// Nagłówki aplikacji
#include "DatabaseConfigDialog.h"
#include "itemList.h"
#include "utils.h"

void loadStyleSheet(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        qDebug() << "Zawartość pliku stylu (" << filePath << "):\n" << style; // Debugowanie
        qApp->setStyleSheet(style);
        file.close();
    } else {
        qWarning("Nie można załadować pliku stylu: %s", qPrintable(filePath));
    }
}


/**
 * @brief Główna funkcja aplikacji inwentaryzacyjnej.
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return Kod zakończenia aplikacji (0 dla sukcesu, inne dla błędów).
 *
 * Inicjalizuje aplikację Qt, ustawia nazwę i wersję aplikacji, ładuje tłumaczenia na podstawie
 * języka systemowego, wyświetla okno dialogowe konfiguracji bazy danych, nawiązuje połączenie
 * z bazą danych (SQLite lub MySQL) i uruchamia główne okno aplikacji (itemList). Jeśli konfiguracja
 * bazy danych zostanie anulowana lub połączenie nie powiedzie się, aplikacja kończy działanie.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Wymuszenie stylu Qt, aby uniknąć nadpisywania przez styl systemowy
    a.setStyle("Fusion");

    // Ładowanie stylu z zasobów Qt (.qrc)
    loadStyleSheet(":/styles/zxspectrum.qss");


    // Ładuj czcionkę Topaz
    int id = QFontDatabase::addApplicationFont(":/images/topaz.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    // Ładowanie czcionki ZX Spectrum
    int zxId = QFontDatabase::addApplicationFont(":/images/zxspectrum.ttf");
    QStringList zxFamilies = QFontDatabase::applicationFontFamilies(zxId);

    if (!zxFamilies.isEmpty()) {
        QFont zxFont(zxFamilies.at(0));
        zxFont.setPointSize(12);
        qApp->setFont(zxFont);
    } else {
        qWarning() << "Nie udało się załadować czcionki ZX Spectrum!";
    }

    qDebug() << "ZX rodzina fontow:" << QFontDatabase::applicationFontFamilies(zxId);



    QFont topazFont(family);
    topazFont.setPointSize(12); // Workbench był bardzo drobny, około 8px na 320x256
    qApp->setFont(topazFont);

    QFont zxFont("Untitled1");
    zxFont.setPointSize(12);
    qApp->setFont(zxFont);

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

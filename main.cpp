#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QCoreApplication>

// Uwzględniamy pliki z naszą implementacją dialogu konfiguracji
#include "DatabaseConfigDialog.h"
// Główne okno – w tym przykładzie używamy klasy itemList, ale możesz podmienić na MainWindow
#include "itemList.h"
// Dołączenie funkcji setupDatabase() – załóżmy, że znajduje się ona w utils.h
#include "utils.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Inwentaryzacja"));
    QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));

    a.setWindowIcon(QIcon(":/icon.png"));
    // Instalacja tłumaczeń (jeśli są dostępne)
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
    if (dbType == "SQLite3") {
        QString filePath = configDlg.sqliteFilePath();
        if (filePath.isEmpty()) {
            qDebug() << "Nie wybrano pliku bazy SQLite!";
            return 0;
        }
        connected = setupDatabase("SQLite3", filePath);
    } else if (dbType == "MySQL") {
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

    // Teraz, gdy połączenie (default_connection) zostało poprawnie utworzone,
    // tworzymy i pokazujemy główne okno aplikacji
    itemList w;
    w.show();

    return a.exec();
}

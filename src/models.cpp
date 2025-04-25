/**
 * @file models.cpp
 * @brief Implementacja klasy models służącej do zarządzania modelami sprzętu w aplikacji inwentaryzacyjnej.
 * @author Stowarzyszenie Miłośników Oldschoolowych Komputerów SMOK
 * @author ChatGPT
 * @author GROK
 * @version 1.1.8
 * @date 2025-04-25
 *
 * Plik zawiera implementację metod klasy models, odpowiedzialnej za zarządzanie modelami
 * sprzętu w bazie danych. Klasa umożliwia dodawanie, edytowanie i usuwanie modeli z tabeli 'models'.
 * Współpracuje z interfejsem użytkownika generowanym przez Qt Designer oraz z klasą MainWindow,
 * zapewniając aktualizację danych w głównym widoku aplikacji.
 */

#include "models.h"
#include "ui_models.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QInputDialog>
#include "mainwindow.h"
#include <QUuid>

/**
 * @brief Konstruktor klasy models.
 * @param parent Wskaźnik na nadrzędny widget (domyślnie nullptr).
 *
 * Inicjalizuje interfejs graficzny, ustawia tytuł okna, nawiązuje połączenie z bazą danych
 * oraz przypisuje odpowiednie sloty do przycisków interfejsu użytkownika.
 */
models::models(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::models),
    m_mainWindow(nullptr),
    m_vendorId(QString()) // Pusty string jako "brak ID"
{
    ui->setupUi(this);
    m_db = QSqlDatabase::database("default_connection");
    setWindowTitle(tr("Zarządzanie modelami sprzętu"));

    // Połączenie przycisków
    connect(ui->pushButton_add, &QPushButton::clicked, this, &models::onAddClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &models::onEditClicked);
    connect(ui->pushButton_delete, &QPushButton::clicked, this, &models::onDeleteClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &models::onOkClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    refreshList();
}

/**
 * @brief Destruktor klasy models.
 *
 * Usuwa obiekt interfejsu użytkownika i zwalnia zasoby.
 */
models::~models()
{
    delete ui;
}

/**
 * @brief Ustawia wskaźnik na główne okno aplikacji.
 * @param mainWindow Wskaźnik na obiekt MainWindow.
 *
 * Umożliwia komunikację z głównym oknem w celu odświeżania danych w combo boxach.
 */
void models::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/**
 * @brief Ustawia identyfikator producenta, powiązany z modelami.
 * @param vendorId QString zawierający identyfikator producenta (np. UUID).
 *
 * Wartość ta jest wykorzystywana podczas dodawania nowego modelu, aby przypisać go do odpowiedniego producenta.
 */
void models::setVendorId(const QString &vendorId)
{
    m_vendorId = vendorId;
}

/**
 * @brief Odświeża listę modeli w widoku listy.
 *
 * Pobiera dane z tabeli 'models' z bazy danych i aktualizuje komponent listView.
 * W przypadku błędu zapytania SQL wyświetlany jest komunikat krytyczny.
 */
void models::refreshList()
{
    QSqlQueryModel *queryModel = new QSqlQueryModel(this);
    queryModel->setQuery("SELECT name FROM models ORDER BY name ASC", m_db);
    if (queryModel->lastError().isValid()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Błąd pobierania danych: %1")
                                                    .arg(queryModel->lastError().text()));
        return;
    }
    ui->listView->setModel(queryModel);
    ui->listView->setModelColumn(0);
}

/**
 * @brief Slot obsługujący dodawanie nowego modelu.
 *
 * Pobiera nazwę modelu z pola tekstowego, generuje nowy identyfikator UUID
 * oraz wstawia dane do tabeli 'models' w bazie danych.
 * W przypadku niepowodzenia wyświetlany jest komunikat o błędzie.
 * Po udanym dodaniu lista modeli jest odświeżana, a pole wejściowe czyszczone.
 */
void models::onAddClicked()
{
    QString newModel = ui->lineEdit->text().trimmed();
    if(newModel.isEmpty()){
        QMessageBox::warning(this, tr("Błąd"), tr("Nazwa modelu nie może być pusta."));
        return;
    }

    QString modelId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO models (id, name, vendor_id) VALUES (:id, :name, :vendor_id)");
    query.bindValue(":id", modelId);

    QString vendorId;
    if (!m_vendorId.isEmpty()) {
        vendorId = m_vendorId;
    } else if (m_mainWindow) {
        vendorId = m_mainWindow->getNewItemVendorComboBox()->currentData().toString();
    }

    query.bindValue(":name", newModel);
    query.bindValue(":vendor_id", vendorId.isEmpty() ? QString("unknown_vendor_uuid") : vendorId);

    if(!query.exec()){
        QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać modelu:\n%1")
                                                    .arg(query.lastError().text()));
    }

    refreshList();
    ui->lineEdit->clear();
}

/**
 * @brief Slot obsługujący edytowanie istniejącego modelu.
 *
 * Użytkownik wybiera model z listy, podaje nową nazwę w oknie dialogowym,
 * a program aktualizuje rekord w bazie danych. W przypadku niepowodzenia
 * wyświetlany jest komunikat o błędzie. Lista modeli jest odświeżana.
 */
void models::onEditClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do edycji."));
        return;
    }

    QString currentName = index.data(Qt::DisplayRole).toString();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Edytuj model"), tr("Nowa nazwa:"), QLineEdit::Normal, currentName, &ok);
    if(ok && !newName.trimmed().isEmpty()){
        QSqlQuery query(m_db);
        query.prepare("SELECT id FROM models WHERE name = :name");
        query.bindValue(":name", currentName);
        if(query.exec() && query.next()){
            QString id = query.value(0).toString();
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE models SET name = :newName WHERE id = :id");
            updateQuery.bindValue(":newName", newName.trimmed());
            updateQuery.bindValue(":id", id);
            if(!updateQuery.exec()){
                QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować modelu:\n%1")
                                                            .arg(updateQuery.lastError().text()));
            }
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący usuwanie modelu.
 *
 * Po zaznaczeniu modelu z listy i potwierdzeniu zamiaru usunięcia przez użytkownika,
 * rekord zostaje usunięty z tabeli 'models'. W przypadku niepowodzenia wyświetlany jest komunikat o błędzie.
 * Lista jest odświeżana po każdej próbie usunięcia.
 */
void models::onDeleteClicked()
{
    QModelIndex index = ui->listView->currentIndex();
    if(!index.isValid()){
        QMessageBox::information(this, tr("Informacja"), tr("Proszę wybrać model do usunięcia."));
        return;
    }

    QString modelName = index.data(Qt::DisplayRole).toString();
    int ret = QMessageBox::question(this, tr("Potwierdzenie"),
                                    tr("Czy na pewno chcesz usunąć model: %1?").arg(modelName),
                                    QMessageBox::Yes | QMessageBox::No);
    if(ret == QMessageBox::Yes){
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM models WHERE name = :name");
        query.bindValue(":name", modelName);
        if(!query.exec()){
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się usunąć modelu:\n%1")
                                                        .arg(query.lastError().text()));
        }
    }

    refreshList();
}

/**
 * @brief Slot obsługujący zatwierdzenie zmian i zamknięcie okna dialogowego.
 *
 * Jeśli główne okno zostało ustawione, metoda odświeża dane modeli w odpowiednim combo boxie.
 * Następnie zamyka okno z wynikiem zaakceptowania (`accept()`).
 */
void models::onOkClicked()
{
    if(m_mainWindow) {
        m_mainWindow->loadComboBoxData("models", m_mainWindow->getNewItemModelComboBox());
    }
    accept();
}

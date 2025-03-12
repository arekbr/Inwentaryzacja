#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <QBuffer>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_editMode(false),
    m_recordId(-1),
    m_selectedPhotoIndex(-1),
    m_photoScene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    // Inicjalizacja bazy danych
    db = QSqlDatabase::database("default_connection");
    if (!db.isOpen()) {
        QMessageBox::critical(this, tr("Błąd"), tr("Baza danych nie jest otwarta."));
        return;
    }

    // Ustawienie sceny w graphicsView
    ui->graphicsView->setScene(m_photoScene);

    connect(ui->New_item_PushButton_OK, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    connect(ui->New_item_PushButton_Cancel, &QPushButton::clicked, this, &MainWindow::onCancelClicked);
    connect(ui->New_item_addPhoto, &QPushButton::clicked, this, &MainWindow::onAddPhotoClicked);
    connect(ui->New_item_removePhoto, &QPushButton::clicked, this, &MainWindow::onRemovePhotoClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_photoScene;
}

void MainWindow::setEditMode(bool edit, int recordId)
{
    m_editMode = edit;
    m_recordId = recordId;
    if (edit && recordId != -1) {
        loadRecord(recordId);
        loadPhotos(recordId);
    } else {
        ui->New_item_name->clear();
        ui->New_item_description->clear();
        m_photoScene->clear(); // Czyścimy scenę
    }
    ui->New_item_PushButton_OK->setEnabled(true);
    ui->New_item_addPhoto->setEnabled(true);
}

void MainWindow::onSaveClicked()
{
    QSqlQuery query(db);
    if (m_editMode) {
        // Aktualizacja istniejącego rekordu
        query.prepare("UPDATE eksponaty SET nazwa = :nazwa, opis = :opis WHERE id = :id");
        query.bindValue(":id", m_recordId);
        query.bindValue(":nazwa", ui->New_item_name->text());
        query.bindValue(":opis", ui->New_item_description->toPlainText()); // Używamy toPlainText() dla QPlainTextEdit
        if (!query.exec()) {
            qDebug() << "Błąd aktualizacji rekordu:" << query.lastError().text();
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się zaktualizować rekordu."));
            return;
        }
    } else {
        // Dodanie nowego rekordu
        query.prepare("INSERT INTO eksponaty (nazwa, opis) VALUES (:nazwa, :opis)");
        query.bindValue(":nazwa", ui->New_item_name->text());
        query.bindValue(":opis", ui->New_item_description->toPlainText()); // Używamy toPlainText() dla QPlainTextEdit
        if (!query.exec()) {
            qDebug() << "Błąd wstawiania rekordu:" << query.lastError().text();
            QMessageBox::critical(this, tr("Błąd"), tr("Nie udało się dodać rekordu."));
            return;
        }
        // Pobierz ID nowo wstawionego rekordu
        int newRecordId = query.lastInsertId().toInt();
        m_recordId = newRecordId; // Zaktualizuj m_recordId
    }

    // Zapisz zdjęcia, jeśli istnieją (przechowujemy je w scenie)
    QList<QGraphicsItem*> items = m_photoScene->items();
    for (QGraphicsItem *item : items) {
        if (auto *photoItem = dynamic_cast<PhotoItem*>(item)) {
            QByteArray imageData;
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            photoItem->pixmap().save(&buffer, "PNG");
            buffer.close();

            query.prepare("INSERT INTO photos (eksponat_id, photo) VALUES (:id, :photo)");
            query.bindValue(":id", m_recordId);
            query.bindValue(":photo", imageData);
            if (!query.exec()) {
                qDebug() << "Błąd wstawiania zdjęcia:" << query.lastError().text();
            }
        }
    }

    QMessageBox::information(this, tr("Sukces"), tr("Rekord został zapisany."));
    emit recordSaved(m_recordId); // Emitujemy sygnał z ID zapisanej pozycji
    close();
}

void MainWindow::onCancelClicked()
{
    close();
}

void MainWindow::onAddPhotoClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Wybierz zdjęcie"), "", tr("Obrazy (*.png *.jpg *.jpeg *.bmp)"));
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            PhotoItem *item = new PhotoItem(pixmap, nullptr);
            m_photoScene->addItem(item);
            ui->graphicsView->fitInView(m_photoScene->sceneRect(), Qt::KeepAspectRatio);
        } else {
            QMessageBox::warning(this, tr("Błąd"), tr("Nie udało się wczytać obrazu."));
        }
    }
}

void MainWindow::loadRecord(int recordId)
{
    QSqlQuery query(db);
    query.prepare("SELECT nazwa, opis FROM eksponaty WHERE id = :id");
    query.bindValue(":id", recordId);
    if (query.exec() && query.next()) {
        ui->New_item_name->setText(query.value("nazwa").toString());
        ui->New_item_description->setPlainText(query.value("opis").toString()); // Używamy setPlainText() dla QPlainTextEdit
    } else {
        qDebug() << "Błąd ładowania rekordu:" << query.lastError().text();
    }
}

void MainWindow::loadPhotos(int recordId)
{
    QSqlQuery query(db);
    query.prepare("SELECT photo FROM photos WHERE eksponat_id = :id");
    query.bindValue(":id", recordId);
    if (query.exec()) {
        m_photoScene->clear();
        while (query.next()) {
            QByteArray imageData = query.value("photo").toByteArray();
            QPixmap pixmap;
            if (pixmap.loadFromData(imageData)) {
                PhotoItem *item = new PhotoItem(pixmap, nullptr);
                m_photoScene->addItem(item);
            }
        }
        ui->graphicsView->fitInView(m_photoScene->sceneRect(), Qt::KeepAspectRatio);
    } else {
        qDebug() << "Błąd ładowania zdjęć:" << query.lastError().text();
    }
}

void MainWindow::onRemovePhotoClicked()
{
    if (m_selectedPhotoIndex >= 0 && m_selectedPhotoIndex < m_photoScene->items().count()) {
        QList<QGraphicsItem*> items = m_photoScene->items();
        delete m_photoScene->items().at(m_selectedPhotoIndex);
        m_selectedPhotoIndex = -1;
        ui->graphicsView->fitInView(m_photoScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void MainWindow::onPhotoClicked(PhotoItem *item)
{
    QList<QGraphicsItem*> items = m_photoScene->items();
    for (int i = 0; i < items.count(); ++i) {
        if (dynamic_cast<PhotoItem*>(items.at(i)) == item) {
            m_selectedPhotoIndex = i;
            break;
        }
    }
}

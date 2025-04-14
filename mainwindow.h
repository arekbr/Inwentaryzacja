#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QComboBox>
#include "photoitem.h"
#include <QList>
#include <QUuid>  // do generowania UUID

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setEditMode(bool edit, const QString &recordId = QString());
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);

    QComboBox* getNewItemTypeComboBox() const;
    QComboBox* getNewItemModelComboBox() const;
    QComboBox* getNewItemVendorComboBox() const;
    QComboBox* getNewItemStatusComboBox() const;
    QComboBox* getNewItemStoragePlaceComboBox() const;

    bool getEditMode() const { return m_editMode; }
    QString getRecordId() const { return m_recordId; }

    void setCloneMode(const QString &recordId);

signals:
    void recordSaved(const QString &recordId);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void onRemovePhotoClicked();
    void loadRecord(const QString &recordId);
    void loadPhotos(const QString &recordId);
    void onPhotoClicked(PhotoItem *item);
    void onAddTypeClicked();
    void onAddVendorClicked();
    void onAddModelClicked();
    void onAddStatusClicked();
    void onAddStoragePlaceClicked();

private:
    void loadPhotosFromBuffer();

    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool m_editMode;
    QString m_recordId;  // ZMIANA: dawniej int
    int m_selectedPhotoIndex;
    QList<QByteArray> m_photoBuffer;
};

#endif // MAINWINDOW_H

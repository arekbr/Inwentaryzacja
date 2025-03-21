#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QComboBox>
#include "photoitem.h"
#include <QList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setEditMode(bool edit, int recordId = -1);
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);

    QComboBox* getNewItemTypeComboBox() const;
    QComboBox* getNewItemModelComboBox() const;
    QComboBox* getNewItemVendorComboBox() const;
    QComboBox* getNewItemStatusComboBox() const;
    QComboBox* getNewItemStoragePlaceComboBox() const;

    bool getEditMode() const { return m_editMode; }
    int getRecordId() const { return m_recordId; }
    void setCloneMode(int recordId);

signals:
    void recordSaved(int recordId);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void onRemovePhotoClicked();
    void loadRecord(int recordId);
    void loadPhotos(int recordId);
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
    int m_recordId;
    int m_selectedPhotoIndex;
    QList<QByteArray> m_photoBuffer;
};

#endif // MAINWINDOW_H

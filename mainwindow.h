#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QComboBox>
#include "photoitem.h" // Dodano, aby zdefiniować PhotoItem

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setEditMode(bool edit, int recordId = -1);
    void loadComboBoxData(const QString &tableName, QComboBox *comboBox);

    // Deklaracje metod dostępowych bez definicji inline
    QComboBox* getNewItemTypeComboBox() const;
    QComboBox* getNewItemModelComboBox() const;
    QComboBox* getNewItemVendorComboBox() const;

    // Gettery dla m_editMode i m_recordId
    bool getEditMode() const { return m_editMode; }
    int getRecordId() const { return m_recordId; }

signals:
    void recordSaved(int recordId);

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onAddPhotoClicked();
    void loadRecord(int recordId);
    void loadPhotos(int recordId);
    void onRemovePhotoClicked();
    void onPhotoClicked(PhotoItem *item);
    void onAddTypeClicked();
    void onAddVendorClicked();
    void onAddModelClicked();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    bool m_editMode;
    int m_recordId;
    int m_selectedPhotoIndex;
};

#endif // MAINWINDOW_H

﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QTreeView>
#include <QListView>
#include <QString>

#include <statgetter.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void setTreeRootIndex(int);
//    void setComboRootIndex(int);
    void processStatRequest(const QModelIndex& index);
    void processProgressBar(int status, const QString& msg);
    void initTreeRoot();
    void unblockCombo();

private:
    void SetPositionCenter();

private:
    Ui::MainWindow* ui_;
    QFileSystemModel* fsComboModel_;
    QFileSystemModel* fsTreeModel_;
    StatGetter* statGetter_;
};

#endif // MAINWINDOW_H

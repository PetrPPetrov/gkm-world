// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <memory>
#include <QObject>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QTreeView>
#include <QTableView>
#include <QThread>
#include <QMdiSubWindow>
#include <QListWidget>
#include "ui_main_window.h"
#include "aux_geometry.h"
#include "common.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void showEvent(QShowEvent* event) override;
    int getPhotoCount() const;
    ImagePtr getPhoto(int index) const;

private:
    void addPhoto(const char* filename);
    void onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    AuxGeometry::Ptr aux_geometry;

    bool first_show = true;
    Ui::MainWindow main_window;

    MeshBuilderWidget* camera_orientation_widget = nullptr;
    QMdiSubWindow* camera_orientation_window = nullptr;

    QPlainTextEdit* log_widget = nullptr;
    QMdiSubWindow* log_window = nullptr;

    QListWidget* photo_list_widget = nullptr;
    QMdiSubWindow* photo_list_window = nullptr;

    std::vector<ImagePtr> photos;
};

extern MainWindow* g_main_window;

// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHeaderView>
#include <QPushButton>
#include "main_window.h"

extern MainWindow* g_main_window = nullptr;

MainWindow::MainWindow()
{
    g_main_window = this;
    main_window.setupUi(this);

    aux_geometry = std::make_shared<AuxGeometry>();
    auto new_box = std::make_shared<Box>();
    new_box->size = QVector3D(1, 1, 1);
    aux_geometry->boxes.push_back(new_box);

    photo_list_widget = new QListWidget(main_window.centralwidget);
    connect(photo_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onPhotoChanged);
    photo_list_window = main_window.centralwidget->addSubWindow(photo_list_widget);
    photo_list_window->setWindowTitle(tr("Photos List"));

    addPhoto("chair01.jpg");
    addPhoto("chair02.jpg");
    addPhoto("chair03.jpg");
    addPhoto("chair04.jpg");
    addPhoto("chair05.jpg");

    log_widget = new QPlainTextEdit(main_window.centralwidget);
    log_widget->setReadOnly(true);
    log_widget->ensureCursorVisible();
    log_widget->setCenterOnScroll(true);
    log_window = main_window.centralwidget->addSubWindow(log_widget);
    log_window->setWindowTitle(tr("Log"));

    camera_orientation_widget = new MeshBuilderWidget(main_window.centralwidget);
    camera_orientation_widget->setAuxGeometry(aux_geometry);
    camera_orientation_window = main_window.centralwidget->addSubWindow(camera_orientation_widget);
    camera_orientation_window->setWindowTitle(tr("3D Camera Orientation for Photo"));
}

int MainWindow::getPhotoCount() const
{
    return static_cast<int>(photos.size());
}

ImagePtr MainWindow::getPhoto(int index) const
{
    return photos.at(index);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;

        camera_orientation_window->resize(QSize(main_window.centralwidget->width() * 3 / 4, main_window.centralwidget->height()));
        camera_orientation_window->move(main_window.centralwidget->width() / 4, 0);

        photo_list_window->resize(QSize(main_window.centralwidget->width() / 4, main_window.centralwidget->height() / 2));
        photo_list_window->move(0, 0);

        log_window->resize(QSize(main_window.centralwidget->width() / 4, main_window.centralwidget->height() / 2));
        log_window->move(0, main_window.centralwidget->height() / 2);
    }
}

void MainWindow::addPhoto(const char* filename)
{
    photo_list_widget->addItem(filename);
    photos.push_back(std::make_shared<QImage>(QString(filename)));
}

void MainWindow::onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (photo_list_widget->selectedItems().size() > 0)
    {
        int index = photo_list_widget->currentIndex().row();
        if (index == 1)
        {
            aux_geometry->boxes.front()->size = QVector3D(1, 2, 3);
            camera_orientation_widget->updateAuxGeometry();
        }
        else if (index == 2)
        {
            aux_geometry->boxes.front()->size = QVector3D(5, 6, 7);
            camera_orientation_widget->updateAuxGeometry();
        }
        else
        {
            aux_geometry->boxes.front()->size = QVector3D(1, 1, 1);
            camera_orientation_widget->updateAuxGeometry();
        }

        log_widget->appendPlainText(photo_list_widget->item(index)->text());
    }
}

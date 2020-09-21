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

    build_info = std::make_shared<BuildInfo>();

    photo_list_widget = new QListWidget(main_window.centralwidget);
    connect(photo_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onPhotoChanged);
    photo_list_window = main_window.centralwidget->addSubWindow(photo_list_widget);
    photo_list_window->setWindowTitle(tr("Photos List"));

    log_widget = new QPlainTextEdit(main_window.centralwidget);
    log_widget->setReadOnly(true);
    log_widget->ensureCursorVisible();
    log_widget->setCenterOnScroll(true);
    log_window = main_window.centralwidget->addSubWindow(log_widget);
    log_window->setWindowTitle(tr("Log"));

    if (fileExists(auto_save_file_name))
    {
        loadBuildInfo(build_info, auto_save_file_name);
    }
    else
    {
        createDefaultProject();
    }
    loadPhotos();
    updatePhotoListWidget();

    camera_orientation_widget = new MeshBuilderWidget(main_window.centralwidget);
    camera_orientation_widget->setAuxGeometry(aux_geometry);
    camera_orientation_widget->setPhoto(build_info->cameras_info[0]);
    camera_orientation_window = main_window.centralwidget->addSubWindow(camera_orientation_widget);
    camera_orientation_window->setWindowTitle(tr("3D Camera Orientation for Photo"));
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

void MainWindow::createDefaultProject()
{
    addPhoto("chair01.jpg");
    addPhoto("chair02.jpg");
    addPhoto("chair03.jpg");
    addPhoto("chair04.jpg");
    addPhoto("chair05.jpg");
    addPhoto("chair06.jpg");
    addPhoto("chair07.jpg");
    addPhoto("chair08.jpg");
    addPhoto("chair09.jpg");
    addPhoto("chair10.jpg");
    addPhoto("chair11.jpg");
    addPhoto("chair12.jpg");
    addPhoto("chair13.jpg");
}

void MainWindow::addPhoto(const char* filename)
{
    CameraInfo::Ptr new_camera_info = std::make_shared<CameraInfo>();
    new_camera_info->photo_image_path = filename;
    new_camera_info->viewer_pos = Eigen::Vector3d(0, 0, 10);
    new_camera_info->viewer_target = Eigen::Vector3d(0, 0, 0);
    new_camera_info->viewer_up = Eigen::Vector3d(0, 1, 0);
    new_camera_info->rotation_radius = 10;
    build_info->cameras_info.push_back(new_camera_info);
}

void MainWindow::loadPhotos()
{
    for (auto camera_info : build_info->cameras_info)
    {
        if (!camera_info->photo_image)
        {
            camera_info->photo_image = std::make_shared<QImage>(QString(camera_info->photo_image_path.c_str()));
        }
    }
}

void MainWindow::updatePhotoListWidget()
{
    for (auto camera_info : build_info->cameras_info)
    {
        photo_list_widget->addItem(QString(camera_info->photo_image_path.c_str()));
    }
}

void MainWindow::onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (photo_list_widget->selectedItems().size() > 0)
    {
        int index = photo_list_widget->currentIndex().row();
        camera_orientation_widget->setPhoto(build_info->cameras_info.at(index));
        camera_orientation_widget->updatePhotoTexture();
        camera_orientation_widget->update();
        log_widget->appendPlainText(photo_list_widget->item(index)->text());
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    saveBuildInfo(build_info, auto_save_file_name);
    event->accept();
}

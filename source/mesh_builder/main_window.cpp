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
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QPushButton>
#include <QFileDialog>
#include "main_window.h"

extern MainWindow* g_main_window = nullptr;

MainWindow::MainWindow()
{
    g_main_window = this;
    main_window.setupUi(this);

    QMenu* project_menu = menuBar()->addMenu("Project");

    new_project_act = new QAction("&New", this);
    new_project_act->setShortcuts(QKeySequence::New);
    new_project_act->setStatusTip("Create new project");
    connect(new_project_act, &QAction::triggered, this, &MainWindow::onNewProject);
    project_menu->addAction(new_project_act);

    project_menu->addSeparator();
    open_project_act = new QAction("&Open", this);
    open_project_act->setShortcuts(QKeySequence::Open);
    open_project_act->setStatusTip("Open existing project");
    connect(open_project_act, &QAction::triggered, this, &MainWindow::onOpenProject);
    project_menu->addAction(open_project_act);

    save_project_act = new QAction("&Save", this);
    save_project_act->setShortcuts(QKeySequence::Save);
    save_project_act->setStatusTip("Save project");
    connect(save_project_act, &QAction::triggered, this, &MainWindow::onSaveProject);
    project_menu->addAction(save_project_act);

    save_project_as_act = new QAction("&Save as...", this);
    save_project_as_act->setShortcuts(QKeySequence::SaveAs);
    save_project_as_act->setStatusTip("Save project as...");
    connect(save_project_as_act, &QAction::triggered, this, &MainWindow::onSaveAsProject);
    project_menu->addAction(save_project_as_act);

    project_menu->addSeparator();
    quit_act = new QAction("&Quit", this);
    quit_act->setShortcuts(QKeySequence::Quit);
    quit_act->setStatusTip("Quit application");
    connect(quit_act, &QAction::triggered, this, &MainWindow::onQuit);
    project_menu->addAction(quit_act);

    QMenu* photo_menu = menuBar()->addMenu("Photo");

    add_photo_act = new QAction("&Add photo", this);
    add_photo_act->setShortcuts(QKeySequence::AddTab);
    add_photo_act->setStatusTip("Add existing photo to project");
    connect(add_photo_act, &QAction::triggered, this, &MainWindow::onAddPhoto);
    photo_menu->addAction(add_photo_act);

    remove_photo_act = new QAction("&Remove photo", this);
    remove_photo_act->setStatusTip("Remove photo from project");
    remove_photo_act->setEnabled(false);
    connect(remove_photo_act, &QAction::triggered, this, &MainWindow::onRemovePhoto);
    photo_menu->addAction(remove_photo_act);

    photo_list_widget = new QListWidget(main_window.centralwidget);
    connect(photo_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onPhotoChanged);
    photo_list_window = main_window.centralwidget->addSubWindow(photo_list_widget);
    photo_list_window->setWindowTitle("Photos List");

    log_widget = new QPlainTextEdit(main_window.centralwidget);
    log_widget->setReadOnly(true);
    log_widget->ensureCursorVisible();
    log_widget->setCenterOnScroll(true);
    log_window = main_window.centralwidget->addSubWindow(log_widget);
    log_window->setWindowTitle("Log");

    camera_orientation_widget = new MeshBuilderWidget(main_window.centralwidget);
    camera_orientation_window = main_window.centralwidget->addSubWindow(camera_orientation_widget);
    camera_orientation_window->setWindowTitle("3D Camera Orientation for Photo");
}

void MainWindow::updateWindowTitle()
{
    QString file_name = "Unnamed";
    if (!mesh_project->file_name.empty())
    {
        file_name = QString(mesh_project->file_name.c_str());
    }
    QString dirty_flag = "";
    if (mesh_project->dirty)
    {
        dirty_flag = "*";
    }
    setWindowTitle(dirty_flag + file_name + QString(" - Gkm-World Mesh-Builder"));
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;

        initial_camera_available_width = camera_available_width = main_window.centralwidget->width() * 2 / 3;
        initial_camera_available_height = camera_available_height = main_window.centralwidget->height();

        camera_orientation_window->setFixedSize(QSize(camera_available_width, camera_available_height));
        camera_orientation_window->move(main_window.centralwidget->width() / 3, 0);

        photo_list_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 2));
        photo_list_window->move(0, 0);

        log_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 4));
        log_window->move(0, main_window.centralwidget->height() * 3 / 4);

        //if (fileExists(auto_save_file_name))
        //{
        //    loadProject(auto_save_file_name);
        //}
        //else
        //{
            onNewProject();
        //}
    }
}

void MainWindow::addPhoto(const char* filename)
{
    CameraInfo::Ptr new_camera_info = std::make_shared<CameraInfo>();
    new_camera_info->photo_image_path = filename;
    new_camera_info->viewer_pos = Eigen::Vector3d(3, 3, 3);
    new_camera_info->viewer_target = Eigen::Vector3d(0, 0, 0);
    new_camera_info->viewer_up = Eigen::Vector3d(0, 0, 1);
    new_camera_info->rotation_radius = (new_camera_info->viewer_pos - new_camera_info->viewer_target).norm();
    mesh_project->build_info->cameras_info.push_back(new_camera_info);

    addPhotoListWidgetItem(new_camera_info);
}

void MainWindow::loadProject(const char* filename)
{
    loadMeshProject(mesh_project, filename);
    mesh_project->file_name = filename;
    fillPhotoListWidget();
    loadPhotos();
    if (mesh_project->build_info->cameras_info.size() > 0)
    {
        photo_list_widget->setCurrentRow(0);
    }
    else
    {
        photo_list_widget->clearSelection();
    }
    updateProject();
}

void MainWindow::loadPhotos()
{
    for (auto camera_info : mesh_project->build_info->cameras_info)
    {
        if (!camera_info->photo_image)
        {
            camera_info->photo_image = std::make_shared<QImage>(QString(camera_info->photo_image_path.c_str()));
        }
    }
}

void MainWindow::updateProject()
{
    camera_orientation_widget->setMeshProject(mesh_project);
    camera_orientation_widget->updateAuxGeometry();
    camera_orientation_widget->updatePhotoTexture();
    updateWindowTitle();
    camera_orientation_widget->update();
}

void MainWindow::fillPhotoListWidget()
{
    photo_list_widget->clear();
    for (auto camera_info : mesh_project->build_info->cameras_info)
    {
        addPhotoListWidgetItem(camera_info);
    }
}

void MainWindow::addPhotoListWidgetItem(const CameraInfo::Ptr& camera_info)
{
    QWidget* widget = new QWidget(photo_list_widget);
    QLabel* label = new QLabel(QString(camera_info->photo_image_path.c_str()), photo_list_widget);

    QCheckBox* locked = new QCheckBox("locked", photo_list_widget);
    locked->setChecked(camera_info->locked);
    locked->setEnabled(false);
    connect(locked, &QCheckBox::stateChanged, this, &MainWindow::onLockedChanged);

    QComboBox* rotation = new QComboBox(photo_list_widget);
    rotation->addItem("0 degree");
    rotation->addItem("90 degree");
    rotation->addItem("180 degree");
    rotation->addItem("270 degree");
    rotation->setEnabled(false);
    switch (camera_info->rotation)
    {
    default:
    case 0:
        rotation->setCurrentIndex(0);
        break;
    case 90:
        rotation->setCurrentIndex(1);
        break;
    case 180:
        rotation->setCurrentIndex(2);
        break;
    case 270:
        rotation->setCurrentIndex(3);
        break;
    }
    connect(rotation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRotationChanged);

    QHBoxLayout* layout = new QHBoxLayout(photo_list_widget);
    layout->addWidget(label);
    layout->addWidget(locked);
    layout->addWidget(rotation);
    layout->addStretch();
    layout->setSizeConstraint(QLayout::SetFixedSize);
    widget->setLayout(layout);

    QListWidgetItem* item = new QListWidgetItem(photo_list_widget);
    item->setSizeHint(widget->sizeHint());
    photo_list_widget->addItem(item);
    photo_list_widget->setItemWidget(item, widget);
}

void MainWindow::updateCameraWidgetSize(const CameraInfo::Ptr& camera_info)
{
    camera_orientation_widget->setPhoto(camera_info);
    int photo_width = camera_info->width();
    int photo_height = camera_info->height();
    double photo_aspect = static_cast<double>(photo_width) / photo_height;
    double available_aspect = static_cast<double>(camera_available_width) / camera_available_height;
    if (available_aspect > photo_aspect)
    {
        camera_orientation_window->setFixedSize(QSize(static_cast<int>(camera_available_height * photo_aspect), camera_available_height));
    }
    else
    {
        camera_orientation_window->setFixedSize(QSize(camera_available_width, static_cast<int>(camera_available_width / photo_aspect)));
    }
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->update();
}

void MainWindow::onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (photo_list_widget->selectedItems().size() > 0)
    {
        int index = photo_list_widget->currentIndex().row();
        if (index >= 0 && static_cast<size_t>(index) < mesh_project->build_info->cameras_info.size())
        {
            remove_photo_act->setEnabled(true);
            photo_list_widget->itemWidget(photo_list_widget->item(index))->layout()->itemAt(1)->widget()->setEnabled(true);
            photo_list_widget->itemWidget(photo_list_widget->item(index))->layout()->itemAt(2)->widget()->setEnabled(true);
            int prev_index = -1;
            if (deselected.size() > 0 && deselected.front().indexes().size() > 0)
            {
                prev_index = deselected.front().indexes().front().row();
            }
            if (prev_index >= 0)
            {
                photo_list_widget->itemWidget(photo_list_widget->item(prev_index))->layout()->itemAt(1)->widget()->setEnabled(false);
                photo_list_widget->itemWidget(photo_list_widget->item(prev_index))->layout()->itemAt(2)->widget()->setEnabled(false);
            }
            auto camera_info = mesh_project->build_info->cameras_info[index];
            updateCameraWidgetSize(camera_info);
        }
    }
    else
    {
        camera_orientation_widget->setPhoto(nullptr);
        remove_photo_act->setEnabled(false);
        camera_orientation_window->setFixedSize(QSize(camera_available_width, camera_available_height));
    }
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->update();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (mesh_project->dirty)
    {
        QMessageBox msg_box;
        msg_box.setText("The project has been modified.");
        msg_box.setWindowTitle("Gkm-World Mesh-Builder");
        msg_box.setInformativeText("Do you want to save your changes?");
        msg_box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msg_box.setDefaultButton(QMessageBox::Save);
        int ret = msg_box.exec();
        switch (ret)
        {
        case QMessageBox::Save:
            onSaveProject();
        case QMessageBox::Discard:
            event->accept();
            break;
        case QMessageBox::Cancel:
            event->ignore();
        default:
            break;
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    int new_camera_available_width = main_window.centralwidget->width() * 2 / 3;
    int new_camera_available_height = main_window.centralwidget->height();
    if (new_camera_available_width > initial_camera_available_width && new_camera_available_height > initial_camera_available_height)
    {
        camera_available_width = new_camera_available_width;
        camera_available_height = new_camera_available_height;
    }
    else
    {
        camera_available_width = initial_camera_available_width;
        camera_available_height = initial_camera_available_height;
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::onNewProject()
{
    mesh_project = std::make_shared<MeshProject>();

    auto new_box = std::make_shared<AuxGeometryBox>();
    new_box->size = QVector3D(1, 1, 1);
    mesh_project->aux_geometry->boxes.push_back(new_box);

    fillPhotoListWidget();
    updateProject();
}

void MainWindow::onOpenProject()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select Mesh-Builder project", QDir::currentPath(), "Mesh-Builder projects (*.gmb)");
    if (!filename.isNull() && fileExists(filename.toStdString()))
    {
        loadProject(filename.toStdString().c_str());
    }
}

void MainWindow::onSaveProject()
{
    if (mesh_project->file_name.empty())
    {
        onSaveAsProject();
        return;
    }

    if (mesh_project->dirty)
    {
        saveMeshProject(mesh_project, mesh_project->file_name);
        mesh_project->dirty = false;
        updateWindowTitle();
    }
}

void MainWindow::onSaveAsProject()
{
    QString filename = QFileDialog::getSaveFileName(this, "Select Mesh-Builder project", QDir::currentPath(), "Mesh-Builder projects (*.gmb)");
    if (!filename.isNull())
    {
        saveMeshProject(mesh_project, filename.toStdString());
        mesh_project->file_name = filename.toStdString();
        mesh_project->dirty = false;
        updateWindowTitle();
    }
}

void MainWindow::onQuit()
{
    QMainWindow::close();
}

void MainWindow::onAddPhoto()
{
    QString filename = QFileDialog::getOpenFileName(this, "Select photo", QDir::currentPath(), "JPEG files (*.jpg *.jpeg);; PNG files (*.png)");
    if (!filename.isNull() && fileExists(filename.toStdString()))
    {
        addPhoto(filename.toStdString().c_str());
        loadPhotos();
        mesh_project->dirty = true;
        updateProject();
    }
}

void MainWindow::onRemovePhoto()
{
    const int index = photo_list_widget->currentRow();
    QString format_string("Are you sure to remove photo %1?");
    QString photo_name(mesh_project->build_info->cameras_info.at(index)->photo_image_path.c_str());

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation", format_string.arg(photo_name), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        auto selected_items = photo_list_widget->selectedItems();
        if (index > 0)
        {
            photo_list_widget->setCurrentRow(index - 1);
        }
        else if (photo_list_widget->count() > 1)
        {
            photo_list_widget->setCurrentRow(1);
        }
        else
        {
            photo_list_widget->clearSelection();
            mesh_project->file_name = "";
            mesh_project->dirty = false;
        }
        mesh_project->build_info->cameras_info.erase(mesh_project->build_info->cameras_info.begin() + index);
        qDeleteAll(selected_items);
        updateProject();
    }
}

void MainWindow::onLockedChanged(int state)
{
    int index = photo_list_widget->currentRow();
    if (index >= 0 && index < mesh_project->build_info->cameras_info.size())
    {
        mesh_project->build_info->cameras_info[index]->locked = state == Qt::Checked;
        mesh_project->dirty = true;
        updateWindowTitle();
    }
}

void MainWindow::onRotationChanged(int rotation_index)
{
    int index = photo_list_widget->currentRow();
    if (index >= 0 && index < mesh_project->build_info->cameras_info.size())
    {
        int new_rotation = 0;
        switch (rotation_index)
        {
        default:
        case 0:
            new_rotation = 0;
            break;
        case 1:
            new_rotation = 90;
            break;
        case 2:
            new_rotation = 180;
            break;
        case 3:
            new_rotation = 270;
            break;
        }
        auto camera_info = mesh_project->build_info->cameras_info[index];
        camera_info->rotation = new_rotation;
        mesh_project->dirty = true;
        updateWindowTitle();
        updateCameraWidgetSize(camera_info);
    }
}

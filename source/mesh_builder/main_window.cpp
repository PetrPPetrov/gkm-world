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
#include <QDoubleSpinBox>
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

    QMenu* aux_geom_menu = menuBar()->addMenu("Aux Geom");

    add_aux_box_act = new QAction("Add aux box", this);
    connect(add_aux_box_act, &QAction::triggered, this, &MainWindow::onAddAuxBox);
    aux_geom_menu->addAction(add_aux_box_act);

    remove_aux_geom_act = new QAction("Remove aux geometry", this);
    remove_aux_geom_act->setEnabled(false);
    connect(remove_aux_geom_act, &QAction::triggered, this, &MainWindow::onRemoveAuxGeom);
    aux_geom_menu->addAction(remove_aux_geom_act);

    QMenu* vertex_menu = menuBar()->addMenu("Vertex");

    add_vertex_act = new QAction("Add vertex", this);
    connect(add_vertex_act, &QAction::triggered, this, &MainWindow::onAddVertex);
    vertex_menu->addAction(add_vertex_act);

    remove_vertex_act = new QAction("Remove vertex", this);
    remove_vertex_act->setEnabled(false);
    connect(remove_vertex_act, &QAction::triggered, this, &MainWindow::onRemoveVertex);
    vertex_menu->addAction(remove_vertex_act);

    remove_vertex_position_on_photo_act = new QAction("Remove vertex position", this);
    remove_vertex_position_on_photo_act->setEnabled(false);
    connect(remove_vertex_position_on_photo_act, &QAction::triggered, this, &MainWindow::onRemoveVertexPosition);
    vertex_menu->addAction(remove_vertex_position_on_photo_act);

    photo_list_widget = new QListWidget(main_window.centralwidget);
    connect(photo_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onPhotoSelected);
    photo_list_window = main_window.centralwidget->addSubWindow(photo_list_widget);
    photo_list_window->setWindowTitle("Photos List");

    aux_geometry_list_widget = new QListWidget(main_window.centralwidget);
    connect(aux_geometry_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onAuxGeometrySelected);
    aux_geometry_list_window = main_window.centralwidget->addSubWindow(aux_geometry_list_widget);
    aux_geometry_list_window->setWindowTitle("Auxiliary Geometry List");

    vertex_list_widget = new QListWidget(main_window.centralwidget);
    connect(vertex_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onVertexSelected);
    vertex_list_window = main_window.centralwidget->addSubWindow(vertex_list_widget);
    vertex_list_window->setWindowTitle("Vertex List");

    vertex_position_on_photo_list_widget = new QListWidget(main_window.centralwidget);
    connect(vertex_position_on_photo_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onVertexPositionSelected);
    vertex_position_on_photo_list_window = main_window.centralwidget->addSubWindow(vertex_position_on_photo_list_widget);
    vertex_position_on_photo_list_window->setWindowTitle("Vertex Position on Photo List");

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

void MainWindow::setVertexPosition(QPointF position)
{
    const int camera_index = photo_list_widget->currentRow();
    const int vertex_index = vertex_list_widget->currentRow();
    if (camera_index >= 0 && vertex_index >= 0)
    {
        auto fit = vertex_list_item_to_vertex_id.find(vertex_list_widget->item(vertex_index));
        if (fit != vertex_list_item_to_vertex_id.end())
        {
            int photo_width = camera_info->width();
            int photo_height = camera_info->height();

            bool found = false;
            for (auto& vertex_position : mesh_project->build_info->vertices[fit->second]->positions)
            {
                if (vertex_position.camera_index == camera_index)
                {
                    vertex_position.x = position.x() * camera_scale_x;
                    vertex_position.y = photo_height - position.y() * camera_scale_y;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                VertexPositionInfo new_vertex_position_info;
                new_vertex_position_info.camera_index = camera_index;
                new_vertex_position_info.x = position.x() * camera_scale_x;
                new_vertex_position_info.y = photo_height - position.y() * camera_scale_y;
                mesh_project->build_info->vertices[fit->second]->positions.push_back(new_vertex_position_info);
            }
            fillVertexPositionInfoWidget();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
            dirtyProject();
        }
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;

        initial_camera_available_width = camera_available_width = main_window.centralwidget->width() * 2 / 3 - main_window.centralwidget->width() / 5;
        initial_camera_available_height = camera_available_height = main_window.centralwidget->height() * 9 / 10;

        camera_orientation_window->resize(QSize(camera_available_width, camera_available_height));
        camera_orientation_window->move(main_window.centralwidget->width() / 3, 0);

        photo_list_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 2));
        photo_list_window->move(0, 0);

        aux_geometry_list_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 4));
        aux_geometry_list_window->move(0, main_window.centralwidget->height() / 2);

        vertex_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        vertex_list_window->move(main_window.centralwidget->width() * 4 / 5, 0);

        vertex_position_on_photo_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        vertex_position_on_photo_list_window->move(main_window.centralwidget->width() * 4 / 5, main_window.centralwidget->height() / 2);

        log_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 4));
        log_window->move(0, main_window.centralwidget->height() * 3 / 4);

        onNewProject();
    }
}

void MainWindow::dirtyProject()
{
    mesh_project->dirty = true;
    updateWindowTitle();
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

void MainWindow::addAuxBox()
{
    std::unordered_set<int> used_id;
    for (auto aux_box : mesh_project->aux_geometry->boxes)
    {
        used_id.emplace(aux_box->id);
    }
    int new_id = 0;
    while (used_id.find(new_id) != used_id.end())
    {
        ++new_id;
    }
    AuxGeometryBox::Ptr new_box = std::make_shared<AuxGeometryBox>();
    new_box->id = new_id;
    new_box->size = QVector3D(1, 1, 1);
    mesh_project->aux_geometry->boxes.push_back(new_box);
    addAuxGeometryListWidgetItem(new_box);
    dirtyProject();
}

void MainWindow::loadProject(const char* filename)
{
    loadMeshProject(mesh_project, filename);
    mesh_project->file_name = filename;
    fillPhotoListWidget();
    fillAuxGeometryListWidget();
    fillVertexListWidget();
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
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->updateLineSetGeometry();
    updateWindowTitle();
    camera_orientation_widget->update();
}

void MainWindow::updateCameraWidgetSize()
{
    camera_orientation_widget->setPhoto(camera_info);
    int photo_width = camera_info->width();
    int photo_height = camera_info->height();
    double photo_aspect = static_cast<double>(photo_width) / photo_height;
    double available_aspect = static_cast<double>(camera_available_width) / camera_available_height;
    if (available_aspect > photo_aspect)
    {
        camera_orientation_widget->setFixedSize(QSize(static_cast<int>(camera_available_height * photo_aspect), camera_available_height));
    }
    else
    {
        camera_orientation_widget->setFixedSize(QSize(camera_available_width, static_cast<int>(camera_available_width / photo_aspect)));
    }
    camera_orientation_window->adjustSize();
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->update();

    camera_scale_x = static_cast<double>(photo_width) / camera_orientation_widget->width();
    camera_scale_y = static_cast<double>(photo_height) / camera_orientation_widget->height();
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
    connect(locked, &QCheckBox::stateChanged, this, &MainWindow::onLockedChanged);

    QLabel* rotation_label = new QLabel("Rotation", photo_list_widget);
    QComboBox* rotation = new QComboBox(photo_list_widget);
    rotation->addItem("0 degree");
    rotation->addItem("90 degree");
    rotation->addItem("180 degree");
    rotation->addItem("270 degree");
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

    QLabel* fov_label = new QLabel("FOV", photo_list_widget);
    QDoubleSpinBox* fov = new QDoubleSpinBox(photo_list_widget);
    fov->setRange(10, 120);
    fov->setValue(camera_info->fov);
    connect(fov, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onFovChanged);

    QGridLayout* layout = new QGridLayout(photo_list_widget);
    layout->addWidget(label, 0, 0);
    layout->addWidget(locked, 0, 1);
    layout->addWidget(rotation_label, 1, 0);
    layout->addWidget(rotation, 1, 1);
    layout->addWidget(fov_label, 1, 2);
    layout->addWidget(fov, 1, 3);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    widget->setLayout(layout);
    widget->setEnabled(false);

    QListWidgetItem* item = new QListWidgetItem(photo_list_widget);
    item->setSizeHint(widget->sizeHint());
    photo_list_widget->addItem(item);
    photo_list_widget->setItemWidget(item, widget);
}

void MainWindow::fillAuxGeometryListWidget()
{
    aux_geometry_list_widget->clear();
    aux_geom_list_item_to_box.clear();
    for (auto aux_box : mesh_project->aux_geometry->boxes)
    {
        addAuxGeometryListWidgetItem(aux_box);
    }
}

void MainWindow::addAuxGeometryListWidgetItem(const AuxGeometryBox::Ptr& aux_box)
{
    QWidget* widget = new QWidget(aux_geometry_list_widget);
    QLabel* label = new QLabel(QString("Box #%1").arg(aux_box->id), aux_geometry_list_widget);

    QLabel* posx_label = new QLabel("Pos.X", aux_geometry_list_widget);
    QDoubleSpinBox* posx = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posx, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosXChanged);
    posx->setRange(-1000, 1000);
    posx->setSingleStep(0.01);
    posx->setValue(aux_box->position.x());

    QLabel* posy_label = new QLabel("Pos.Y", aux_geometry_list_widget);
    QDoubleSpinBox* posy = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posy, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosYChanged);
    posy->setRange(-1000, 1000);
    posy->setSingleStep(0.01);
    posy->setValue(aux_box->position.y());

    QLabel* posz_label = new QLabel("Pos.Z", aux_geometry_list_widget);
    QDoubleSpinBox* posz = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posz, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosZChanged);
    posz->setRange(-1000, 1000);
    posz->setSingleStep(0.01);
    posz->setValue(aux_box->position.z());

    QLabel* sizex_label = new QLabel("Size.X", aux_geometry_list_widget);
    QDoubleSpinBox* sizex = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizex, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeXChanged);
    sizex->setRange(-1000, 1000);
    sizex->setSingleStep(0.01);
    sizex->setValue(aux_box->size.x());

    QLabel* sizey_label = new QLabel("Size.Y", aux_geometry_list_widget);
    QDoubleSpinBox* sizey = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizey, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeYChanged);
    sizey->setRange(-1000, 1000);
    sizey->setSingleStep(0.01);
    sizey->setValue(aux_box->size.y());

    QLabel* sizez_label = new QLabel("Size.Z", aux_geometry_list_widget);
    QDoubleSpinBox* sizez = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizez, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeZChanged);
    sizez->setRange(-1000, 1000);
    sizez->setSingleStep(0.01);
    sizez->setValue(aux_box->size.z());

    QGridLayout* layout = new QGridLayout(aux_geometry_list_widget);
    layout->addWidget(label, 0, 0);
    layout->addWidget(posx_label, 1, 0);
    layout->addWidget(posx, 1, 1);
    layout->addWidget(posy_label, 1, 2);
    layout->addWidget(posy, 1, 3);
    layout->addWidget(posz_label, 1, 4);
    layout->addWidget(posz, 1, 5);
    layout->addWidget(sizex_label, 2, 0);
    layout->addWidget(sizex, 2, 1);
    layout->addWidget(sizey_label, 2, 2);
    layout->addWidget(sizey, 2, 3);
    layout->addWidget(sizez_label, 2, 4);
    layout->addWidget(sizez, 2, 5);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    widget->setLayout(layout);
    widget->setEnabled(false);

    QListWidgetItem* item = new QListWidgetItem(aux_geometry_list_widget);
    item->setSizeHint(widget->sizeHint());
    aux_geometry_list_widget->addItem(item);
    aux_geometry_list_widget->setItemWidget(item, widget);

    aux_geom_list_item_to_box.emplace(item, aux_box);
}

void MainWindow::fillVertexListWidget()
{
    vertex_list_widget->clear();
    vertex_list_item_to_vertex_id.clear();
    for (auto vertex : mesh_project->build_info->vertices)
    {
        addVertexListWidgetItem(vertex);
    }
}

void MainWindow::addVertexListWidgetItem(const Vertex::Ptr& vertex)
{
    if (vertex->id != -1)
    {
        QLabel* label = new QLabel(QString("Vertex #%1").arg(vertex->id), vertex_list_widget);
        QListWidgetItem* item = new QListWidgetItem(vertex_list_widget);
        vertex_list_widget->addItem(item);
        vertex_list_widget->setItemWidget(item, label);
        vertex_list_item_to_vertex_id.emplace(item, vertex->id);
    }
}

void MainWindow::fillVertexPositionInfoWidget()
{
    vertex_position_on_photo_list_widget->clear();
    const int vertex_index = vertex_list_widget->currentRow();
    if (vertex_index >= 0)
    {
        auto fit = vertex_list_item_to_vertex_id.find(vertex_list_widget->item(vertex_index));
        if (fit != vertex_list_item_to_vertex_id.end())
        {
            for (auto& vertex_position : mesh_project->build_info->vertices[fit->second]->positions)
            {
                addVertexPositionInfoWidgetItem(fit->second, vertex_position);
            }
        }
    }
}

void MainWindow::addVertexPositionInfoWidgetItem(unsigned vertex_id, const VertexPositionInfo& vertex_position)
{
    QWidget* widget = new QWidget(vertex_position_on_photo_list_widget);
    QLabel* label0 = new QLabel(QString("Vertex #%1").arg(vertex_id), vertex_position_on_photo_list_widget);
    QLabel* label1 = new QLabel(QString("Camera #%1").arg(vertex_position.camera_index), vertex_position_on_photo_list_widget);
    QLabel* label2 = new QLabel(QString("X %1").arg(vertex_position.x), vertex_position_on_photo_list_widget);
    QLabel* label3 = new QLabel(QString("Y %1").arg(vertex_position.y), vertex_position_on_photo_list_widget);

    QGridLayout* layout = new QGridLayout(vertex_position_on_photo_list_widget);
    layout->addWidget(label0, 0, 0);
    layout->addWidget(label1, 0, 1);
    layout->addWidget(label2, 1, 0);
    layout->addWidget(label3, 1, 1);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    widget->setLayout(layout);

    QListWidgetItem* item = new QListWidgetItem(vertex_position_on_photo_list_widget);
    item->setSizeHint(widget->sizeHint());
    vertex_position_on_photo_list_widget->addItem(item);
    vertex_position_on_photo_list_widget->setItemWidget(item, widget);
}

void MainWindow::onPhotoSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (photo_list_widget->selectedItems().size() > 0)
    {
        int index = photo_list_widget->currentIndex().row();
        if (index >= 0 && static_cast<size_t>(index) < mesh_project->build_info->cameras_info.size())
        {
            remove_photo_act->setEnabled(true);
            photo_list_widget->itemWidget(photo_list_widget->item(index))->setEnabled(true);
            int prev_index = -1;
            if (deselected.size() > 0 && deselected.front().indexes().size() > 0)
            {
                prev_index = deselected.front().indexes().front().row();
            }
            if (prev_index >= 0)
            {
                photo_list_widget->itemWidget(photo_list_widget->item(prev_index))->setEnabled(false);
            }
            camera_info = mesh_project->build_info->cameras_info[index];
            updateCameraWidgetSize();
        }
    }
    else
    {
        camera_info = nullptr;
        camera_orientation_widget->setPhoto(nullptr);
        remove_photo_act->setEnabled(false);
        camera_orientation_window->setFixedSize(QSize(camera_available_width, camera_available_height));
    }
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxGeometrySelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (aux_geometry_list_widget->selectedItems().size() > 0)
    {
        remove_aux_geom_act->setEnabled(true);
        int index = aux_geometry_list_widget->currentIndex().row();
        aux_geometry_list_widget->itemWidget(aux_geometry_list_widget->item(index))->setEnabled(true);
        int prev_index = -1;
        if (deselected.size() > 0 && deselected.front().indexes().size() > 0)
        {
            prev_index = deselected.front().indexes().front().row();
        }
        if (prev_index >= 0)
        {
            aux_geometry_list_widget->itemWidget(aux_geometry_list_widget->item(prev_index))->setEnabled(false);
        }
    }
    else
    {
        remove_aux_geom_act->setEnabled(false);
    }
}

void MainWindow::onVertexSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (vertex_list_widget->selectedItems().size() > 0)
    {
        remove_vertex_act->setEnabled(true);
        fillVertexPositionInfoWidget();
    }
    else
    {
        remove_vertex_act->setEnabled(false);
    }
}

void MainWindow::onVertexPositionSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (vertex_position_on_photo_list_widget->selectedItems().size() > 0)
    {
        remove_vertex_position_on_photo_act->setEnabled(true);
    }
    else
    {
        remove_vertex_position_on_photo_act->setEnabled(false);
    }
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
    int new_camera_available_width = main_window.centralwidget->width() * 2 / 3 - main_window.centralwidget->width() / 5;
    int new_camera_available_height = main_window.centralwidget->height() * 9 / 10;
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

    addAuxBox();
    mesh_project->dirty = false;
    fillPhotoListWidget();
    fillAuxGeometryListWidget();
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
        dirtyProject();
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
        }
        mesh_project->build_info->cameras_info.erase(mesh_project->build_info->cameras_info.begin() + index);
        for (auto vertex : mesh_project->build_info->vertices)
        {
            auto vertex_position_it = vertex->positions.begin();
            while (vertex_position_it != vertex->positions.end())
            {
                if (vertex_position_it->camera_index == index)
                {
                    vertex->positions.erase(vertex_position_it);
                    vertex_position_it = vertex->positions.begin();
                }
                else
                {
                    ++vertex_position_it;
                }
            }
        }
        qDeleteAll(selected_items);
        updateProject();
        fillVertexPositionInfoWidget();
    }
}

void MainWindow::onAddAuxBox()
{
    addAuxBox();
    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onRemoveAuxGeom()
{
    dirtyProject();
    const int index = aux_geometry_list_widget->currentRow();
    auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
    auto& boxes = mesh_project->aux_geometry->boxes;
    for (auto it = boxes.begin(); it != boxes.end(); ++it)
    {
        if (*it == fit->second)
        {
            mesh_project->aux_geometry->boxes.erase(it);
            break;
        }
    }
    aux_geom_list_item_to_box.erase(fit);

    auto selected_items = aux_geometry_list_widget->selectedItems();
    remove_aux_geom_act->setEnabled(false);
    aux_geometry_list_widget->clearSelection();
    qDeleteAll(selected_items);
    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAddVertex()
{
    dirtyProject();
    auto& vertices = mesh_project->build_info->vertices;
    const unsigned count = static_cast<unsigned>(vertices.size());
    bool added = false;
    for (unsigned i = 0; i < count; ++i)
    {
        if (vertices[i]->id == -1)
        {
            // Found free vertex
            vertices[i]->id = i;
            vertices[i]->positions.clear();
            added = true;
            break;
        }
    }
    if (!added)
    {
        auto new_vertex = std::make_shared<Vertex>();
        new_vertex->id = count;
        vertices.push_back(new_vertex);
    }

    fillVertexListWidget();
}

void MainWindow::onRemoveVertex()
{
    dirtyProject();
    const int index = vertex_list_widget->currentRow();
    auto fit = vertex_list_item_to_vertex_id.find(vertex_list_widget->item(index));
    if (fit != vertex_list_item_to_vertex_id.end())
    {
        mesh_project->build_info->vertices.at(fit->second)->id = -1;
        fillVertexListWidget();
        if (index < vertex_list_widget->count())
        {
            vertex_list_widget->setCurrentRow(index);
        }
        else if (vertex_list_widget->count() > 0)
        {
            vertex_list_widget->setCurrentRow(vertex_list_widget->count() - 1);
        }
        else
        {
            vertex_list_widget->clearSelection();
        }
    }
}

void MainWindow::onRemoveVertexPosition()
{
    dirtyProject();
    const int index = vertex_list_widget->currentRow();
    auto fit = vertex_list_item_to_vertex_id.find(vertex_list_widget->item(index));
    if (fit != vertex_list_item_to_vertex_id.end())
    {
        const int vertex_position_index = vertex_position_on_photo_list_widget->currentRow();
        if (vertex_position_index >= 0)
        {
            auto& positions = mesh_project->build_info->vertices.at(fit->second)->positions;
            positions.erase(positions.begin() + vertex_position_index);
            fillVertexPositionInfoWidget();
        }
    }
}

void MainWindow::onLockedChanged(int state)
{
    int index = photo_list_widget->currentRow();
    if (index >= 0 && index < mesh_project->build_info->cameras_info.size())
    {
        mesh_project->build_info->cameras_info[index]->locked = state == Qt::Checked;
        dirtyProject();
    }
}

void MainWindow::onRotationChanged(int rotation_index)
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
    camera_info->rotation = new_rotation;
    dirtyProject();
    updateCameraWidgetSize();
}

void MainWindow::onFovChanged(double value)
{
    camera_info->fov = value;
    dirtyProject();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxPosXChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->position.setX(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

void MainWindow::onAuxBoxPosYChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->position.setY(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

void MainWindow::onAuxBoxPosZChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->position.setZ(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

void MainWindow::onAuxBoxSizeXChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->size.setX(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

void MainWindow::onAuxBoxSizeYChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->size.setY(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

void MainWindow::onAuxBoxSizeZChanged(double value)
{
    int index = aux_geometry_list_widget->currentRow();
    if (index >= 0)
    {
        auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(index));
        if (fit != aux_geom_list_item_to_box.end())
        {
            fit->second->size.setZ(value);
            dirtyProject();
            camera_orientation_widget->updateLineSetGeometry();
            camera_orientation_widget->update();
        }
    }
}

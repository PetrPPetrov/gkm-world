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
#include "build_mesh.h"
#include "color_hasher.h"

extern MainWindow* g_main_window = nullptr;

MainWindow::MainWindow()
{
    g_main_window = this;
    main_window.setupUi(this);

    initializeMenu();
    initializeWidgets();
}

void MainWindow::initializeMenu()
{
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

    vertex_menu->addSeparator();
    add_current_vertex_act = new QAction("Add current vertex", this);
    add_current_vertex_act->setEnabled(false);
    connect(add_current_vertex_act, &QAction::triggered, this, &MainWindow::onAddCurrentVertex);
    vertex_menu->addAction(add_current_vertex_act);

    remove_current_vertex_act = new QAction("Remove current vertex", this);
    remove_current_vertex_act->setEnabled(false);
    connect(remove_current_vertex_act, &QAction::triggered, this, &MainWindow::onRemoveCurrentVertex);
    vertex_menu->addAction(remove_current_vertex_act);

    QMenu* triangle_menu = menuBar()->addMenu("Triangle");

    add_triangle_act = new QAction("Add triangle", this);
    connect(add_triangle_act, &QAction::triggered, this, &MainWindow::onAddTriangle);
    triangle_menu->addAction(add_triangle_act);

    remove_triangle_act = new QAction("Remove triangle", this);
    remove_triangle_act->setEnabled(false);
    connect(remove_triangle_act, &QAction::triggered, this, &MainWindow::onRemoveTriangle);
    triangle_menu->addAction(remove_triangle_act);

    triangle_menu->addSeparator();
    use_selected_vertex_in_triangle_act = new QAction("Use vertex", this);
    use_selected_vertex_in_triangle_act->setEnabled(false);
    connect(use_selected_vertex_in_triangle_act, &QAction::triggered, this, &MainWindow::onUseVertex);
    triangle_menu->addAction(use_selected_vertex_in_triangle_act);

    QMenu* build_menu = menuBar()->addMenu("Build");

    build_mesh_act = new QAction("Build Mesh", this);
    build_mesh_act->setEnabled(false);
    connect(build_mesh_act, &QAction::triggered, this, &MainWindow::onBuildMesh);
    build_menu->addAction(build_mesh_act);

    build_menu->addSeparator();
    set_output_file_act = new QAction("Set Output File", this);
    connect(set_output_file_act, &QAction::triggered, this, &MainWindow::onSetOutputFile);
    build_menu->addAction(set_output_file_act);
}

void MainWindow::initializeWidgets()
{
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

    current_vertex_list_widget = new QListWidget(main_window.centralwidget);
    connect(current_vertex_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onCurrentVertexSelected);
    current_vertex_list_window = main_window.centralwidget->addSubWindow(current_vertex_list_widget);
    current_vertex_list_window->setWindowTitle("Current Vertex List");

    triangle_list_widget = new QListWidget(main_window.centralwidget);
    connect(triangle_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onTriangleSelected);
    triangle_list_window = main_window.centralwidget->addSubWindow(triangle_list_widget);
    triangle_list_window->setWindowTitle("Triangle List");

    current_triangle_list_widget = new QListWidget(main_window.centralwidget);
    connect(current_triangle_list_widget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::onCurrentTriangleSelected);
    current_triangle_list_window = main_window.centralwidget->addSubWindow(current_triangle_list_widget);
    current_triangle_list_window->setWindowTitle("Current Triangle");

    log_widget = new QPlainTextEdit(main_window.centralwidget);
    log_widget->setReadOnly(true);
    log_widget->ensureCursorVisible();
    log_widget->setCenterOnScroll(true);
    log_window = main_window.centralwidget->addSubWindow(log_widget);
    log_window->setWindowTitle("Log");

    camera_orientation_widget = new MeshBuilderWidget(main_window.centralwidget);
    camera_orientation_window = main_window.centralwidget->addSubWindow(camera_orientation_widget);
    camera_orientation_window->setWindowTitle("3D Camera for Photo");
    camera_orientation_widget->setMinimumWidth(camera_minimum_width);
    camera_orientation_widget->setMinimumHeight(camera_minimum_height);
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
    //const int camera_index = photo_list_widget->currentRow();
    //if (camera_index >= 0)
    //{
    //    auto selected_vertex = getVertex(vertex_list_widget->currentRow());
    //    if (selected_vertex)
    //    {
    //        int photo_width = camera_info->width();
    //        int photo_height = camera_info->height();

    //        bool found = false;
    //        for (auto& vertex_position : selected_vertex->positions)
    //        {
    //            if (vertex_position.camera_index == camera_index)
    //            {
    //                vertex_position.x = position.x() * camera_scale_x;
    //                vertex_position.y = photo_height - position.y() * camera_scale_y;
    //                found = true;
    //                break;
    //            }
    //        }
    //        if (!found)
    //        {
    //            if (selected_vertex->positions.size() < 2)
    //            {
    //                VertexPositionInfo new_vertex_position_info;
    //                new_vertex_position_info.camera_index = camera_index;
    //                new_vertex_position_info.x = position.x() * camera_scale_x;
    //                new_vertex_position_info.y = photo_height - position.y() * camera_scale_y;
    //                selected_vertex->positions.push_back(new_vertex_position_info);
    //            }
    //            else
    //            {
    //                log_widget->appendPlainText(QString("Vertex #%1 has 2 binding positions").arg(selected_vertex->id));
    //            }
    //        }
    //        fillVertexPositionInfoWidget();
    //        camera_orientation_widget->updateLineSetGeometry();
    //        camera_orientation_widget->update();
    //        dirtyProject();
    //    }
    //}
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;

        updateCameraWidgetAvailableSize();

        camera_orientation_window->resize(QSize(camera_available_width, camera_available_height));
        camera_orientation_window->move(main_window.centralwidget->width() / 3, 0);

        photo_list_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 2));
        photo_list_window->move(0, 0);

        aux_geometry_list_window->resize(QSize(main_window.centralwidget->width() / 3, main_window.centralwidget->height() / 4));
        aux_geometry_list_window->move(0, main_window.centralwidget->height() / 2);

        vertex_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        vertex_list_window->move(main_window.centralwidget->width() * 4 / 5, 0);

        current_vertex_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        current_vertex_list_window->move(main_window.centralwidget->width() * 4 / 5, main_window.centralwidget->height() / 2);

        triangle_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        triangle_list_window->move(main_window.centralwidget->width() * 3 / 5, 0);

        current_triangle_list_window->resize(QSize(main_window.centralwidget->width() / 5, main_window.centralwidget->height() / 2));
        current_triangle_list_window->move(main_window.centralwidget->width() * 3 / 5, main_window.centralwidget->height() / 2);

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

void MainWindow::loadProject(const char* filename)
{
    mesh_project = loadMeshProject(filename);
    mesh_project->file_name = filename;
    initCurrentVariables();

    updateProject();

    if (mesh_project->cameras.size() > 0)
    {
        photo_list_widget->setCurrentRow(0);
    }
    else
    {
        photo_list_widget->clearSelection();
    }
}

void MainWindow::loadPhotos()
{
    for (auto camera_info : mesh_project->cameras)
    {
        if (!camera_info->photo_image)
        {
            camera_info->photo_image = std::make_shared<QImage>(QString(camera_info->photo_image_path.c_str()));
        }
    }
}

void MainWindow::updateProject()
{
    fillPhotoListWidget();
    fillAuxGeometryListWidget();
    fillVertexListWidget();
    fillCurrentVertexListWidget();
    fillTriangleListWidget();
    fillCurrentTriangleListWidget();

    loadPhotos();
    updateCameraWidget();
    updateActionState();
    updateWindowTitle();
}

void MainWindow::updateActionState()
{
    remove_photo_act->setEnabled(current_camera ? true : false);
    remove_aux_geom_act->setEnabled(current_aux_box ? true : false);
    remove_vertex_act->setEnabled(current_vertex ? true : false);
    add_current_vertex_act->setEnabled(current_camera && current_vertex);
    remove_current_vertex_act->setEnabled(current_vertex_photo_position ? true : false);
    remove_triangle_act->setEnabled(current_triangle ? true : false);
    use_selected_vertex_in_triangle_act->setEnabled(current_vertex && current_triangle && current_triangle_item >= 0 && current_triangle_item <= 2);
    build_mesh_act->setEnabled(!mesh_project->output_file_name.empty());
}

void MainWindow::updateCameraWidget()
{
    camera_orientation_widget->setMeshProject(mesh_project);
    camera_orientation_widget->setPhoto(current_camera);
    camera_orientation_widget->updatePhotoTexture();
    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::updateCameraWidgetSize()
{
    if (current_camera)
    {
        camera_orientation_widget->setPhoto(current_camera);
        const int photo_width = cameraGetWidth(current_camera);
        const int photo_height = cameraGetHeight(current_camera);
        double photo_aspect = static_cast<double>(photo_width) / photo_height;
        double available_aspect = static_cast<double>(camera_available_width) / camera_available_height;
        if (available_aspect > photo_aspect)
        {
            camera_orientation_window->resize(QSize(static_cast<int>(camera_available_height * photo_aspect), camera_available_height));
        }
        else
        {
            camera_orientation_window->resize(QSize(camera_available_width, static_cast<int>(camera_available_width / photo_aspect)));
        }
        camera_orientation_widget->update();

        camera_scale_x = static_cast<double>(photo_width) / camera_orientation_widget->width();
        camera_scale_y = static_cast<double>(photo_height) / camera_orientation_widget->height();
    }
    else
    {
        camera_orientation_window->resize(QSize(camera_available_width, camera_available_height));
        camera_scale_x = 1.0;
        camera_scale_y = 1.0;
    }
}

void MainWindow::updateCameraWidgetAvailableSize()
{
    camera_available_width = main_window.centralwidget->width() * 2 / 3 - main_window.centralwidget->width() / 5;
    camera_available_height = main_window.centralwidget->height() * 9 / 10;
}

void MainWindow::fillPhotoListWidget()
{
    photo_list_widget->clear();
    photo_list_item_to_camera.clear();
    for (auto camera_info : mesh_project->cameras)
    {
        addPhotoListWidgetItem(camera_info);
    }
}

void MainWindow::addPhotoListWidgetItem(const Camera::Ptr& camera)
{
    QWidget* widget = new QWidget(photo_list_widget);
    QLabel* label = new QLabel(QString(camera->photo_image_path.c_str()), photo_list_widget);

    QCheckBox* locked = new QCheckBox("locked", photo_list_widget);
    locked->setChecked(camera->locked);
    connect(locked, &QCheckBox::stateChanged, this, &MainWindow::onLockedChanged);

    QLabel* rotation_label = new QLabel("Rotation", photo_list_widget);
    QComboBox* rotation = new QComboBox(photo_list_widget);
    rotation->addItem("0 degree");
    rotation->addItem("90 degree");
    rotation->addItem("180 degree");
    rotation->addItem("270 degree");
    rotation->setCurrentIndex(cameraGetRotationIndex(camera));
    connect(rotation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onRotationChanged);

    QLabel* fov_label = new QLabel("FOV", photo_list_widget);
    QDoubleSpinBox* fov = new QDoubleSpinBox(photo_list_widget);
    fov->setRange(10, 120);
    fov->setValue(camera->fov);
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

    photo_list_item_to_camera.emplace(item, camera);
}

void MainWindow::fillAuxGeometryListWidget()
{
    aux_geometry_list_widget->clear();
    aux_geom_list_item_to_box.clear();
    for (auto aux_box : mesh_project->aux_geometry->boxes)
    {
        if (aux_box->id != -1)
        {
            addAuxGeometryListWidgetItem(aux_box);
        }
    }
}

void MainWindow::addAuxGeometryListWidgetItem(const AuxBox::Ptr& aux_box)
{
    QWidget* widget = new QWidget(aux_geometry_list_widget);
    QLabel* label = new QLabel(QString("Box #%1").arg(aux_box->id), aux_geometry_list_widget);

    QLabel* posx_label = new QLabel("Pos.X", aux_geometry_list_widget);
    QDoubleSpinBox* posx = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posx, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosXChanged);
    posx->setRange(-1000, 1000);
    posx->setSingleStep(0.01);
    posx->blockSignals(true);
    posx->setValue(aux_box->position.x());
    posx->blockSignals(false);

    QLabel* posy_label = new QLabel("Pos.Y", aux_geometry_list_widget);
    QDoubleSpinBox* posy = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posy, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosYChanged);
    posy->setRange(-1000, 1000);
    posy->setSingleStep(0.01);
    posy->blockSignals(true);
    posy->setValue(aux_box->position.y());
    posy->blockSignals(false);

    QLabel* posz_label = new QLabel("Pos.Z", aux_geometry_list_widget);
    QDoubleSpinBox* posz = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(posz, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxPosZChanged);
    posz->setRange(-1000, 1000);
    posz->setSingleStep(0.01);
    posz->blockSignals(true);
    posz->setValue(aux_box->position.z());
    posz->blockSignals(false);

    QLabel* sizex_label = new QLabel("Size.X", aux_geometry_list_widget);
    QDoubleSpinBox* sizex = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizex, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeXChanged);
    sizex->setRange(-1000, 1000);
    sizex->setSingleStep(0.01);
    sizex->blockSignals(true);
    sizex->setValue(aux_box->size.x());
    sizex->blockSignals(false);

    QLabel* sizey_label = new QLabel("Size.Y", aux_geometry_list_widget);
    QDoubleSpinBox* sizey = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizey, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeYChanged);
    sizey->setRange(-1000, 1000);
    sizey->setSingleStep(0.01);
    sizey->blockSignals(true);
    sizey->setValue(aux_box->size.y());
    sizey->blockSignals(false);

    QLabel* sizez_label = new QLabel("Size.Z", aux_geometry_list_widget);
    QDoubleSpinBox* sizez = new QDoubleSpinBox(aux_geometry_list_widget);
    connect(sizez, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::onAuxBoxSizeZChanged);
    sizez->setRange(-1000, 1000);
    sizez->setSingleStep(0.01);
    sizez->blockSignals(true);
    sizez->setValue(aux_box->size.z());
    sizez->blockSignals(false);

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
    vertex_list_item_to_vertex.clear();
    for (auto vertex : mesh_project->vertices)
    {
        if (vertex->id != -1)
        {
            addVertexListWidgetItem(vertex);
        }
    }
}

void MainWindow::addVertexListWidgetItem(const Vertex::Ptr& vertex)
{
    using namespace ColorHasher;

    QColor color(ColorHasher::getRed(vertex->id), ColorHasher::getGreen(vertex->id), ColorHasher::getBlue(vertex->id));
    QLabel* label = new QLabel(vertex_list_widget);
    label->setText(QString("Vertex #%1").arg(vertex->id) + QString(" [<font color=") + color.name() + QString(">+</font>]"));
    QListWidgetItem* item = new QListWidgetItem(vertex_list_widget);
    vertex_list_widget->addItem(item);
    vertex_list_widget->setItemWidget(item, label);
    vertex_list_item_to_vertex.emplace(item, vertex);
}

void MainWindow::fillCurrentVertexListWidget()
{
    current_vertex_list_widget->clear();
    current_vertex_list_item_to_vertex_position.clear();
    if (current_camera)
    {
        for (auto& vertex_position : current_camera->positions)
        {
            addCurrentVertexListWidgetItem(vertex_position);
        }
    }
}

void MainWindow::addCurrentVertexListWidgetItem(const VertexPhotoPosition::Ptr& vertex_position)
{
    QWidget* widget = new QWidget(current_vertex_list_widget);
    QLabel* label0 = new QLabel(QString("Vertex #%1").arg(vertex_position->vertex_id), current_vertex_list_widget);
    QLabel* label2 = new QLabel(QString("X %1").arg(vertex_position->x), current_vertex_list_widget);
    QLabel* label3 = new QLabel(QString("Y %1").arg(vertex_position->y), current_vertex_list_widget);

    QGridLayout* layout = new QGridLayout(current_vertex_list_widget);
    layout->addWidget(label0, 0, 0);
    layout->addWidget(label2, 0, 1);
    layout->addWidget(label3, 0, 2);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    widget->setLayout(layout);

    QListWidgetItem* item = new QListWidgetItem(current_vertex_list_widget);
    item->setSizeHint(widget->sizeHint());
    current_vertex_list_widget->addItem(item);
    current_vertex_list_widget->setItemWidget(item, widget);
    current_vertex_list_item_to_vertex_position.emplace(item, vertex_position);
}

void MainWindow::fillTriangleListWidget()
{
    triangle_list_widget->clear();
    triangle_list_item_to_triangle.clear();
    for (auto triangle : mesh_project->triangles)
    {
        if (triangle->id != -1)
        {
            addTriangleListWidgetItem(triangle);
        }
    }
}

void MainWindow::addTriangleListWidgetItem(const Triangle::Ptr& triangle)
{
    using namespace ColorHasher;

    QColor color(ColorHasher::getRed(triangle->id), ColorHasher::getGreen(triangle->id), ColorHasher::getBlue(triangle->id));
    QLabel* label = new QLabel(triangle_list_widget);
    label->setText(QString("Triangle #%1").arg(triangle->id) + QString(" [<font color=") + color.name() + QString(">=</font>]"));
    QListWidgetItem* item = new QListWidgetItem(triangle_list_widget);
    triangle_list_widget->addItem(item);
    triangle_list_widget->setItemWidget(item, label);
    triangle_list_item_to_triangle.emplace(item, triangle);
}

void MainWindow::fillCurrentTriangleListWidget()
{
    current_triangle_list_widget->clear();
    current_triangle_item = -1;
    if (current_triangle)
    {
        addCurrentTriangleListWidgetItem(current_triangle->vertices[0]);
        addCurrentTriangleListWidgetItem(current_triangle->vertices[1]);
        addCurrentTriangleListWidgetItem(current_triangle->vertices[2]);
    }
}

void MainWindow::addCurrentTriangleListWidgetItem(int vertex_id)
{
    using namespace ColorHasher;

    QColor color(ColorHasher::getRed(vertex_id), ColorHasher::getGreen(vertex_id), ColorHasher::getBlue(vertex_id));
    QLabel* label = new QLabel(current_triangle_list_widget);
    if (vertex_id == -1)
    {
        label->setText("Vertex N/A");
    }
    else
    {
        label->setText(QString("Vertex #%1").arg(vertex_id) + QString(" [<font color=") + color.name() + QString(">+</font>]"));
    }
    QListWidgetItem* item = new QListWidgetItem(current_triangle_list_widget);
    current_triangle_list_widget->addItem(item);
    current_triangle_list_widget->setItemWidget(item, label);
}

Camera::Ptr MainWindow::getCamera(int row_index) const
{
    auto fit = photo_list_item_to_camera.find(photo_list_widget->item(row_index));
    if (fit != photo_list_item_to_camera.end())
    {
        return fit->second;
    }
    return nullptr;
}

AuxBox::Ptr MainWindow::getAuxBox(int row_index) const
{
    auto fit = aux_geom_list_item_to_box.find(aux_geometry_list_widget->item(row_index));
    if (fit != aux_geom_list_item_to_box.end())
    {
        return fit->second;
    }
    return nullptr;
}

Vertex::Ptr MainWindow::getVertex(int row_index) const
{
    auto fit = vertex_list_item_to_vertex.find(vertex_list_widget->item(row_index));
    if (fit != vertex_list_item_to_vertex.end())
    {
        return fit->second;
    }
    return nullptr;
}

VertexPhotoPosition::Ptr MainWindow::getCurrentVertex(int row_index) const
{
    auto fit = current_vertex_list_item_to_vertex_position.find(current_vertex_list_widget->item(row_index));
    if (fit != current_vertex_list_item_to_vertex_position.end())
    {
        return fit->second;
    }
    return nullptr;
}

Triangle::Ptr MainWindow::getTriangle(int row_index) const
{
    auto fit = triangle_list_item_to_triangle.find(triangle_list_widget->item(row_index));
    if (fit != triangle_list_item_to_triangle.end())
    {
        return fit->second;
    }
    return nullptr;
}

void MainWindow::initCurrentVariables()
{
    current_camera = nullptr;
    current_aux_box = nullptr;
    current_vertex = nullptr;
    current_vertex_photo_position = nullptr;
    current_triangle = nullptr;
    current_triangle_item = -1;
}

bool MainWindow::closeProject()
{
    if (mesh_project && mesh_project->dirty)
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
            return true;
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
            return false;
        default:
            return false;
        }
    }
    return true;
}

void MainWindow::onNewProject()
{
    if (closeProject())
    {
        mesh_project = std::make_shared<MeshProject>();
        initCurrentVariables();
        projectAddAuxBox(mesh_project);
        updateProject();
        mesh_project->dirty = false;
        updateWindowTitle();
    }
}

void MainWindow::onOpenProject()
{
    if (closeProject())
    {
        QString filename = QFileDialog::getOpenFileName(this, "Select Mesh-Builder project", QDir::currentPath(), "Mesh-Builder projects (*.gmb)");
        if (!filename.isNull() && fileExists(filename.toStdString()))
        {
            loadProject(filename.toStdString().c_str());
        }
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
        dirtyProject();
        current_camera = projectAddPhoto(mesh_project, filename.toStdString().c_str());
        loadPhotos();
        addPhotoListWidgetItem(current_camera);
        photo_list_widget->setCurrentRow(photo_list_widget->count() - 1);

        updateActionState();
    }
}

void MainWindow::onRemovePhoto()
{
    const int index = photo_list_widget->currentRow();
    QString format_string("Are you sure to remove photo %1?");
    QString photo_name(current_camera->photo_image_path.c_str());

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirmation", format_string.arg(photo_name), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        dirtyProject();
        projectRemovePhoto(mesh_project, current_camera);
        auto selected_items = photo_list_widget->selectedItems();
        current_vertex_photo_position = nullptr;

        photoSaveSelection();
        qDeleteAll(selected_items);
        photoRestoreSelection();

        updateActionState();
    }
}

void MainWindow::onAddAuxBox()
{
    current_aux_box = projectAddAuxBox(mesh_project);
    dirtyProject();

    addAuxGeometryListWidgetItem(current_aux_box);
    aux_geometry_list_widget->setCurrentRow(aux_geometry_list_widget->count() - 1);

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    updateActionState();
}

void MainWindow::onRemoveAuxGeom()
{
    projectRemoveAuxBox(mesh_project, current_aux_box);
    auto selected_items = aux_geometry_list_widget->selectedItems();
    dirtyProject();

    auxGeometrySaveSelection();
    qDeleteAll(selected_items);
    auxGeometryRestoreSelection();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    updateActionState();
}

void MainWindow::onAddVertex()
{
    current_vertex = projectAddVertex(mesh_project);
    dirtyProject();

    addVertexListWidgetItem(current_vertex);
    vertex_list_widget->setCurrentRow(vertex_list_widget->count() - 1);

    updateActionState();
}

void MainWindow::onRemoveVertex()
{
    projectRemoveVertex(mesh_project, current_vertex);
    auto selected_items = vertex_list_widget->selectedItems();
    dirtyProject();

    vertexSaveSelection();
    qDeleteAll(selected_items);
    vertexRestoreSelection();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    updateActionState();
}

void MainWindow::onAddCurrentVertex()
{
    current_vertex_photo_position = projectAddCurrentVertex(mesh_project, current_camera, current_vertex);
    dirtyProject();

    addCurrentVertexListWidgetItem(current_vertex_photo_position);
    current_vertex_list_widget->setCurrentRow(current_vertex_list_widget->count() - 1);

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();

    updateActionState();
}

void MainWindow::onRemoveCurrentVertex()
{
    projectRemoveCurrentVertex(mesh_project, current_camera, current_vertex);
    auto selected_items = current_vertex_list_widget->selectedItems();
    dirtyProject();

    currentVertexSaveSelection();
    qDeleteAll(selected_items);
    currentVertexRestoreSelection();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    updateActionState();
}

void MainWindow::onAddTriangle()
{
    current_triangle = projectAddTriangle(mesh_project);
    dirtyProject();

    addTriangleListWidgetItem(current_triangle);
    triangle_list_widget->setCurrentRow(triangle_list_widget->count() - 1);

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();

    updateActionState();
}

void MainWindow::onRemoveTriangle()
{
    projectRemoveTriangle(mesh_project, current_triangle);
    auto selected_items = triangle_list_widget->selectedItems();
    dirtyProject();

    triangleSaveSelection();
    qDeleteAll(selected_items);
    triangleRestoreSelection();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    updateActionState();
}

void MainWindow::onUseVertex()
{
    projectUseVertex(mesh_project, current_triangle, current_triangle_item, current_vertex);
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
    currentTriangleSaveSelection();
    fillCurrentTriangleListWidget();
    currentTriangleRestoreSelection();
    updateActionState();
}

void MainWindow::onBuildMesh()
{
    if (mesh_project->output_file_name.empty())
    {
        onSetOutputFile();
    }

    if (mesh_project->output_file_name.empty())
    {
        log_widget->appendPlainText("Output file is empty, ignoring...");
        return;
    }

    buildMesh(mesh_project);
}

void MainWindow::onSetOutputFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "Select Mesh Output File", QDir::currentPath(), "OBJ files (*.obj)");
    if (!filename.isNull())
    {
        mesh_project->output_file_name = filename.toStdString();
        dirtyProject();
        updateActionState();
    }
}

void MainWindow::onLockedChanged(int state)
{
    current_camera->locked = state == Qt::Checked;
    dirtyProject();
}

void MainWindow::onRotationChanged(int rotation_index)
{
    cameraSetRotationFromIndex(current_camera, rotation_index);
    dirtyProject();

    updateCameraWidgetSize();
}

void MainWindow::onFovChanged(double value)
{
    current_camera->fov = value;
    dirtyProject();

    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxPosXChanged(double value)
{
    current_aux_box->position.x() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxPosYChanged(double value)
{
    current_aux_box->position.y() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxPosZChanged(double value)
{
    current_aux_box->position.z() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxSizeXChanged(double value)
{
    current_aux_box->size.x() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxSizeYChanged(double value)
{
    current_aux_box->size.y() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onAuxBoxSizeZChanged(double value)
{
    current_aux_box->size.z() = value;
    dirtyProject();

    camera_orientation_widget->updateLineSetGeometry();
    camera_orientation_widget->update();
}

void MainWindow::onPhotoSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    const int index = photo_list_widget->currentRow();
    current_camera = getCamera(index);
    if (current_camera)
    {
        photo_list_widget->itemWidget(photo_list_widget->item(index))->setEnabled(true);
        int prev_index = -1;
        if (deselected.size() > 0 && deselected.front().indexes().size() > 0)
        {
            prev_index = deselected.front().indexes().front().row();
        }
        if (prev_index >= 0 && prev_index < photo_list_widget->count())
        {
            photo_list_widget->itemWidget(photo_list_widget->item(prev_index))->setEnabled(false);
        }
    }
    updateCameraWidget();
    updateCameraWidgetSize();

    updateActionState();

    fillCurrentVertexListWidget();
}

void MainWindow::onAuxGeometrySelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    const int index = aux_geometry_list_widget->currentRow();
    current_aux_box = getAuxBox(index);
    if (current_aux_box)
    {
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
    updateActionState();
}

void MainWindow::onVertexSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    const int index = vertex_list_widget->currentRow();
    current_vertex = getVertex(index);

    updateActionState();
}

void MainWindow::onCurrentVertexSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    const int index = current_vertex_list_widget->currentRow();
    current_vertex_photo_position = getCurrentVertex(index);

    updateActionState();
}

void MainWindow::onTriangleSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    const int index = triangle_list_widget->currentRow();
    current_triangle = getTriangle(index);

    fillCurrentTriangleListWidget();
    updateActionState();
}

void MainWindow::onCurrentTriangleSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    current_triangle_item = current_triangle_list_widget->currentRow();

    updateActionState();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (closeProject())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    updateCameraWidgetAvailableSize();
    QMainWindow::resizeEvent(event);
}

void MainWindow::saveSelection()
{
    photoSaveSelection();
    auxGeometrySaveSelection();
    vertexSaveSelection();
    currentVertexSaveSelection();
    triangleSaveSelection();
    currentTriangleSaveSelection();
}

void MainWindow::restoreSelection()
{
    photoRestoreSelection();
    auxGeometryRestoreSelection();
    vertexRestoreSelection();
    currentVertexRestoreSelection();
    triangleRestoreSelection();
    currentTriangleRestoreSelection();
}

void MainWindow::restoreSelection(int saved_index, int saved_size, QListWidget* list_widget)
{
    if (list_widget->count() < saved_size) // Remove operation
    {
        if (list_widget->count() == 0)
        {
            list_widget->clearSelection();
        }
        else if (saved_index < list_widget->count())
        {
            list_widget->setCurrentRow(saved_index);
        }
        else
        {
            list_widget->setCurrentRow(list_widget->count() - 1);
        }
    }
    else
    { // Addition or nothing operation
        if (saved_index >= 0)
        {
            list_widget->setCurrentRow(saved_index);
        }
    }
}

void MainWindow::photoSaveSelection()
{
    saved_photo_list_selection = photo_list_widget->currentRow();
    saved_photo_list_size = photo_list_widget->count();
}

void MainWindow::photoRestoreSelection()
{
    restoreSelection(saved_photo_list_selection, saved_photo_list_size, photo_list_widget);
}

void MainWindow::auxGeometrySaveSelection()
{
    saved_aux_geometry_list_selection = aux_geometry_list_widget->currentRow();
    saved_aux_geometry_list_size = aux_geometry_list_widget->count();
}

void MainWindow::auxGeometryRestoreSelection()
{
    restoreSelection(saved_aux_geometry_list_selection, saved_aux_geometry_list_size, aux_geometry_list_widget);
}

void MainWindow::vertexSaveSelection()
{
    saved_vertex_list_selection = vertex_list_widget->currentRow();
    saved_vertex_list_size = vertex_list_widget->count();
}

void MainWindow::vertexRestoreSelection()
{
    restoreSelection(saved_vertex_list_selection, saved_vertex_list_size, vertex_list_widget);
}

void MainWindow::currentVertexSaveSelection()
{
    saved_current_vertex_list_selection = current_vertex_list_widget->currentRow();
    saved_current_vertex_list_size = current_vertex_list_widget->count();
}

void MainWindow::currentVertexRestoreSelection()
{
    restoreSelection(saved_current_vertex_list_selection, saved_current_vertex_list_size, current_vertex_list_widget);
}

void MainWindow::triangleSaveSelection()
{
    saved_triangle_list_selection = triangle_list_widget->currentRow();
    saved_triangle_list_size = triangle_list_widget->count();
}

void MainWindow::triangleRestoreSelection()
{
    restoreSelection(saved_triangle_list_selection, saved_triangle_list_size, triangle_list_widget);
}

void MainWindow::currentTriangleSaveSelection()
{
    saved_current_triangle_list_selection = current_triangle_list_widget->currentRow();
    saved_current_triangle_list_size = current_triangle_list_widget->count();
}

void MainWindow::currentTriangleRestoreSelection()
{
    restoreSelection(saved_current_triangle_list_selection, saved_current_triangle_list_size, current_triangle_list_widget);
}

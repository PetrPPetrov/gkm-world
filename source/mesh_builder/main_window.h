// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
#include <QCloseEvent>
#include <QResizeEvent>
#include "ui_main_window.h"
#include "common.h"
#include "mesh_project.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private:
    void initializeMenu();
    void initializeWidgets();

public:
    void updateWindowTitle();
    void setVertexPosition(QPointF position);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void dirtyProject();

    void addPhoto(const char* filename);
    void addAuxBox();

    void loadProject(const char* filename);
    void loadPhotos();
    void updateProject();
    void updateCameraWidgetSize();
    void updateCameraWidgetAvailableSize();

    void fillPhotoListWidget();
    void addPhotoListWidgetItem(const CameraInfo::Ptr& camera_info);
    void fillAuxGeometryListWidget();
    void addAuxGeometryListWidgetItem(const AuxGeometryBox::Ptr& aux_box);
    void fillVertexListWidget();
    void addVertexListWidgetItem(const Vertex::Ptr& vertex);
    void fillCurrentVertexWidget();
    void addCurrentVertexWidgetItem(const VertexPositionInfo::Ptr& vertex_position);
    void fillTriangleListWidget();
    void addTriangleListWidgetItem(const Triangle::Ptr& triangle);
    void fillCurrentTriangleListWidget();
    void addCurrentTriangleListWidgetItem(int vertex_id);

    int getCurrentCameraIndex() const;
    AuxGeometryBox::Ptr getAuxBox(int row_index) const;
    Vertex::Ptr getVertex(int row_index) const;
    VertexPositionInfo::Ptr getCurrentVertex(int row_index) const;
    Triangle::Ptr getTriangle(int row_index) const;

    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveAsProject();
    void onQuit();

    void onAddPhoto();
    void onRemovePhoto();

    void onAddAuxBox();
    void onRemoveAuxGeom();

    void onAddVertex();
    void onRemoveVertex();
    void onAddCurrentVertex();
    void onRemoveCurrentVertex();

    void onAddTriangle();
    void onRemoveTriangle();
    void onUseVertex();

    void onBuildMesh();
    void onSetOutputFile();

    void onLockedChanged(int state);
    void onRotationChanged(int index);
    void onFovChanged(double value);

    void onAuxBoxPosXChanged(double value);
    void onAuxBoxPosYChanged(double value);
    void onAuxBoxPosZChanged(double value);
    void onAuxBoxSizeXChanged(double value);
    void onAuxBoxSizeYChanged(double value);
    void onAuxBoxSizeZChanged(double value);

    void onPhotoSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void onAuxGeometrySelected(const QItemSelection& selected, const QItemSelection& deselected);
    void onVertexSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void onCurrentVertexSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void onTriangleSelected(const QItemSelection& selected, const QItemSelection& deselected);
    void onCurrentTriangleSelected(const QItemSelection& selected, const QItemSelection& deselected);

    void closeEvent(QCloseEvent* event);
    void resizeEvent(QResizeEvent* event);

private:
    bool first_show = true;
    Ui::MainWindow main_window;

    QAction* new_project_act = nullptr;
    QAction* open_project_act = nullptr;
    QAction* save_project_act = nullptr;
    QAction* save_project_as_act = nullptr;
    QAction* close_project_act = nullptr;
    QAction* quit_act = nullptr;

    QAction* add_photo_act = nullptr;
    QAction* remove_photo_act = nullptr;

    QAction* add_aux_box_act = nullptr;
    QAction* remove_aux_geom_act = nullptr;

    QAction* add_vertex_act = nullptr;
    QAction* remove_vertex_act = nullptr;
    QAction* add_current_vertex_act = nullptr;
    QAction* remove_current_vertex_act = nullptr;

    QAction* add_triangle_act = nullptr;
    QAction* remove_triangle_act = nullptr;
    QAction* use_selected_vertex_in_triangle_act = nullptr;

    QAction* build_mesh_act = nullptr;
    QAction* set_output_file_act = nullptr;

    MeshBuilderWidget* camera_orientation_widget = nullptr;
    QMdiSubWindow* camera_orientation_window = nullptr;
    int camera_available_width = 300;
    int camera_available_height = 200;
    double camera_scale_x = 1.0;
    double camera_scale_y = 1.0;

    QPlainTextEdit* log_widget = nullptr;
    QMdiSubWindow* log_window = nullptr;

    QListWidget* photo_list_widget = nullptr;
    QMdiSubWindow* photo_list_window = nullptr;

    QListWidget* aux_geometry_list_widget = nullptr;
    QMdiSubWindow* aux_geometry_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, AuxGeometryBox::Ptr> aux_geom_list_item_to_box;

    QListWidget* vertex_list_widget = nullptr;
    QMdiSubWindow* vertex_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, unsigned> vertex_list_item_to_vertex_id;

    QListWidget* current_vertex_list_widget = nullptr;
    QMdiSubWindow* current_vertex_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, VertexPositionInfo::Ptr> current_vertex_list_item_to_vertex_position;

    QListWidget* triangle_list_widget = nullptr;
    QMdiSubWindow* triangle_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, unsigned> triangle_list_item_to_triangle_id;

    QListWidget* current_triangle_list_widget = nullptr;
    QMdiSubWindow* current_triangle_list_window = nullptr;

    const std::string auto_save_file_name = "autosave.gmb";

    MeshProject::Ptr mesh_project;
    CameraInfo::Ptr camera_info;
};

extern MainWindow* g_main_window;

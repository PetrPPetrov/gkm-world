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
#include <QProgressBar>
#include "ui_main_window.h"
#include "common.h"
#include "mesh_project.h"
#include "mesh_builder.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

signals:
    void setMeshBuildingProgressPercents(unsigned percents);
    void setMeshBuildingProgressStatus(QString status);
    void meshBuildingFinished();
    void meshBuildingAborted();

private:
    void initializeMenu();
    void initializeWidgets();

public:
    void updateWindowTitle();
    VertexPhotoPosition::Ptr getCurrentVertexPhotoPosition() const;
    void updateCurrentVertexPhotoPosition();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void dirtyProject();

    void loadProject(const char* filename);
    void loadPhotos();
    void updateProject();
    void updateActionState();
    void updateCameraWidget();
    void updateCameraWidgetSize();
    void updateCameraWidgetAvailableSize();

    void fillPhotoListWidget();
    void addPhotoListWidgetItem(const Camera::Ptr& camera);
    void fillAuxGeometryListWidget();
    void addAuxGeometryListWidgetItem(const AuxBox::Ptr& aux_box);
    void fillVertexListWidget();
    void addVertexListWidgetItem(const Vertex::Ptr& vertex);
    void fillCurrentVertexListWidget();
    void addCurrentVertexListWidgetItem(const VertexPhotoPosition::Ptr& vertex_position);
    void fillTriangleListWidget();
    void addTriangleListWidgetItem(const Triangle::Ptr& triangle);
    void fillCurrentTriangleListWidget();
    void addCurrentTriangleListWidgetItem(int vertex_id);

    Camera::Ptr getCamera(int row_index) const;
    AuxBox::Ptr getAuxBox(int row_index) const;
    Vertex::Ptr getVertex(int row_index) const;
    VertexPhotoPosition::Ptr getCurrentVertex(int row_index) const;
    Triangle::Ptr getTriangle(int row_index) const;

    bool closeProject();
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
    void onBuildOptions();

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

    void onMeshBuildingDialogCanceled();
    void onMeshBuildingDialogFinished();

    void closeEvent(QCloseEvent* event);
    void resizeEvent(QResizeEvent* event);

    void saveSelection();
    void restoreSelection();
    void restoreSelection(int saved_index, int saved_size, QListWidget* list_widget);

    void photoSaveSelection();
    void photoRestoreSelection();
    void auxGeometrySaveSelection();
    void auxGeometryRestoreSelection();
    void vertexSaveSelection();
    void vertexRestoreSelection();
    void currentVertexSaveSelection();
    void currentVertexRestoreSelection();
    void triangleSaveSelection();
    void triangleRestoreSelection();
    void currentTriangleSaveSelection();
    void currentTriangleRestoreSelection();

private slots:
    void onSetMeshBuildingProgressPercents(unsigned percents);
    void onSetMeshBuildingProgressStatus(QString status);
    void onMeshBuildingFinished();
    void onMeshBuildingAborted();

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
    QAction* build_options_act = nullptr;

    MeshBuilderWidget* camera_orientation_widget = nullptr;
    QMdiSubWindow* camera_orientation_window = nullptr;
    const int camera_minimum_width = 256;
    const int camera_minimum_height = 128;
    int camera_available_width = 300;
    int camera_available_height = 200;

    QPlainTextEdit* log_widget = nullptr;
    QMdiSubWindow* log_window = nullptr;

    QListWidget* photo_list_widget = nullptr;
    QMdiSubWindow* photo_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, Camera::Ptr> photo_list_item_to_camera;

    QListWidget* aux_geometry_list_widget = nullptr;
    QMdiSubWindow* aux_geometry_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, AuxBox::Ptr> aux_geom_list_item_to_box;

    QListWidget* vertex_list_widget = nullptr;
    QMdiSubWindow* vertex_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, Vertex::Ptr> vertex_list_item_to_vertex;

    QListWidget* current_vertex_list_widget = nullptr;
    QMdiSubWindow* current_vertex_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, VertexPhotoPosition::Ptr> current_vertex_list_item_to_vertex_position;

    QListWidget* triangle_list_widget = nullptr;
    QMdiSubWindow* triangle_list_window = nullptr;
    std::unordered_map<QListWidgetItem*, Triangle::Ptr> triangle_list_item_to_triangle;

    QListWidget* current_triangle_list_widget = nullptr;
    QMdiSubWindow* current_triangle_list_window = nullptr;

    int saved_photo_list_selection;
    int saved_aux_geometry_list_selection;
    int saved_vertex_list_selection;
    int saved_current_vertex_list_selection;
    int saved_triangle_list_selection;
    int saved_current_triangle_list_selection;

    int saved_photo_list_size;
    int saved_aux_geometry_list_size;
    int saved_vertex_list_size;
    int saved_current_vertex_list_size;
    int saved_triangle_list_size;
    int saved_current_triangle_list_size;

    MeshProject::Ptr mesh_project;
    Camera::Ptr current_camera;
    AuxBox::Ptr current_aux_box;
    Vertex::Ptr current_vertex;
    VertexPhotoPosition::Ptr current_vertex_photo_position;
    Triangle::Ptr current_triangle;
    int current_triangle_item = -1;

    std::unique_ptr<MeshBuilder> mesh_builder;
    QDialog* mesh_building_dialog = nullptr;
    QLabel* mesh_building_status_label = nullptr;
    QProgressBar* mesh_building_progress_indicator = nullptr;
    QPushButton* mesh_building_cancel_button = nullptr;
};

extern MainWindow* g_main_window;

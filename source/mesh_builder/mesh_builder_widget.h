// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include "aux_geometry.h"

class MeshBuilderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MeshBuilderWidget(QWidget *parent);
    void setAuxGeometry(const AuxGeometry::Ptr& geometry);
    void updateAuxGeometry();

protected:
    void initializeGL() override;
    void initializeAuxGeomLineSet();
    void initializePhoto();
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setDefaultCamera();

private:
    AuxGeometry::Ptr aux_geometry;

    std::unique_ptr<QOpenGLShaderProgram> aux_geom_line_set_program;
    std::unique_ptr<QOpenGLVertexArrayObject> aux_geom_line_set_vao;
    std::unique_ptr<QOpenGLBuffer> aux_geom_line_set_vbo;
    int aux_geom_line_set_vbo_size;
    int aux_geom_line_set_matrix_location;

    std::unique_ptr<QOpenGLShaderProgram> photo_program;
    std::unique_ptr<QOpenGLVertexArrayObject> photo_vao;
    std::unique_ptr<QOpenGLBuffer> photo_vbo;
    int photo_matrix_location;
    int photo_texture_location;

    std::unique_ptr<QOpenGLTexture> photo;
    int photo_width;
    int photo_height;
    float photo_aspect;

    QVector3D viewer_pos;
    QVector3D viewer_target;
    QVector3D viewer_up;

    QVector3D viewer_previous_pos;
    QVector3D viewer_previous_target;
    QVector3D viewer_previous_up;

    double minimum_rotation_radius = 0.1;
    double maximum_rotation_radius = 1000.0;
    double rotation_radius = 10.0;
    bool left_mouse_pressed = false;
    bool right_mouse_pressed = false;
    QPointF previous_point;
};

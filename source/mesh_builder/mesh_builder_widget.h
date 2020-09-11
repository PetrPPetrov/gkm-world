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
#include <QVector3D>

class MeshBuilderWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MeshBuilderWidget(QWidget *parent);

protected:
    void initializeGL() override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setDefaultCamera();

private:
    std::unique_ptr<QOpenGLShaderProgram> program;
    //std::unique_ptr<QOpenGLTexture> texture;
    QOpenGLBuffer vbo;

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

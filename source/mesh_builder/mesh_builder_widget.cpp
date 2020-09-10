// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <boost/polygon/polygon.hpp>
#include <QPainter>
#include <QApplication>
#include <QStatusBar>
#include <QOpenGLShader>
#include "main_window.h"
#include "mesh_builder_widget.h"

MeshBuilderWidget::MeshBuilderWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setDefaultCamera();
}

void MeshBuilderWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void MeshBuilderWidget::paintGL()
{
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(0);

    QMatrix4x4 projection_matrix;
    projection_matrix.perspective(50.0f, static_cast<float>(width()) / height(), 0.125f, 1024.0f);

    glMatrixMode(GL_PROJECTION_MATRIX);
    glLoadMatrixf(projection_matrix.constData());

    QMatrix4x4 view_matrix;
    view_matrix.lookAt(viewer_pos, viewer_target, viewer_up);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view_matrix.constData());

    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glColor3f(1, 0, 0);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glColor3f(1, 0, 0);
    glEnd();
}

void MeshBuilderWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (right_mouse_pressed)
    {
        // Pan mode
        QPointF current_point = event->localPos();
        double delta_x = (previous_point.x() - current_point.x()) / width() * rotation_radius;
        double delta_y = (current_point.y() - previous_point.y()) / height() * rotation_radius;
        auto user_position = viewer_previous_pos - viewer_previous_target;
        auto right = QVector3D::crossProduct(viewer_up, user_position).normalized();
        auto offset = right * delta_x + viewer_up * delta_y;
        viewer_pos = viewer_previous_pos + offset;
        viewer_target = viewer_previous_target + offset;
        // Restore rotation orbit radius
        viewer_target = viewer_pos + (viewer_target - viewer_pos).normalized() * rotation_radius;
        update();
    }
    else if (left_mouse_pressed)
    {
        // Rotation mode
        QPointF current_point = event->localPos();
        double delta_x = previous_point.x() - current_point.x();
        double delta_y = current_point.y() - previous_point.y();
        double x_rotation_angle = delta_x / width() * 180.0f;
        double y_rotation_angle = delta_y / height() * 180.0f;
        auto user_position = viewer_previous_pos - viewer_previous_target;
        QMatrix4x4 rotation_x;
        rotation_x.rotate(x_rotation_angle, viewer_previous_up);
        auto temp_user_position = rotation_x * user_position;
        auto left = QVector3D::crossProduct(temp_user_position, viewer_previous_up).normalized();
        QMatrix4x4 rotation_y;
        rotation_y.rotate(y_rotation_angle, left);
        auto result_rotation = rotation_y * rotation_x;
        auto rotated_user_position = (result_rotation * user_position).normalized() * rotation_radius;
        viewer_pos = viewer_previous_target + rotated_user_position;
        viewer_up = (result_rotation * viewer_previous_up).normalized();
        // Restore up vector property: up vector must be orthogonal to direction vector
        auto new_left = QVector3D::crossProduct(rotated_user_position, viewer_up);
        viewer_up = QVector3D::crossProduct(new_left, rotated_user_position).normalized();
        update();
    }
}

void MeshBuilderWidget::mousePressEvent(QMouseEvent* event)
{
    bool left_or_right = false;
    if (event->buttons() & Qt::LeftButton)
    {
        left_mouse_pressed = true;
        left_or_right = true;
    }
    if (event->buttons() & Qt::RightButton)
    {
        right_mouse_pressed = true;
        left_or_right = true;
    }
    if (left_or_right)
    {
        previous_point = event->localPos();
        viewer_previous_pos = viewer_pos;
        viewer_previous_target = viewer_target;
        viewer_previous_up = viewer_up;
    }
    update();
}

void MeshBuilderWidget::mouseReleaseEvent(QMouseEvent* event)
{
    bool left_or_right = false;
    if (!(event->buttons() & Qt::LeftButton))
    {
        left_mouse_pressed = false;
        left_or_right = true;
    }
    if (!(event->buttons() & Qt::RightButton))
    {
        right_mouse_pressed = false;
        left_or_right = true;
    }
    if (left_or_right)
    {
        previous_point = event->localPos();
        viewer_previous_pos = viewer_pos;
        viewer_previous_target = viewer_target;
        viewer_previous_up = viewer_up;
    }
    update();
}

void MeshBuilderWidget::wheelEvent(QWheelEvent* event)
{
    QPoint delta = event->angleDelta();
    rotation_radius += delta.y() / 1000.0f * rotation_radius;
    if (rotation_radius < minimum_rotation_radius)
    {
        rotation_radius = minimum_rotation_radius;
    }
    if (rotation_radius > maximum_rotation_radius)
    {
        rotation_radius = maximum_rotation_radius;
    }
    auto user_position = viewer_pos - viewer_target;
    auto new_user_position = user_position.normalized() * rotation_radius;
    viewer_pos = viewer_target + new_user_position;
    update();
}

void MeshBuilderWidget::setDefaultCamera()
{
    viewer_pos = QVector3D(0.5, 0.5, 3);
    viewer_target = QVector3D(0.5, 0.5, 0.5);
    viewer_up = QVector3D(0, 1, 0);
    rotation_radius = viewer_pos.distanceToPoint(viewer_target);
    viewer_previous_pos = viewer_pos;
    viewer_previous_target = viewer_target;
    viewer_previous_up = viewer_up;
    minimum_rotation_radius = 0.1;
    maximum_rotation_radius = 1000.0;
}

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

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_COLOR_ATTRIBUTE 1

struct VertexPositionColor
{
    float x, y, z;
    std::uint32_t abgr;
};

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

    //texture = std::make_unique<QOpenGLTexture>(QImage(QString("texture.jpg")));

    const static VertexPositionColor vertex_buffer[] =
    {
        { 0.0f, 0.0f, 0.0f, 0xff0000ff },
        { 1.0f, 0.0f, 0.0f, 0xff0000ff },
        { 0.0f, 0.0f, 0.0f, 0xff00ff00 },
        { 0.0f, 1.0f, 0.0f, 0xff00ff00 },
        { 0.0f, 0.0f, 0.0f, 0xffff0000 },
        { 0.0f, 0.0f, 1.0f, 0xffff0000 },
        { 1.0f, 0.0f, 0.0f, 0xffffffff },
        { 1.0f, 1.0f, 0.0f, 0xffffffff },
        { 1.0f, 1.0f, 0.0f, 0xffffffff },
        { 0.0f, 1.0f, 0.0f, 0xffffffff },
        { 0.0f, 0.0f, 1.0f, 0xffffffff },
        { 1.0f, 0.0f, 1.0f, 0xffffffff },
        { 0.0f, 0.0f, 1.0f, 0xffffffff },
        { 0.0f, 1.0f, 1.0f, 0xffffffff },
        { 1.0f, 0.0f, 1.0f, 0xffffffff },
        { 1.0f, 1.0f, 1.0f, 0xffffffff },
        { 1.0f, 1.0f, 1.0f, 0xffffffff },
        { 0.0f, 1.0f, 1.0f, 0xffffffff },
        { 1.0f, 0.0f, 0.0f, 0xffffffff },
        { 1.0f, 0.0f, 1.0f, 0xffffffff },
        { 0.0f, 1.0f, 0.0f, 0xffffffff },
        { 0.0f, 1.0f, 1.0f, 0xffffffff },
        { 1.0f, 1.0f, 0.0f, 0xffffffff },
        { 1.0f, 1.0f, 1.0f, 0xffffffff }
    };

    vbo.create();
    vbo.bind();
    vbo.allocate(vertex_buffer, sizeof(vertex_buffer));

    QOpenGLShader* vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute lowp vec4 color;\n"
        "varying lowp vec4 vcolor;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    vcolor = color;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader* fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* fsrc =
        "varying lowp vec4 vcolor;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vcolor;\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program = std::make_unique<QOpenGLShaderProgram>();
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    program->bindAttributeLocation("color", PROGRAM_COLOR_ATTRIBUTE);
    program->link();

    program->bind();
    //program->setUniformValue("texture", 0);
}

void MeshBuilderWidget::paintGL()
{
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 projection_matrix;
    projection_matrix.perspective(50.0f, static_cast<float>(width()) / height(), 0.125f, 1024.0f);

    QMatrix4x4 view_matrix;
    view_matrix.lookAt(viewer_pos, viewer_target, viewer_up);

    program->setUniformValue("matrix", projection_matrix * view_matrix);
    program->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    program->enableAttributeArray(PROGRAM_COLOR_ATTRIBUTE);
    program->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, sizeof(VertexPositionColor));
    program->setAttributeBuffer(PROGRAM_COLOR_ATTRIBUTE, GL_UNSIGNED_BYTE, 3 * sizeof(float), 4, sizeof(VertexPositionColor));

    //texture->bind();

    glDrawArrays(GL_LINES, 0, 24);
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
    viewer_pos = QVector3D(3, 3, 3);
    viewer_target = QVector3D(0.5, 0.5, 0.5);
    viewer_up = QVector3D(0, 0, 1);
    rotation_radius = viewer_pos.distanceToPoint(viewer_target);
    viewer_previous_pos = viewer_pos;
    viewer_previous_target = viewer_target;
    viewer_previous_up = viewer_up;
    minimum_rotation_radius = 0.1;
    maximum_rotation_radius = 10000.0;
}

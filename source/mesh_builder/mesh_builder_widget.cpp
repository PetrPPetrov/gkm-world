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
#include "vbo.h"
#include "vbo_fill.h"

MeshBuilderWidget::MeshBuilderWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setDefaultCamera();
}

void MeshBuilderWidget::setMeshProject(const MeshProject::Ptr& project)
{
    mesh_project = project;
}

void MeshBuilderWidget::updateLineSetGeometry()
{
    std::vector<VertexPositionColor> vertex_buffer;
    fillLineSet(
        vertex_buffer,
        mesh_project,
        current_camera,
        aux_geom_line_set_vbo_start,
        aux_geom_line_set_vbo_size,
        hub_points_line_set_vbo_start,
        hub_points_line_set_vbo_size,
        vertices_line_set_vbo_start,
        vertices_line_set_vbo_size,
        photo_x_low, photo_y_low
    );

    const size_t vbo_size = vertex_buffer.size();
    if (vbo_size > 0)
    {
        line_set_vbo->bind();
        line_set_vbo->write(0, &vertex_buffer[0], static_cast<int>(vbo_size * sizeof(VertexPositionColor)));
    }
}

void MeshBuilderWidget::setPhoto(const Camera::Ptr& camera)
{
    current_camera = camera;
    if (current_camera)
    {
        viewer_previous_pos = viewer_pos = to_qt(current_camera->viewer_pos);
        viewer_previous_target = viewer_target = to_qt(current_camera->viewer_target);
        viewer_previous_up = viewer_up = to_qt(current_camera->viewer_up);
        left_mouse_pressed = false;
        right_mouse_pressed = false;
        rotation_radius = current_camera->rotation_radius;
    }
    else
    {
        setDefaultCamera();
    }
}

void MeshBuilderWidget::updatePhotoTexture()
{
    int rotation = 0;
    if (current_camera)
    {
        photo_texture = std::make_unique<QOpenGLTexture>(*current_camera->photo_image);
        rotation = current_camera->rotation;
    }
    else
    {
        QImage dummy_image(2, 2, QImage::Format::Format_RGB888);
        dummy_image.fill(QColor(Qt::gray).rgb());
        photo_texture = std::make_unique<QOpenGLTexture>(dummy_image);
    }

    photo_width = current_camera ? getCameraWidth(current_camera) : photo_texture->width();
    photo_height = current_camera ? getCameraHeight(current_camera) : photo_texture->height();;

    photo_x_low = -photo_width / 2;
    photo_x_high = photo_x_low + photo_width;
    photo_y_low = -photo_height / 2;
    photo_y_high = photo_y_low + photo_height;

    VertexPositionTexCoord vertex_buffer[] =
    {
        { static_cast<float>(photo_x_low), static_cast<float>(photo_y_low), -1.0f, 0.0f, 1.0f },
        { static_cast<float>(photo_x_high), static_cast<float>(photo_y_low), -1.0f, 1.0f, 1.0f },
        { static_cast<float>(photo_x_high), static_cast<float>(photo_y_high), -1.0f, 1.0f, 0.0f },
        { static_cast<float>(photo_x_high), static_cast<float>(photo_y_high), -1.0f, 1.0f, 0.0f },
        { static_cast<float>(photo_x_low), static_cast<float>(photo_y_high), -1.0f, 0.0f, 0.0f },
        { static_cast<float>(photo_x_low), static_cast<float>(photo_y_low), -1.0f, 0.0f, 1.0f }
    };
    switch (rotation)
    {
    case 90:
    {
        vertex_buffer[0].u = 0.0f;
        vertex_buffer[0].v = 0.0f;
        vertex_buffer[1].u = 0.0f;
        vertex_buffer[1].v = 1.0f;
        vertex_buffer[2].u = 1.0f;
        vertex_buffer[2].v = 1.0f;
        vertex_buffer[3].u = 1.0f;
        vertex_buffer[3].v = 1.0f;
        vertex_buffer[4].u = 1.0f;
        vertex_buffer[4].v = 0.0f;
        vertex_buffer[5].u = 0.0f;
        vertex_buffer[5].v = 0.0f;
    }
        break;
    case 180:
    {
        vertex_buffer[0].u = 1.0f;
        vertex_buffer[0].v = 0.0f;
        vertex_buffer[1].u = 0.0f;
        vertex_buffer[1].v = 0.0f;
        vertex_buffer[2].u = 0.0f;
        vertex_buffer[2].v = 1.0f;
        vertex_buffer[3].u = 0.0f;
        vertex_buffer[3].v = 1.0f;
        vertex_buffer[4].u = 1.0f;
        vertex_buffer[4].v = 1.0f;
        vertex_buffer[5].u = 1.0f;
        vertex_buffer[5].v = 0.0f;
    }
        break;
    case 270:
    {
        vertex_buffer[0].u = 1.0f;
        vertex_buffer[0].v = 1.0f;
        vertex_buffer[1].u = 1.0f;
        vertex_buffer[1].v = 0.0f;
        vertex_buffer[2].u = 0.0f;
        vertex_buffer[2].v = 0.0f;
        vertex_buffer[3].u = 0.0f;
        vertex_buffer[3].v = 0.0f;
        vertex_buffer[4].u = 0.0f;
        vertex_buffer[4].v = 1.0f;
        vertex_buffer[5].u = 1.0f;
        vertex_buffer[5].v = 1.0f;
    }
        break;
    default:
        break;
    }

    photo_vbo->bind();
    photo_vbo->write(0, vertex_buffer, static_cast<int>(PHOTO_MAX_VERTEX_COUNT * sizeof(VertexPositionTexCoord)));
}

void MeshBuilderWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    initializeAuxGeomLineSet();
    initializePhoto();
}

void MeshBuilderWidget::initializeAuxGeomLineSet()
{
    line_set_vao = std::make_unique<QOpenGLVertexArrayObject>();
    line_set_vao->create();
    line_set_vao->bind();

    line_set_vbo = std::make_unique<QOpenGLBuffer>();
    line_set_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    line_set_vbo->create();
    line_set_vbo->bind();
    line_set_vbo->allocate(LINE_SET_VBO_MAX_VERTEX_COUNT * sizeof(VertexPositionColor));

    updateLineSetGeometry();

    line_set_program = std::make_unique<QOpenGLShaderProgram>();
    line_set_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "attribute lowp vec4 color;\n"
        "varying lowp vec4 vcolor;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    vcolor = color;\n"
        "}\n");

    line_set_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "varying lowp vec4 vcolor;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vcolor;\n"
        "}\n");
    line_set_program->link();

    const int vertex_location = line_set_program->attributeLocation("vertex");
    const int color_location = line_set_program->attributeLocation("color");
    line_set_matrix_location = line_set_program->uniformLocation("matrix");

    line_set_program->enableAttributeArray(vertex_location);
    line_set_program->enableAttributeArray(color_location);
    line_set_program->setAttributeBuffer(vertex_location, GL_FLOAT, 0, 3, sizeof(VertexPositionColor));
    line_set_program->setAttributeBuffer(color_location, GL_UNSIGNED_BYTE, 3 * sizeof(float), 4, sizeof(VertexPositionColor));
}

void MeshBuilderWidget::initializePhoto()
{
    photo_vao = std::make_unique<QOpenGLVertexArrayObject>();
    photo_vao->create();
    photo_vao->bind();

    photo_vbo = std::make_unique<QOpenGLBuffer>();
    photo_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    photo_vbo->create();
    photo_vbo->bind();
    photo_vbo->allocate(PHOTO_MAX_VERTEX_COUNT * sizeof(VertexPositionTexCoord));

    updatePhotoTexture();

    photo_program = std::make_unique<QOpenGLShaderProgram>();
    photo_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec2 texcoord;\n"
        "varying mediump vec2 vtexcoord;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    vtexcoord = texcoord;\n"
        "}\n");
    photo_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "uniform sampler2D texture;\n"
        "varying mediump vec2 vtexcoord;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, vtexcoord);\n"
        "}\n");
    photo_program->link();

    const int vertex_location = photo_program->attributeLocation("vertex");
    const int texcoord_location = photo_program->attributeLocation("texcoord");
    photo_matrix_location = photo_program->uniformLocation("matrix");
    photo_texture_location = photo_program->uniformLocation("texture");

    photo_program->enableAttributeArray(vertex_location);
    photo_program->enableAttributeArray(texcoord_location);
    photo_program->setAttributeBuffer(vertex_location, GL_FLOAT, 0, 3, sizeof(VertexPositionTexCoord));
    photo_program->setAttributeBuffer(texcoord_location, GL_FLOAT, 3 * sizeof(float), 2, sizeof(VertexPositionTexCoord));
}

void MeshBuilderWidget::paintGL()
{
    float fov = 50.0;
    if (current_camera)
    {
        fov = static_cast<float>(current_camera->fov);
    }

    const int viewport_width = width();
    const int viewport_height = height();
    const double viewport_aspect = static_cast<double>(viewport_width) / viewport_height;

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 ortho_projection;
    ortho_projection.ortho(photo_y_low * viewport_aspect, photo_y_high * viewport_aspect, photo_y_low, photo_y_high, 0.125f, 2048.0f);

    photo_vao->bind();
    photo_program->bind();
    photo_program->setUniformValue(photo_matrix_location, ortho_projection);
    photo_texture->bind();
    glDrawArrays(GL_TRIANGLES, 0, PHOTO_MAX_VERTEX_COUNT);

    if (hub_points_line_set_vbo_size > 0)
    {
        glDisable(GL_DEPTH_TEST);
        line_set_vao->bind();
        line_set_program->bind();
        line_set_program->setUniformValue(line_set_matrix_location, ortho_projection);
        glDrawArrays(GL_LINES, hub_points_line_set_vbo_start, hub_points_line_set_vbo_size);
    }

    QMatrix4x4 projection_matrix;
    projection_matrix.perspective(fov, static_cast<float>(viewport_aspect), 0.125f, 1024.0f);
    QMatrix4x4 view_matrix;
    view_matrix.lookAt(viewer_pos, viewer_target, viewer_up);
    QMatrix4x4 mvp_matrix = projection_matrix * view_matrix;

    if (aux_geom_line_set_vbo_size > 0)
    {
        glDisable(GL_DEPTH_TEST);
        line_set_vao->bind();
        line_set_program->bind();
        line_set_program->setUniformValue(line_set_matrix_location, mvp_matrix);
        glDrawArrays(GL_LINES, aux_geom_line_set_vbo_start, aux_geom_line_set_vbo_size);
    }

    if (vertices_line_set_vbo_size > 0)
    {
        glDisable(GL_DEPTH_TEST);
        line_set_vao->bind();
        line_set_program->bind();
        line_set_program->setUniformValue(line_set_matrix_location, mvp_matrix);
        glDrawArrays(GL_LINES, vertices_line_set_vbo_start, vertices_line_set_vbo_size);
    }
}

void MeshBuilderWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (current_camera && current_camera->locked)
    {
        resetNavigation();
        return;
    }

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
        updateCameraInfo();
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
        updateCameraInfo();
    }
}

void MeshBuilderWidget::mousePressEvent(QMouseEvent* event)
{
    if (current_camera && current_camera->locked)
    {
        resetNavigation();
        g_main_window->setVertexPosition(event->localPos());
        return;
    }

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
    updateCameraInfo();
}

void MeshBuilderWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (current_camera && current_camera->locked)
    {
        resetNavigation();
        return;
    }

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
    updateCameraInfo();
}

void MeshBuilderWidget::wheelEvent(QWheelEvent* event)
{
    if (current_camera && current_camera->locked)
    {
        resetNavigation();
        return;
    }

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
    updateCameraInfo();
}

void MeshBuilderWidget::setDefaultCamera()
{
    viewer_pos = QVector3D(3, 3, 3);
    viewer_target = QVector3D(0, 0, 0);
    viewer_up = QVector3D(0, 0, 1);
    rotation_radius = viewer_pos.distanceToPoint(viewer_target);
    viewer_previous_pos = viewer_pos;
    viewer_previous_target = viewer_target;
    viewer_previous_up = viewer_up;
}

void MeshBuilderWidget::updateCameraInfo()
{
    if (current_camera)
    {
        current_camera->viewer_pos = to_eigen(viewer_pos);
        current_camera->viewer_target = to_eigen(viewer_target);
        current_camera->viewer_up = to_eigen(viewer_up);
        current_camera->rotation_radius = rotation_radius;
        if (mesh_project)
        {
            mesh_project->dirty = true;
            g_main_window->updateWindowTitle();
        }
    }
}

void MeshBuilderWidget::resetNavigation()
{
    viewer_previous_pos = viewer_pos;
    viewer_previous_target = viewer_target;
    viewer_previous_up = viewer_up;
    left_mouse_pressed = false;
    right_mouse_pressed = false;
}

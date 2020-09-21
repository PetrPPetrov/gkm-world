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

constexpr static int AUX_GEOM_VBO_MAX_VERTEX_COUNT = 16 * 1024;

struct VertexPositionColor
{
    float x, y, z;
    std::uint32_t abgr;
};

struct VertexPositionTexCoord
{
    float x, y, z;
    float u, v;
};

MeshBuilderWidget::MeshBuilderWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    setDefaultCamera();
}

void MeshBuilderWidget::setAuxGeometry(const AuxGeometry::Ptr& geometry)
{
    aux_geometry = geometry;
}

void MeshBuilderWidget::updateAuxGeometry()
{
    // Need to create new or re-create VBO, however, re-creating VBO does not have any effects
    const static VertexPositionColor box_vertex_buffer[] =
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
    const size_t box_vertex_buffer_size = sizeof(box_vertex_buffer) / sizeof(box_vertex_buffer[0]);
    const size_t vertex_count = std::min(box_vertex_buffer_size * aux_geometry->boxes.size(), static_cast<size_t>(AUX_GEOM_VBO_MAX_VERTEX_COUNT));

    std::vector<VertexPositionColor> vertex_buffer;
    vertex_buffer.reserve(vertex_count);

    for (auto box : aux_geometry->boxes)
    {
        for (size_t i = 0; i < box_vertex_buffer_size; ++i)
        {
            VertexPositionColor cur_vertex = box_vertex_buffer[i];
            cur_vertex.x = box->position.x() + cur_vertex.x * box->size.x();
            cur_vertex.y = box->position.y() + cur_vertex.y * box->size.y();
            cur_vertex.z = box->position.z() + cur_vertex.z * box->size.z();
            if (vertex_buffer.size() < vertex_count)
            {
                vertex_buffer.push_back(cur_vertex);
            }
        }
    }

    aux_geom_line_set_vbo->bind();
    aux_geom_line_set_vbo->write(0, &vertex_buffer[0], static_cast<int>(vertex_count * sizeof(VertexPositionColor)));

    aux_geom_line_set_vbo_size = static_cast<int>(vertex_count);
}

void MeshBuilderWidget::setPhoto(const CameraInfo::Ptr& camera_info_)
{
    camera_info = camera_info_;
    viewer_previous_pos = viewer_pos = to_qt(camera_info->viewer_pos);
    viewer_previous_target = viewer_target = to_qt(camera_info->viewer_target);
    viewer_previous_up = viewer_up = to_qt(camera_info->viewer_up);
    left_mouse_pressed = false;
    right_mouse_pressed = false;
    rotation_radius = camera_info->rotation_radius;
}

void MeshBuilderWidget::updatePhotoTexture()
{
    photo_texture = std::make_unique<QOpenGLTexture>(*camera_info->photo_image);
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
    aux_geom_line_set_vao = std::make_unique<QOpenGLVertexArrayObject>();
    aux_geom_line_set_vao->create();
    aux_geom_line_set_vao->bind();

    aux_geom_line_set_vbo = std::make_unique<QOpenGLBuffer>();
    aux_geom_line_set_vbo->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    aux_geom_line_set_vbo->create();
    aux_geom_line_set_vbo->bind();
    aux_geom_line_set_vbo->allocate(AUX_GEOM_VBO_MAX_VERTEX_COUNT * sizeof(VertexPositionColor));

    updateAuxGeometry();

    aux_geom_line_set_program = std::make_unique<QOpenGLShaderProgram>();
    aux_geom_line_set_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
        "attribute highp vec4 vertex;\n"
        "attribute lowp vec4 color;\n"
        "varying lowp vec4 vcolor;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    vcolor = color;\n"
        "}\n");

    aux_geom_line_set_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
        "varying lowp vec4 vcolor;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vcolor;\n"
        "}\n");
    aux_geom_line_set_program->link();

    const int vertex_location = aux_geom_line_set_program->attributeLocation("vertex");
    const int color_location = aux_geom_line_set_program->attributeLocation("color");
    aux_geom_line_set_matrix_location = aux_geom_line_set_program->uniformLocation("matrix");

    aux_geom_line_set_program->enableAttributeArray(vertex_location);
    aux_geom_line_set_program->enableAttributeArray(color_location);
    aux_geom_line_set_program->setAttributeBuffer(vertex_location, GL_FLOAT, 0, 3, sizeof(VertexPositionColor));
    aux_geom_line_set_program->setAttributeBuffer(color_location, GL_UNSIGNED_BYTE, 3 * sizeof(float), 4, sizeof(VertexPositionColor));
}

void MeshBuilderWidget::initializePhoto()
{
    updatePhotoTexture();

    photo_width = photo_texture->width();
    photo_height = photo_texture->height();
    photo_aspect = static_cast<float>(photo_width) / photo_height;

    const float x_low = -photo_width / 2;
    const float x_high = x_low + photo_width;
    const float y_low = -photo_height / 2;
    const float y_high = y_low + photo_height;

    const static VertexPositionTexCoord vertex_buffer[] =
    {
        { x_low, y_low, -1.0f, 0.0f, 1.0f },
        { x_high, y_low, -1.0f, 1.0f, 1.0f },
        { x_high, y_high, -1.0f, 1.0f, 0.0f },
        { x_high, y_high, -1.0f, 1.0f, 0.0f },
        { x_low, y_high, -1.0f, 0.0f, 0.0f },
        { x_low, y_low, -1.0f, 0.0f, 1.0f }
    };

    photo_vao = std::make_unique<QOpenGLVertexArrayObject>();
    photo_vao->create();
    photo_vao->bind();
    photo_vbo = std::make_unique<QOpenGLBuffer>();
    photo_vbo->create();
    photo_vbo->bind();
    photo_vbo->allocate(vertex_buffer, sizeof(vertex_buffer));

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
    photo_program->setUniformValue(photo_texture_location, photo_texture->textureId());
}

void MeshBuilderWidget::paintGL()
{
    const int viewport_width = width();
    const int viewport_height = height();
    const float viewport_aspect = static_cast<double>(viewport_width) / viewport_height;

    float x_low = -photo_width / 2;
    float x_high = x_low + photo_width;
    float y_low = -photo_height / 2;
    float y_high = y_low + photo_height;

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 ortho_projection;
    ortho_projection.ortho(x_low, x_high, y_low, y_high, 0.125f, 1024.0f);
    photo_vao->bind();
    photo_program->bind();
    photo_program->setUniformValue(photo_matrix_location, ortho_projection);
    photo_texture->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    QMatrix4x4 projection_matrix;
    projection_matrix.perspective(50.0f, viewport_aspect, 0.125f, 1024.0f);

    QMatrix4x4 view_matrix;
    view_matrix.lookAt(viewer_pos, viewer_target, viewer_up);

    QMatrix4x4 mvp_matrix = projection_matrix * view_matrix;
    glDisable(GL_DEPTH_TEST);
    aux_geom_line_set_vao->bind();
    aux_geom_line_set_program->bind();
    aux_geom_line_set_program->setUniformValue(aux_geom_line_set_matrix_location, mvp_matrix);
    glDrawArrays(GL_LINES, 0, aux_geom_line_set_vbo_size);
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
    viewer_target = QVector3D(0.5, 0.5, 0.5);
    viewer_up = QVector3D(0, 0, 1);
    rotation_radius = viewer_pos.distanceToPoint(viewer_target);
    viewer_previous_pos = viewer_pos;
    viewer_previous_target = viewer_target;
    viewer_previous_up = viewer_up;
}

void MeshBuilderWidget::updateCameraInfo()
{
    camera_info->viewer_pos = to_eigen(viewer_pos);
    camera_info->viewer_target = to_eigen(viewer_target);
    camera_info->viewer_up = to_eigen(viewer_up);
    camera_info->rotation_radius = rotation_radius;
}

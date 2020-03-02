// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <algorithm>
#include <QPainter>
#include "texture_widget.h"

TextureWidget::TextureWidget(QWidget* parent, const ImagePtr& image_)
    : QWidget(parent), image(image_)
{
    view_point_x = image->width() / 2.0;
    view_point_y = image->height() / 2.0;
    previous_view_point_x = view_point_x;
    previous_view_point_y = view_point_y;
}

void TextureWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (first_show)
    {
        first_show = false;
        scale = std::min(static_cast<double>(width()) / image->width(), static_cast<double>(height()) / image->height());
        cached_zoom = zoom();
    }
}

void TextureWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    const QRect viewport = painter.viewport();
    screen_center_x = viewport.center().x();
    screen_center_y = viewport.center().y();

    int draw_x = screen_center_x - view_point_x * cached_zoom;
    int draw_y = screen_center_y - view_point_y * cached_zoom;

    painter.drawImage(QRect(draw_x, draw_y, image->width() * cached_zoom, image->height() * cached_zoom), *image);
}

void TextureWidget::wheelEvent(QWheelEvent* event)
{
    zoom_step += (event->angleDelta().y() / 120);
    if (zoom_step < MIN_ZOOM_STEP)
    {
        zoom_step = MIN_ZOOM_STEP;
    }
    if (zoom_step > MAX_ZOOM_STEP)
    {
        zoom_step = MAX_ZOOM_STEP;
    }
    cached_zoom = zoom();
    update();
}

void TextureWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF current_point = event->localPos();

    if (right_mouse_pressed)
    {
        const QPointF delta = previous_point - current_point;
        const double actual_zoom = zoom();
        view_point_x = previous_view_point_x + delta.x() / actual_zoom;
        view_point_y = previous_view_point_y + delta.y() / actual_zoom;
        update();
    }
}

void TextureWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        left_mouse_pressed = true;
    }
    if (event->buttons() & Qt::RightButton)
    {
        right_mouse_pressed = true;
    }
    if (right_mouse_pressed)
    {
        previous_point = event->localPos();
        previous_view_point_x = view_point_x;
        previous_view_point_y = view_point_y;
    }
}

void TextureWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton))
    {
        left_mouse_pressed = false;
    }
    if (!(event->buttons() & Qt::RightButton))
    {
        right_mouse_pressed = false;
    }
    previous_point = event->localPos();
    previous_view_point_x = view_point_x;
    previous_view_point_y = view_point_y;
}

double TextureWidget::zoom() const
{
    const double norm_scale = std::pow(ZOOM_BASE, INITIAL_ZOOM_STEP);
    return std::pow(ZOOM_BASE, zoom_step) / norm_scale * scale;
}

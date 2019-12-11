// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <QPainter>
#include "balance_monitor_widget.h"

BalanceMonitorWidget::BalanceMonitorWidget(QWidget *parent) : QWidget(parent)
{
}

void BalanceMonitorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    cached_zoom = zoom();
    const QRect viewport = painter.viewport();
    screen_center_x = viewport.center().x();
    screen_center_y = viewport.center().y();

    const double world_to_draw_width = viewport.width() / cached_zoom;
    const double world_to_draw_height = viewport.height() / cached_zoom;
    const double actual_cell_size = CELL_SIZE * cached_zoom;
    bool draw_cell_lines = true;
    if (actual_cell_size < 8.0)
    {
        draw_cell_lines = false;
    }

    if (draw_cell_lines)
    {
        const std::int32_t cells_to_draw_width = static_cast<std::int32_t>(world_to_draw_width / CELL_SIZE) + 1;
        const std::int32_t cells_to_draw_height = static_cast<std::int32_t>(world_to_draw_height / CELL_SIZE) + 1;
        const std::int32_t cur_cell_x = static_cast<std::int32_t>(view_point_x / CELL_SIZE);
        const std::int32_t cur_cell_y = static_cast<std::int32_t>(view_point_y / CELL_SIZE);
        const std::int32_t start_cell_x = cur_cell_x - cells_to_draw_width / 2 - 1;
        const std::int32_t start_cell_y = cur_cell_y - cells_to_draw_height / 2 - 1;
        const std::int32_t end_cell_x = cur_cell_x + cells_to_draw_width / 2 + 1;
        const std::int32_t end_cell_y = cur_cell_y + cells_to_draw_height / 2 + 1;

        QPen thin_pen;
        thin_pen.setColor(QColor(0, 148, 0));
        thin_pen.setStyle(Qt::DotLine);
        painter.setPen(thin_pen);

        for (std::int32_t x = start_cell_x; x <= end_cell_x; ++x)
        {
            painter.drawLine(
                to_screen_x(x * CELL_SIZE), to_screen_y(start_cell_y * CELL_SIZE),
                to_screen_x(x * CELL_SIZE), to_screen_y((end_cell_y + 1) * CELL_SIZE)
            );
        }
        for (std::int32_t y = start_cell_y; y <= end_cell_y; ++y)
        {
            painter.drawLine(
                to_screen_x(start_cell_x * CELL_SIZE), to_screen_y(y * CELL_SIZE),
                to_screen_x((end_cell_x + 1) * CELL_SIZE), to_screen_y(y * CELL_SIZE)
            );
        }
    }
}

void BalanceMonitorWidget::wheelEvent(QWheelEvent* event)
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
    update();
}

void BalanceMonitorWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (left_mouse_pressed)
    {
        const QPointF current_point = event->localPos();
        const QPointF delta = previous_point - current_point;
        const double actual_zoom = zoom();
        view_point_x = previous_view_point_x + delta.x() / actual_zoom;
        view_point_y = previous_view_point_y - delta.y() / actual_zoom;
        update();
    }
}

void BalanceMonitorWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        left_mouse_pressed = true;
        previous_point = event->localPos();
        previous_view_point_x = view_point_x;
        previous_view_point_y = view_point_y;
    }
}

void BalanceMonitorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    left_mouse_pressed = false;
    previous_point = event->localPos();
    previous_view_point_x = view_point_x;
    previous_view_point_y = view_point_y;
}

int BalanceMonitorWidget::to_screen_x(double x_) const
{
    return static_cast<int>((x_ - view_point_x) * cached_zoom + screen_center_x);
}

int BalanceMonitorWidget::to_screen_y(double y_) const
{
    return static_cast<int>((y_ - view_point_y) * -cached_zoom + screen_center_y);
}

double BalanceMonitorWidget::zoom() const
{
    const double scale = std::pow(ZOOM_BASE, INITIAL_ZOOM_STEP);
    return std::pow(ZOOM_BASE, zoom_step) / scale;
}

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
    QRect viewport = painter.viewport();

    QMatrix set_center;
    set_center.translate(viewport.center().x(), viewport.center().y());
    QMatrix mirror_y;
    mirror_y.scale(1.0, -1.0);
    QTransform result_transformation(mirror_y * set_center);
    painter.setTransform(result_transformation);

    std::int32_t width = viewport.width() / actualCellSize();
    std::int32_t height = viewport.height() / actualCellSize();

    QPen thin_pen;
    thin_pen.setColor(QColor(0, 48, 0));
    thin_pen.setStyle(Qt::DashDotLine);
    painter.setPen(thin_pen);

    const std::int32_t min_cell_x_to_draw = -width / 2 - 1;
    const std::int32_t max_cell_x_to_draw = width / 2 + 1;
    const std::int32_t min_cell_y_to_draw = -height / 2 - 1;
    const std::int32_t max_cell_y_to_draw = height / 2 + 1;

    int min_x_to_draw = static_cast<int>(actualCellSize() * min_cell_x_to_draw);
    int max_x_to_draw = static_cast<int>(actualCellSize() * max_cell_x_to_draw);
    int min_y_to_draw = static_cast<int>(actualCellSize() * min_cell_y_to_draw);
    int max_y_to_draw = static_cast<int>(actualCellSize() * max_cell_y_to_draw);

    for (std::int32_t x = min_cell_x_to_draw; x < max_cell_x_to_draw; ++x)
    {
        int cur_x = static_cast<int>(actualCellSize() * x);
        painter.drawLine(cur_x, min_y_to_draw, cur_x, max_y_to_draw);
    }
    for (std::int32_t y = min_cell_y_to_draw; y < max_cell_y_to_draw; ++y)
    {
        int cur_y = static_cast<int>(actualCellSize() * y);
        painter.drawLine(min_x_to_draw, cur_y, max_x_to_draw, cur_y);
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

double BalanceMonitorWidget::actualCellSize() const
{
    const double scale = std::pow(ZOOM_BASE, INITIAL_ZOOM_STEP);
    return CELL_SIZE * std::pow(ZOOM_BASE, zoom_step) / scale;
}

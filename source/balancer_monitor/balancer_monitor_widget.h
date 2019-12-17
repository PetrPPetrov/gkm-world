// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QWidget>
#include <QResizeEvent>
#include "global_types.h"

class BalancerMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    BalancerMonitorWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    double view_point_x = 0.0;
    double view_point_y = 0.0;
    double previous_view_point_x = 0.0;
    double previous_view_point_y = 0.0;

    QPointF previous_point;
    bool left_mouse_pressed = false;

    double screen_center_x = 0.0;
    double screen_center_y = 0.0;

    int to_screen_x(double x) const; // We do not use worldTransformation property of QPainter
    int to_screen_y(double y) const;
    int to_screen_w(double w) const;
    int to_screen_h(double h) const;
    double zoom() const;
    double cached_zoom = 1.0;
    const double ZOOM_BASE = 1.1;
    const std::int32_t INITIAL_ZOOM_STEP = 20;
    const std::int32_t MIN_ZOOM_STEP = 5;
    const std::int32_t MAX_ZOOM_STEP = 60;
    std::int32_t zoom_step = INITIAL_ZOOM_STEP;
};

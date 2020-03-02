// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <QObject>
#include <QWidget>
#include <QWheelEvent>

typedef std::shared_ptr<QImage> ImagePtr;

class TextureWidget : public QWidget
{
    Q_OBJECT

public:
    TextureWidget(QWidget* parent, const ImagePtr& image);

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    bool first_show = true;
    ImagePtr image;

    double view_point_x = 0.0;
    double view_point_y = 0.0;
    double previous_view_point_x = 0.0;
    double previous_view_point_y = 0.0;
    double scale = 1.0;

    QPointF previous_point;
    bool left_mouse_pressed = false;
    bool right_mouse_pressed = false;

    double screen_center_x = 0.0;
    double screen_center_y = 0.0;

    double zoom() const;
    double cached_zoom = 1.0;
    const double ZOOM_BASE = 1.1;
    const std::int32_t INITIAL_ZOOM_STEP = 90;
    const std::int32_t MIN_ZOOM_STEP = 2;
    const std::int32_t MAX_ZOOM_STEP = 140;
    std::int32_t zoom_step = INITIAL_ZOOM_STEP;
};

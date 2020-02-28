// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <QObject>
#include <QWidget>

typedef std::shared_ptr<QImage> ImagePtr;

class TextureWidget : public QWidget
{
    Q_OBJECT

public:
    TextureWidget(QWidget* parent, const ImagePtr& image);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ImagePtr image;

    double view_point_x = 0.0;
    double view_point_y = 0.0;
    double previous_view_point_x = 0.0;
    double previous_view_point_y = 0.0;
    double scale_x = 1.0;
    double scale_y = 1.0;

    QPointF previous_point;
    bool left_mouse_pressed = false;

    double screen_center_x = 0.0;
    double screen_center_y = 0.0;
};

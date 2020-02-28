// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

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

void TextureWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    const QRect viewport = painter.viewport();
    int screen_center_x = viewport.center().x();
    int screen_center_y = viewport.center().y();

    int draw_x = screen_center_x - view_point_x;
    int draw_y = screen_center_y - view_point_y;

    painter.drawImage(QPoint(draw_x, draw_y), *image);
}

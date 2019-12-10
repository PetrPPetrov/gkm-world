// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QWidget>
#include <QResizeEvent>
#include "global_types.h"

class BalanceMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    BalanceMonitorWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    double actualCellSize() const;

private:
    CellIndex view_point;
    const double ZOOM_BASE = 1.1;
    const std::int32_t INITIAL_ZOOM_STEP = 20;
    const std::int32_t MIN_ZOOM_STEP = 13;
    const std::int32_t MAX_ZOOM_STEP = 60;
    std::int32_t zoom_step = INITIAL_ZOOM_STEP;
};

// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QWidget>
#include <QResizeEvent>

class BalanceMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    BalanceMonitorWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent* event) override;
};

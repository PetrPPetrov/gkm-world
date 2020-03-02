// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHeaderView>
#include <QPushButton>
#include "main_window.h"

extern MainWindow* g_main_window = nullptr;

MainWindow::MainWindow()
{
    g_main_window = this;
    main_window.setupUi(this);

    images.push_back(std::make_shared<QImage>(QString("texture.jpg")));
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;
        log_dock = new QDockWidget(tr("Log"), this);
        log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        log = new QPlainTextEdit(log_dock);
        log->setReadOnly(true);
        log->ensureCursorVisible();
        log->setCenterOnScroll(true);
        log_dock->setWidget(log);
        addDockWidget(Qt::BottomDockWidgetArea, log_dock);

        texture1_dock = new QDockWidget(tr("Texture 1"), this);
        texture1_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        texture1_view = new TextureWidget(texture1_dock, images[0]);
        texture1_view->setMinimumWidth(width() / 4);
        texture1_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        texture1_dock->setWidget(texture1_view);
        addDockWidget(Qt::LeftDockWidgetArea, texture1_dock);

        texture2_dock = new QDockWidget(tr("Texture 2"), this);
        texture2_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        texture2_view = new TextureWidget(texture2_dock, images[0]);
        texture2_view->setMinimumWidth(width() / 4);
        texture2_dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        texture2_dock->setWidget(texture2_view);
        addDockWidget(Qt::RightDockWidgetArea, texture2_dock);
    }
}

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

    open_gl_view = new MeshBuilderWidget(main_window.centralwidget);
    open_gl_window = main_window.centralwidget->addSubWindow(open_gl_view);

    log_view = new QPlainTextEdit(main_window.centralwidget);
    log_view->setReadOnly(true);
    log_view->ensureCursorVisible();
    log_view->setCenterOnScroll(true);
    log_window = main_window.centralwidget->addSubWindow(log_view);

    texture1_view = new TextureWidget(main_window.centralwidget, images[0]);
    texture1_window = main_window.centralwidget->addSubWindow(texture1_view);

    texture2_view = new TextureWidget(main_window.centralwidget, images[0]);
    texture2_window = main_window.centralwidget->addSubWindow(texture2_view);
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;
        open_gl_window->resize(QSize(main_window.centralwidget->width() / 2, main_window.centralwidget->height()));
        open_gl_window->move(main_window.centralwidget->width() / 2, 0);
        texture1_window->resize(QSize(main_window.centralwidget->width() / 2, main_window.centralwidget->height() / 2));
        texture1_window->move(0, 0);
        texture1_view->setDefaultSize(main_window.centralwidget->width() / 2, main_window.centralwidget->height() / 2);
        texture2_window->resize(QSize(main_window.centralwidget->width() / 2, main_window.centralwidget->height() / 2));
        texture2_window->move(0, main_window.centralwidget->height() / 2);
        texture2_view->setDefaultSize(main_window.centralwidget->width() / 2, main_window.centralwidget->height() / 2);
    }
}

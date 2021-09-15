// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <boost/asio/impl/src.hpp>
#include <QDesktopWidget>
#include <QSplashScreen>
#include <QApplication>
#include <QStringList>
#include <QTranslator>
#include <QSettings>
#include <QLocale>
#include <QDebug>
#include <QtGlobal>
#include "main_monitor_window.h"

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    MainMonitorWindow main_monitor_window;

    // Center main window on desktop
    QDesktopWidget screen;
    QRect screen_rect = screen.screenGeometry(&main_monitor_window);
    QPoint position((screen_rect.width() - main_monitor_window.width()) / 2, (screen_rect.height() - main_monitor_window.height()) / 2);
    main_monitor_window.move(position);
    main_monitor_window.show();
    return application.exec();
}

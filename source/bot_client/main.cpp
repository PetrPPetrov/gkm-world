// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

// It must be before of <Windows.h>
#include "udp_connection.h"

#include <iostream>
#include <functional>
#include <cstdlib>
#include <fstream>
#include <csignal>
#include <windows.h>
#include <ctime>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/uuid/random_generator.hpp>
#include "Eigen/Eigen"
#include "log.h"
#include "config.h"
#include "logic.h"

UDPConnection* g_connection = nullptr;
KeyboardState g_keyboard_state;
PlayerLocation g_player_location;
bool g_is_running = true;
extern Log::Logger* g_logger = nullptr;

void printCurrentPlayerState(double target_x_pos, double target_y_pos)
{
#ifdef DEBUG_MONITOR
    COORD xy;
    xy.X = 0;
    xy.Y = 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);

    std::cout << LINE_SEPARATOR << std::endl;
    std::cout << "target_x_pos: " << target_x_pos << std::endl;
    std::cout << "target_y_pos: " << target_y_pos << std::endl;
    std::cout << LINE_SEPARATOR << std::endl;
    std::cout << "x_pos: " << std::fixed << std::setw(10) << std::setprecision(3) << g_player_location.x_pos << std::endl;
    std::cout << "y_pos: " << g_player_location.y_pos << std::endl;
    std::cout << "direction: " << g_player_location.direction << std::endl;
    std::cout << LINE_SEPARATOR << std::endl;
#endif
}

void moveToTarget(UDPConnection& connection, double target_x_pos, double target_y_pos)
{
    double distance_to_target = 10.0;
    while (distance_to_target > 5.0)
    { 
        Eigen::Vector2d to_target_vector(target_x_pos - g_player_location.x_pos, target_y_pos - g_player_location.y_pos);
        distance_to_target = to_target_vector.norm();
        to_target_vector.normalize();
        double target_angle = acos(to_target_vector.x());
        double target_angle_y = asin(to_target_vector.y());
        if (target_angle_y < 0.0)
        {
            target_angle = -target_angle;
        }
        target_angle *= RAD_TO_GRAD;
        target_angle = 90.0 - target_angle;
        target_angle = normalizeDirection(target_angle);
        double delta_angle = target_angle - g_player_location.direction;
        delta_angle = normalizeDelta(delta_angle);
        if (delta_angle > 0.0)
        {
            g_keyboard_state.right = true;
        }
        else if (delta_angle < 0.0)
        {
            g_keyboard_state.left = true;
        }

        if (fabs(delta_angle) < 90.0)
        {
            g_keyboard_state.up = true;
        }

        connection.sendPlayerState(g_keyboard_state, g_player_location);
        printCurrentPlayerState(target_x_pos, target_y_pos);

        g_keyboard_state.up = false;
        g_keyboard_state.right = false;
        g_keyboard_state.left = false;
    }

    connection.sendPlayerState(g_keyboard_state, g_player_location);
}

void program(int argc, char* argv[])
{
    std::string configuration_file_name = "bot_client.cfg";
    if (argc >= 2)
    {
        configuration_file_name = argv[1];
    }

    std::string server_ip_address = "127.0.0.1";
    std::uint16_t server_port_number = 17012;
    std::list<double> control_point_x_positions;
    std::list<double> control_point_y_positions;
    g_player_location.x_pos = 5000;
    g_player_location.y_pos = 5000;
    g_player_location.direction = 0;

    std::ifstream config_file(configuration_file_name);
    ConfigurationReader config_reader;
    config_reader.addParameter("server_ip_address", server_ip_address);
    config_reader.addParameter("server_port_number", server_port_number);
    config_reader.addParameter("bot_start_x_pos", g_player_location.x_pos);
    config_reader.addParameter("bot_start_y_pos", g_player_location.y_pos);
    config_reader.addParameter("bot_start_direction", g_player_location.direction);
    config_reader.addParameter("control_point_x_pos", control_point_x_positions);
    config_reader.addParameter("control_point_y_pos", control_point_y_positions);
    config_reader.read(config_file);

    if (control_point_x_positions.empty())
    {
        LOG_FATAL << "control_point_x_positions array is empty";
        throw std::runtime_error("control_point_x_positions array is empty");
    }
    if (control_point_y_positions.empty())
    {
        LOG_FATAL << "control_point_y_positions array is empty";
        throw std::runtime_error("control_point_y_positions array is empty");
    }

    UDPConnection connection(server_ip_address, server_port_number);
    g_connection = &connection;

    // Login on the server
    boost::uuids::random_generator generator;
    std::string random_login = to_string(generator());
    std::string password = "difficult_password";
    std::string full_name = "bot tester";
    connection.login(random_login, password, full_name);

    // Initialize position on the server
    connection.initializePosition(g_player_location);

    std::signal(SIGINT, [](int signum)
    {
        g_is_running = false;
        g_connection->logout();
        exit(EXIT_FAILURE);
    });

    std::list<double>::const_iterator x_pos = control_point_x_positions.begin();
    std::list<double>::const_iterator y_pos = control_point_y_positions.begin();
    while (g_is_running)
    {
        moveToTarget(connection, *x_pos, *y_pos);
        x_pos++;
        y_pos++;
        if (x_pos == control_point_x_positions.end())
        {
            x_pos = control_point_x_positions.begin();
        }
        if (y_pos == control_point_y_positions.end())
        {
            y_pos = control_point_y_positions.begin();
        }
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Gkm-World Bot Client Copyright (c) 2018 by Petr Petrovich Petrov" << std::endl;

    Log::Holder log_holder;
    std::string log_file_name("bot_client_" + std::to_string(GetCurrentProcessId()) + ".log");
    g_logger = new Log::Logger(Packet::ESeverityType::DebugMessage, log_file_name, false, true);
    LOG_INFO << "Bot Client is starting...";

    try
    {
        program(argc, argv);
    }
    catch (boost::system::system_error& error)
    {
        LOG_FATAL << "boost::system::system_error: " << error.what();
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        LOG_FATAL << "std::exception: " << exception.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_FATAL << "Unknown error";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

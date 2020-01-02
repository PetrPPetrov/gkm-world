// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <cstdint>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <boost/polygon/polygon.hpp>
#include <QPainter>
#include <QApplication>
#include <QStatusBar>
#include "main_monitor_window.h"
#include "balancer_monitor_widget.h"

BalancerMonitorWidget::BalancerMonitorWidget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    cached_zoom = zoom();
}

void BalancerMonitorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    const QRect viewport = painter.viewport();
    screen_center_x = viewport.center().x();
    screen_center_y = viewport.center().y();

    const double world_to_draw_width = viewport.width() / cached_zoom;
    const double world_to_draw_height = viewport.height() / cached_zoom;
    const double actual_cell_size = CELL_SIZE * cached_zoom;
    bool draw_cell_lines = true;
    if (actual_cell_size < 8.0)
    {
        draw_cell_lines = false;
    }
    bool print_neighbors = true;
    if (actual_cell_size < 50.0)
    {
        print_neighbors = false;
    }

    const std::int32_t cells_to_draw_width = static_cast<std::int32_t>(world_to_draw_width / CELL_SIZE) + 1;
    const std::int32_t cells_to_draw_height = static_cast<std::int32_t>(world_to_draw_height / CELL_SIZE) + 1;
    const std::int32_t cur_cell_x = static_cast<std::int32_t>(view_point_x / CELL_SIZE);
    const std::int32_t cur_cell_y = static_cast<std::int32_t>(view_point_y / CELL_SIZE);
    const std::int32_t start_cell_x = cur_cell_x - cells_to_draw_width / 2 - 1;
    const std::int32_t start_cell_y = cur_cell_y - cells_to_draw_height / 2 - 1;
    const std::int32_t end_cell_x = cur_cell_x + cells_to_draw_width / 2 + 1;
    const std::int32_t end_cell_y = cur_cell_y + cells_to_draw_height / 2 + 1;

    auto server_info = g_main_window->getServerInfo();
    if (server_info)
    {
        typedef boost::polygon::point_data<std::int32_t> Point;
        typedef boost::polygon::rectangle_data<std::int32_t> Rectangle;
        typedef boost::polygon::polygon_90_data<std::int32_t> Polygon90;
        typedef boost::polygon::polygon_90_set_data<std::int32_t> Polygon90Set;
        using namespace boost::polygon::operators;

        Polygon90Set result_outside_polygon;

        Rectangle outer_rectangle(start_cell_x, start_cell_y, end_cell_x + 1, end_cell_y + 1);
        result_outside_polygon += outer_rectangle;

        SquareCell global_box = server_info->bounding_box;
        Rectangle global_box_rectangle(global_box.start.x, global_box.start.y, global_box.start.x + global_box.size, global_box.start.y + global_box.size);
        result_outside_polygon -= global_box_rectangle;

        QColor out_of_global_box(150, 0, 0);
        QBrush out_of_global_box_brush(out_of_global_box, Qt::BDiagPattern);

        std::vector<Rectangle> rectangles;
        result_outside_polygon.get(rectangles);
        const size_t rectangle_count = rectangles.size();
        for (size_t i = 0; i < rectangle_count; ++i)
        {
            Rectangle& rectangle = rectangles[i];
            painter.fillRect(
                to_screen_x(xl(rectangle) * CELL_SIZE),
                to_screen_y(yl(rectangle) * CELL_SIZE),
                to_screen_w((xh(rectangle) - xl(rectangle)) * CELL_SIZE),
                to_screen_h((yh(rectangle) - yl(rectangle)) * CELL_SIZE),
                out_of_global_box_brush
            );
        }

        if (g_main_window->isShowLeafNodes())
        {
            BalancerTreeInfo* cur_node = server_info->token_to_tree_node.find(server_info->tree_root_token);
            std::list<BalancerTreeInfo*> parent_stack;
            parent_stack.push_back(cur_node);
            while (!parent_stack.empty())
            {
                cur_node = parent_stack.back();
                parent_stack.pop_back();
                if (cur_node)
                {
                    if (cur_node->leaf_node)
                    {
                        QBrush inside_node_brush(getColor(cur_node->token), Qt::SolidPattern);
                        Polygon90Set result_polygon;
                        SquareCell node_box = cur_node->bounding_box;
                        Rectangle node_box_rectangle(node_box.start.x, node_box.start.y, node_box.start.x + node_box.size, node_box.start.y + node_box.size);
                        result_polygon += node_box_rectangle;
                        result_polygon *= outer_rectangle;

                        rectangles.clear();
                        result_polygon.get(rectangles);
                        const size_t rectangle_count = rectangles.size();
                        for (size_t i = 0; i < rectangle_count; ++i)
                        {
                            Rectangle& rectangle = rectangles[i];
                            painter.fillRect(
                                to_screen_x(xl(rectangle) * CELL_SIZE),
                                to_screen_y(yl(rectangle) * CELL_SIZE),
                                to_screen_w((xh(rectangle) - xl(rectangle)) * CELL_SIZE),
                                to_screen_h((yh(rectangle) - yl(rectangle)) * CELL_SIZE),
                                inside_node_brush
                            );
                            painter.drawText(
                                to_screen_x(xl(rectangle) * CELL_SIZE),
                                to_screen_y(yl(rectangle) * CELL_SIZE),
                                to_screen_w((xh(rectangle) - xl(rectangle)) * CELL_SIZE),
                                to_screen_h((yh(rectangle) - yl(rectangle)) * CELL_SIZE),
                                Qt::AlignCenter,
                                tr("Node Server %1").arg(cur_node->token)
                            );
                        }
                    }
                    else
                    {
                        for (std::uint8_t i = ChildFirst; i < ChildLast; ++i)
                        {
                            std::uint32_t child_token = cur_node->children[i];
                            if (child_token)
                            {
                                parent_stack.push_back(server_info->token_to_tree_node.find(child_token));
                            }
                        }
                    }
                }
            }
        }

        if (g_main_window->isShowSelectedNode() && server_info->selected_node)
        {
            Polygon90Set result_selection_polygon;
            SquareCell selection_box = server_info->selected_node->bounding_box;
            Rectangle selection_box_rectangle(selection_box.start.x, selection_box.start.y, selection_box.start.x + selection_box.size, selection_box.start.y + selection_box.size);
            result_selection_polygon += selection_box_rectangle;
            result_selection_polygon *= outer_rectangle;

            QColor inside_selected_node(0, 250, 0);
            QBrush inside_selected_node_brush(inside_selected_node, Qt::DiagCrossPattern);

            rectangles.clear();
            result_selection_polygon.get(rectangles);
            const size_t rectangle_count = rectangles.size();
            for (size_t i = 0; i < rectangle_count; ++i)
            {
                Rectangle& rectangle = rectangles[i];
                painter.fillRect(
                    to_screen_x(xl(rectangle) * CELL_SIZE),
                    to_screen_y(yl(rectangle) * CELL_SIZE),
                    to_screen_w((xh(rectangle) - xl(rectangle)) * CELL_SIZE),
                    to_screen_h((yh(rectangle) - yl(rectangle)) * CELL_SIZE),
                    inside_selected_node_brush
                );
            }

            if (print_neighbors && g_main_window->isShowNeighbor())
            {
                for (std::int32_t x = start_cell_x; x <= end_cell_x; ++x)
                {
                    for (std::int32_t y = start_cell_y; y <= end_cell_y; ++y)
                    {
                        std::pair<std::int32_t, std::int32_t> coordinate(x, y);
                        auto find_neighbor = server_info->selected_node->neighbor_nodes.find(coordinate);
                        if (find_neighbor != server_info->selected_node->neighbor_nodes.end())
                        {
                            painter.drawText(
                                to_screen_x(x * CELL_SIZE),
                                to_screen_y(y * CELL_SIZE),
                                to_screen_w(CELL_SIZE),
                                to_screen_h(CELL_SIZE),
                                Qt::AlignCenter,
                                tr("Neighbor %1").arg(find_neighbor->second)
                            );
                        }
                    }
                }
            }
        }
    }

    if (draw_cell_lines)
    {
        QPen thin_pen;
        thin_pen.setColor(QColor(0, 0, 0));
        thin_pen.setStyle(Qt::DotLine);
        painter.setPen(thin_pen);

        for (std::int32_t x = start_cell_x; x <= end_cell_x; ++x)
        {
            painter.drawLine(
                to_screen_x(x * CELL_SIZE), to_screen_y(start_cell_y * CELL_SIZE),
                to_screen_x(x * CELL_SIZE), to_screen_y((end_cell_y + 1) * CELL_SIZE)
            );
        }
        for (std::int32_t y = start_cell_y; y <= end_cell_y; ++y)
        {
            painter.drawLine(
                to_screen_x(start_cell_x * CELL_SIZE), to_screen_y(y * CELL_SIZE),
                to_screen_x((end_cell_x + 1) * CELL_SIZE), to_screen_y(y * CELL_SIZE)
            );
        }
    }
}

void BalancerMonitorWidget::wheelEvent(QWheelEvent* event)
{
    zoom_step += (event->angleDelta().y() / 120);
    if (zoom_step < MIN_ZOOM_STEP)
    {
        zoom_step = MIN_ZOOM_STEP;
    }
    if (zoom_step > MAX_ZOOM_STEP)
    {
        zoom_step = MAX_ZOOM_STEP;
    }
    cached_zoom = zoom();
    update();
}

void BalancerMonitorWidget::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF current_point = event->localPos();

    if (left_mouse_pressed)
    {
        const QPointF delta = previous_point - current_point;
        const double actual_zoom = zoom();
        view_point_x = previous_view_point_x + delta.x() / actual_zoom;
        view_point_y = previous_view_point_y - delta.y() / actual_zoom;
        update();
    }

    const QPoint center = rect().center();
    const QPoint delta = current_point.toPoint() - center;
    const double world_delta_x = delta.x() / cached_zoom;
    const double world_delta_y = delta.y() / cached_zoom;
    const double gkm_world_position_x = view_point_x + world_delta_x;
    const double gkm_world_position_y = view_point_y - world_delta_y;

    QString mouse_position_in_status = tr("Gkm-World Position: ");
    std::stringstream mouse_position;
    mouse_position << std::fixed << std::setprecision(2) << gkm_world_position_x << "; " << gkm_world_position_y;
    mouse_position_in_status += mouse_position.str().c_str();
    g_main_window->statusBar()->showMessage(mouse_position_in_status);
}

void BalancerMonitorWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        left_mouse_pressed = true;
        previous_point = event->localPos();
        previous_view_point_x = view_point_x;
        previous_view_point_y = view_point_y;
    }
}

void BalancerMonitorWidget::mouseReleaseEvent(QMouseEvent* event)
{
    left_mouse_pressed = false;
    previous_point = event->localPos();
    previous_view_point_x = view_point_x;
    previous_view_point_y = view_point_y;
}

int BalancerMonitorWidget::to_screen_x(double x) const
{
    return static_cast<int>((x - view_point_x) * cached_zoom + screen_center_x);
}

int BalancerMonitorWidget::to_screen_y(double y) const
{
    return static_cast<int>((y - view_point_y) * -cached_zoom + screen_center_y);
}

int BalancerMonitorWidget::to_screen_w(double w) const
{
    return static_cast<int>(w * cached_zoom);
}

int BalancerMonitorWidget::to_screen_h(double h) const
{
    return static_cast<int>(h * -cached_zoom);
}

double BalancerMonitorWidget::zoom() const
{
    const double scale = std::pow(ZOOM_BASE, INITIAL_ZOOM_STEP);
    return std::pow(ZOOM_BASE, zoom_step) / scale;
}

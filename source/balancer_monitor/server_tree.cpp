// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QString>
#include <QPixmap>
#include "server_tree.h"

TreeItem::~TreeItem()
{
}

TreeItem::TreeItem(const QString& property_value) : TreeItem(property_value, nullptr)
{
}

TreeItem::TreeItem(const QString& name, const QString& value) : parent_item(nullptr)
{
    item_data.push_back(name);
    item_data.push_back(value);
}

TreeItem::TreeItem(const QString& property_value, TreeItem* parent) : parent_item(parent)
{
    item_data.push_back(property_value);
}

TreeItem* TreeItem::child(int row)
{
    if (row < 0 || row >= children.size())
        return nullptr;
    return children.at(row).get();
}

int TreeItem::childCount() const
{
    return static_cast<int>(children.size());
}

int TreeItem::columnCount() const
{
    return static_cast<int>(item_data.size());
}

QVariant TreeItem::data(int column) const
{
    if (column < 0 || column >= item_data.size())
        return QVariant();
    return item_data.at(column);
}

int TreeItem::row() const
{
    if (parent_item)
    {
        for (int i = 0; i < parent_item->children.size(); ++i)
        {
            if (parent_item->children.at(i).get() == const_cast<TreeItem*>(this))
            {
                return i;
            }
        }
    }

    return 0;
}

TreeItem* TreeItem::parentItem()
{
    return parent_item;
}

BalancerItem::BalancerItem(const ServerInfo::Ptr& server_info)
    : TreeItem(QString("Balancer Server"))
{
    if (server_info->tree_root_token)
    {
        BalancerTreeInfo* root_node = server_info->token_to_tree_node.find(server_info->tree_root_token);
        if (root_node)
        {
            children.push_back(std::make_shared<NodeItem>(server_info, root_node, this));
        }
    }
}

NodeItem::NodeItem(const ServerInfo::Ptr& server_info, BalancerTreeInfo* node_, TreeItem* parent)
    : TreeItem(QString(node_->leaf_node ? "Node Server %1" : "Group Node %1").arg(node_->token), parent), node(node_)
{
    for (auto child_token : node->children)
    {
        if (child_token)
        {
            BalancerTreeInfo* child_node = server_info->token_to_tree_node.find(child_token);
            if (child_node)
            {
                children.push_back(std::make_shared<NodeItem>(server_info, child_node, this));
            }
        }
    }
}

BalancerTreeInfo* NodeItem::getNode() const
{
    return node;
}

ProxyItem::ProxyItem(std::uint32_t proxy_index_)
    : TreeItem(QString("Proxy Server %1").arg(proxy_index_))
{
    proxy_index = proxy_index_;
}

std::uint32_t ProxyItem::getProxyIndex() const
{
    return proxy_index;
}

std::vector<TreeItem::Ptr> buildServerHierarchy(const ServerInfo::Ptr& server_info)
{
    std::vector<TreeItem::Ptr> result;
    if (server_info->tree_root_token)
    {
        BalancerItem::Ptr balancer_server = std::make_shared<BalancerItem>(server_info);
        result.push_back(balancer_server);
    }
    for (auto proxy_server_info : server_info->id_to_proxy)
    {
        ProxyItem::Ptr proxy_server = std::make_shared<ProxyItem>(proxy_server_info.first);
        result.push_back(proxy_server);
    }
    return result;
}

std::vector<TreeItem::Ptr> buildNodePropertyList(BalancerTreeInfo* node)
{
    std::vector<TreeItem::Ptr> result;
    if (node->leaf_node)
    {
        result.push_back(std::make_shared<TreeItem>(QString("type"), QString("node server")));
    }
    else
    {
        result.push_back(std::make_shared<TreeItem>(QString("type"), QString("group node")));
    }
    result.push_back(std::make_shared<TreeItem>(QString("token"), QString("%1").arg(node->token)));
    result.push_back(std::make_shared<TreeItem>(QString("level"), QString("%1").arg(node->level)));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.x"), QString("%1").arg(node->bounding_box.start.x)));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.y"), QString("%1").arg(node->bounding_box.start.y)));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.size"), QString("%1").arg(node->bounding_box.size)));
    result.push_back(std::make_shared<TreeItem>(QString("user_count"), QString("%1").arg(node->user_count)));
    if (!node->leaf_node)
    {
        for (unsigned char i = ChildFirst; i < ChildLast; ++i)
        {
            result.push_back(std::make_shared<TreeItem>(QString("child %1").arg(i), QString("%1").arg(node->children[i])));
        }
    }
    else
    {
        if (node->node_server_port_number == 0)
        {
            result.push_back(std::make_shared<TreeItem>(QString("is_running"), QString("false")));
        }
        else
        {
            result.push_back(std::make_shared<TreeItem>(QString("is_running"), QString("true")));
        }
    }
    return result;
}

std::vector<TreeItem::Ptr> buildBalancerPropertyList(const ServerInfo::Ptr& server_info, MonitorUDPConnection* connection)
{
    std::vector<TreeItem::Ptr> result;
    result.push_back(std::make_shared<TreeItem>(QString("type"), QString("balancer server")));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.x"), QString("%1").arg(server_info->bounding_box.start.x)));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.y"), QString("%1").arg(server_info->bounding_box.start.y)));
    result.push_back(std::make_shared<TreeItem>(QString("bbox.start.size"), QString("%1").arg(server_info->bounding_box.size)));
    result.push_back(std::make_shared<TreeItem>(QString("root node token"), QString("%1").arg(server_info->tree_root_token)));
    result.push_back(std::make_shared<TreeItem>(QString("ip_address"), QString("%1").arg(connection->getBalancerIpAddress())));
    result.push_back(std::make_shared<TreeItem>(QString("port_number"), QString("%1").arg(connection->getBalancerPortNumber())));
    return result;
}

std::vector<TreeItem::Ptr> buildProxyPropertyList(const ServerInfo::Ptr& server_info, std::uint32_t proxy_index)
{
    auto find_it = server_info->id_to_proxy.find(proxy_index);
    if (find_it != server_info->id_to_proxy.end())
    {
        std::vector<TreeItem::Ptr> result;
        result.push_back(std::make_shared<TreeItem>(QString("type"), QString("proxy server")));
        result.push_back(std::make_shared<TreeItem>(QString("proxy.index"), QString("%1").arg(proxy_index)));
        result.push_back(std::make_shared<TreeItem>(QString("ip_address"), QString("%1").arg(find_it->second.ip_address.c_str())));
        result.push_back(std::make_shared<TreeItem>(QString("port_number"), QString("%1").arg(find_it->second.port_number)));
        return result;
    }
    std::vector<TreeItem::Ptr> result;
    result.push_back(std::make_shared<TreeItem>(QString("type"), QString("proxy server")));
    result.push_back(std::make_shared<TreeItem>(QString("proxy.index"), QString("%1").arg(proxy_index)));
    result.push_back(std::make_shared<TreeItem>(QString("status"), QString("not found")));
    return result;
}

TreeModel::TreeModel(const std::vector<TreeItem::Ptr>& items_, QObject* parent)
    : QAbstractItemModel(parent), items(items_)
{
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::DecorationRole)
    {
        NodeItem* node_item = dynamic_cast<NodeItem*>(item);
        if (node_item)
        {
            BalancerTreeInfo* tree_info = node_item->getNode();
            if (tree_info && tree_info->leaf_node)
            {
                QColor color = getColor(tree_info->token);
                QPixmap icon(16, 16);
                icon.fill(color);
                return icon;
            }
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return false;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& index) const
{
    if (!hasIndex(row, column, index))
    {
        return QModelIndex();
    }
    TreeItem* item = nullptr;
    if (index.isValid())
    {
        item = static_cast<TreeItem*>(index.internalPointer())->child(row);
    }
    else
    {
        if (row >= 0 && row < items.size())
        {
            item = items[row].get();
        }
    }
    if (item)
    {
        return createIndex(row, column, item);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem* current_item = static_cast<TreeItem*>(index.internalPointer());
    if (current_item->parentItem())
    {
        return createIndex(current_item->row(), 0, current_item->parentItem());
    }
    else
    {
        return QModelIndex(); // Return invalid index for the root nodes
    }
}

int TreeModel::rowCount(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<TreeItem*>(index.internalPointer())->childCount();
    }
    else
    {
        return static_cast<int>(items.size());
    }
}

int TreeModel::columnCount(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<TreeItem*>(index.internalPointer())->columnCount();
    }
    else
    {
        if (items.size() > 0)
        {
            return items[0]->columnCount();
        }
        else
        {
            return 0;
        }
    }
}

ListModel::ListModel(const std::vector<TreeItem::Ptr>& items_, QObject* parent)
    : QAbstractItemModel(parent), items(items_)
{
}

QVariant ListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    return false;
}

Qt::ItemFlags ListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QModelIndex ListModel::index(int row, int column, const QModelIndex& index) const
{
    if (!hasIndex(row, column, index))
    {
        return QModelIndex();
    }
    if (row >= 0 && row < items.size())
    {
        TreeItem* item = items[row].get();
        return createIndex(row, column, item);
    }
    return QModelIndex();
}

QModelIndex ListModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int ListModel::rowCount(const QModelIndex& index) const
{
    return static_cast<int>(items.size());
}

int ListModel::columnCount(const QModelIndex& index) const
{
    return 2;
}

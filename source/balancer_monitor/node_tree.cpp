// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QString>
#include "node_tree.h"

NodeTreeItem::NodeTreeItem(const BalancerServerInfo::Ptr& server_info)
    : parent_item(nullptr), node(nullptr)
{
    QString node_name = QString("Balancer Server");
    item_data.push_back(node_name);
    if (server_info->tree_root_token)
    {
        BalancerTreeInfo* root_node = server_info->token_to_tree_node.find(server_info->tree_root_token);
        if (root_node)
        {
            children.push_back(std::make_shared<NodeTreeItem>(server_info, root_node, this));
        }
    }
}

NodeTreeItem::NodeTreeItem(const BalancerServerInfo::Ptr& server_info, BalancerTreeInfo* node_, NodeTreeItem* parent)
    : parent_item(parent), node(node_)
{
    QString node_name = QString("Node %1").arg(node->token);
    item_data.push_back(node_name);
    for (auto child_token : node->children)
    {
        if (child_token)
        {
            BalancerTreeInfo* child_node = server_info->token_to_tree_node.find(child_token);
            if (child_node)
            {
                children.push_back(std::make_shared<NodeTreeItem>(server_info, child_node, this));
            }
        }
    }
}

NodeTreeItem* NodeTreeItem::child(int row)
{
    if (row < 0 || row >= children.size())
        return nullptr;
    return children.at(row).get();
}

int NodeTreeItem::childCount() const
{
    return static_cast<int>(children.size());
}

int NodeTreeItem::columnCount() const
{
    return static_cast<int>(item_data.size());
}

QVariant NodeTreeItem::data(int column) const
{
    if (column < 0 || column >= item_data.size())
        return QVariant();
    return item_data.at(column);
}

int NodeTreeItem::row() const
{
    if (parent_item)
    {
        for (int i = 0; i < parent_item->children.size(); ++i)
        {
            if (parent_item->children.at(i).get() == const_cast<NodeTreeItem*>(this))
            {
                return i;
            }
        }
    }

    return 0;
}

NodeTreeItem* NodeTreeItem::parentItem()
{
    return parent_item;
}

NodeTree::NodeTree(const BalancerServerInfo::Ptr& server_info_, QObject* parent)
    : QAbstractItemModel(parent), server_info(server_info_)
{
    root_item = std::make_shared<NodeTreeItem>(server_info);
}

QVariant NodeTree::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    NodeTreeItem* item = static_cast<NodeTreeItem*>(index.internalPointer());

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

bool NodeTree::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::CheckStateRole)
    {
        return false;
    }

    NodeTreeItem* item = static_cast<NodeTreeItem*>(index.internalPointer());
    bool checked = static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked;
    for (int child_number = 0; child_number < rowCount(index); ++child_number)
    {
        QModelIndex child_index = index.child(child_number, 0);
        setData(child_index, value, Qt::CheckStateRole);
    }
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags NodeTree::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QModelIndex NodeTree::index(int row, int column, const QModelIndex& index) const
{
    if (!hasIndex(row, column, index))
    {
        return QModelIndex();
    }
    NodeTreeItem* item = root_item.get();
    if (index.isValid())
    {
        item = static_cast<NodeTreeItem*>(index.internalPointer())->child(row);
    }
    return createIndex(row, column, item);
}

QModelIndex NodeTree::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    NodeTreeItem* current_item = static_cast<NodeTreeItem*>(index.internalPointer());
    if (current_item == root_item.get())
        return QModelIndex(); // Return invalid index for the root node

    return createIndex(current_item->row(), 0, current_item->parentItem());
}

int NodeTree::rowCount(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<NodeTreeItem*>(index.internalPointer())->childCount();
    }
    else
    {
        return 1; // We have only 1 root node, it is "root_item" field
    }
}

int NodeTree::columnCount(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return static_cast<NodeTreeItem*>(index.internalPointer())->columnCount();
    }
    else
    {
        return root_item->columnCount();
    }
}

// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <vector>
#include <QVariant>
#include <QVector>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "balancer_server_info.h"

class NodeTreeItem
{
public:
    typedef std::shared_ptr<NodeTreeItem> Ptr;

    explicit NodeTreeItem(const BalancerServerInfo::Ptr& server_info);
    explicit NodeTreeItem(const BalancerServerInfo::Ptr& server_info, BalancerTreeInfo* node, NodeTreeItem* parent);

    NodeTreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    NodeTreeItem* parentItem();

private:
    BalancerTreeInfo* node;
    std::vector<Ptr> children;
    std::vector<QVariant> item_data;
    NodeTreeItem* parent_item;
};

class NodeTree : public QAbstractItemModel
{
    Q_OBJECT

public:
    typedef std::shared_ptr<NodeTree> Ptr;

    explicit NodeTree(const BalancerServerInfo::Ptr& server_info, QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& item = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;

private:
    NodeTreeItem::Ptr root_item;
    BalancerServerInfo::Ptr server_info;
};

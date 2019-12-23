// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <vector>
#include <QVariant>
#include <QVector>
#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QVariant>
#include "balancer_server_info.h"

class TreeItem
{
public:
    typedef std::shared_ptr<TreeItem> Ptr;

    virtual ~TreeItem();
    explicit TreeItem(const QString& property_value);
    explicit TreeItem(const QString& name, const QString& value);
    explicit TreeItem(const QString& property_value, TreeItem* parent);

    TreeItem* child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeItem* parentItem();

protected:
    std::vector<Ptr> children;
    std::vector<QVariant> item_data;
    TreeItem* parent_item;
};

class NodeTreeItem : public TreeItem
{
public:
    typedef std::shared_ptr<NodeTreeItem> Ptr;

    explicit NodeTreeItem(const BalancerServerInfo::Ptr& server_info);
    explicit NodeTreeItem(const BalancerServerInfo::Ptr& server_info, BalancerTreeInfo* node, NodeTreeItem* parent);

    BalancerTreeInfo* getNode() const;

private:
    BalancerTreeInfo* node;
};

std::vector<TreeItem::Ptr> getPropertyList(BalancerTreeInfo* node);
std::vector<TreeItem::Ptr> getPropertyList(const BalancerServerInfo::Ptr& server_info);

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    typedef std::shared_ptr<TreeModel> Ptr;

    explicit TreeModel(const TreeItem::Ptr& root_item, QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;

private:
    TreeItem::Ptr root_item;
};

class ListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    typedef std::shared_ptr<ListModel> Ptr;

    explicit ListModel(const std::vector<TreeItem::Ptr>& items, QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& index = QModelIndex()) const override;
    int columnCount(const QModelIndex& index = QModelIndex()) const override;

private:
    std::vector<TreeItem::Ptr> items;
};

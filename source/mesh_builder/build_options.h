// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include "mesh_project.h"

class BuildOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    BuildOptionsDialog(const MeshProject::Ptr& mesh_project, QWidget* parent);

private:
    void loadMaxTextureSize();
    void saveMaxTextureSize();

    void onResetToDefault();
    void onOk();
    void onCancel();

private:
    QSpinBox* protection_offset = nullptr;
    QSpinBox* tolerance_divider = nullptr;
    QSpinBox* rotation_count = nullptr;
    QSpinBox* population_size = nullptr;
    QSpinBox* generation_count = nullptr;
    QSpinBox* mutation_rate = nullptr;
    QComboBox* max_texture_size = nullptr;

    QPushButton* reset_button = nullptr;
    QPushButton* ok_button = nullptr;
    QPushButton* cancel_button = nullptr;

    MeshProject::Ptr mesh_project;
};

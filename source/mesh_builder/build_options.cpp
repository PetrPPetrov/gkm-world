// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include "build_options.h"
#include "main_window.h"

BuildOptionsDialog::BuildOptionsDialog(const MeshProject::Ptr& mesh_project_, QWidget* parent) : QDialog(parent), mesh_project(mesh_project_)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Build Options");
    setModal(true);

    QGridLayout* layout = new QGridLayout(this);

    QGroupBox* nesting_options = new QGroupBox("Nesting Parameters");
    QGridLayout* nesting_layout = new QGridLayout(nesting_options);

    QLabel* protection_offset_label = new QLabel(this);
    protection_offset_label->setText("Protection Offset");
    nesting_layout->addWidget(protection_offset_label, 0, 0);

    protection_offset = new QSpinBox(this);
    protection_offset->setMinimum(0);
    protection_offset->setMaximum(64);
    protection_offset->setSuffix("pixel(s)");
    protection_offset->setValue(mesh_project->protection_offset);
    nesting_layout->addWidget(protection_offset, 0, 1);

    nesting_options->setLayout(nesting_layout);
    layout->addWidget(nesting_options, 0, 0);

    QGroupBox* genetic_nesting_options = new QGroupBox("Genetic Nesting Optimization Parameters");
    QGridLayout* genetic_layout = new QGridLayout(genetic_nesting_options);

    QLabel* tolerance_divider_label = new QLabel(this);
    tolerance_divider_label->setText("Tex. Coord Tolerance 1/");
    genetic_layout->addWidget(tolerance_divider_label, 0, 0);

    tolerance_divider = new QSpinBox(this);
    tolerance_divider->setMinimum(1);
    tolerance_divider->setMaximum(4096);
    tolerance_divider->setValue(mesh_project->scale);
    genetic_layout->addWidget(tolerance_divider, 0, 1);

    QLabel* rotation_count_label = new QLabel(this);
    rotation_count_label->setText("Rotation Count");
    genetic_layout->addWidget(rotation_count_label, 1, 0);

    rotation_count = new QSpinBox(this);
    rotation_count->setMinimum(1);
    rotation_count->setMaximum(64);
    rotation_count->setValue(mesh_project->rotation_count);
    genetic_layout->addWidget(rotation_count, 1, 1);

    QLabel* population_size_label = new QLabel(this);
    population_size_label->setText("Population Size");
    genetic_layout->addWidget(population_size_label, 2, 0);

    population_size = new QSpinBox(this);
    population_size->setMinimum(1);
    population_size->setMaximum(2048);
    population_size->setValue(mesh_project->population_size);
    genetic_layout->addWidget(population_size, 2, 1);

    QLabel* generation_count_label = new QLabel(this);
    generation_count_label->setText("Generation Count");
    genetic_layout->addWidget(generation_count_label, 3, 0);

    generation_count = new QSpinBox(this);
    generation_count->setMinimum(1);
    generation_count->setMaximum(1024);
    generation_count->setValue(mesh_project->generation_count);
    genetic_layout->addWidget(generation_count, 3, 1);

    QLabel* mutation_rate_label = new QLabel(this);
    mutation_rate_label->setText("Mutation Rate");
    genetic_layout->addWidget(mutation_rate_label, 4, 0);

    mutation_rate = new QSpinBox(this);
    mutation_rate->setMinimum(1);
    mutation_rate->setMaximum(64);
    mutation_rate->setSuffix("%");
    mutation_rate->setValue(mesh_project->mutation_rate);
    genetic_layout->addWidget(mutation_rate, 4, 1);

    genetic_nesting_options->setLayout(genetic_layout);
    layout->addWidget(genetic_nesting_options, 1, 0);

    QGroupBox* texture_options = new QGroupBox("Output Texture Parameters");
    QGridLayout* texture_layout = new QGridLayout(texture_options);

    QLabel* triangle_texture_density_label = new QLabel(this);
    triangle_texture_density_label->setText("Triangle Texture Density");
    texture_layout->addWidget(triangle_texture_density_label, 0, 0);

    triangle_texture_density = new QComboBox(this);
    triangle_texture_density->addItem("Average");
    triangle_texture_density->addItem("Maximum");
    texture_layout->addWidget(triangle_texture_density, 0, 1);

    QLabel* atlas_texture_density_label = new QLabel(this);
    atlas_texture_density_label->setText("Texture Atlas Density");
    texture_layout->addWidget(atlas_texture_density_label, 1, 0);

    atlas_texture_density = new QComboBox(this);
    atlas_texture_density->addItem("Average");
    atlas_texture_density->addItem("Maximum");
    texture_layout->addWidget(atlas_texture_density, 1, 1);

    QLabel* max_texture_size_label = new QLabel(this);
    max_texture_size_label->setText("Max. Texture Size");
    texture_layout->addWidget(max_texture_size_label, 2, 0);

    max_texture_size = new QComboBox(this);
    max_texture_size->addItem("256 x 256");
    max_texture_size->addItem("512 x 512");
    max_texture_size->addItem("1024 x 1024");
    max_texture_size->addItem("2048 x 2048");
    max_texture_size->addItem("4096 x 4096");
    texture_layout->addWidget(max_texture_size, 2, 1);
    loadMaxTextureSize();

    texture_options->setLayout(texture_layout);
    layout->addWidget(texture_options, 0, 1);

    reset_button = new QPushButton("Reset To Default", this);
    connect(reset_button, &QPushButton::pressed, this, &BuildOptionsDialog::onResetToDefault);
    layout->addWidget(reset_button, 2, 0);

    ok_button = new QPushButton("OK", this);
    connect(ok_button, &QPushButton::pressed, this, &BuildOptionsDialog::onOk);
    layout->addWidget(ok_button, 2, 2);

    cancel_button = new QPushButton("Cancel", this);
    connect(cancel_button, &QPushButton::pressed, this, &BuildOptionsDialog::onCancel);
    layout->addWidget(cancel_button, 2, 3);
}

void BuildOptionsDialog::loadMaxTextureSize()
{
    switch (mesh_project->max_texture_size)
    {
    case 256:
        max_texture_size->setCurrentIndex(0);
        break;
    case 512:
        max_texture_size->setCurrentIndex(1);
        break;
    case 1024:
        max_texture_size->setCurrentIndex(2);
        break;
    case 2048:
        max_texture_size->setCurrentIndex(3);
        break;
    case 4096:
    default:
        max_texture_size->setCurrentIndex(4);
        break;
    }
}

void BuildOptionsDialog::saveMaxTextureSize()
{
    switch (max_texture_size->currentIndex())
    {
    case 0:
        mesh_project->max_texture_size = 256;
        break;
    case 1:
        mesh_project->max_texture_size = 512;
        break;
    case 2:
        mesh_project->max_texture_size = 1024;
        break;
    case 3:
        mesh_project->max_texture_size = 2048;
        break;
    case 4:
    default:
        mesh_project->max_texture_size = 4096;
        break;
    }
}

void BuildOptionsDialog::onResetToDefault()
{
    protection_offset->setValue(8);
    tolerance_divider->setValue(256);
    rotation_count->setValue(8);
    population_size->setValue(128);
    generation_count->setValue(32);
    mutation_rate->setValue(10);
}

void BuildOptionsDialog::onOk()
{
    if (mesh_project->protection_offset != protection_offset->value())
    {
        mesh_project->protection_offset = protection_offset->value();
        g_main_window->dirtyProject();
    }
    if (mesh_project->scale != tolerance_divider->value())
    {
        mesh_project->scale = tolerance_divider->value();
        g_main_window->dirtyProject();
    }
    if (mesh_project->rotation_count != rotation_count->value())
    {
        mesh_project->rotation_count = rotation_count->value();
        g_main_window->dirtyProject();
    }
    if (mesh_project->population_size != population_size->value())
    {
        mesh_project->population_size = population_size->value();
        g_main_window->dirtyProject();
    }
    if (mesh_project->generation_count != generation_count->value())
    {
        mesh_project->generation_count = generation_count->value();
        g_main_window->dirtyProject();
    }
    if (mesh_project->mutation_rate != mutation_rate->value())
    {
        mesh_project->mutation_rate = mutation_rate->value();
        g_main_window->dirtyProject();
    }
    unsigned prev_max_texture_size = mesh_project->max_texture_size;
    saveMaxTextureSize();
    if (mesh_project->max_texture_size != prev_max_texture_size)
    {
        g_main_window->dirtyProject();
    }
    accept();
}

void BuildOptionsDialog::onCancel()
{
    accept();
}

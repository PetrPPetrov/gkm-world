// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include "build_options.h"

BuildOptionsDialog::BuildOptionsDialog(const MeshProject::Ptr& mesh_project, QWidget* parent) : QDialog(parent)
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
    rotation_count->setValue(static_cast<int>(mesh_project->rotation_count));
    genetic_layout->addWidget(rotation_count, 1, 1);

    QLabel* population_size_label = new QLabel(this);
    population_size_label->setText("Population Size");
    genetic_layout->addWidget(population_size_label, 2, 0);

    population_size = new QSpinBox(this);
    population_size->setMinimum(1);
    population_size->setMaximum(2048);
    population_size->setValue(static_cast<int>(mesh_project->population_size));
    genetic_layout->addWidget(population_size, 2, 1);

    QLabel* generation_count_label = new QLabel(this);
    generation_count_label->setText("Generation Count");
    genetic_layout->addWidget(generation_count_label, 3, 0);

    generation_count = new QSpinBox(this);
    generation_count->setMinimum(1);
    generation_count->setMaximum(1024);
    generation_count->setValue(static_cast<int>(mesh_project->generation_count));
    genetic_layout->addWidget(generation_count, 3, 1);

    QLabel* mutation_rate_label = new QLabel(this);
    mutation_rate_label->setText("Mutation Rate");
    genetic_layout->addWidget(mutation_rate_label, 4, 0);

    mutation_rate = new QSpinBox(this);
    mutation_rate->setMinimum(1);
    mutation_rate->setMaximum(64);
    mutation_rate->setSuffix("%");
    mutation_rate->setValue(static_cast<int>(mesh_project->mutation_rate));
    genetic_layout->addWidget(mutation_rate, 4, 1);

    genetic_nesting_options->setLayout(genetic_layout);
    layout->addWidget(genetic_nesting_options, 1, 0);

    ok_button = new QPushButton("OK", this);
    connect(ok_button, &QPushButton::pressed, this, &BuildOptionsDialog::onOk);
    layout->addWidget(ok_button, 2, 2);

    cancel_button = new QPushButton("Cancel", this);
    connect(cancel_button, &QPushButton::pressed, this, &BuildOptionsDialog::onCancel);
    layout->addWidget(cancel_button, 2, 3);
}

void BuildOptionsDialog::onOk()
{
    accept();
}

void BuildOptionsDialog::onCancel()
{
    accept();
}

// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QSpinBox>

class BuildOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    BuildOptionsDialog(QWidget* parent);

private:
    void onOk();
    void onCancel();

private:
    QSpinBox* protection_offset = nullptr;
    QSpinBox* tolerance_divider = nullptr;
    QSpinBox* rotation_count = nullptr;
    QSpinBox* population_size = nullptr;
    QSpinBox* generation_count = nullptr;
    QSpinBox* mutation_rate = nullptr;

    QPushButton* ok_button = nullptr;
    QPushButton* cancel_button = nullptr;
};

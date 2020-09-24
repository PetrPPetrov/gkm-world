// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <string>
#include <fstream>
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QVector3D>
#include "Eigen/Eigen"

typedef std::shared_ptr<QImage> ImagePtr;

inline Eigen::Vector3d to_eigen(const QVector3D& vector)
{
    return Eigen::Vector3d(vector.x(), vector.y(), vector.z());
}

inline QVector3D to_qt(const Eigen::Vector3d& vector)
{
    return QVector3D(vector.x(), vector.y(), vector.z());
}

inline bool fileExists(const std::string& filename)
{
    QFileInfo check_file(QString(filename.c_str()));
    if (check_file.exists() && check_file.isFile())
    {
        return true;
    }
    else
    {
        return false;
    }
}

inline std::string loadToken(const std::string& expected_line, std::ifstream& file_in)
{
    std::string read_line;
    while (!file_in.eof())
    {
        std::getline(file_in, read_line);
        if (expected_line == read_line)
        {
            return read_line;
        }
    }
    return read_line;
}

inline void loadEigenVector3d(Eigen::Vector3d& vector, std::ifstream& file_in)
{
    loadToken("x", file_in);
    file_in >> vector.x();
    loadToken("y", file_in);
    file_in >> vector.y();
    loadToken("z", file_in);
    file_in >> vector.z();
}

inline void saveEigenVector3d(const Eigen::Vector3d& vector, std::ofstream& file_out)
{
    file_out << "x\n";
    file_out << vector.x() << "\n";
    file_out << "y\n";
    file_out << vector.y() << "\n";
    file_out << "z\n";
    file_out << vector.z() << "\n";
}

inline void loadQtVector3d(QVector3D& vector, std::ifstream& file_in)
{
    float value;

    loadToken("x", file_in);
    file_in >> value;
    vector.setX(value);
    loadToken("y", file_in);
    file_in >> value;
    vector.setY(value);
    loadToken("z", file_in);
    file_in >> value;
    vector.setZ(value);
}

inline void saveQtVector3d(const QVector3D& vector, std::ofstream& file_out)
{
    file_out << "x\n";
    file_out << vector.x() << "\n";
    file_out << "y\n";
    file_out << vector.y() << "\n";
    file_out << "z\n";
    file_out << vector.z() << "\n";
}

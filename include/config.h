// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <list>
#include <iostream>
#include <cassert>
#include <map>
#include <boost/core/noncopyable.hpp>
#include "log.h"

class ConfigurationReader
{
    struct IConfigurationParameter : public boost::noncopyable
    {
        virtual ~IConfigurationParameter()
        {
        }
        virtual void read(std::istream& stream) = 0;
    };

    template<class Type>
    class ConfigurationParameter : public IConfigurationParameter
    {
        const std::string name;
        Type& parameter;
    public:
        ConfigurationParameter(const std::string& name_, Type& parameter_) :
            name(name_), parameter(parameter_)
        {
        }
        virtual void read(std::istream& stream)
        {
            stream >> parameter;
        }
    };

    template<class ElementType>
    class ConfigurationParameterList : public IConfigurationParameter
    {
        const std::string name;
        std::list<ElementType>& parameters;
    public:
        ConfigurationParameterList(const std::string& name_, std::list<ElementType>& parameters_) :
            name(name_), parameters(parameters_)
        {
        }
        virtual void read(std::istream& stream)
        {
            ElementType new_element;
            stream >> new_element;
            parameters.push_back(new_element);
        }
    };

    typedef std::map<std::string, std::shared_ptr<IConfigurationParameter> > name_to_parameter_t;
    name_to_parameter_t parameters;
public:
    template<class Type>
    ConfigurationReader& addParameter(const std::string& name, Type& parameter)
    {
        parameters.insert(name_to_parameter_t::value_type(name, std::make_shared<ConfigurationParameter<Type> >(name, parameter)));
        return *this;
    }
    template<class ElementType>
    ConfigurationReader& addParameter(const std::string& name, std::list<ElementType>& parameter_list)
    {
        parameters.insert(name_to_parameter_t::value_type(name, std::make_shared<ConfigurationParameterList<ElementType> >(name, parameter_list)));
        return *this;
    }
    void read(std::istream& stream)
    {
        while (!stream.eof() && stream.good())
        {
            std::string keyword;
            stream >> keyword;
            auto fit = parameters.find(keyword);
            if (fit != parameters.end())
            {
                fit->second->read(stream);
            }
            else
            {
                // Bypass value for the current unkown keyword
                std::string dummy_value;
                stream >> dummy_value;
            }
        }
    }
};

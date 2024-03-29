// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <list>
#include <iostream>
#include <fstream>
#include <cassert>
#include <map>

class ConfigurationReader {
    struct IConfigurationParameter {
        virtual ~IConfigurationParameter() {}
        virtual void read(std::istream& stream) = 0;

        IConfigurationParameter() = default;
        IConfigurationParameter(const IConfigurationParameter&) = delete;
        IConfigurationParameter& operator=(const IConfigurationParameter&) = delete;
    };

    template<class Type>
    class ConfigurationParameter : public IConfigurationParameter {
        const std::string name;
        Type& parameter;
    public:
        ConfigurationParameter(const std::string& name_, Type& parameter_) :
            name(name_), parameter(parameter_) {}
        virtual void read(std::istream& stream) {
            stream >> parameter;
        }
    };

    template<class ElementType>
    class ConfigurationParameterList : public IConfigurationParameter {
        const std::string name;
        std::list<ElementType>& parameters;
    public:
        ConfigurationParameterList(const std::string& name_, std::list<ElementType>& parameters_) :
            name(name_), parameters(parameters_) {}
        virtual void read(std::istream& stream) {
            ElementType new_element;
            stream >> new_element;
            parameters.push_back(new_element);
        }
    };

    typedef std::map<std::string, std::shared_ptr<IConfigurationParameter> > NameToParameter;
    NameToParameter parameters;
public:
    template<class Type>
    ConfigurationReader& addParameter(const std::string& name, Type& parameter) {
        parameters.insert(NameToParameter::value_type(name, std::make_shared<ConfigurationParameter<Type> >(name, parameter)));
        return *this;
    }
    template<class ElementType>
    ConfigurationReader& addParameter(const std::string& name, std::list<ElementType>& parameter_list) {
        parameters.insert(NameToParameter::value_type(name, std::make_shared<ConfigurationParameterList<ElementType> >(name, parameter_list)));
        return *this;
    }
    void read(std::istream& stream) {
        while (!stream.eof() && stream.good()) {
            std::string keyword;
            stream >> keyword;
            auto fit = parameters.find(keyword);
            if (fit != parameters.end()) {
                fit->second->read(stream);
            } else {
                // Bypass value for the current unkown keyword
                std::string dummy_value;
                stream >> dummy_value;
            }
        }
    }
};

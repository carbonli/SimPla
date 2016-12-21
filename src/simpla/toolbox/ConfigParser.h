/**
 * @file  config_parser.h
 *
 *  Created on: 2014-11-24
 *      Author: salmon
 */

#ifndef CORE_UTILITIES_CONFIG_PARSER_H_
#define CORE_UTILITIES_CONFIG_PARSER_H_

#include <simpla/SIMPLA_config.h>

#include <algorithm>
#include <cstdbool>
#include <map>
#include <string>
#include <utility>

#include "type_cast.h"
#include "LuaObject.h"
#include "LuaObjectExt.h"
#include "ConfigParser.h"
#include "parse_command_line.h"

namespace simpla { namespace toolbox
{

/**
 * @ingroup toolbox
 */
struct ConfigParser
{
    int argc;
    char **argv;

    ConfigParser();

    ~ConfigParser();

    void parse(std::string const &url, std::string const &prologue = "", std::string const &epilogue = "");

    void add(std::string const &k, std::string const &v);

    struct DictObject : public toolbox::LuaObject
    {
        DictObject()
                : toolbox::LuaObject(), m_value_("")
        {

        }

        DictObject(DictObject const &other)
                : toolbox::LuaObject(other), m_value_(other.m_value_)
        {

        }

        DictObject(DictObject &&other)
                : toolbox::LuaObject(other), m_value_(other.m_value_)
        {

        }

        DictObject(toolbox::LuaObject const &lua_obj)
                : toolbox::LuaObject(lua_obj), m_value_("")
        {

        }

        DictObject(std::string const &value)
                : /*lua::GeoObject(),*/
                m_value_(value)
        {
        }

        ~DictObject()
        {
        }

        void swap(DictObject &other)
        {
            toolbox::LuaObject::swap(other);
            std::swap(m_value_, other.m_value_);
        }

        DictObject &operator=(DictObject const &other)
        {
            DictObject(other).swap(*this);
            return *this;
        }

        operator bool() const
        {
            return m_value_ != "" || toolbox::LuaObject::operator bool();
        }

        template<typename T>
        T as() const
        {
            if (m_value_ != "")
            {
                return std::move(type_cast<T>(m_value_));
            } else if (toolbox::LuaObject::is_null())
            {

                THROW_EXCEPTION_RUNTIME_ERROR("undefined lua object!");
            }
            return std::move(toolbox::LuaObject::template as<T>());

        }

        template<typename T>
        T as(T const &default_value) const
        {
            if (m_value_ != "")
            {
                return std::move(type_cast<T>(m_value_));
            } else if (!toolbox::LuaObject::is_null())
            {
                return std::move(toolbox::LuaObject::template as<T>(default_value));
            } else
            {
                return default_value;
            }
        }

        template<typename T>
        void as(T *v) const
        {
            *v = as<T>(*v);
        }

        bool as(Properties *v) const
        {
            return toolbox::LuaObject::as(v);
        }

        template<typename T>
        operator T() const
        {
            return this->template as<T>();
        }

    private:
        std::string m_value_;
    };

    DictObject operator[](std::string const &key) const
    {

        auto it = m_kv_map_.find(key);
        if (it != m_kv_map_.end())
        {
            return std::move(DictObject(it->second));
        } else if (!m_lua_object_.is_null())
        {
            return std::move(DictObject(m_lua_object_[key]));
        } else
        {
            return std::move(DictObject());
        }
    }

private:

    toolbox::LuaObject m_lua_object_;
    std::map<std::string, std::string> m_kv_map_;
};

}} // namespace simpla

#endif /* CORE_UTILITIES_CONFIG_PARSER_H_ */

/*
* BSD 3-Clause License
*
* Copyright (c) 2018, Dolby Laboratories
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* * Neither the name of the copyright holder nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <fstream>
#include "GenericPlugin.h"

#ifdef WIN32
static std::string dir_separator = "\\";
#else
static std::string dir_separator = "/";
#endif

GenericPlugin::GenericPlugin()
{
    msg.clear();
    pluginPath.clear();
    configPath.clear();
}

Status    
GenericPlugin::setProperty(const Property* property)
{
    std::string name(property->name);
    std::string value(property->value);

    if ("plugin_path" == name)
    {
        pluginPath = value;
        return STATUS_OK;
    }
    else if ("config_path" == name)
    {
        configPath = value;
        return STATUS_OK;
    }

    return STATUS_ERROR;
}

const char* 
GenericPlugin::getMessage()
{
    return msg.empty() ? NULL : msg.c_str();
}

std::vector<PropertyInfo>
GenericPlugin::getGenericProperties()
{
    std::vector<PropertyInfo> propVector;
    propVector.push_back({ "plugin_path", PROPERTY_TYPE_STRING, "Path to this plugin.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT });
    propVector.push_back({ "config_path", PROPERTY_TYPE_STRING, "Path to DEE config file.", NULL, NULL, 1, 1, ACCESS_TYPE_WRITE_INIT });

    return propVector;
}

static 
bool 
fileExists(const std::string& name) 
{
    std::ifstream f(name.c_str());
    return f.good();
}

Status
GenericPlugin::expandPath(std::string& path)
{
    if (fileExists(path))
    {
        return STATUS_OK;
    }
    if (fileExists(configPath + dir_separator + path))
    {
        path = configPath + dir_separator + path;
        return STATUS_OK;
    }
    if (fileExists(pluginPath + dir_separator + path))
    {
        path = pluginPath + dir_separator + path;
        return STATUS_OK;
    }

    return STATUS_ERROR;
}
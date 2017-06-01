/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

/*
 * Utility classes and functions
 */

#include "utils.h"
#include "CvXcode.h"

// TODO: Uncomment this to enable translation of non-ANSI characters to lowercase. Tested and
// working on windows, but requires extensive testing on devices with different C locales).
// Requires C++11 (codecvt).
//#define USE_UNICODE_TO_LOWER

#ifdef USE_UNICODE_TO_LOWER
#include <locale>
#include <codecvt>
#endif // USE_UNICODE_TO_LOWER

namespace util
{

/*
 * String class
 */

void String::Overwrite(char c)
{
    OverwriteString(*this, c);
}

String& String::Trim(const std::string& chars)
{
    TrimLeft(chars);
    TrimRight(chars);

    return *this;
}

int String::GetHash() const
{
    // Implements java style hashcode: h(s)=\sum_{i=0}^{n-1}s[i] \cdot 31^{n-1-i}
    int hash = 0;
    for(const_iterator i = begin(); i != end(); ++i)
    {
        hash = ((hash << 5) - hash) + *i; //hash = hash * 31 + *i;
    }

    return hash;
}

#ifdef USE_UNICODE_TO_LOWER
namespace
{
    std::wstring ToWString(const std::string& s)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(s);
    }

    std::string FromWString(const std::wstring& s)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(s);
    }

    const std::locale locale("");

    wchar_t WCharToLower(wchar_t c)
    {
        return std::tolower(c, locale);
    }
}

String String::ToLower() const
{
    std::string res;
    std::wstring wsrc = ToWString(*this);
    std::wstring wres;
    std::transform(wsrc.begin(), wsrc.end(), std::back_inserter(wres), WCharToLower);
    return FromWString(wres);
}
#else
String String::ToLower() const
{
    String res;
    res.resize(length());
    std::transform(begin(), end(), res.begin(), tolower);
    return res;
}
#endif // USE_UNICODE_TO_LOWER

bool String::EndsWith(const std::string& suffix) const
{
    size_t len = length();
    size_t suffixLen = suffix.length();

    if (suffixLen > len)
    {
        return false;
    }

    return compare(len - suffixLen, suffixLen, suffix) == 0;
}

String::~String()
{
    Overwrite();
}

void OverwriteString(std::string& str, char c)
{
    for(size_t i = 0; i < str.length(); ++i)
    {
        str[i] = c;
    }
}


/*
 * JsonObject class
 */

JsonObject::JsonObject()
{
}

JsonObject::JsonObject(const json::Object& other)
{
    *this = other;
}

JsonObject& JsonObject::operator =(const json::Object& other)
{
    Clear();
    Copy(other);
    m_parseError = "";
    return *this;
}
    
JsonObject::~JsonObject()
{
    OverwriteJsonValues(*this);
}

std::string JsonObject::ToString() const
{
    std::stringstream jsonStream;
    json::Writer::Write(*this, jsonStream);
    return jsonStream.str();
}

bool JsonObject::Parse(const char* str)
{
    try
    {
        Clear();

        if(str == NULL || *str == '\0')
        {
            throw json::Exception("Failed to find root element");
        }
        
        std::istringstream istream(str);
        json::Reader::Read(*this, istream);
        
        m_parseError = "";

        return true;
    }
    catch(const json::Exception& e)
    {
        m_parseError.Format("Failed to parse '%s' json. Error='%s'\n", str, e.what());
        return false;
    }
}

const char * JsonObject::GetStringParam(const char *name, const char *defaultValue) const
{
    try
    {
        return ((const json::String&) (*this)[name]).Value().c_str();
    }
    catch(const json::Exception&)
    {
        return defaultValue;
    }
}

int JsonObject::GetIntParam(const char *name, int defaultValue) const
{
    try
    {
        return (int) ((const json::Number&) (*this)[name]).Value();
    }
    catch(const json::Exception&)
    {
        return defaultValue;
    }
}
    
int64_t JsonObject::GetInt64Param(const char *name, int64_t defaultValue) const
{
    try
    {
        return (int64_t)((const json::Number&) (*this)[name]).Value();
    }
    catch(const json::Exception&)
    {
        return defaultValue;
    }
}

bool JsonObject::GetBoolParam(const char *name, bool defaultValue) const
{
    try
    {
        return ((const json::Boolean&) (*this)[name]).Value();
    }
    catch(const json::Exception&)
    {
        return defaultValue;
    }
}

void JsonObject::PutIfNotEmpty(const std::string& name, const std::string& value)
{
    if (!value.empty())
    {
        (*this)[name] = json::String(value);
    }
}

std::string JsonObject::GetParseError() const
{
    return m_parseError;
}

void JsonObject::Copy(const json::Object& other)
{
    for(json::Object::const_iterator i = other.Begin(); i != other.End(); ++i)
    {
        Insert(*i);
    }
}

std::string GetOptionalStringParam(const json::Object& object, const std::string& name, const std::string& defaultValue)
{
    json::Object::const_iterator i = object.Find(name);
    if (i == object.End())
    {
        return defaultValue;
    }
    return ((const json::String&) i->element).Value();
}

int GetOptionalIntParam(const json::Object& object, const std::string& name, int defaultValue)
{
    json::Object::const_iterator i = object.Find(name);
    if (i == object.End())
    {
        return defaultValue;
    }
    return static_cast<int>(((const json::Number&) i->element).Value());
}

namespace
{
    class JsonVisitor : public json::Visitor
    {
    public:
        virtual void Visit(json::Array& array)
        {
            OverwriteJsonValues(array);
        }

        virtual void Visit(json::Object& object)
        {
            OverwriteJsonValues(object);
        }

        virtual void Visit(json::Number& number)
        {
            number.Value() = 0.0;
        }

        virtual void Visit(json::String& string)
        {
            OverwriteString(string.Value());
        }

        virtual void Visit(json::Boolean& boolean)
        {
            boolean.Value() = false;
        }

        virtual void Visit(json::Null& null)
        {
        }
    };
}

void OverwriteJsonValues(json::Object& object)
{
    for(json::Object::iterator i = object.Begin(); i != object.End(); ++i)
    {
        OverwriteJsonValues(i->element);
    }
}

void OverwriteJsonValues(json::Array& array)
{
    for(json::Array::iterator i = array.Begin(); i != array.End(); ++i)
    {
        OverwriteJsonValues(*i);
    }
}

void OverwriteJsonValues(json::UnknownElement& element)
{
    JsonVisitor visitor;
    element.Accept(visitor);
}


/*
 * StringMap class
 */

StringMap::StringMap(const json::Object & object)
{
    for (json::Object::const_iterator i = object.Begin(); i != object.End(); ++i)
    {
        (*this)[i->name] = ((const json::String&) i->element).Value();
    }
}

bool StringMap::Put(const String& key, const String& value)
{
    iterator i = find(key);
    if(i != end())
    {
        return false;
    }

    (*this)[key] = value;

    return true;
}

void StringMap::PutAll(const StringMap& other)
{
    for (StringMap::const_iterator i = other.begin(); i != other.end(); ++i)
    {
        (*this)[i->first] = i->second;
    }
}

const char * StringMap::Get(const String& key) const
{
    const_iterator i = find(key);
    if(i == end())
    {
        return "";
    }

    return i->second.c_str();
}


/*
 * Hex encoding/decoding
 */

std::string HexEncode(const char* str, size_t len)
{
    std::string hexEncodedStr;
    CvShared::CvHex::Encode((const unsigned char *) str, len, hexEncodedStr);
    return hexEncodedStr;
}

std::string HexEncode(const std::string& str)
{
    return HexEncode(str.c_str(), str.size());
}

std::string HexDecode(const std::string& str)
{
    std::string hexDecodedStr;
    CvShared::CvHex::Decode(str, hexDecodedStr);
    return hexDecodedStr;
}


/*
 * Url parsing
 */

namespace
{
    std::string URL_SCHEME_END = "://";
}

Url::Url(const std::string& url)
{
    String reminder = url;

    size_t pos = reminder.find(URL_SCHEME_END);
    if (pos != String::npos)
    {
        m_scheme = String(reminder.substr(0, pos)).ToLower();
        reminder = String(reminder.substr(pos + URL_SCHEME_END.length())).TrimLeft("/");
    }

    pos = reminder.find_first_of('/');
    if (pos != String::npos)
    {
        m_path = reminder.substr(pos + 1);
        reminder = reminder.substr(0, pos);
    }

    pos = reminder.find_first_of(':');
    if (pos != String::npos)
    {
        m_port = reminder.substr(pos + 1);
    }

    m_host = String(reminder.substr(0, pos)).ToLower();
}

Url::Url(const std::string & scheme, const std::string & host, const std::string & port, const std::string & path) :
    m_scheme(scheme), m_host(host), m_port(port), m_path(path) {}

const String& Url::GetScheme() const
{
    return m_scheme;
}

const String& Url::GetHost() const
{
    return m_host;
}

const String& Url::GetPort() const
{
    return m_port;
}

const String& Url::GetPath() const
{
    return m_path;
}

bool Url::operator==(const Url & other)
{
    return !(*this != other);
}

bool Url::operator!=(const Url & other)
{
    return m_scheme != other.m_scheme || m_host != other.m_host || m_port != other.m_port || m_path != other.m_path;
}

}

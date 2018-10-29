#pragma once
#include "Common.h"
using namespace std;
#if FV_NLOHMANJSON
    #include "../3rdParty/nlohmann/json.hpp"
    using json = nlohmann::json;
#endif


namespace fv
{
    class TextSerializer
    {
    public:
        FV_DLL TextSerializer(); // This assumes to write a file.
        FV_DLL TextSerializer(const char* inputFilePath); // This assumes to read a file.
        FV_DLL ~TextSerializer();
        FV_DLL void writeToFile( const char* outputFilePath );

        FV_DLL void pushArray(const String& name);
        FV_DLL bool beginArrayElement();
        FV_DLL void endArrayElement();
        FV_DLL void popArray();
        FV_DLL void pushObject(const String& name);
        FV_DLL void popObject();

        template <class T>
        void serialize(const String& key, T& value);
        template <class T>
        void serializeVector(const String& key, Vector<T>& value);
        template <class K, class V>
        void serializeMap(const String& key, Map<K, V>& value);

        bool hasSerializeErrors() const { return m_HasSerializeErrors; }
        bool isWriting() const { return m_IsWriting; }

    private:
        bool m_IsWriting = false;
        bool m_HasSerializeErrors = false;

    #if FV_NLOHMANJSON
        json m_Document;
        Stack<json*> m_Stack;
        json* m_Active;
        Stack<json::iterator> m_ArrayIt;
    #endif
    };


    template <class T>
    void TextSerializer::serialize(const String& key, T& value)
    {
        if ( m_IsWriting )
        {
            (*m_Active)[key] = value;
        }
        else 
        {
            value = (*m_Active)[key];
        }
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Vec3& v)
    {
        if ( m_IsWriting )
        {
            (*m_Active)[key] = { v.x, v.y, v.z };
        }
        else
        {
            auto& val = (*m_Active)[key];
            if ( !(val.is_array() && val.size()==3) )
                throw;
            v.x = val[0];
            v.y = val[1];
            v.z = val[2];
        }
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Quat& v)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            (*m_Active)[key] = { v.x, v.y, v.z, v.w };
        }
        else
        {
            auto& val = (*m_Active)[key];
            if ( !(val.is_array() && val.size()==4) )
                throw;
            v.x = val[0];
            v.y = val[1];
            v.z = val[2];
            v.w = val[3];
        }
    #else
        #error no implementation
    #endif
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Vec4& v) { serialize(key, (Vec4&)v); }

    template <class T>
    void TextSerializer::serializeVector(const String& key, Vector<T>& value)
    {
        #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            json j;
            for ( auto& v : value )
                j.emplace_back( v );
            (*m_Active)[key] = j;
        }
        else
        {
            json& j = (*m_Active)[key];
            for ( auto& v : j )
            {
                value.emplace_back( v );
            }
        }
        #else
        #error no implementation
        #endif
    }

    template <class K, class V>
    void TextSerializer::serializeMap(const String& key, Map<K, V>& value)
    {
        #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            json j;
            for ( auto& kvp : value )
                j[kvp.first] = kvp.second;
            (*m_Active)[key] = j;
        }
        else
        {
            json& j = (*m_Active)[key];
            for (auto& kvp : j.items())
            {
                value[ kvp.key() ] = kvp.value();
            }
        }
        #else
        #error no implementation
        #endif
    }

}
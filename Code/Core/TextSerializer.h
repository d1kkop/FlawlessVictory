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

        FV_DLL void pushArray();
        FV_DLL void beginArrayElement();
        FV_DLL void endArrayElement();
        FV_DLL void popArray(const String& name);
        FV_DLL void pushObject();
        FV_DLL void popObject(const String& name);

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
        Stack<json> m_Document;
        json m_ArrayElement{};
        bool m_IsArray = false;
    #endif
    };


    template <class T>
    void TextSerializer::serialize(const String& key, T& value)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            m_Document.top()[ key ] = value;
        }
        else 
        {
            value = m_Document.top()[ key ];
        }
    #else
        #error no implementation
    #endif
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Vec3& v)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            m_Document.top()[key] = { v.x, v.y, v.z };
        }
        else
        {
            auto& val = m_Document.top()[key];
            if ( val.is_array() && val.size()==3 )
            {
                v.x = val[0];
                v.y = val[1];
                v.z = val[2];
            }
        }
    #else
        #error no implementation
    #endif
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Quat& v)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            m_Document.top()[key] = { v.x, v.y, v.z, v.w };
        }
        else
        {
            auto& val = m_Document.top()[key];
            if ( val.is_array() && val.size()==4 )
            {
                v.x = val[0];
                v.y = val[1];
                v.z = val[2];
                v.w = val[3];
            }
        }
    #else
        #error no implementation
    #endif
    }

    template <>
    inline void TextSerializer::serialize(const String& key, Vec4& v) { serialize(key, (Quat&)v); }

    template <class T>
    void TextSerializer::serializeVector(const String& key, Vector<T>& value)
    {
        #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            json j;
            for ( auto& v : value )
                j.emplace_back( v );
            m_Document.top()[key] = j;
        }
        else
        {
            json& j = m_Document.top()[key];
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
            m_Document.top()[key] = j;
        }
        else
        {
            json& j = m_Document.top()[key];
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
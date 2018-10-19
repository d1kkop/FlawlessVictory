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

        template <class T>
        void serialize(const String& key, T& value);
        template <class T>
        void serializeVector(const String& key, Vector<T>& value);
        template <class K, class V>
        void serializeMap(const String& key, Map<K, V>& value);

        bool hasSerializeErrors() const { return m_HasSerializeErrors; }

    private:
        bool m_IsWriting = false;
        bool m_HasSerializeErrors = false;

    #if FV_NLOHMANJSON
        json m_Document;
    #endif
    };


    template <class T>
    void TextSerializer::serialize(const String& key, T& value)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            m_Document[ key ] = value;
        }
        else 
        {
            value = m_Document[key];
        }
    #else
        #error no implementation
    #endif
    }

    template <class T>
    void TextSerializer::serializeVector(const String& key, Vector<T>& value)
    {
        #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            json j;
            for ( auto& v : value )
                j.emplace_back( v );
            m_Document[key] = j;
        }
        else
        {
            json& j = m_Document[key];
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
            m_Document[key] = j;
        }
        else
        {
            json& j = m_Document[key];
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
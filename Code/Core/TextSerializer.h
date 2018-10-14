#pragma once
#include "Common.h"
using namespace std;
#if FV_NLOHMANJSON
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

}
#include "PCH.h"
#include "TextSerializer.h"
#include "Functions.h"
#include "LogManager.h"

namespace fv
{
    TextSerializer::TextSerializer():
        m_IsWriting(true)
    {
    #if FV_NLOHMANJSON
        m_Document.push(json());
    #else
    #endif
    }

    TextSerializer::TextSerializer(const char* inputFilePath):
        m_IsWriting(false)
    {
        ifstream file( inputFilePath );
        if ( file.is_open() )
        {
            try
            {
            #if FV_NLOHMANJSON
                m_Document.push(json());
                file >> m_Document.top();
            #else
                #error no implementation
            #endif

                file.close();
            }
            catch (...)
            {
                if ( file.is_open() ) file.close();
                m_HasSerializeErrors = true;
            }
        } else m_HasSerializeErrors = true;
    }

    TextSerializer::~TextSerializer()
    {
    }

    void TextSerializer::writeToFile(const char* outputFilePath)
    {
        if ( !m_IsWriting )
        {
            LOGW("Tried to write to a serializer that was opened for reading.");
            m_HasSerializeErrors = true;
            return;
        }

        ofstream file( outputFilePath );
        if ( file.is_open() )
        {
            try
            {
            #if FV_NLOHMANJSON
                file << setw(4) << m_Document.top() << endl;
            #else
                #error no implementation
            #endif

                file.close();
            }
            catch ( ... )
            {
                if ( file.is_open() ) file.close();
                m_HasSerializeErrors = true;
            }
        }
        else m_HasSerializeErrors = true;
    }

    void TextSerializer::pushArray()
    {
        #if FV_NLOHMANJSON
        json jarray = json::array();
        m_Document.push( jarray );
        #endif
        
    }

    void TextSerializer::beginArrayElement()
    {
        pushObject();
    }

    void TextSerializer::endArrayElement()
    {
        json jobject = m_Document.top();
        m_Document.pop();
        m_Document.top().push_back( jobject );
    }

    void TextSerializer::popArray(const String& name)
    {
        json jarray = m_Document.top();
        m_Document.pop();
        m_Document.top()[name] = jarray;
    }

    void TextSerializer::pushObject()
    {
        json jobject = json::object();
        m_Document.push( jobject );
    }

    void TextSerializer::popObject(const String& name)
    {
        json jobject = m_Document.top();
        m_Document.pop();
        m_Document.top()[name] = jobject;
    }
}
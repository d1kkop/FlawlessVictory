#include "PCH.h"
#include "TextSerializer.h"
#include "Functions.h"
#include "LogManager.h"

namespace fv
{
    TextSerializer::TextSerializer():
        m_IsWriting(true)
    {
        m_Stack.push(&m_Document);
        m_Active = m_Stack.top();
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
                file >> m_Document;
            #else
                #error no implementation
            #endif
                file.close();
                m_Stack.push(&m_Document);
                m_Active = m_Stack.top();
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
                file << setw(4) << m_Document << endl;
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

    bool TextSerializer::beginArrayElement()
    {
        if ( m_IsWriting )
        {
            m_Active->push_back( json::object() );
            m_Stack.push( &m_Active->back() );
            m_Active = m_Stack.top();
            return true;
        }
        else
        {
            if ( m_ArrayIt.top() != m_Active->end() )
            {
                m_Stack.push( &(*m_ArrayIt.top()) );
                m_Active = m_Stack.top();
                return true;
            }
        }
        return false;
    }

    void TextSerializer::endArrayElement()
    {
        if ( m_IsWriting )
        {
            m_Stack.pop();
            m_Active = m_Stack.top();
        }
        else
        {
            m_ArrayIt.top()++;
            m_Stack.pop();
            m_Active = m_Stack.top();
        }
    }

    void TextSerializer::pushArray(const String& name)
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            (*m_Active)[name] = json::array();
            m_Stack.push( &(*m_Active)[name] );
            m_Active = m_Stack.top();
        }
        else
        {
            m_Stack.push( &(*m_Active)[name] );
            m_Active = m_Stack.top();
            m_ArrayIt.push( m_Active->begin() );
        }
    #endif
    }

    void TextSerializer::popArray()
    {
    #if FV_NLOHMANJSON
        if ( m_IsWriting )
        {
            m_Stack.pop();
            m_Active = m_Stack.top();
        }
        else
        {
            m_Stack.pop();
            m_Active = m_Stack.top();
            m_ArrayIt.pop();
        }
    #endif
    }

    void TextSerializer::pushObject(const String& name)
    {
    #if FV_NLOHMANJSON
        (*m_Active)[name] = json::object();
        m_Stack.push( &(*m_Active)[name] );
        m_Active = m_Stack.top();
    #endif
    }

    void TextSerializer::popObject()
    {
    #if FV_NLOHMANJSON
        m_Stack.pop();
        m_Active = m_Stack.top();
    #endif
    }
}
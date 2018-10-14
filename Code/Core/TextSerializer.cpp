#include "PCH.h"
#include "TextSerializer.h"
#include "Functions.h"
#include "LogManager.h"

namespace fv
{
    TextSerializer::TextSerializer():
        m_IsWriting(true)
    {
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

}
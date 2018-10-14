#pragma once
#include "Common.h"
using namespace std;

namespace fv
{
    template <class S, class T>
    void BinWrite(S& file, T& data)
    {
        file.write((const char*)&data, sizeof(T));
    }

    template <class S, class T>
    void BinRead(S& file, T& data)
    {
        file.read((char*)&data, sizeof(T));
    }

    template <class S, class T>
    void BinWriteVector(S& file, T& data)
    {
        u32 num = (u32)data.size();
        file.write((const char*)&num, 4);
        if ( !data.empty() )
        {
            file.write((const char*)data.data(), data.size()*sizeof(data[0]));
        }
    }
    
    template <class S, class T>
    void BinReadVector(S& file, T& data)
    {
        u32 num = 0;
        file.read((char*)&num, 4);
        if ( num > 0 ) 
        {
            data.resize(num);
            file.read((char*)data.data(), data.size()*sizeof(data[0]));
        }
    }
}
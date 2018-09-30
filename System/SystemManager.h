#pragma once

namespace fv
{
    class SystemManager
    {
    public:
        void initialize();
        void mainloop();

    private:
        bool m_Done = false;
    };
}
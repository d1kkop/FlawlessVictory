#pragma once

namespace fv
{
    class SystemManager
    {
    public:
        void initialize();
        void mainloop();

        float m_UpdateRate = 1.f/60.f;
    private:
        bool m_Done = false;
    };
}
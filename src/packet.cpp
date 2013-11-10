#include "global.h"
#include "packet.h"

void Packet::put(uint8 num)
{
    m_data.push_back(num);
}

uint8 Packet::get()
{
    if (pos >= m_data.size())
        return 0;

    return m_data[pos++];
}

uint8* Packet::pipe()
{
    pipe_data = new uint8[m_data.size()];

    for (uint32 i = 0; i < m_data.size(); i++)
        pipe_data[i] = m_data[i];

    return pipe_data;
}

bool Packet::at_end()
{
    return pos >= m_data.size();
}

uint8 Packet::size()
{
    return m_data.size();
}

void Packet::finalize()
{
    if (pipe_data)
        delete pipe_data;

    m_data.clear();

    pos = 0;
}

#ifndef CUB_PACKET_H
#define CUB_PACKET_H

#include <vector>

#define PACKET_END 0xFF

class Packet
{
    public:
        Packet()
        {
            pos = 0;
            pipe_data = NULL;
        }

        void put(uint8 num);
        uint8 get();

        uint8* pipe();
        uint8 size();

        void finalize();

        bool at_end();

    private:
        std::vector<uint8> m_data;
        uint8* pipe_data;

        uint8 pos;
};

#endif

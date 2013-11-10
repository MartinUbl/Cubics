#ifndef CUB_GAMEPLAY_CLASS
#define CUB_GAMEPLAY_CLASS

#include "singleton.h"

#include <vector>

struct Cubic
{
    Cubic()
    {
        memset(this, 0, sizeof(Cubic));
    };

    bool present;
    bool treasure;
    uint8 player; // 0 / 1
};

typedef std::vector< std::vector<Cubic> > CubeMap;

#include <windows.h>
#include <winsock.h>
#include <cstdarg>

#define BUFFER_LEN 8*1024
#define APP_PORT 1410

#define TMP_HOST "127.0.0.1"//"90.181.26.98"

#define SOCK SOCKET
#define ADDRLEN int

#define SOCKETWOULDBLOCK WSAEWOULDBLOCK
#define SOCKETCONNRESET  WSAECONNRESET
#define LASTERROR() WSAGetLastError()

enum Opcodes
{
    MSG_YOUR_TURN = 0x01,
    MSG_PUT_CUBE  = 0x02,
    MSG_TREASURES = 0x03,
    MSG_TREASURE  = 0x04,
    MSG_TIME      = 0x05,
    MSG_AGAIN     = 0x06,
    MSG_CLEAR     = 0x07,
};

#define BEGIN_X 15
#define BEGIN_Y 40
#define COUNT_X 40
#define COUNT_Y 40
#define CELL_SIZE 13
#define TREASURE_COUNT 200

#define GAME_TIME 45 //secs

#define SERVER_COLOR 0x5555AAFF
#define CLIENT_COLOR 0x449944FF

#define SERVER_WIN_COLOR 0xAAAAFFFF
#define CLIENT_WIN_COLOR 0xAAFFAAFF

class CubicGameplay
{
    public:
        CubicGameplay()
        {
            memset(this, 0, sizeof(CubicGameplay));

            cubes.resize(COUNT_X);
            for (uint32 i = 0; i < COUNT_X; i++)
                cubes[i].resize(COUNT_Y);
        }

        bool my_turn;
        bool end_game;
        clock_t got_turn;

        time_t timeend;

        uint32 mytime;
        uint32 optime;

        bool my_again;
        bool op_again;

        bool i_am_server;

        bool listening;
        clock_t connectPause;

        uint8 mesg;
        uint32 err;

        char* message;
        clock_t message_time;
        void ShowMessage(uint32 time, const char* msg, ...)
        {
            message = new char[256];
            va_list ap;

            if (msg == NULL)
                return;

            va_start(ap, msg);
              vsprintf(message, msg, ap);
            va_end(ap);

            message_time = clock() + time;
        };

        CubeMap cubes;
};

#define sCubic Singleton<CubicGameplay>::instance()

#define MY_CUBE_COLOR (sCubic->i_am_server ? SERVER_COLOR : CLIENT_COLOR)

#endif


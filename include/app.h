#ifndef CUB_APPLICATION_H
#define CUB_APPLICATION_H

#include "singleton.h"
#include "packet.h"

enum FontType
{
    FONT_DEFAULT    = 0,
    FONT_TITLE      = 1,
    /*FONT_MENU_ITEM  = 2,
    FONT_HINT       = 3,*/
    MAX_FONT
};

enum Stage
{
    STAGE_BEGIN      = 0,
    STAGE_CONNECTING = 1,
    STAGE_WAITING    = 2,
    STAGE_GAME       = 3,
};

class Application
{
    public:
        Application();
        bool Init();
        void Run();

        void RunBegin();
        void RunConnecting();
        void RunWaiting();
        void RunGame();

        void RunGameplay();

        bool CheckWin();

        void ProcessPacket(Packet* pkt);

        void SetFontId(FontType pos, uint32 id) { m_fontMap[pos] = id; };
        int32 GetFontId(FontType pos) { return m_fontMap[pos]; };

        void SetMouseXY(uint32 x, uint32 y)
        {
            m_mousePos[0] = x;
            m_mousePos[1] = y;
        }
        uint32 GetMouseX() { return m_mousePos[0]; };
        uint32 GetMouseY() { return m_mousePos[1]; };

        void MousePress(bool left, bool press);
        void KeyEvent(uint8 key, bool press);

        // //////
        void DrawGrid();

    private:
        uint32 m_windowWidth;
        uint32 m_windowHeight;

        uint32 m_mousePos[2];

        int32 m_fontMap[MAX_FONT];

        Stage stage;

        SOCKET m_socket;
        SOCKET m_opponent;
        sockaddr_in m_sockName;
        sockaddr_in m_opponentSockName;
        hostent* m_hostent;
};

#define sApplication Singleton<Application>::instance()

#define FNT(x) sApplication->GetFontId(x)
#define FNTCOLOR_WHITE glColor4ub(255,255,255,255)
#define FNTCOLOR_BLACK glColor4ub(0,0,0,255)
#define FNTCOLOR_RED glColor4ub(255,0,0,255)
#define FNTCOLOR_YELLOW glColor4ub(255,255,0,255)

#endif

#include "global.h"
#include "packet.h"
#include "app.h"
#include "gameplay.h"

#define IN_RANGE(x,y,lx,ly,rx,ry) (x > lx && x < rx && y > ly && y < ry)

void Application::KeyEvent(uint8 key, bool press)
{
}

void Application::MousePress(bool left, bool press)
{
    if (stage == STAGE_BEGIN)
    {
        if (IN_RANGE(GetMouseX(),GetMouseY(), 174+50,400,174+50+75,400+34))
        {
            sCubic->connectPause = clock()+300;
            sCubic->i_am_server = false;
            sCubic->my_turn = false;
            stage = STAGE_CONNECTING;
        }
        else if (IN_RANGE(GetMouseX(),GetMouseY(), 174+150,400,174+150+92,400+34))
        {
            sCubic->i_am_server = true;
            sCubic->my_turn = true;
            stage = STAGE_WAITING;
        }

        return;
    }

    if (stage == STAGE_GAME)
    {
        if (sCubic->end_game)
        {
            if (!sCubic->my_again && IN_RANGE(GetMouseX(), GetMouseY(), BEGIN_X+520+10, BEGIN_Y+70, BEGIN_X+520+10+85, BEGIN_Y+70+34))
            {
                Packet pkt;
                pkt.put(MSG_AGAIN);
                pkt.put(PACKET_END);
                send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
                pkt.finalize();

                sCubic->my_again = true;
            }
        }
        if (sCubic->my_turn)
        {
            if (GetMouseX() > BEGIN_X && GetMouseX() < BEGIN_X+520 && GetMouseY() > BEGIN_Y && GetMouseY() < BEGIN_Y+520)
            {
                int32 cx = (GetMouseX()-BEGIN_X)/CELL_SIZE;
                int32 cy = (GetMouseY()-BEGIN_Y)/CELL_SIZE;

                if (cx < 0)
                    cx = 0;
                if (cy < 0)
                    cy = 0;
                if (cx > COUNT_X)
                    cx = COUNT_X;
                if (cy > COUNT_Y)
                    cy = COUNT_Y;

                if (sCubic->cubes[cx][cy].present)
                {
                    sCubic->ShowMessage(1000, "Toto pole je obsazeno");
                    return;
                }

                sCubic->cubes[cx][cy].present = true;
                sCubic->cubes[cx][cy].player = sCubic->i_am_server ? 0 : 1;

                if (CheckWin())
                    return;

                Packet pkt;
                pkt.put(MSG_PUT_CUBE);
                pkt.put((uint8)cx);
                pkt.put((uint8)cy);
                pkt.put(PACKET_END);
                send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
                pkt.finalize();

                if (sCubic->cubes[cx][cy].treasure)
                {
                    sCubic->ShowMessage(500, "Poklad!");

                    pkt.put(MSG_TREASURE);
                    pkt.put((uint8)cx);
                    pkt.put((uint8)cy);
                    pkt.put(PACKET_END);
                    send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
                    pkt.finalize();
                }

                uint32 mytime = clock() - sCubic->got_turn;

                sCubic->mytime += mytime;

                pkt.put(MSG_YOUR_TURN);
                pkt.put((uint8)(mytime));
                pkt.put((uint8)(mytime >> 8));
                pkt.put((uint8)(mytime >> 16));
                pkt.put((uint8)(mytime >> 24));
                pkt.put(PACKET_END);
                send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
                pkt.finalize();

                sCubic->my_turn = false;
            }
        }
    }
}

void Application::RunBegin()
{
    SF->Drawing->DrawRectangle(174+50,400,75,34, MAKE_COLOR_RGBA(127,0,127,255));
    SF->Drawing->DrawRectangle(174+150,400,92,34, MAKE_COLOR_RGBA(127,0,127,255));

    FNTCOLOR_WHITE;
    //int x = SF->Drawing->GetTextWidth(FNT(FONT_DEFAULT), 0, L"by Kennny");
    SF->Drawing->PrintText(FNT(FONT_TITLE),268,60,0,0,L"Cubics");

    SF->Drawing->PrintText(FNT(FONT_DEFAULT),277,140,0,0,L"by Kennny");


    SF->Drawing->PrintText(FNT(FONT_DEFAULT),174+60,410,0,0,L"Pøipojit");
    SF->Drawing->PrintText(FNT(FONT_DEFAULT),174+160,410,0,0,L"Hostovat");

    FNTCOLOR_RED;
    if (sCubic->mesg == 1)
        SF->Drawing->PrintText(FNT(FONT_DEFAULT),80,510,0,0,L"Spojení ztraceno.");
    if (sCubic->mesg == 2)
        SF->Drawing->PrintText(FNT(FONT_DEFAULT),80,510,0,0,L"Chyba spojení: %u",sCubic->err);
}

void Application::RunConnecting()
{
    FNTCOLOR_WHITE;
    SF->Drawing->PrintText(FNT(FONT_DEFAULT),30,30,0,0,L"Pøipojuji se k %S ...", TMP_HOST);

    if (sCubic->connectPause > clock())
        return;

    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA data;

    if (WSAStartup(wVersionRequested, &data) != 0)
        return;

    if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
        return;

    if ((m_hostent = gethostbyname(TMP_HOST)) == NULL)
        return;

    m_opponentSockName.sin_family = AF_INET;
    m_opponentSockName.sin_port = htons(APP_PORT);
    memcpy(&(m_opponentSockName.sin_addr), m_hostent->h_addr, m_hostent->h_length);
    if (connect(m_socket, (sockaddr*)&m_opponentSockName, sizeof(m_opponentSockName)) == -1)
        return;

    u_long arg = 1;
    if (ioctlsocket(m_socket, FIONBIO, &arg) == SOCKET_ERROR)
        return;

    m_opponent = m_socket;

    sCubic->end_game = false;
    sCubic->timeend = (uint32)time(NULL)+GAME_TIME;

    stage = STAGE_GAME;
}

void Application::RunWaiting()
{
    if (!sCubic->listening)
    {
        WORD version = MAKEWORD(1,1);
        WSADATA data;
        if (WSAStartup(version, &data) != 0)
            return;

        if ((m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
            return;

        m_sockName.sin_family = AF_INET;
        m_sockName.sin_port = htons(APP_PORT);
        m_sockName.sin_addr.s_addr = INADDR_ANY;
        if (bind(m_socket, (sockaddr*)&m_sockName, sizeof(m_sockName)) == -1)
            return;

        if (listen(m_socket, 10) == -1)
            return;

        u_long arg = 1;
        if (ioctlsocket(m_socket, FIONBIO, &arg) == SOCKET_ERROR)
            return;

        sCubic->listening = true;
    }

    FNTCOLOR_WHITE;
    SF->Drawing->PrintText(FNT(FONT_DEFAULT),30,30,0,0,L"Naslouchám...");

    char* buf = new char[BUFFER_LEN];
    int error;
    SOCKET res;
    sockaddr_in sockInfo;
    int addrlen = sizeof(sockInfo);

    res = accept(m_socket, (sockaddr*)&sockInfo, &addrlen);
    error = LASTERROR();

    if (res == INVALID_SOCKET && error == SOCKETWOULDBLOCK)
    {
        // -
    }
    else if (res == INVALID_SOCKET && error != SOCKETWOULDBLOCK)
    {
        sCubic->mesg = 2;
        sCubic->err = error;
        stage = STAGE_BEGIN;
        return;
    }
    else
    {
        m_opponent = res;
        m_opponentSockName = sockInfo;

        uint32 a, b;
        srand((unsigned int)time(NULL));

        /*Packet pkt;
        pkt.put(MSG_TREASURES);
        pkt.put(TREASURE_COUNT);*/

        for (uint32 i = 0; i < TREASURE_COUNT; )
        {
            a = rand()%COUNT_X;
            b = rand()%COUNT_Y;

            if (sCubic->cubes[a][b].treasure)
                continue;

            i++;

            /*pkt.put((uint8)a);
            pkt.put((uint8)b);*/

            sCubic->cubes[a][b].treasure = true;
        }

        /*pkt.put(PACKET_END);
        send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
        pkt.finalize();*/

        Packet pkt;
        pkt.put(MSG_TIME);
        uint32 t = (uint32)time(NULL)+GAME_TIME;
        pkt.put(t);
        pkt.put(t >> 8);
        pkt.put(t >> 16);
        pkt.put(t >> 24);
        pkt.put(PACKET_END);
        send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
        pkt.finalize();

        sCubic->timeend = t;

        stage = STAGE_GAME;

        sCubic->end_game = false;
        sCubic->got_turn = clock();
    }
}

void Application::RunGame()
{
    sCubic->mesg = 0;

    char* buf = new char[BUFFER_LEN];
    int result, error;

    memset(buf,0,BUFFER_LEN);
    result = recv(m_opponent, buf, BUFFER_LEN, 0);
    error = LASTERROR();

    if (result > 0)
    {
        Packet pkt;
        for (uint32 i = 0; i < result; i++)
            pkt.put(buf[i]);

        ProcessPacket(&pkt);

        pkt.finalize();
    }
    else if (result == 0 || error == SOCKETCONNRESET)
    {
        sCubic->mesg = 1;
        stage = STAGE_BEGIN;
        return;
    }
    else
    {
        if (error != SOCKETWOULDBLOCK && error != 0)
        {
            sCubic->mesg = 2;
            sCubic->err = error;
            stage = STAGE_BEGIN;
            return;
        }
    }

    /////////////////////////////

    if (sCubic->end_game && sCubic->my_again && sCubic->op_again && sCubic->i_am_server)
    {
        Packet pkt;
        pkt.put(MSG_CLEAR);
        pkt.put(PACKET_END);
        send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
        pkt.finalize();

        for (uint32 i = 0; i < COUNT_X; i++)
        {
            for (uint32 j = 0; j < COUNT_Y; j++)
            {
                sCubic->cubes[i][j].present = false;
                sCubic->cubes[i][j].treasure = false;
            }
        }
        sCubic->mytime = 0;
        sCubic->optime = 0;
        sCubic->end_game = false;
        sCubic->ShowMessage(0, "");
        sCubic->my_turn = false;
        sCubic->timeend = time(NULL) + GAME_TIME;

        uint32 a, b;
        srand((unsigned int)time(NULL));

        for (uint32 i = 0; i < TREASURE_COUNT; )
        {
            a = rand()%COUNT_X;
            b = rand()%COUNT_Y;

            if (sCubic->cubes[a][b].treasure)
                continue;

            i++;

            sCubic->cubes[a][b].treasure = true;
        }

        pkt.put(MSG_TIME);
        uint32 t = (uint32)time(NULL)+GAME_TIME;
        pkt.put(t);
        pkt.put(t >> 8);
        pkt.put(t >> 16);
        pkt.put(t >> 24);
        pkt.put(PACKET_END);
        send(m_opponent, (const char*)pkt.pipe(), pkt.size(), 0);
        pkt.finalize();

        sCubic->timeend = t;

        sCubic->end_game = false;
        sCubic->got_turn = clock();
        sCubic->my_turn = true;

        sCubic->my_again = false;
        sCubic->op_again = false;
    }

    /////////////////////////////

    int32 cx = (GetMouseX()-BEGIN_X)/CELL_SIZE+1;
    int32 cy = (GetMouseY()-BEGIN_Y)/CELL_SIZE+1;

    if (cx < 1)
        cx = 1;
    if (cy < 1)
        cy = 1;
    if (cx > COUNT_X)
        cx = COUNT_X;
    if (cy > COUNT_Y)
        cy = COUNT_Y;

    FNTCOLOR_WHITE;
    SF->Drawing->PrintText(FNT(FONT_DEFAULT),10,10,0,0,L"X: %i, Y: %i",cx,cy);

    if (sCubic->message)
    {
        if (sCubic->message_time > clock())
            SF->Drawing->PrintText(FNT(FONT_DEFAULT),200,10,0,0,L"%S",sCubic->message);
        else
            sCubic->message = NULL;
    }

    DrawGrid();

    uint8 mytres = 0, optres = 0;

    for (uint32 x = 0; x < COUNT_X; x++)
    {
        for (uint32 y = 0; y < COUNT_Y; y++)
        {
            if (sCubic->cubes[x][y].present)
            {
                if (!sCubic->cubes[x][y].treasure)
                    SF->Drawing->DrawRectangle(BEGIN_X+(x)*CELL_SIZE+2, BEGIN_Y+(y)*CELL_SIZE+2, CELL_SIZE-3, CELL_SIZE-3, sCubic->cubes[x][y].player != 0 ? CLIENT_COLOR : SERVER_COLOR);
                else
                {
                    if ((sCubic->cubes[x][y].player == 0 && sCubic->i_am_server) || (sCubic->cubes[x][y].player != 0 && !sCubic->i_am_server))
                        mytres++;
                    else
                        optres++;

                    SF->Drawing->DrawRectangle(BEGIN_X+(x)*CELL_SIZE+2, BEGIN_Y+(y)*CELL_SIZE+2, CELL_SIZE-3, CELL_SIZE-3, sCubic->cubes[x][y].player != 0 ? CLIENT_WIN_COLOR : SERVER_WIN_COLOR);
                }
            }
        }
    }

    float sc1 = 10000*((float)mytres)/((float)sCubic->mytime > 0 ? sCubic->mytime : 1);
    float sc2 = 10000*((float)optres)/((float)sCubic->optime > 0 ? sCubic->optime : 1);

    FNTCOLOR_WHITE;
    if (!sCubic->end_game)
        SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+5, BEGIN_Y+520+10, 0,0,L"Moje skóre: %u (èas %.2fs)  Skóre protivníka: %u (èas %.2fs)", mytres, (float)sCubic->mytime/1000.0f, optres, (float)sCubic->optime/1000.0f);
    else
        SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+5, BEGIN_Y+520+10, 0,0,L"Moje skóre: %.2f  Skóre protivníka: %.2f", sc1, sc2);

    int32 remtime = (uint32)(sCubic->timeend - time(NULL));
    if (remtime > 0)
    {
        FNTCOLOR_YELLOW;
        SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+10, BEGIN_Y+10,0,0,L"%u:%02u",remtime/60,remtime%60);
        FNTCOLOR_WHITE;

        sCubic->end_game = false;
        sCubic->my_again = false;
        sCubic->op_again = false;
    }
    else
    {
        FNTCOLOR_RED;
        SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+10, BEGIN_Y+10,0,0,L"0:00");
        if (sc1 > sc2)
        {
            FNTCOLOR_YELLOW;
            SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+10, BEGIN_Y+40,0,0,L"Vyhrál jsi!");
        }
        else if (sc1 < sc2)
        {
            FNTCOLOR_RED;
            SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+10, BEGIN_Y+40,0,0,L"Prohrál jsi!");
        }
        else
        {
            FNTCOLOR_WHITE;
            SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+10, BEGIN_Y+40,0,0,L"Remíza");
        }

        sCubic->ShowMessage(1000,"Hra je u konce!");
        sCubic->my_turn = false;
        sCubic->end_game = true;

        uint32 clrr = MAKE_COLOR_RGBA(127,0,127,255);
        if (sCubic->my_again)
            clrr = MAKE_COLOR_RGBA(127,127,0,255);
        if (sCubic->op_again)
            clrr = MAKE_COLOR_RGBA(0,127,127,255);
        SF->Drawing->DrawRectangle(BEGIN_X+520+10, BEGIN_Y+70, 85, 34, clrr);
        FNTCOLOR_WHITE;
        SF->Drawing->PrintText(FNT(FONT_DEFAULT), BEGIN_X+520+20, BEGIN_Y+80,0,0,L"Znovu");
    }

    // 520x520

    if (sCubic->my_turn)
    {
        if (GetMouseX() > BEGIN_X && GetMouseX() < BEGIN_X+520 && GetMouseY() > BEGIN_Y && GetMouseY() < BEGIN_Y+520)
        {
            SF->Drawing->DrawRectangle(GetMouseX(), BEGIN_Y, 1, (COUNT_X)*CELL_SIZE+1, 0x444444FF);
            SF->Drawing->DrawRectangle(BEGIN_X, GetMouseY(), (COUNT_Y)*CELL_SIZE+1, 1, 0x444444FF);

            SF->Drawing->DrawRectangle(BEGIN_X+(cx-1)*CELL_SIZE+2, BEGIN_Y+(cy-1)*CELL_SIZE+2, CELL_SIZE-3, CELL_SIZE-3, MY_CUBE_COLOR);
        }
    }
}

void Application::RunGameplay()
{
    switch (stage)
    {
        case STAGE_BEGIN:
            RunBegin();
            break;
        case STAGE_CONNECTING:
            RunConnecting();
            break;
        case STAGE_WAITING:
            RunWaiting();
            break;
        case STAGE_GAME:
            RunGame();
            break;
    }
}

void Application::DrawGrid()
{
    // 30,30

    for (uint32 i = 0; i < COUNT_X+1; i++)
    {
        SF->Drawing->DrawRectangle(BEGIN_X+i*CELL_SIZE, BEGIN_Y, 1, (COUNT_X)*CELL_SIZE+1, 0x333333FF);
        SF->Drawing->DrawRectangle(BEGIN_X, BEGIN_Y+i*CELL_SIZE, (COUNT_Y)*CELL_SIZE+1, 1, 0x333333FF);
    }
}

void Application::ProcessPacket(Packet *pkt)
{
    if (!pkt)
        return;

    uint8 opcode = 0;

    while (!pkt->at_end())
    {
        switch (pkt->get())
        {
            case 0:
            default:
                break;
            case MSG_YOUR_TURN:
            {
                uint32 optime = 0;
                optime = pkt->get();
                optime |= ((uint32)pkt->get()) << 8;
                optime |= ((uint32)pkt->get()) << 16;
                optime |= ((uint32)pkt->get()) << 24;
                sCubic->optime += optime;

                sCubic->my_turn = true;
                sCubic->got_turn = clock();
                break;
            }
            case MSG_PUT_CUBE:
            {
                uint8 x = pkt->get();
                uint8 y = pkt->get();
                sCubic->cubes[x][y].present = true;
                sCubic->cubes[x][y].player = sCubic->i_am_server ? 1 : 0;

                if (sCubic->cubes[x][y].treasure)
                {
                    sCubic->ShowMessage(500, "Poklad!");
                    Packet res;
                    res.put(MSG_TREASURE);
                    res.put(x);
                    res.put(y);
                    res.put(PACKET_END);
                    send(m_opponent, (const char*)res.pipe(), res.size(), 0);
                    res.finalize();
                }
                break;
            }
            case MSG_TREASURES:
            {
                uint8 x,y,cnt;
                cnt = pkt->get();

                for (uint32 i = 0; i < cnt; i++)
                {
                    x = pkt->get();
                    y = pkt->get();
                    sCubic->cubes[x][y].treasure = true;
                }
                break;
            }
            case MSG_TREASURE:
            {
                sCubic->ShowMessage(500, "Poklad!");
                uint8 x = pkt->get();
                uint8 y = pkt->get();
                sCubic->cubes[x][y].treasure = true;
                break;
            }
            case MSG_TIME:
            {
                uint32 t = 0;
                t |= pkt->get();
                t |= ((uint32)pkt->get()) << 8;
                t |= ((uint32)pkt->get()) << 16;
                t |= ((uint32)pkt->get()) << 24;
                sCubic->timeend = t;
                break;
            }
            case MSG_AGAIN:
            {
                sCubic->op_again = true;
                break;
            }
            case MSG_CLEAR:
            {
                for (uint32 i = 0; i < COUNT_X; i++)
                {
                    for (uint32 j = 0; j < COUNT_Y; j++)
                    {
                        sCubic->cubes[i][j].present = false;
                        sCubic->cubes[i][j].treasure = false;
                    }
                }
                sCubic->mytime = 0;
                sCubic->optime = 0;
                sCubic->end_game = false;
                sCubic->ShowMessage(0, "");
                sCubic->my_turn = false;
                sCubic->timeend = time(NULL) + GAME_TIME;
                break;
            }
        }

        pkt->get(); // PACKET_END
    }
}

bool Application::CheckWin()
{
    return false;
}

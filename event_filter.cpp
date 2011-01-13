///////////////////////////////////////////////////////////////////////////////
//
// Name: event_filter.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "event_filter.h"

// xwindow
#include <X11/Xlib.h>
#include <glib.h>
extern "C" {
    int list_windows (Display *disp);
    Window *get_client_list (Display *disp, unsigned long *size);
    gchar * doro_get_window_title(Display *disp, Window *window);
    gchar * doro_get_window_icon(Display *disp, Window *window, unsigned long *size);
}



bool MouseEventFilter::update(EventPacketPtr p)
{
    if (p->getType() < MOUSE_DIRECTION_UP 
            || p->getType() > MOUSE_MAX)
        return true;
    char *buf = p->getPayload();

    // Mouse Click
    if (p->getType() == MOUSE_LCLICK) {
        system("xdotool click 1");
        return false;
    }
    if (p->getType() == MOUSE_RCLICK) {
        system("xdotool click 3");
        return false;
    }

    // Mouse Move
    short mouse_x = 0, mouse_y = 0;
    short movement = ((0x0FF & (int)(buf[1])) << 8) 
        + (0x0FF & (int)(buf[0]));
    short movement2 = ((0x0FF & (int)(buf[3])) << 8) 
        + (0x0FF & (int)(buf[2]));
    switch (p->getType()) {
        case MOUSE_DIRECTION_UP: 
            //mouse_y -= movement;
            break;
        case MOUSE_DIRECTION_RIGHT:
            //mouse_x += movement;
            break;
        case MOUSE_DIRECTION_DOWN:
            //mouse_y += movement;
            break;
        case MOUSE_DIRECTION_LEFT:
            //mouse_x -= movement;
            break;
        case MOUSE_MOVE:
            mouse_x += movement;
            mouse_y += movement2;
            break;
        default:
            break;
    }
    char command[128];
    snprintf(command, sizeof(command), "xdotool mousemove_relative -- %hd %hd", mouse_x, mouse_y);
    LOG_I("%s\n", command);

    system(command);
    
    return false;
}


bool KeyboardEventFilter::update(EventPacketPtr p)
{
    static const char * S_KEYBOARD_STR_MAP[KEY_MAX - MOUSE_MAX - 1] = {
        "Up",
        "Right",
        "Down",
        "Left",
        "Prior",
        "Next",
        "BackSpace",
        "Return"
    };

    if (p->getType() < KEY_UP 
            || p->getType() > KEY_MAX)
        return true;

    char *buf = p->getPayload();
    char command[128];
    if (p->getType() == KEY_CUSTOM) {
        if (buf[0] == ' ') {
            snprintf(command, sizeof(command), "xdotool key space");
        } else if (std::isalnum(buf[0])) {
            snprintf(command, sizeof(command), "xdotool key '%c'", buf[0]);
        } else {
            LOG_I("Not a alpha or number: keycode = %02X\n", buf[0]); 
            return false;
        }
    } else {
        const char *key = S_KEYBOARD_STR_MAP[p->getType() - MOUSE_MAX - 1];
        snprintf(command, sizeof(command), "xdotool key %s", key);
    }
    LOG_I("%s\n", command);
    system(command);

    return false;
}

bool WindowEventFilter::update(EventPacketPtr p) 
{
    if (p->getType() < SP_WIN_LIST
            || p->getType() > SP_WIN_LIST_ICON)
        return true;

    switch (p->getType()) {
        case SP_WIN_LIST: 
        {
            Window *client_list;
            unsigned long client_list_size;
            unsigned long icon_len;
            int iWindow;
            char *buf; 
            int lenBuf;
            Display *disp;

            // get display
            if (! (disp = XOpenDisplay(NULL))) {
                break;
            }

            // get window list
            if ((client_list = get_client_list(disp, &client_list_size)) == NULL) {
                XCloseDisplay(disp);
                break;
            }

            // send title
            buf = new char[MAX_BUFFER_SIZE];
            lenBuf = 0;
            for (iWindow = 0; iWindow < client_list_size / sizeof(Window); iWindow++) {
                gchar *title_out = doro_get_window_title(disp, client_list + iWindow);
                gchar *iterTitle = title_out;
                if (title_out == 0)
                    continue;
                LOG_V("title[%d]: %s, %d\n", iWindow, title_out, lenBuf);
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >>  0) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >>  8) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >> 16) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >> 24) & 0x0FF;
                while (*iterTitle != 0) {
                    buf[lenBuf++] = *iterTitle++;
                }
                buf[lenBuf++] = 0;
                g_free(title_out);
            }
            EventPacketPtr packet(new EventPacket(SP_WIN_LIST, buf, lenBuf));
            mQueue->push(packet);

            // send icon 
            for (iWindow = 0, lenBuf = 0; iWindow < client_list_size / sizeof(Window); iWindow++) {
                buf = new char[MAX_BUFFER_SIZE];
                gchar *icon= doro_get_window_icon(disp, client_list + iWindow, &icon_len);
                if (icon == 0)
                    continue;
                lenBuf = 0;
                LOG_V("icon[%d]: %ld\n", iWindow, icon_len);
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >>  0) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >>  8) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >> 16) & 0x0FF;
                buf[lenBuf++] = (char)((unsigned long)client_list[iWindow] >> 24) & 0x0FF;
                //memcpy(&(buf[lenBuf]), icon, icon_len);
                for (int i = 0; i < icon_len ; ++i) {
                    buf[lenBuf++] = icon[i];
                }
                g_free(icon);
                LOG_V("icon[%d]: %d\n", iWindow, lenBuf);
                EventPacketPtr packet(new EventPacket(SP_WIN_LIST_ICON, buf, lenBuf));
                mQueue->push(packet);
                //if (iWindow >= 8)
                //    break;
            }

            g_free(client_list);
            XCloseDisplay(disp);

            break;
        }
        case SP_WIN_FOCUS: 
        {
            int id;
            char command[128];
            char *buf = p->getPayload();
            id  = ((int)(buf[0]) & 0x000000FFL) <<  0;
            id |= ((int)(buf[1]) & 0x000000FFL) <<  8;
            id |= ((int)(buf[2]) & 0x000000FFL) << 16;
            id |= ((int)(buf[3]) & 0x000000FFL) << 24;
            snprintf(command, sizeof(command), "wmctrl -a 0x%08X -i", id);
            LOG_I("%s\n", command);
            system(command);
            break;
        }
    }
    return false;
}

bool FileEventFilter::update(EventPacketPtr p)
{
    if (p->getType() != SP_FILE_SEND)
        return true;
    LOG_V("FileEventFilter\n");
    //char buf[32768]; 
    //int iPayload;
    //int filesize;
    //int nRead;
    //int lenFilename;
    //FILE * fp = fopen(filename, "rb");

    //if (!fp) {
    //    LOG_E("file not found: [%s]\n", filename);
    //    return -1;
    //}

    //lenFilename = strlen(filename);
    //fseek(fp, 0L, SEEK_END);
    //filesize = ftell(fp);
    //fseek(fp, 0L, SEEK_SET);

    //iPayload = 0;
    //buf[iPayload++] = type;
    //buf[iPayload++] = 0;
    //buf[iPayload++] = 0;
    //buf[iPayload++] = 0;
    //buf[iPayload++] = 0;
    ////strcpy(buf + iPayload, filename);
    //memcpy(buf + iPayload, filename, lenFilename + 1);
    //iPayload += lenFilename + 1;       // 1 means null terminator
    //filesize += lenFilename + 1 + 5;   // 5 means header size
    //buf[1] = ((filesize >>  0) & 0x0FF);
    //buf[2] = ((filesize >>  8) & 0x0FF);
    //buf[3] = ((filesize >> 16) & 0x0FF);
    //buf[4] = ((filesize >> 24) & 0x0FF);


    //LOG_I("File = %s\n", filename);
    //while (!feof(fp)) {
    //    nRead = fread(buf + iPayload, sizeof(char), sizeof(buf) - iPayload, fp);
    //    write(g_res.Socket, buf, iPayload + nRead);
    //    LOG_I("nRead = %d\n", nRead);
    //    iPayload = 0;
    //}
    char * buf;
    int lenBuf;
    int nRead = 0;
    int retVal;
    int lenFilename;
    char *filename = p->getPayload();
    FILE * fp = fopen(filename, "rb");

    if (!fp) {
        LOG_E("file not found: [%s]\n", filename);
        return false;
    }

    lenFilename = strlen(filename) + 1;
    fseek(fp, 0L, SEEK_END);
    lenBuf = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    LOG_V("lenBuf = %d, lenFilename = %d\n", lenBuf, lenFilename);
    lenBuf += lenFilename;   // 5 means header size

    buf = new char[lenBuf];
    if (buf == NULL) {
        fclose(fp);
        return false;
    }
    memcpy(buf, filename, lenFilename + 1);

    while ((lenBuf - lenFilename) > nRead && !feof(fp)) {
        retVal = fread(buf + lenFilename + nRead, sizeof(char), lenBuf - lenFilename - nRead, fp);
        if (retVal > 0) {
            nRead += retVal;
        } else {
            fclose(fp);
            delete buf;
            return false;
        }
        LOG_V("nRead = %d\n", nRead);
    }
    LOG_V("lenBuf = %d, lenFilename = %d, nRead = %d\n", lenBuf, lenFilename, nRead);
    EventPacketPtr packet(new EventPacket(SP_FILE_SEND, buf, lenBuf));
    mQueue->push(packet);

    fclose(fp);

    //fclose(fp);
    return false;
}


bool WebcamEventFilter::update(EventPacketPtr p)
{
    if (p->getType() != SP_WEBCAM)
        return true;
    LOG_V("WebcamEventFilter\n");
    do {
        char * buf;
        int lenBuf;
        int nRead = 0;
        int retVal;
        FILE * fp = fopen("/home/dorowu/devel/motion/motion-3.2.12/pic/lastsnap.jpg", "rb");

        if (!fp) {
            LOG_E("file not found: [%s]\n", "/home/dorowu/devel/motion/motion-3.2.12/lastsnap.jpg");
            sleep(1);
            continue;
        }

        fseek(fp, 0L, SEEK_END);
        lenBuf = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        buf = new char[lenBuf];

        while (lenBuf > nRead && !feof(fp)) {
            retVal = fread(buf + nRead, sizeof(char), lenBuf - nRead, fp);
            if (retVal > 0) {
                nRead += retVal;
            } else {
                continue;
            }
        }
        EventPacketPtr packet(new EventPacket(SP_WEBCAM, buf, lenBuf));
        mQueue->push(packet);

        fclose(fp);
        break;
    } while (1);

    return false;
}

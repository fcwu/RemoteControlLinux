///////////////////////////////////////////////////////////////////////////////
//
// Name: session.cpp
// Author: doro_wu@asus.com
// 
///////////////////////////////////////////////////////////////////////////////

#include "session.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <dbus/dbus.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#include <X11/Xlib.h>
#include <glib.h>

const int MAX_LENGTH = 32768;

extern "C" {
    int list_windows (Display *disp);
    Window *get_client_list (Display *disp, unsigned long *size);
    gchar * doro_get_window_title(Display *disp, Window *window);
    gchar * doro_get_window_icon(Display *disp, Window *window, unsigned long *size);
}


static const char * g_keyboard_str_map[KEY_MAX - MOUSE_MAX - 1] = {
    "Up",
    "Right",
    "Down",
    "Left",
    "Prior",
    "Next",
    "BackSpace",
    "Return"
};

static void session_sp_cmd(char *buf) 
{
    unsigned int data_start = 0;

    switch (buf[data_start]) {
        case SP_WIN_LIST: 
            {
                Window *client_list;
                unsigned long client_list_size;
                unsigned long icon_len;
                int iWindow;
                char payload[32768]; 
                int iPayload;
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
                iPayload = 0;
                buf[iPayload++] = SP_WIN_LIST;
                buf[iPayload++] = 0;
                buf[iPayload++] = 0;
                buf[iPayload++] = 0;
                buf[iPayload++] = 0;
                for (iWindow = 0; iWindow < client_list_size / sizeof(Window); iWindow++) {
                    gchar *title_out = doro_get_window_title(disp, client_list + iWindow);
                    gchar *iterTitle = title_out;
                    if (title_out == 0)
                        continue;
                    LOG_I("title[%d]: %s, %d\n", iWindow, title_out, iPayload);
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >>  0) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >>  8) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >> 16) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >> 24) & 0x0FF;
                    while (*iterTitle != 0) {
                        buf[iPayload++] = *iterTitle++;
                    }
                    buf[iPayload++] = 0;
                    g_free(title_out);
                }
                buf[1] = ((iPayload >>  0) & 0x0FF);
                buf[2] = ((iPayload >>  8) & 0x0FF);
                buf[3] = ((iPayload >> 16) & 0x0FF);
                buf[4] = ((iPayload >> 24) & 0x0FF);
                write(g_res.Socket, buf, iPayload);
                //boost::asio::write(*sock, boost::asio::buffer(buf, iPayload));

                //// send icon 
                for (iWindow = 0, iPayload = 0; iWindow < client_list_size / sizeof(Window); iWindow++) {
                    gchar *icon= doro_get_window_icon(disp, client_list + iWindow, &icon_len);
                    if (icon == 0)
                        continue;
                    iPayload = 0;
                    LOG_I("icon[%d]: %ld\n", iWindow, icon_len);
                    buf[iPayload++] = SP_WIN_LIST_ICON;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >>  0) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >>  8) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >> 16) & 0x0FF;
                    buf[iPayload++] = (char)((unsigned long)client_list[iWindow] >> 24) & 0x0FF;
                    //memcpy(&(buf[iPayload]), icon, icon_len);
                    for (int i = 0; i < icon_len ; ++i) {
                        buf[iPayload++] = icon[i];
                    }
                    g_free(icon);
                    //iPayload += icon_len;
                    buf[1] = ((iPayload >>  0) & 0x0FF);
                    buf[2] = ((iPayload >>  8) & 0x0FF);
                    buf[3] = ((iPayload >> 16) & 0x0FF);
                    buf[4] = ((iPayload >> 24) & 0x0FF);
                    LOG_I("icon[%d]: %d\n", iWindow, iPayload);
                    //boost::asio::write(*sock, boost::asio::buffer(buf, iPayload));
                    write(g_res.Socket, buf, iPayload);
                    //if (iWindow >= 8)
                    //    break;
                }

                g_free(client_list);
                XCloseDisplay(disp);
            }
            break;
        case SP_WIN_FOCUS: 
            {
                int id;
                char command[128];
                id  = ((int)(buf[1]) & 0x000000FFL) <<  0;
                id |= ((int)(buf[2]) & 0x000000FFL) <<  8;
                id |= ((int)(buf[3]) & 0x000000FFL) << 16;
                id |= ((int)(buf[4]) & 0x000000FFL) << 24;
                snprintf(command, sizeof(command), "wmctrl -a 0x%08X -i", id);
                LOG_I("%s\n", command);
                cout << command << std::endl;
                system(command);
            }
            break;
        case SP_WEBCAM:
            {
                do {
                    char buf[32768*3]; 
                    int iPayload;
                    int filesize;
                    int nRead;
                    FILE * fp = fopen("/home/dorowu/devel/motion/motion-3.2.12/pic/lastsnap.jpg", "rb");
                    //FILE * fp = fopen("/tmp/motion/01-20110102040827-00.jpg", "rb");

                    if (!fp) {
                        LOG_E("file not found: [%s]\n", "/home/dorowu/devel/motion/motion-3.2.12/lastsnap.jpg");
                        sleep(1);
                        continue;
                    }

                    fseek(fp, 0L, SEEK_END);
                    filesize = ftell(fp);
                    fseek(fp, 0L, SEEK_SET);

                    iPayload = 0;
                    buf[iPayload++] = SP_WEBCAM;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    buf[iPayload++] = 0;
                    iPayload += filesize;
                    buf[1] = ((iPayload >>  0) & 0x0FF);
                    buf[2] = ((iPayload >>  8) & 0x0FF);
                    buf[3] = ((iPayload >> 16) & 0x0FF);
                    buf[4] = ((iPayload >> 24) & 0x0FF);
                    iPayload = 5;


                    while (!feof(fp)) {
                        nRead = fread(buf + iPayload, sizeof(char), sizeof(buf) - iPayload, fp);
                        write(g_res.Socket, buf, iPayload + nRead);
                        LOG_I("nRead = %d [5] = %d\n", nRead, buf[5]);
                        iPayload = 0;
                    }

                    fclose(fp);
                    break;
                } while (1);
            }
            break;
    }
}

static int dispatcher_socket()
{
    try
    {
        char buf[MAX_LENGTH];
        fd_set rfds;
        struct timeval tv;
        int retval;

        /* Watch stdin (fd 0) to see when it has input. */
        FD_ZERO(&rfds);
        FD_SET(g_res.Socket, &rfds);

        /* Wait up to one seconds. */
        tv.tv_sec = 1;
        tv.tv_usec = 0;


        retval = select(g_res.Socket + 1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (retval == -1) {
            perror("select()");
            throw ;
        } else if (retval) {
            //LOG_I("Data is available now.\n");
            /* FD_ISSET(0, &rfds) will be true. */
        } else {
            //LOG_I("No data within 1 seconds\n");
            return 0;
        }


        size_t length = read(g_res.Socket, buf, sizeof(buf));
        if (length == 0) {
            throw; // Connection closed cleanly by peer.
        } else if (length < 0) {
            throw ;
        } else if (length > sizeof(buf)) {
            return 0;
        }

        for (int i = 0; i < length; ++i) {
            cout << boost::format("0x%02X ") % (0x00FF & (int)buf[i]);
        }
        cout << std::endl;

        unsigned int data_start;
        for (data_start = 0; length > 0; length -= 5, data_start += 5) {

            // Special commands
            if (buf[data_start] > KEY_MAX && buf[data_start] < SP_MAX) {
                session_sp_cmd(buf + data_start);
                continue;
            }

            // Keyboard
            if (buf[data_start] > MOUSE_MAX && buf[data_start] < KEY_MAX) {
                char command[128];
                if (buf[data_start] == KEY_CUSTOM) {
                    if (buf[data_start + 1] == ' ') {
                        snprintf(command, sizeof(command), "xdotool key space");
                    } else if (std::isalnum(buf[data_start + 1])) {
                        snprintf(command, sizeof(command), "xdotool key '%c'", buf[data_start + 1]);
                    } else {
                        std::cout << "Not a alpha or number: keycode = " << (int)buf[data_start + 1] << std::endl;
                        continue;
                    }
                } else {
                    const char *key = g_keyboard_str_map[buf[data_start] - MOUSE_MAX - 1];
                    snprintf(command, sizeof(command), "xdotool key %s", key);
                }
                std::cout << command << std::endl;
                system(command);

                continue;
            }

            // Mouse Click
            if (buf[data_start] == MOUSE_LCLICK) {
                system("xdotool click 1");
                continue;
            }
            if (buf[data_start] == MOUSE_RCLICK) {
                system("xdotool click 3");
                continue;
            }

            // Mouse Move
            short mouse_x = 0, mouse_y = 0;
            short movement = ((0x0FF & (int)(buf[data_start + 2])) << 8) 
                + (0x0FF & (int)(buf[data_start + 1]));
            short movement2 = ((0x0FF & (int)(buf[data_start + 4])) << 8) 
                + (0x0FF & (int)(buf[data_start + 3]));
            switch (buf[data_start]) {
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
            std::cout << command << std::endl;

            system(command);

        } //end for

    }
    catch (std::exception& e)
    {
        cerr << "Exception in thread: " << e.what() << "\n";
        return -1;
    }
    catch (...)
    {
        cerr << "Exception in thread: " << "Unknown" << "\n";
        return -1;
    }

    return 0;
}

static void file_send(const char *filename)
{
    int iBuf = 0;
    int iFile = 0;
    int iRead;
    int fileSize;
    char buf[MAX_LENGTH];

    FILE *fp = fopen("rb", filename);
    if (!fp) {
        cerr << "Failed to open file " << filename << endl;
        return ;
    }

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);

    cout << "Sending file " << filename << " which size is " << fileSize << endl;

    // ID | FILE_SIZE | DATA
    buf[iBuf++] = SP_FILE_SEND;
    buf[iBuf++] = ((fileSize >>  0) & 0x0FF);
    buf[iBuf++] = ((fileSize >>  8) & 0x0FF);
    buf[iBuf++] = ((fileSize >> 16) & 0x0FF);
    buf[iBuf++] = ((fileSize >> 24) & 0x0FF);
    while ((iRead = read(fileno(fp), &buf[iBuf], MAX_LENGTH - iBuf)) > 0) {
        iFile += iRead;
        write(g_res.Socket, buf, iRead);
        iBuf = 0;
    }

    cout << "Sent file " << filename << " finished\n";
    cout << "  Expected size: " << fileSize << " Transfered size: " << iFile << endl;

}

static int session_sp_cmd_send_file(int type, const char * filename)
{
    char buf[32768]; 
    int iPayload;
    int filesize;
    int nRead;
    int lenFilename;
    FILE * fp = fopen(filename, "rb");

    if (!fp) {
        LOG_E("file not found: [%s]\n", filename);
        return -1;
    }

    lenFilename = strlen(filename);
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    iPayload = 0;
    buf[iPayload++] = type;
    buf[iPayload++] = 0;
    buf[iPayload++] = 0;
    buf[iPayload++] = 0;
    buf[iPayload++] = 0;
    //strcpy(buf + iPayload, filename);
    memcpy(buf + iPayload, filename, lenFilename + 1);
    iPayload += lenFilename + 1;       // 1 means null terminator
    filesize += lenFilename + 1 + 5;   // 5 means header size
    buf[1] = ((filesize >>  0) & 0x0FF);
    buf[2] = ((filesize >>  8) & 0x0FF);
    buf[3] = ((filesize >> 16) & 0x0FF);
    buf[4] = ((filesize >> 24) & 0x0FF);


    LOG_I("File = %s\n", filename);
    while (!feof(fp)) {
        nRead = fread(buf + iPayload, sizeof(char), sizeof(buf) - iPayload, fp);
        write(g_res.Socket, buf, iPayload + nRead);
        LOG_I("nRead = %d\n", nRead);
        iPayload = 0;
    }

    fclose(fp);
}


int Session::start()
{
    for (;;)
    {
        dispatcher_dbus();
        if (dispatcher_socket() < 0)
            break;
    }

    // close connection
    close(sock);
}


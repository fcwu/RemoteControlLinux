///////////////////////////////////////////////////////////////////////////////
//
// Name: session.cpp
// Author: doro_wu@asus.com
// 
// Copyright 2010 by Doro Wu. All right reserved
//
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <boost/format.hpp>
#include <dbus/dbus.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#include <X11/Xlib.h>
#include <glib.h>

using std::cout;
using std::cerr;
using std::endl;

const int MAX_LENGTH = 32768;

extern "C" {
    int list_windows (Display *disp);
    Window *get_client_list (Display *disp, unsigned long *size);
    gchar * doro_get_window_title(Display *disp, Window *window);
    gchar * doro_get_window_icon(Display *disp, Window *window, unsigned long *size);
}


enum {
    MOUSE_DIRECTION_UP = 0,
    MOUSE_DIRECTION_RIGHT,
    MOUSE_DIRECTION_DOWN,
    MOUSE_DIRECTION_LEFT,
    MOUSE_LCLICK,
    MOUSE_RCLICK,
    MOUSE_MOVE,
    MOUSE_MAX,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_CUSTOM,
    KEY_MAX,
    SP_WIN_LIST,
    SP_WIN_FOCUS,
    SP_WIN_LIST_ICON,
    SP_FILE_SEND,
    SP_MAX
};

class SessionResource
{
    public:
        SessionResource() {}
        SessionResource(int socket, DBusConnection * conn):
            Socket(socket), DBusConn(conn)
        {
        }

        int Socket;
        DBusConnection * DBusConn;
};

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

#define LOG_I(pcString, ...) \
    do {\
        fprintf(stderr, "%s(%d): ", __FILE__, __LINE__);\
        fprintf(stderr, pcString, ## __VA_ARGS__);\
    } while (0)

static SessionResource g_res;

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
                    printf("title[%d]: %s, %d\n", iWindow, title_out, iPayload);
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
                    printf("icon[%d]: %ld\n", iWindow, icon_len);
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
                    printf("icon[%d]: %d\n", iWindow, iPayload);
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
                std::cout << command << std::endl;
                system(command);
            }
            break;
    }
}

static void session_socket()
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
            return;
        } else if (retval) {
            //printf("Data is available now.\n");
            /* FD_ISSET(0, &rfds) will be true. */
        } else {
            //printf("No data within 1 seconds\n");
            return;
        }


        size_t length = read(g_res.Socket, buf, sizeof(buf));
        if (length == 0)
            return ; // Connection closed cleanly by peer.
        else if (length < 0)
            throw ;
        else if (length > sizeof(buf))
            return ;

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
    }
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

static int session_sp_cmd_send_file(const char * filename)
{
    char buf[32768]; 
    int iPayload;
    int filesize;
    int nRead;
    FILE * fp = fopen(filename, "rb");

    if (!fp) {
        fprintf(stderr, "file not found: %s\n", filename);
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    iPayload = 0;
    buf[iPayload++] = SP_FILE_SEND;
    buf[iPayload++] = ((filesize >>  0) & 0x0FF);
    buf[iPayload++] = ((filesize >>  8) & 0x0FF);
    buf[iPayload++] = ((filesize >> 16) & 0x0FF);
    buf[iPayload++] = ((filesize >> 24) & 0x0FF);

    LOG_I("File = %s\n", filename);
    while (!feof(fp)) {
        nRead = fread(buf + iPayload, sizeof(char), sizeof(buf) - iPayload, fp);
        write(g_res.Socket, buf, iPayload + nRead);
        LOG_I("nRead = %d\n", nRead);
        iPayload = 0;
    }

    fclose(fp);
}

static void session_dbus()
{
    DBusMessage* msg;
    DBusMessageIter args;
    char* sigvalue;

    // non blocking read of the next available message
    dbus_connection_read_write(g_res.DBusConn, 0);
    msg = dbus_connection_pop_message(g_res.DBusConn);

    // loop again if we haven't read a message
    if (NULL == msg) { 
        return;
    }

    // check if the message is a signal from the correct interface
    // and with the correct name
    if (dbus_message_is_signal(msg, "server.file.signal.Type", "send")) {
        // read the parameters
        if (!dbus_message_iter_init(msg, &args)) {
            fprintf(stderr, "Message has no arguments!\n"); 
        } else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
            fprintf(stderr, "Argument is not string!\n"); 
        } else {
            dbus_message_iter_get_basic(&args, &sigvalue);
            printf("Got Signal with value %s\n", sigvalue);
            session_sp_cmd_send_file(sigvalue);
        }
    } else {
    }

    // free the message
    dbus_message_unref(msg);
}


void session(int sock, DBusConnection *conn)
{
    g_res.Socket = sock;
    g_res.DBusConn = conn;

    for (;;)
    {
        session_dbus();
        session_socket();
    }

    // close connection
    close(sock);
}

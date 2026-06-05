/*
 * Copyright (c) 2026 vec2pt
 *
 * Licensed under the MIT License.
 * See the LICENSE for details.
 */

#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main()
{
    xcb_connection_t* connection;
    xcb_screen_t* screen;
    xcb_get_geometry_reply_t* geom = XCB_NONE;
    xcb_button_press_event_t start = { 0 };
    xcb_generic_event_t* ev;

    if (xcb_connection_has_error(connection = xcb_connect(NULL, NULL))) return 1;
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    xcb_key_symbols_t* symbols = xcb_key_symbols_alloc(connection);
    xcb_keycode_t* codes = xcb_key_symbols_get_keycode(symbols, 0xffbe); /* F1 */
    xcb_grab_key(connection, 0, screen->root,
        XCB_MOD_MASK_ANY, codes[0],
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    xcb_grab_button(connection, 0, screen->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, screen->root, XCB_NONE, 1, XCB_MOD_MASK_1);
    xcb_grab_button(connection, 0, screen->root,
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION,
        XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, screen->root, XCB_NONE, 3, XCB_MOD_MASK_1);
    xcb_flush(connection);

    start.child = XCB_NONE;
    while ((ev = xcb_wait_for_event(connection)))
    {
        uint8_t response_type = ev->response_type & ~0x80;
        if (response_type == XCB_KEY_PRESS)
        {
            xcb_key_press_event_t* e = (xcb_key_press_event_t*)ev;
            if (e->detail == codes[0] && e->child != XCB_NONE)
            {
                uint32_t values[] = { XCB_STACK_MODE_ABOVE };
                xcb_configure_window(connection, e->child, XCB_CONFIG_WINDOW_STACK_MODE, values);
                xcb_flush(connection);
            }
        }
        else if (response_type == XCB_BUTTON_PRESS)
        {
            xcb_button_press_event_t* e = (xcb_button_press_event_t*)ev;
            if (e->child != XCB_NONE)
            {
                geom = xcb_get_geometry_reply(connection, xcb_get_geometry(connection, e->child), NULL);
                start = *e;
            }
        }
        else if (response_type == XCB_MOTION_NOTIFY && start.child != XCB_NONE)
        {
            xcb_button_press_event_t* e = (xcb_button_press_event_t*)ev;

            int xdiff = e->root_x - start.root_x;
            int ydiff = e->root_y - start.root_y;

            uint32_t values[4] = {
                geom->x + (start.detail == 1 ? xdiff : 0),
                geom->y + (start.detail == 1 ? ydiff : 0),
                MAX(1, geom->width + (start.detail == 3 ? xdiff : 0)),
                MAX(1, geom->height + (start.detail == 3 ? ydiff : 0))
            };
            xcb_configure_window(connection, start.child,
                XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                values);

            xcb_flush(connection);
        }
        else if (response_type == XCB_BUTTON_RELEASE)
            start.child = XCB_NONE;

        free(ev);
    }

    free(geom);
    xcb_key_symbols_free(symbols);
    xcb_disconnect(connection);
}

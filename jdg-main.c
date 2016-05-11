/*
 *  JLU Drcom Gtk - Client to login to Jilin University network
 *  Copyright (C) 2016  Jonathan Kang
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <glib-object.h>

#include <string.h>
#include <arpa/inet.h>

#include "drcom.h"

static int sock;
static guint source_id;

static void
on_destroy (GtkWidget *widget,
            gpointer user_data)
{
    GPtrArray *array = (GPtrArray *) user_data;

    g_ptr_array_free (array, FALSE);

    if (source_id)
    {
        g_source_remove (source_id);
        source_id = 0;
    }

    gtk_main_quit ();
}

static void
on_check_button_toggled (GtkWidget *check_button,
                         gpointer user_data)
{
    GKeyFile *key_file;

    key_file = g_key_file_new ();
    g_key_file_load_from_file (key_file, "jdg.ini",
                               G_KEY_FILE_NONE, NULL);

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button)))
    {
        g_key_file_set_boolean (key_file,
                                "config", "remember_password", TRUE);
    }
    else
    {
        g_key_file_set_boolean (key_file,
                                "config", "remember_password", FALSE);

        if (g_key_file_has_key (key_file, "config", "username", NULL))
        {
            g_key_file_remove_key (key_file, "config", "username", NULL);
            g_key_file_remove_key (key_file, "config", "password", NULL);
        }
    }

    g_key_file_save_to_file (key_file, "jdg.ini", NULL);

    g_key_file_free (key_file);
}

static void
on_login_button_clicked (GtkWidget *button,
                         gpointer user_data)
{
    GPtrArray *array;
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;

    array = (GPtrArray *) user_data;
    username_buffer = g_ptr_array_index (array, 0);
    password_buffer = g_ptr_array_index (array, 1);

    if (gtk_entry_buffer_get_length (username_buffer) == 0
        || gtk_entry_buffer_get_length (password_buffer) == 0)
    {
        GtkWidget *dialog;
        GtkWidget *toplevel;

        toplevel = gtk_widget_get_toplevel (button);
        dialog = gtk_message_dialog_new (GTK_WINDOW (toplevel),
                                         GTK_DIALOG_MODAL,
                                         GTK_MESSAGE_WARNING,
                                         GTK_BUTTONS_CLOSE,
                                         "Please input username or password");

        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
    }
    else
    {
        const char *username;
        const char *password;
        GKeyFile *key_file;
        GtkWidget *check_button;
        GtkWidget *revealer;
        GtkWidget *stack;
        GtkWidget *text_view;
        GtkTextBuffer *text_buffer;

        /* get username and password from entry buffers */
        username = gtk_entry_buffer_get_text (username_buffer);
        password = gtk_entry_buffer_get_text (password_buffer);

        check_button = g_ptr_array_index (array, 2);
        revealer = g_ptr_array_index (array, 3);
        stack = g_ptr_array_index (array, 4);
        text_view = g_ptr_array_index (array, 5);

        text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

        key_file = g_key_file_new ();
        g_key_file_load_from_file (key_file, "jdg.ini",
                                   G_KEY_FILE_NONE, NULL);

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button)))
        {
            if (g_key_file_has_key (key_file, "config", "username", NULL))
            {
                gchar *str_un;

                str_un = g_key_file_get_string (key_file,
                                                "config", "username", NULL);

                if (g_strcmp0 (username, str_un) != 0)
                {
                    g_key_file_set_string (key_file,
                                           "config", "username", username);
                    g_key_file_set_string (key_file,
                                           "config", "password", password);
                }

                g_free (str_un);
            }
            else
            {
                g_key_file_set_string (key_file,
                                       "config", "username", username);
                g_key_file_set_string (key_file,
                                       "config", "password", password);
            }
        }
        else
        {
            if (g_key_file_has_key (key_file, "config", "username", NULL))
            {
                g_key_file_remove_key (key_file, "config", "username", NULL);
                g_key_file_remove_key (key_file, "config", "password", NULL);
            }
        }

        g_key_file_save_to_file (key_file, "jdg.ini", NULL);

        g_key_file_free (key_file);

        gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), TRUE);
        gtk_stack_set_visible_child_name (GTK_STACK (stack), "info");

        /* login */
        int ret;
        unsigned char send_data[SEND_DATA_SIZE];
        char recv_data[RECV_DATA_SIZE];
        struct sockaddr_in serv_addr;
        struct user_info_pkt user_info;

        sock = socket (AF_INET, SOCK_DGRAM, 0);
        if (sock < 0)
        {
            gtk_text_buffer_insert_at_cursor (text_buffer,
                                              "[drcom]: create sock failed.\n",
                                              strlen ("[drcom]: create sock failed.\n"));
            fprintf (stdout, "[drcom]: create sock failed.\n");
            //exit (EXIT_FAILURE);
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
        serv_addr.sin_port = htons(SERVER_PORT);

        // get user information
        get_user_info (&user_info, username, password);

        // challenge data length 20
        challenge (sock, serv_addr,
                   send_data, 20, recv_data, RECV_DATA_SIZE, text_buffer);

        // login data length 338, salt length 4
        set_login_data (&user_info,
                        send_data, 338, (unsigned char *)(recv_data + 4), 4);
        memset (recv_data, 0x00, RECV_DATA_SIZE);
        login (sock, serv_addr,
               send_data, 338, recv_data, RECV_DATA_SIZE, text_buffer);

        random_num = rand () % 0xFFFF;
        keep_alive (&sock);
        source_id = g_timeout_add_full (G_PRIORITY_DEFAULT, 5000,
                                        keep_alive, &sock, NULL);
    }
}

static void
on_logout_button_clicked (GtkWidget *button,
                          gpointer user_data)
{
    GPtrArray *array;
    GtkEntryBuffer *password_buffer;
    GtkWidget *check_button;
    GtkWidget *revealer;
    GtkWidget *stack;

    array = (GPtrArray *) user_data;
    password_buffer = g_ptr_array_index (array, 1);
    check_button = g_ptr_array_index (array, 2);
    revealer = g_ptr_array_index (array, 3);
    stack = g_ptr_array_index (array, 4);

    gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), FALSE);
    gtk_stack_set_visible_child_name (GTK_STACK (stack), "login");

    if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (check_button)))
    {
        const gchar *password;

        password = gtk_entry_buffer_get_text (password_buffer);
        gtk_entry_buffer_delete_text (password_buffer, 0, strlen (password));
    }

    /* stop keep_alive */
    if (source_id)
    {
        g_source_remove (source_id);
        source_id = 0;
    }
}

int
main (int argc, char *argv[])
{
    GdkPixbuf *icon;
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *grid;
    GtkWidget *username_label;
    GtkWidget *password_label;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *check_button;
    GtkWidget *login_button;
    GtkWidget *logout_button;
    GtkWidget *revealer;
    GtkWidget *stack;
    GtkWidget *text_view;
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;
    GPtrArray *array;
    GKeyFile *key_file;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "jlu-drcom-gtk");
    gtk_window_set_default_size (GTK_WINDOW (window), 600, 200);
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

    icon = gdk_pixbuf_new_from_file ("jdg.png", NULL);
    gtk_window_set_icon (GTK_WINDOW (window), icon);

    username_label = gtk_label_new ("Username");
    password_label = gtk_label_new ("Password");

    username_buffer = gtk_entry_buffer_new (NULL, -1);
    password_buffer = gtk_entry_buffer_new (NULL, -1);
    username_entry = gtk_entry_new_with_buffer (username_buffer);
    password_entry = gtk_entry_new_with_buffer (password_buffer);
    gtk_entry_set_input_purpose (GTK_ENTRY (password_entry),
                                 GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility (GTK_ENTRY (password_entry), FALSE);

    check_button = gtk_check_button_new_with_label ("Remember Password");
    login_button = gtk_button_new_with_label ("Login");

    revealer = gtk_revealer_new ();
    gtk_revealer_set_transition_duration (GTK_REVEALER (revealer), 500);

    /* Stack to switch between login page and info page */
    stack = gtk_stack_new ();

    /* GtkTextView to show output information */
    text_view = gtk_text_view_new ();
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
    gtk_widget_set_size_request (text_view, -1, 200);
    gtk_widget_set_vexpand (text_view, TRUE);
    gtk_widget_set_visible (text_view, TRUE);

    gtk_container_add (GTK_CONTAINER (revealer), text_view);

    array = g_ptr_array_new ();
    g_ptr_array_add (array, (gpointer) username_buffer);
    g_ptr_array_add (array, (gpointer) password_buffer);
    g_ptr_array_add (array, (gpointer) check_button);
    g_ptr_array_add (array, (gpointer) revealer);
    g_ptr_array_add (array, (gpointer) stack);
    g_ptr_array_add (array, (gpointer) text_view);

    key_file = g_key_file_new ();
    g_key_file_load_from_file (key_file, "jdg.ini",
                               G_KEY_FILE_NONE, NULL);
    if (g_key_file_get_boolean (key_file, "config", "remember_password", NULL))
    {
        gchar *username;
        gchar *password;

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), TRUE);

        username = g_key_file_get_string (key_file,
                                          "config", "username", NULL);
        password = g_key_file_get_string (key_file,
                                          "config", "password", NULL);

        if (username != NULL && password != NULL)
        {
            gtk_entry_buffer_set_text (username_buffer, username, -1);
            gtk_entry_buffer_set_text (password_buffer, password, -1);
        }

        g_free (username);
        g_free (password);
    }
    else
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), FALSE);
    }
    g_key_file_free (key_file);

    /* Login grid */
    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 8);

    gtk_grid_attach (GTK_GRID (grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), check_button, 0, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), login_button, 1, 2, 1, 1);

    logout_button = gtk_button_new_with_label ("logout");
    gtk_widget_set_halign (logout_button, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (logout_button, GTK_ALIGN_CENTER);

    gtk_stack_add_named (GTK_STACK (stack), grid, "login");
    gtk_stack_add_named (GTK_STACK (stack), logout_button, "info");
    gtk_stack_set_visible_child_name (GTK_STACK (stack), "login");

    main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start (main_box, 5);
    gtk_widget_set_margin_end (main_box, 5);
    gtk_box_pack_start (GTK_BOX (main_box), stack,
                        TRUE, TRUE, 5);
    gtk_box_pack_end (GTK_BOX (main_box), revealer,
                      TRUE, TRUE, 5);

    gtk_container_add (GTK_CONTAINER (window), main_box);
    gtk_widget_show_all (window);

    /* Signals */
    g_signal_connect (window, "destroy", G_CALLBACK (on_destroy), array);
    g_signal_connect (password_entry, "activate",
                      G_CALLBACK (on_login_button_clicked), array);
    g_signal_connect (check_button, "toggled",
                      G_CALLBACK (on_check_button_toggled), NULL);
    g_signal_connect (login_button, "clicked",
                      G_CALLBACK (on_login_button_clicked), array);
    g_signal_connect (logout_button, "clicked",
                      G_CALLBACK (on_logout_button_clicked), array);

    g_object_unref (icon);

    gtk_main ();

    return 0;
}

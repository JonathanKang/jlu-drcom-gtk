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

#include "drcom.h"

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
    }

    g_key_file_save_to_file (key_file, "jdg.ini", NULL);

    g_key_file_free (key_file);
}

static void
on_login_button_clicked (GtkWidget *button,
                         gpointer user_data)
{
    const char *username;
    const char *password;
    GKeyFile *key_file;
    GPtrArray *buffer_array;
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;
    GtkWidget *check_button;
    GtkWidget *revealer;
    GtkWidget *stack;
    GtkWidget *text_view;
    GtkTextBuffer *text_buffer;

    buffer_array = (GPtrArray *) user_data;
    username_buffer = g_ptr_array_index (buffer_array, 0);
    password_buffer = g_ptr_array_index (buffer_array, 1);
    check_button = g_ptr_array_index (buffer_array, 2);
    revealer = g_ptr_array_index (buffer_array, 3);
    stack = g_ptr_array_index (buffer_array, 4);

    /* GtkTextView to show output information */
    text_view = gtk_text_view_new ();
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
    gtk_widget_set_size_request (text_view, -1, 400);
    gtk_widget_set_vexpand (text_view, TRUE);
    gtk_widget_set_visible (text_view, TRUE);
    text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    gtk_container_add (GTK_CONTAINER (revealer), text_view);
    gtk_widget_show (revealer);
    gtk_revealer_set_reveal_child (GTK_REVEALER (revealer), TRUE);

    gtk_stack_set_visible_child_name (GTK_STACK (stack), "info");

    /* get username and password from entry buffers */
    username = gtk_entry_buffer_get_text (username_buffer);
    password = gtk_entry_buffer_get_text (password_buffer);

    g_ptr_array_free (buffer_array, FALSE);

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
            g_key_file_set_string (key_file, "config", "username", username);
            g_key_file_set_string (key_file, "config", "password", password);
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

    //on_login (username, password);
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
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;
    GPtrArray *buffer_array;
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
    gtk_widget_hide (revealer);

    /* Stack to switch between login page and info page */
    stack = gtk_stack_new ();

    buffer_array = g_ptr_array_new ();
    g_ptr_array_add (buffer_array, (gpointer) username_buffer);
    g_ptr_array_add (buffer_array, (gpointer) password_buffer);
    g_ptr_array_add (buffer_array, (gpointer) check_button);
    g_ptr_array_add (buffer_array, (gpointer) revealer);
    g_ptr_array_add (buffer_array, (gpointer) stack);

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
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (password_entry, "activate",
                      G_CALLBACK (on_login_button_clicked), buffer_array);
    g_signal_connect (check_button, "toggled",
                      G_CALLBACK (on_check_button_toggled), NULL);
    g_signal_connect (login_button, "clicked",
                      G_CALLBACK (on_login_button_clicked), buffer_array);

    g_object_unref (icon);

    gtk_main ();

    return 0;
}

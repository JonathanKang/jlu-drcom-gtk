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

int
main (int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *username_label;
    GtkWidget *password_label;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *check_button;
    GtkWidget *login_button;
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;
    GPtrArray *buffer_array;
    GKeyFile *key_file;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "jlu-drcom-gtk");
    gtk_window_set_default_size (GTK_WINDOW (window), 600, 200);
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

    username_label = gtk_label_new ("Username");
    password_label = gtk_label_new ("Password");

    username_buffer = gtk_entry_buffer_new (NULL, -1);
    password_buffer = gtk_entry_buffer_new (NULL, -1);
    username_entry = gtk_entry_new_with_buffer (username_buffer);
    password_entry = gtk_entry_new_with_buffer (password_buffer);
    gtk_entry_set_input_purpose (GTK_ENTRY (password_entry),
                                 GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility (GTK_ENTRY (password_entry), FALSE);

    buffer_array = g_ptr_array_new ();
    g_ptr_array_add (buffer_array, (gpointer) username_buffer);
    g_ptr_array_add (buffer_array, (gpointer) password_buffer);

    check_button = gtk_check_button_new_with_label ("Remember Password");
    login_button = gtk_button_new_with_label ("Login");

    key_file = g_key_file_new ();
    g_key_file_load_from_file (key_file, "jdg.ini",
                               G_KEY_FILE_NONE, NULL);
    if (g_key_file_get_boolean (key_file, "config", "remember_password", NULL))
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), TRUE);
    }
    else
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), FALSE);
    }
    g_key_file_free (key_file);

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 8);

    gtk_grid_attach (GTK_GRID (grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), check_button, 0, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), login_button, 1, 2, 1, 1);

    gtk_container_add (GTK_CONTAINER (window), grid);
    gtk_widget_show_all (window);

    /* Signals */
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect (password_entry, "activate",
                      G_CALLBACK (on_login), buffer_array);
    g_signal_connect (check_button, "toggled",
                      G_CALLBACK (on_check_button_toggled), NULL);
    g_signal_connect (login_button, "clicked",
                      G_CALLBACK (on_login), buffer_array);

    gtk_main ();

    return 0;
}

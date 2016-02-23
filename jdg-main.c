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

int
main (int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *username_label;
    GtkWidget *password_label;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_button;
    GtkEntryBuffer *username_buffer;
    GtkEntryBuffer *password_buffer;

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

    login_button = gtk_button_new_with_label ("Login");

    grid = gtk_grid_new ();
    gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 8);

    gtk_grid_attach (GTK_GRID (grid), username_label, 0, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_label, 0, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), username_entry, 1, 0, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), password_entry, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), login_button, 1, 2, 1, 1);

    gtk_container_add (GTK_CONTAINER (window), grid);
    gtk_widget_show_all (window);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_main ();

    return 0;
}

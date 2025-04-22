#include <gtk/gtk.h>
#include "health_logic.h"

// Global variables for UI components
GtkWidget *window;
GtkWidget *calendar;
GtkWidget *entry_height, *entry_weight, *entry_bp_sys, *entry_bp_dia, *entry_blood_sugar, *entry_temp;
GtkWidget *text_view;
GtkTextBuffer *text_buffer;

// Helper function to show message dialog
void show_message(const char *message, GtkMessageType type) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                               GTK_DIALOG_MODAL,
                                               type,
                                               GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Helper function to create a date selection dialog
GtkWidget* create_date_dialog(const char *title, gboolean need_two_dates) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title,
                                                   GTK_WINDOW(window),
                                                   GTK_DIALOG_MODAL,
                                                   "OK", GTK_RESPONSE_OK,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   NULL);
                                                   
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = need_two_dates ? 
                    gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10) : 
                    gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    gtk_container_add(GTK_CONTAINER(content_area), box);
    
    return dialog;
}

// Helper function to create a text view with scrolling capability
GtkWidget* create_scrollable_text_view() {
    // gtk_text_view_new() - Creates a new empty text view widget
    GtkWidget *text_view = gtk_text_view_new();
    
    // gtk_text_view_get_buffer(text_view) - Gets the buffer associated with the text view
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // gtk_text_view_set_wrap_mode(text_view, mode) - Sets text wrapping mode (none, char, word)
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    
    // gtk_text_view_set_editable(text_view, editable) - Makes text view read-only (FALSE) or editable (TRUE)
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    
    // gtk_text_view_set_cursor_visible(text_view, visible) - Shows/hides the text cursor
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    
    // gtk_scrolled_window_new(h_adjustment, v_adjustment) - Creates a scrollable container
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    
    // gtk_scrolled_window_set_policy(scrolled_window, h_policy, v_policy) - Sets scrollbar policy
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC,  // Show horizontal scrollbar when needed
                                  GTK_POLICY_AUTOMATIC); // Show vertical scrollbar when needed
    
    // gtk_container_add(container, widget) - Adds a widget to a container
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    
    return scrolled_window;
}

// Function to set text in a text view
void set_text_in_text_view(GtkWidget *scrolled_window, const char *text) {
    // gtk_bin_get_child(bin) - Gets the child widget from a bin container
    GtkWidget *text_view = gtk_bin_get_child(GTK_BIN(scrolled_window));
    
    // gtk_text_view_get_buffer(text_view) - Gets the buffer of the text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // gtk_text_buffer_set_text(buffer, text, length) - Sets the text in the buffer (-1 means use all text)
    gtk_text_buffer_set_text(buffer, text, -1);
}

// Function to show output in new window
void show_output_in_new_window(const char *title, const char *result) {
    // gtk_window_new(type) - Creates a new window (TOPLEVEL = normal window with decorations)
    GtkWidget *result_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    
    // gtk_window_set_title(window, title) - Sets the window's title bar text
    gtk_window_set_title(GTK_WINDOW(result_window), title);
    
    // gtk_window_set_default_size(window, width, height) - Sets initial window size
    gtk_window_set_default_size(GTK_WINDOW(result_window), 500, 500);
    
    // gtk_window_set_keep_above(window, setting) - Makes window stay on top of others
    gtk_window_set_keep_above(GTK_WINDOW(result_window), TRUE);
    
    // Create a scrollable text view for the result
    GtkWidget *scrollable_result = create_scrollable_text_view();
    gtk_container_add(GTK_CONTAINER(result_window), scrollable_result);
    set_text_in_text_view(scrollable_result, result);

    // gtk_widget_show_all(widget) - Shows a widget and all its children
    gtk_widget_show_all(result_window);

    // g_signal_connect(instance, signal, callback, data) - Connects a signal to a function
    g_signal_connect(result_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}

// Get date from calendar in YYYY-MM-DD format
char* get_date_from_calendar(GtkCalendar *calendar) {
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);
    char *date = g_strdup_printf("%04u-%02u-%02u", year, month + 1, day);
    return date;
}

// Callback for "Patient's Input" button
void on_patients_input(GtkWidget *widget, gpointer data) {
    // gtk_dialog_new_with_buttons(title, parent, flags, ...) - Creates a dialog with buttons
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Enter Patient Data",           // Dialog title
        GTK_WINDOW(window),             // Parent window
        GTK_DIALOG_MODAL,               // Modal dialog (blocks main window)
        "Save", GTK_RESPONSE_OK,        // Button label and response code
        "Cancel", GTK_RESPONSE_CANCEL,  // Button label and response code
        NULL);                          // NULL-terminated list

    // gtk_dialog_get_content_area(dialog) - Gets the content area of the dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // gtk_grid_new() - Creates a new grid layout container
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // gtk_label_new(text) - Creates a new label with the given text
    GtkWidget *label_date = gtk_label_new("Select Date:");
    GtkWidget *label_height = gtk_label_new("Height (cm):");
    GtkWidget *label_weight = gtk_label_new("Weight (kg):");
    GtkWidget *label_bp_sys = gtk_label_new("Blood Pressure (Systolic):");
    GtkWidget *label_bp_dia = gtk_label_new("Blood Pressure (Diastolic):");
    GtkWidget *label_blood_sugar = gtk_label_new("Blood Sugar (mg/dL):");
    GtkWidget *label_temp = gtk_label_new("Body Temperature (°C):");

    // gtk_calendar_new() - Creates a new calendar widget
    calendar = gtk_calendar_new();
    
    // gtk_entry_new() - Creates a new text entry field
    entry_height = gtk_entry_new();
    entry_weight = gtk_entry_new();
    entry_bp_sys = gtk_entry_new();
    entry_bp_dia = gtk_entry_new();
    entry_blood_sugar = gtk_entry_new();
    entry_temp = gtk_entry_new();

    // gtk_grid_attach(grid, child, left, top, width, height) - Places widget in grid at position
    gtk_grid_attach(GTK_GRID(grid), label_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_height, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_height, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_weight, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_weight, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_bp_sys, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_bp_sys, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_bp_dia, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_bp_dia, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_blood_sugar, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_blood_sugar, 1, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_temp, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_temp, 1, 6, 2, 1);

    gtk_widget_show_all(dialog);

    // gtk_dialog_run(dialog) - Runs the dialog and waits for response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        unsigned int year, month, day;
        
        // gtk_calendar_get_date(calendar, year, month, day) - Gets selected date
        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        char date[11];
        snprintf(date, sizeof(date), "%04u-%02u-%02u", year, month + 1, day);

        // gtk_entry_get_text(entry) - Gets text from an entry field
        const char *height = gtk_entry_get_text(GTK_ENTRY(entry_height));
        const char *weight = gtk_entry_get_text(GTK_ENTRY(entry_weight));
        const char *bp_sys = gtk_entry_get_text(GTK_ENTRY(entry_bp_sys));
        const char *bp_dia = gtk_entry_get_text(GTK_ENTRY(entry_bp_dia));
        const char *blood_sugar = gtk_entry_get_text(GTK_ENTRY(entry_blood_sugar));
        const char *temp = gtk_entry_get_text(GTK_ENTRY(entry_temp));

        if (strlen(height) && strlen(weight) && strlen(bp_sys) && strlen(bp_dia) && 
            strlen(blood_sugar) && strlen(temp)) {
            // Save data to file using the logic module
            write_data_to_file(date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
            
            // gtk_message_dialog_new(parent, flags, type, buttons, format, ...) - Creates message dialog
            GtkWidget *message_dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),             // Parent window
                GTK_DIALOG_MODAL,               // Modal dialog
                GTK_MESSAGE_INFO,               // Info message type
                GTK_BUTTONS_OK,                 // OK button only
                "Data saved successfully for %s", date);  // Message with formatting
                
            gtk_dialog_run(GTK_DIALOG(message_dialog));
            gtk_widget_destroy(message_dialog);
        } else {
            // Error message dialog for empty fields
            GtkWidget *message_dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,              // Error message type
                GTK_BUTTONS_OK,
                "All fields must be filled in before saving data.");
                
            gtk_dialog_run(GTK_DIALOG(message_dialog));
            gtk_widget_destroy(message_dialog);
        }
    }

    // gtk_widget_destroy(widget) - Destroys a widget
    gtk_widget_destroy(dialog);
}

// Callback for "Daily Report" button
void on_daily_report(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Daily Report",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "OK", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget *label_date = gtk_label_new("Select Date:");
    calendar = gtk_calendar_new();

    gtk_grid_attach(GTK_GRID(grid), label_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 2, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        unsigned int year, month, day;
        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        char date[11];
        snprintf(date, sizeof(date), "%04u-%02u-%02u", year, month + 1, day);

        // Create a label to show the progress
        GtkWidget *output_label = gtk_label_new("Fetching data...");
        gtk_grid_attach(GTK_GRID(grid), output_label, 0, 1, 3, 1);
        gtk_widget_show_all(dialog);

        // Get and display comparison with previous day
        char* comparison_data = get_comparison_with_previous_day(date);
        if (comparison_data) {
            show_output_in_new_window("Comparison with Previous Day", comparison_data);
            free(comparison_data);
        }

        // gtk_label_set_text(label, text) - Updates label text
        gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Random Range Report" button
void on_random_range_report(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Random Range Report",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "Generate", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    GtkWidget *label_start_date = gtk_label_new("Select Start Date:");
    GtkWidget *label_end_date = gtk_label_new("Select End Date:");
    GtkWidget *calendar_start = gtk_calendar_new();
    GtkWidget *calendar_end = gtk_calendar_new();

    gtk_grid_attach(GTK_GRID(grid), label_start_date, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_start, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label_end_date, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_end, 1, 1, 2, 1);

    GtkWidget *output_label = gtk_label_new("Generating report...");
    gtk_grid_attach(GTK_GRID(grid), output_label, 0, 2, 3, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        unsigned int start_year, start_month, start_day;
        unsigned int end_year, end_month, end_day;

        gtk_calendar_get_date(GTK_CALENDAR(calendar_start), &start_year, &start_month, &start_day);
        gtk_calendar_get_date(GTK_CALENDAR(calendar_end), &end_year, &end_month, &end_day);

        char start_date[11], end_date[11];
        snprintf(start_date, sizeof(start_date), "%04u-%02u-%02u", start_year, start_month + 1, start_day);
        snprintf(end_date, sizeof(end_date), "%04u-%02u-%02u", end_year, end_month + 1, end_day);

        // Ensure start date is before or equal to end date
        if (strcmp(start_date, end_date) <= 0) {
            // g_strdup_printf(format, ...) - Creates a new formatted string (must be freed)
            gchar *result = g_strdup_printf("Generating report from %s to %s...", start_date, end_date);
            gtk_label_set_text(GTK_LABEL(output_label), result);
            g_free(result);  // Free the allocated string

            // Calculate and display stats using logic module
            char* stats = calculate_stats_for_range(start_date, end_date);
            if (stats) {
                show_output_in_new_window("Statistics Report", stats);
                free(stats);
            }

            gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
        } else {
            gtk_label_set_text(GTK_LABEL(output_label), "Error: Start date must be before or equal to end date.");
        }
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Abnormalities and Advice" button
void on_abnormalities_button_click(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = create_date_dialog("Select Date Range", TRUE);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);

    // Start date selection
    GtkWidget *label_start = gtk_label_new("Start Date:");
    GtkWidget *calendar_start = gtk_calendar_new();
    
    // End date selection
    GtkWidget *label_end = gtk_label_new("End Date:");
    GtkWidget *calendar_end = gtk_calendar_new();
    
    // Layout
    gtk_grid_attach(GTK_GRID(grid), label_start, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_start, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_end, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_end, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char *start_date = get_date_from_calendar(GTK_CALENDAR(calendar_start));
        char *end_date = get_date_from_calendar(GTK_CALENDAR(calendar_end));
        
        int abnormal_weight = 0, abnormal_bp = 0, abnormal_sugar = 0, abnormal_temp = 0;
        
        // Check for abnormalities in the specified date range
        check_for_abnormalities_typewise_in_range(start_date, end_date, 
                                                &abnormal_weight, &abnormal_bp, 
                                                &abnormal_sugar, &abnormal_temp);
        
        // Get advice based on abnormalities
        char* advice = get_advice_based_on_abnormalities(abnormal_weight, abnormal_bp, 
                                                      abnormal_sugar, abnormal_temp);
        if (advice) {
            show_output_in_new_window("Abnormalities and Advice", advice);
            free(advice);
        }
        
        g_free(start_date);
        g_free(end_date);
    }

    gtk_widget_destroy(dialog);
}

// Main function - entry point of the program
int main(int argc, char *argv[]) {
    // gtk_init(argc, argv) - Initializes GTK library
    gtk_init(&argc, &argv);

    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Health Report");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create vertical box layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);  // 5px spacing
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create text view for output display
    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(text_buffer, "", -1);
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    // gtk_button_new_with_label(label) - Creates a button with text
    GtkWidget *btn_patients_input = gtk_button_new_with_label("Patient's Input");
    GtkWidget *btn_daily_report = gtk_button_new_with_label("Daily Report");
    GtkWidget *btn_random_range_report = gtk_button_new_with_label("Random Range Report");
    GtkWidget *btn_abnormalities_advice = gtk_button_new_with_label("Abnormalities and Advice");

    // Connect button signals to callback functions
    g_signal_connect(btn_patients_input, "clicked", G_CALLBACK(on_patients_input), NULL);
    g_signal_connect(btn_daily_report, "clicked", G_CALLBACK(on_daily_report), NULL);
    g_signal_connect(btn_random_range_report, "clicked", G_CALLBACK(on_random_range_report), NULL);
    g_signal_connect(btn_abnormalities_advice, "clicked", G_CALLBACK(on_abnormalities_button_click), NULL);

    // Add buttons to layout
    gtk_box_pack_start(GTK_BOX(vbox), btn_patients_input, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_daily_report, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_random_range_report, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_abnormalities_advice, TRUE, TRUE, 0);

    // Show all widgets in the window
    gtk_widget_show_all(window);
    
    // gtk_main() - Starts the GTK main event loop
    gtk_main();

    return 0;
}

// cd "D:\VS STUDIO\HealthAnalyzer" ; gcc health_logic.c health_ui.c -o health_analyzer $(pkg-config --cflags --libs gtk+-3.0)
// ./health_analyzer
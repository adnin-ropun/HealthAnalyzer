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

// Helper function to get date from calendar in YYYY-MM-DD format
char* get_date_from_calendar(GtkCalendar *calendar) {
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);
    char *date = g_strdup_printf("%04u-%02u-%02u", year, month + 1, day);
    return date;
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

// Helper function to create a label and add it to a grid
void add_label_to_grid(GtkWidget *grid, const char *text, int left, int top) {
    GtkWidget *label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, left, top, 1, 1);
}

// Helper function to create a text view with scrolling capability
GtkWidget* create_scrollable_text_view() {
    // Create the text view
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Configure the text view
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);
    
    // Create a scrolled window and add the text view to it
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    
    return scrolled_window;
}

// Function to set text in a text view
void set_text_in_text_view(GtkWidget *scrolled_window, const char *text) {
    GtkWidget *text_view = gtk_bin_get_child(GTK_BIN(scrolled_window));
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, text, -1);
}

// Function to create and show new windows with results
void show_output_in_new_window(const char *title, const char *result) {
    GtkWidget *result_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(result_window), title);
    gtk_window_set_default_size(GTK_WINDOW(result_window), 640, 350);
    gtk_window_set_keep_above(GTK_WINDOW(result_window), TRUE);
    
    // Create a container with padding around the edges
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(content_box), 15);
    gtk_container_add(GTK_CONTAINER(result_window), content_box);
    
    // Create the scrollable text view
    GtkWidget *scrollable_result = create_scrollable_text_view();
    gtk_box_pack_start(GTK_BOX(content_box), scrollable_result, TRUE, TRUE, 0);
    
    // Set the text content
    set_text_in_text_view(scrollable_result, result);

    gtk_widget_show_all(result_window);
    g_signal_connect(result_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}

// Helper function to validate that all entry fields are filled
gboolean validate_patient_entries(const char *height, const char *weight, 
                                const char *bp_sys, const char *bp_dia,
                                const char *blood_sugar, const char *temp) {
    return (strlen(height) && strlen(weight) && strlen(bp_sys) && strlen(bp_dia) && 
            strlen(blood_sugar) && strlen(temp));
}

// Callback for "Patient's Input" button
void on_patients_input(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Enter Patient Data",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "Save", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // Add labels using helper function
    add_label_to_grid(grid, "Select Date:", 0, 0);
    add_label_to_grid(grid, "Height (cm):", 0, 1);
    add_label_to_grid(grid, "Weight (kg):", 0, 2);
    add_label_to_grid(grid, "Blood Pressure (Systolic):", 0, 3);
    add_label_to_grid(grid, "Blood Pressure (Diastolic):", 0, 4);
    add_label_to_grid(grid, "Blood Sugar (mg/dL):", 0, 5);
    add_label_to_grid(grid, "Body Temperature (°C):", 0, 6);

    calendar = gtk_calendar_new();
    entry_height = gtk_entry_new();
    entry_weight = gtk_entry_new();
    entry_bp_sys = gtk_entry_new();
    entry_bp_dia = gtk_entry_new();
    entry_blood_sugar = gtk_entry_new();
    entry_temp = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_height, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_weight, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_bp_sys, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_bp_dia, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_blood_sugar, 1, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_temp, 1, 6, 2, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Get date using helper function
        char *date = get_date_from_calendar(GTK_CALENDAR(calendar));

        // Get values from entry fields
        const char *height = gtk_entry_get_text(GTK_ENTRY(entry_height));
        const char *weight = gtk_entry_get_text(GTK_ENTRY(entry_weight));
        const char *bp_sys = gtk_entry_get_text(GTK_ENTRY(entry_bp_sys));
        const char *bp_dia = gtk_entry_get_text(GTK_ENTRY(entry_bp_dia));
        const char *blood_sugar = gtk_entry_get_text(GTK_ENTRY(entry_blood_sugar));
        const char *temp = gtk_entry_get_text(GTK_ENTRY(entry_temp));

        // Validate entries using helper function
        if (validate_patient_entries(height, weight, bp_sys, bp_dia, blood_sugar, temp)) {
            // Save data to file
            write_data_to_file(date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
            
            // Show success message using helper function
            char success_message[100];
            snprintf(success_message, sizeof(success_message), "Data saved successfully for %s", date);
            show_message(success_message, GTK_MESSAGE_INFO);
        } else {
            // Show error message using helper function
            show_message("All fields must be filled in before saving data.", GTK_MESSAGE_ERROR);
        }
        
        g_free(date); // Free the allocated date string
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Daily Report" button
void on_daily_report(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = create_date_dialog("Daily Report", FALSE);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    add_label_to_grid(grid, "Select Date:", 0, 0);
    calendar = gtk_calendar_new();
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 2, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Get date using helper function
        char *date = get_date_from_calendar(GTK_CALENDAR(calendar));
        
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

        gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
        g_free(date); // Free the allocated date string
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

    add_label_to_grid(grid, "Select Start Date:", 0, 0);
    add_label_to_grid(grid, "Select End Date:", 0, 1);
    
    GtkWidget *calendar_start = gtk_calendar_new();
    GtkWidget *calendar_end = gtk_calendar_new();

    gtk_grid_attach(GTK_GRID(grid), calendar_start, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_end, 1, 1, 2, 1);

    GtkWidget *output_label = gtk_label_new("Generating report...");
    gtk_grid_attach(GTK_GRID(grid), output_label, 0, 2, 3, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        // Get dates using helper function
        char *start_date = get_date_from_calendar(GTK_CALENDAR(calendar_start));
        char *end_date = get_date_from_calendar(GTK_CALENDAR(calendar_end));

        // Ensure start date is before or equal to end date
        if (strcmp(start_date, end_date) <= 0) {
            char progress_msg[100];
            snprintf(progress_msg, sizeof(progress_msg), "Generating report from %s to %s...", start_date, end_date);
            gtk_label_set_text(GTK_LABEL(output_label), progress_msg);

            // Calculate and display stats
            char* stats = calculate_stats_for_range(start_date, end_date);
            if (stats) {
                show_output_in_new_window("Statistics Report", stats);
                free(stats);
            }

            gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
        } else {
            show_message("Error: Start date must be before or equal to end date.", GTK_MESSAGE_ERROR);
        }
        
        g_free(start_date);
        g_free(end_date);
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
    add_label_to_grid(grid, "Start Date:", 0, 0);
    GtkWidget *calendar_start = gtk_calendar_new();
    
    // End date selection
    add_label_to_grid(grid, "End Date:", 1, 0);
    GtkWidget *calendar_end = gtk_calendar_new();
    
    // Layout
    gtk_grid_attach(GTK_GRID(grid), calendar_start, 0, 1, 1, 1);
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
    gtk_init(&argc, &argv);

    // Create main window with larger size
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Health Report");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400); // Wider, but not as tall
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create vertical box layout with padding
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12); // Increased spacing
    
    // Add padding around the content
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20); // Increased border padding
    
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create larger buttons with custom styling
    GtkWidget *btn_patients_input = gtk_button_new_with_label("Patient's Input");
    GtkWidget *btn_daily_report = gtk_button_new_with_label("Daily Report");
    GtkWidget *btn_random_range_report = gtk_button_new_with_label("Random Range Report");
    GtkWidget *btn_abnormalities_advice = gtk_button_new_with_label("Abnormalities and Advice");
    
    // Set minimum height for buttons
    gtk_widget_set_size_request(btn_patients_input, -1, 50);
    gtk_widget_set_size_request(btn_daily_report, -1, 50);
    gtk_widget_set_size_request(btn_random_range_report, -1, 50);
    gtk_widget_set_size_request(btn_abnormalities_advice, -1, 50);

    // Connect button signals to callback functions
    g_signal_connect(btn_patients_input, "clicked", G_CALLBACK(on_patients_input), NULL);
    g_signal_connect(btn_daily_report, "clicked", G_CALLBACK(on_daily_report), NULL);
    g_signal_connect(btn_random_range_report, "clicked", G_CALLBACK(on_random_range_report), NULL);
    g_signal_connect(btn_abnormalities_advice, "clicked", G_CALLBACK(on_abnormalities_button_click), NULL);

    // Add buttons to layout with more padding between them
    gtk_box_pack_start(GTK_BOX(vbox), btn_patients_input, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_daily_report, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_random_range_report, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_abnormalities_advice, FALSE, TRUE, 10);

    // Show all widgets in the window
    gtk_widget_show_all(window);
    
    // Start the GTK main event loop
    gtk_main();

    return 0;
}

// cd "D:\VS STUDIO\HealthAnalyzer" ; gcc health_logic.c health_ui.c -o health_analyzer $(pkg-config --cflags --libs gtk+-3.0)
// ./health_analyzer
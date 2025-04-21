#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global variables
GtkWidget *window;
GtkWidget *calendar;
GtkWidget *entry_height, *entry_weight, *entry_bp_sys, *entry_bp_dia, *entry_blood_sugar, *entry_temp;
GtkWidget *text_view; // Text view for displaying the output
GtkTextBuffer *text_buffer; // Text buffer for the text view

// Helper function prototypes
void write_data_to_file(const char *date, const char *height, const char *weight, const char *bp_sys, const char *bp_dia, const char *blood_sugar, const char *temp);
void calculate_stats(const char *start_date, const char *end_date);
void retrieve_daily_data(const char *date);
void compare_with_previous_day(const char *current_date);

// Callback function prototypes
void on_patients_input(GtkWidget *widget, gpointer data);
void on_daily_report(GtkWidget *widget, gpointer data);
// void on_random_range_report(GtkWidget *widget, gpointer data);
// void on_random_range_report(GtkWidget *widget, gpointer data);
void on_random_range_report(GtkWidget *widget, gpointer data);



// Function definitions

// Helper function to write data to file
void write_data_to_file(const char *date, const char *height, const char *weight, const char *bp_sys, const char *bp_dia, const char *blood_sugar, const char *temp) {
    // Open the file in append mode
    FILE *file = fopen("input.txt", "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%s,%s,%s,%s\n", date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
        fclose(file);
    }
}

void show_output_in_new_window(const char *title, const char *result) {
    // Create a new GTK window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    // Make the window appear on top by default
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    // Create a GtkTextView and add it to the window
    GtkWidget *text_view = gtk_text_view_new();
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);  // Enable word wrapping

    // Set the result text in the text buffer
    gtk_text_buffer_set_text(text_buffer, result, -1);

    // Add the text view to the window
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), text_view);
    gtk_container_add(GTK_CONTAINER(window), scroll);

    // Show everything
    gtk_widget_show_all(window);

    // Connect the destroy signal to ensure the window closes properly
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
}


// Helper function to calculate averages, standard deviations, and deviations from standard values
void calculate_stats(const char *start_date, const char *end_date) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        gtk_text_buffer_set_text(text_buffer, "Error: Could not open input.txt\n", -1);
        return;
    }

    const double STANDARD_HEIGHT = 170.0;
    const double STANDARD_WEIGHT = 70.0;
    const double STANDARD_SYS = 120.0;
    const double STANDARD_DIA = 80.0;
    const double STANDARD_SUGAR = 100.0;
    const double STANDARD_TEMP = 37.0;

    double height_sum = 0, weight_sum = 0, bp_sys_sum = 0, bp_dia_sum = 0, sugar_sum = 0, temp_sum = 0;
    double height_sq_sum = 0, weight_sq_sum = 0, bp_sys_sq_sum = 0, bp_dia_sq_sum = 0, sugar_sq_sum = 0, temp_sq_sum = 0;
    double height_dev_sum = 0, weight_dev_sum = 0, bp_sys_dev_sum = 0, bp_dia_dev_sum = 0, sugar_dev_sum = 0, temp_dev_sum = 0;
    int count = 0;

    char line[256], date[20], height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        // Check if the date is within the range
        if (strcmp(date, start_date) >= 0 && strcmp(date, end_date) <= 0) {
            double h = atof(height), w = atof(weight), s = atof(sugar), t = atof(temp);
            double bp_s = atof(bp_sys), bp_d = atof(bp_dia);

            height_sum += h;
            weight_sum += w;
            bp_sys_sum += bp_s;
            bp_dia_sum += bp_d;
            sugar_sum += s;
            temp_sum += t;

            height_sq_sum += h * h;
            weight_sq_sum += w * w;
            bp_sys_sq_sum += bp_s * bp_s;
            bp_dia_sq_sum += bp_d * bp_d;
            sugar_sq_sum += s * s;
            temp_sq_sum += t * t;

            // Calculate deviation from standard values
            height_dev_sum += (h - STANDARD_HEIGHT) * (h - STANDARD_HEIGHT);
            weight_dev_sum += (w - STANDARD_WEIGHT) * (w - STANDARD_WEIGHT);
            bp_sys_dev_sum += (bp_s - STANDARD_SYS) * (bp_s - STANDARD_SYS);
            bp_dia_dev_sum += (bp_d - STANDARD_DIA) * (bp_d - STANDARD_DIA);
            sugar_dev_sum += (s - STANDARD_SUGAR) * (s - STANDARD_SUGAR);
            temp_dev_sum += (t - STANDARD_TEMP) * (t - STANDARD_TEMP);

            count++;
        }
    }

    fclose(file);

    if (count == 0) {
        gtk_text_buffer_set_text(text_buffer, "No data found for the specified range.\n", -1);
        return;
    }

    // Calculate averages
    double height_avg = height_sum / count;
    double weight_avg = weight_sum / count;
    double bp_sys_avg = bp_sys_sum / count;
    double bp_dia_avg = bp_dia_sum / count;
    double sugar_avg = sugar_sum / count;
    double temp_avg = temp_sum / count;

    // Calculate standard deviations from the standard value
    double height_sd = sqrt(height_dev_sum / count);
    double weight_sd = sqrt(weight_dev_sum / count);
    double bp_sys_sd = sqrt(bp_sys_dev_sum / count);
    double bp_dia_sd = sqrt(bp_dia_dev_sum / count);
    double sugar_sd = sqrt(sugar_dev_sum / count);
    double temp_sd = sqrt(temp_dev_sum / count);

    // Prepare the result text
    char result[1024];
    snprintf(result, sizeof(result), "Stats from %s to %s:\n"
                                    "Height - Avg: %.2f, SD: %.2f\n"
                                    "Weight - Avg: %.2f, SD: %.2f\n"
                                    "Blood Pressure (Sys) - Avg: %.2f, SD: %.2f\n"
                                    "Blood Pressure (Dia) - Avg: %.2f, SD: %.2f\n"
                                    "Blood Sugar - Avg: %.2f, SD: %.2f\n"
                                    "Body Temperature - Avg: %.2f, SD: %.2f\n",
                                    start_date, end_date, height_avg, height_sd,
                                    weight_avg, weight_sd, bp_sys_avg, bp_sys_sd,
                                    bp_dia_avg, bp_dia_sd, sugar_avg, sugar_sd,
                                    temp_avg, temp_sd);

    show_output_in_new_window("Stats Output", result);
}

// Helper function to retrieve daily data
void retrieve_daily_data(const char *date) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        gtk_text_buffer_set_text(text_buffer, "Error: Could not open input.txt\n", -1);
        return;
    }

    char line[256];
    char file_date[20], height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", file_date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (strcmp(file_date, date) == 0) {
            char result[256];
            snprintf(result, sizeof(result), "Data for %s:\n"
                                            "Height: %s cm\n"
                                            "Weight: %s kg\n"
                                            "Blood Pressure: %s/%s mmHg\n"
                                            "Blood Sugar: %s mg/dL\n"
                                            "Body Temperature: %s °C\n", date, height, weight, bp_sys, bp_dia, sugar, temp);
            show_output_in_new_window("Daily Data Output", result);
            found = 1;
            break;
        }
    }

    fclose(file);

    if (!found) {
        gtk_text_buffer_set_text(text_buffer, "No data found for the selected date.\n", -1);
    }
}


void compare_with_previous_day(const char *current_date) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        printf("Error: Could not open input.txt\n");
        return;
    }

    char line[256];
    char date[20], prev_date[20] = "", height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    char prev_height[10], prev_weight[10], prev_bp_sys[10], prev_bp_dia[10], prev_sugar[10], prev_temp[10];
    int found = 0, prev_found = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (strcmp(date, current_date) == 0) {
            found = 1;
            break;
        }

        strcpy(prev_date, date);
        strcpy(prev_height, height);
        strcpy(prev_weight, weight);
        strcpy(prev_bp_sys, bp_sys);
        strcpy(prev_bp_dia, bp_dia);
        strcpy(prev_sugar, sugar);
        strcpy(prev_temp, temp);
        prev_found = 1;
    }

    fclose(file);

    char output[1024];
    if (!found) {
        snprintf(output, sizeof(output), "No data found for the selected date: %s", current_date);
    } else {
        snprintf(output, sizeof(output), "Data for %s:\nHeight: %s cm\nWeight: %s kg\nBlood Pressure: %s/%s mmHg\nBlood Sugar: %s mg/dL\nBody Temperature: %s °C\n", 
                 current_date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (prev_found) {
            snprintf(output + strlen(output), sizeof(output) - strlen(output),
                     "\nComparison with Previous Day (%s):\nWeight Change: %.2f kg\nBlood Pressure Change (Sys/Dia): %.2f/%.2f mmHg\nBlood Sugar Change: %.2f mg/dL\n", 
                     prev_date, atof(weight) - atof(prev_weight), atof(bp_sys) - atof(prev_bp_sys), atof(bp_dia) - atof(prev_bp_dia), atof(sugar) - atof(prev_sugar));
        } else {
            snprintf(output + strlen(output), sizeof(output) - strlen(output), "\nNo previous day's data available for comparison.\n");
        }
    }

    show_output_in_new_window("Comparison with Previous Day", output);
}
// Callback for "Patient's Input"
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

    GtkWidget *label_date = gtk_label_new("Select Date:");
    GtkWidget *label_height = gtk_label_new("Height (cm):");
    GtkWidget *label_weight = gtk_label_new("Weight (kg):");
    GtkWidget *label_bp_sys = gtk_label_new("Blood Pressure (Systolic):");
    GtkWidget *label_bp_dia = gtk_label_new("Blood Pressure (Diastolic):");
    GtkWidget *label_blood_sugar = gtk_label_new("Blood Sugar (mg/dL):");
    GtkWidget *label_temp = gtk_label_new("Body Temperature (°C):");

    calendar = gtk_calendar_new();
    entry_height = gtk_entry_new();
    entry_weight = gtk_entry_new();
    entry_bp_sys = gtk_entry_new();
    entry_bp_dia = gtk_entry_new();
    entry_blood_sugar = gtk_entry_new();
    entry_temp = gtk_entry_new();

    // Add widgets to the grid
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

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        unsigned int year, month, day;
        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        char date[11];
        snprintf(date, sizeof(date), "%04u-%02u-%02u", year, month + 1, day);

        const char *height = gtk_entry_get_text(GTK_ENTRY(entry_height));
        const char *weight = gtk_entry_get_text(GTK_ENTRY(entry_weight));
        const char *bp_sys = gtk_entry_get_text(GTK_ENTRY(entry_bp_sys));
        const char *bp_dia = gtk_entry_get_text(GTK_ENTRY(entry_bp_dia));
        const char *blood_sugar = gtk_entry_get_text(GTK_ENTRY(entry_blood_sugar));
        const char *temp = gtk_entry_get_text(GTK_ENTRY(entry_temp));

        if (strlen(height) && strlen(weight) && strlen(bp_sys) && strlen(bp_dia) && strlen(blood_sugar) && strlen(temp)) {
            // Save data to file
            write_data_to_file(date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
            
            // Show success message using GTK
            GtkWidget *message_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                                GTK_DIALOG_MODAL,
                                                                GTK_MESSAGE_INFO,
                                                                GTK_BUTTONS_OK,
                                                                "Data saved successfully for %s", date);
            gtk_dialog_run(GTK_DIALOG(message_dialog));
            gtk_widget_destroy(message_dialog);
        } else {
            // Show error message using GTK if fields are not filled
            GtkWidget *message_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                                                                GTK_DIALOG_MODAL,
                                                                GTK_MESSAGE_ERROR,
                                                                GTK_BUTTONS_OK,
                                                                "All fields must be filled in before saving data.");
            gtk_dialog_run(GTK_DIALOG(message_dialog));
            gtk_widget_destroy(message_dialog);
        }
    }

    gtk_widget_destroy(dialog);
}

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

        // Create a label to show the output in GTK window
        GtkWidget *output_label = gtk_label_new("Fetching data...");
        gtk_grid_attach(GTK_GRID(grid), output_label, 0, 1, 3, 1);

        gtk_widget_show_all(dialog);

        // Simulate fetching and comparing data
        gchar *result = g_strdup_printf("Fetching data for %s...\n", date);
        gtk_label_set_text(GTK_LABEL(output_label), result);
        g_free(result);

        // Retrieve and display the data for the selected date
        retrieve_daily_data(date);

        // Compare with previous day's data if available
        compare_with_previous_day(date);

        // Update the label with the result of the operations
        gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
    }

    gtk_widget_destroy(dialog);
}




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

    // Create a label to show the output in GTK window
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
            gchar *result = g_strdup_printf("Generating report from %s to %s...", start_date, end_date);
            gtk_label_set_text(GTK_LABEL(output_label), result);
            g_free(result);

            // Simulate generating the report and calculating stats
            calculate_stats(start_date, end_date);

            // Update the label with the result of the operations
            gtk_label_set_text(GTK_LABEL(output_label), "Report generated successfully!");
        } else {
            gtk_label_set_text(GTK_LABEL(output_label), "Error: Start date must be before or equal to end date.");
        }
    }

    gtk_widget_destroy(dialog);
}


// Function to check for abnormalities within a date range
void check_for_abnormalities_typewise_in_range(const char *start_date, const char *end_date, 
                                                int *abnormal_weight, int *abnormal_bp, 
                                                int *abnormal_sugar, int *abnormal_temp) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        printf("Error: Could not open input.txt\n");
        return;
    }

    char line[256], date[20], height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    int found = 0;

    // Reset the counts
    *abnormal_weight = 0;
    *abnormal_bp = 0;
    *abnormal_sugar = 0;
    *abnormal_temp = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        // Check if date falls within the specified range
        if (strcmp(date, start_date) >= 0 && strcmp(date, end_date) <= 0) {
            // Check for abnormality for each category
            if (atof(weight) < 30.0 || atof(weight) > 100.0) (*abnormal_weight)++;
            if (atoi(bp_sys) > 140 || atoi(bp_dia) > 90) (*abnormal_bp)++;
            if (atoi(sugar) > 200) (*abnormal_sugar)++;
            if (atof(temp) > 38.0 || atof(temp) < 35.0) (*abnormal_temp)++;
            found = 1;
        }
    }

    fclose(file);

    if (!found) {
        printf("No data found in the given range.\n");
    }
}

// Function to provide advice based on the abnormalities
void provide_advice_based_on_abnormalities(int abnormal_weight, int abnormal_bp, 
                                           int abnormal_sugar, int abnormal_temp, 
                                           char *advice) {
    // Provide advice based on abnormalities
    if (abnormal_weight > 0) {
        strcat(advice, 
               " - Consider tracking your diet and exercise to maintain a\n"
               "   healthy weight.\n");
    }
    if (abnormal_bp > 0) {
        strcat(advice, 
               " - High blood pressure may require lifestyle changes,\n"
               "   medication, or further monitoring. Please consult a\n"
               "   healthcare professional.\n");
    }
    if (abnormal_sugar > 0) {
        strcat(advice, 
               " - Abnormal blood sugar levels may indicate the need for\n"
               "   diet changes or medication. Please consult a healthcare\n"
               "   provider for proper testing.\n");
    }
    if (abnormal_temp > 0) {
        strcat(advice, 
               " - Abnormal body temperature could be a sign of infection\n"
               "   or illness. Consult a healthcare provider for diagnosis\n"
               "   and treatment.\n");
    }
}

// Callback function for "Abnormalities and Advice" button
void on_abnormalities_button_click(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *content_area, *calendar_start, *calendar_end, *hbox;
    GtkCalendar *calendar_start_widget, *calendar_end_widget;
    gchar *start_date, *end_date;
    int abnormal_weight = 0, abnormal_bp = 0, abnormal_sugar = 0, abnormal_temp = 0;
    char advice[2048] = "Abnormalities Count and Advice:\n\n";

    // Create a dialog for date selection
    dialog = gtk_dialog_new_with_buttons("Select Date Range", GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL, "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_OK", GTK_RESPONSE_ACCEPT, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Create calendars for start and end date
    calendar_start_widget = GTK_CALENDAR(gtk_calendar_new());
    calendar_end_widget = GTK_CALENDAR(gtk_calendar_new());

    // Create a horizontal box to hold the two calendars
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(calendar_start_widget), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(calendar_end_widget), TRUE, TRUE, 0);

    // Add the calendar box to the dialog's content area
    gtk_container_add(GTK_CONTAINER(content_area), hbox);

    gtk_widget_show_all(dialog);

    // Run the dialog and get the response
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Get selected dates from both calendars
        guint year_start, month_start, day_start;
        guint year_end, month_end, day_end;

        // Get the date from the start calendar
        gtk_calendar_get_date(calendar_start_widget, &year_start, &month_start, &day_start);
        
        // Get the date from the end calendar
        gtk_calendar_get_date(calendar_end_widget, &year_end, &month_end, &day_end);

        // Format the dates as strings (e.g., "2025-01-15")
        start_date = g_strdup_printf("%04d-%02d-%02d", year_start, month_start + 1, day_start);
        end_date = g_strdup_printf("%04d-%02d-%02d", year_end, month_end + 1, day_end);

        // Step 1: Check for abnormalities in the specified date range
        check_for_abnormalities_typewise_in_range(start_date, end_date, 
                                                  &abnormal_weight, &abnormal_bp, 
                                                  &abnormal_sugar, &abnormal_temp);

        // Step 2: Add the type-wise abnormalities count to the advice string
        strcat(advice, "Weight Abnormalities: ");
        char weight_count[50];
        sprintf(weight_count, "%d\n", abnormal_weight);
        strcat(advice, weight_count);

        strcat(advice, "Blood Pressure Abnormalities: ");
        char bp_count[50];
        sprintf(bp_count, "%d\n", abnormal_bp);
        strcat(advice, bp_count);

        strcat(advice, "Blood Sugar Abnormalities: ");
        char sugar_count[50];
        sprintf(sugar_count, "%d\n", abnormal_sugar);
        strcat(advice, sugar_count);

        strcat(advice, "Temperature Abnormalities: ");
        char temp_count[50];
        sprintf(temp_count, "%d\n\n", abnormal_temp);
        strcat(advice, temp_count);

        // Step 3: Provide advice based on the abnormalities
        provide_advice_based_on_abnormalities(abnormal_weight, abnormal_bp, abnormal_sugar, abnormal_temp, advice);

        // Step 4: Create a new window to show the result
        GtkWidget *result_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(result_window), "Abnormalities and Advice");
        gtk_window_set_default_size(GTK_WINDOW(result_window), 400, 300);

        // Text view setup for displaying the result
        GtkWidget *text_view_result = gtk_text_view_new();
        GtkTextBuffer *text_buffer_result = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_result));
        gtk_text_buffer_set_text(text_buffer_result, advice, -1);  // Set the advice text

        // Pack the text view into the result window
        gtk_container_add(GTK_CONTAINER(result_window), text_view_result);

        // Show the result window
        gtk_widget_show_all(result_window);

        // Free the date strings
        g_free(start_date);
        g_free(end_date);
    }

    // Destroy the dialog after use
    gtk_widget_destroy(dialog);
}



// Main function
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Window setup
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Health Report");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Layout setup
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Text view setup for displaying the output
    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(text_buffer, "", -1);  // Clear text initially
    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    // Buttons
    GtkWidget *btn_patients_input = gtk_button_new_with_label("Patient's Input");
    GtkWidget *btn_daily_report = gtk_button_new_with_label("Daily Report");
    GtkWidget *btn_random_range_report = gtk_button_new_with_label("Random Range Report");
    GtkWidget *btn_abnormalities_advice = gtk_button_new_with_label("Abnormalities and Advice");  // New button

    // Button signals
    g_signal_connect(btn_patients_input, "clicked", G_CALLBACK(on_patients_input), NULL);
    g_signal_connect(btn_daily_report, "clicked", G_CALLBACK(on_daily_report), NULL);
    g_signal_connect(btn_random_range_report, "clicked", G_CALLBACK(on_random_range_report), NULL);
    g_signal_connect(btn_abnormalities_advice, "clicked", G_CALLBACK(on_abnormalities_button_click), NULL);  // Connect new button

    // Add buttons to layout
    gtk_box_pack_start(GTK_BOX(vbox), btn_patients_input, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_daily_report, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_random_range_report, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), btn_abnormalities_advice, TRUE, TRUE, 0);  // Add new button to layout

    // Show all widgets
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

// gcc -g f1.c -o f1 $(pkg-config --cflags --libs gtk+-3.0)

#include <gtk/gtk.h>
#include <cairo.h>
#include "health_logic.h"

// Global variables for UI components
GtkWidget *window;
GtkWidget *calendar;
GtkWidget *entry_height, *entry_weight, *entry_bp_sys, *entry_bp_dia, *entry_blood_sugar, *entry_temp;

// Simple CSS styling
void apply_clean_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css_data = 
        ".recommendations-text {"
        "  font-family: Arial;"
        "  font-size: 11pt;"
        "  background-color: #ffffff;"
        "}";
    
    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                             GTK_STYLE_PROVIDER(provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

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

// Helper function to add a label to a grid
void add_label_to_grid(GtkWidget *grid, const char *text, int left, int top) {
    GtkWidget *label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, left, top, 1, 1);
}

// Function to create table view for comparison data
GtkWidget* create_comparison_table(const char *current_date) {
    ComparisonTableData *data;
    int row_count = get_comparison_table_data(current_date, &data);
    
    if (row_count == 0) {
        GtkWidget *label = gtk_label_new("No data found for the selected date.");
        return label;
    }

    GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Health Metric", renderer, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Current Value", renderer, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Previous Value", renderer, "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Change", renderer, "text", 3, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 4, NULL));

    for (int i = 0; i < row_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, data[i].metric,
                          1, data[i].current_value,
                          2, data[i].previous_value,
                          3, data[i].change,
                          4, data[i].status,
                          -1);
    }

    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree_view));
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 0)), 200);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 1)), 120);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 2)), 120);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 3)), 100);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 4)), 120);

    free(data);
    g_object_unref(store);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);

    return scrolled_window;
}

// Function to create table view for statistics data
GtkWidget* create_stats_table(const char *start_date, const char *end_date) {
    StatsTableData *data;
    int row_count = get_stats_table_data(start_date, end_date, &data);
    
    if (row_count == 0) {
        GtkWidget *label = gtk_label_new("No data found for the specified range.");
        return label;
    }

    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Health Metric", renderer, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Average", renderer, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Std Deviation", renderer, "text", 2, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 3, NULL));

    for (int i = 0; i < row_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, data[i].metric,
                          1, data[i].average,
                          2, data[i].std_deviation,
                          3, data[i].status,
                          -1);
    }

    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree_view));
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 0)), 200);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 1)), 120);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 2)), 120);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 3)), 120);

    free(data);
    g_object_unref(store);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);

    return scrolled_window;
}

// Function to create table view for health check data
GtkWidget* create_health_check_table(const char *start_date, const char *end_date) {
    AbnormalityTableData *data;
    int row_count = get_abnormality_table_data(start_date, end_date, &data);
    
    if (row_count == 0) {
        GtkWidget *label = gtk_label_new("No data found for the specified range.");
        return label;
    }

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    
    GtkListStore *store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Health Category", renderer, "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Issues Found", renderer, "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),
                               gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 2, NULL));

    for (int i = 0; i < row_count; i++) {
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, data[i].category,
                          1, data[i].count,
                          2, data[i].advice,
                          -1);
    }

    GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree_view));
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 0)), 200);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 1)), 120);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(g_list_nth_data(columns, 2)), 200);

    free(data);
    g_object_unref(store);

    GtkWidget *table_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(table_scrolled),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(table_scrolled, -1, 200);
    gtk_container_add(GTK_CONTAINER(table_scrolled), tree_view);

    gtk_box_pack_start(GTK_BOX(main_box), table_scrolled, FALSE, FALSE, 0);

    char* recommendations = get_health_recommendations(start_date, end_date);
    if (recommendations) {
        GtkWidget *recommendations_view = gtk_text_view_new();
        GtkTextBuffer *recommendations_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(recommendations_view));
        
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(recommendations_view), GTK_WRAP_WORD);
        gtk_text_view_set_editable(GTK_TEXT_VIEW(recommendations_view), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(recommendations_view), FALSE);
        
        GtkStyleContext *context = gtk_widget_get_style_context(recommendations_view);
        gtk_style_context_add_class(context, "recommendations-text");
        
        gtk_text_view_set_left_margin(GTK_TEXT_VIEW(recommendations_view), 10);
        gtk_text_view_set_right_margin(GTK_TEXT_VIEW(recommendations_view), 10);
        gtk_text_view_set_top_margin(GTK_TEXT_VIEW(recommendations_view), 10);
        gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(recommendations_view), 10);
        
        gtk_text_buffer_set_text(recommendations_buffer, recommendations, -1);
        
        GtkWidget *recommendations_scrolled = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(recommendations_scrolled),
                                      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(recommendations_scrolled), GTK_SHADOW_NONE);
        
        gtk_container_add(GTK_CONTAINER(recommendations_scrolled), recommendations_view);
        
        gtk_box_pack_start(GTK_BOX(main_box), recommendations_scrolled, TRUE, TRUE, 0);
        
        free(recommendations);
    }

    return main_box;
}

// Function to create and show new windows with table results
void show_table_in_new_window(const char *title, GtkWidget *table_widget) {
    GtkWidget *result_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(result_window), title);
    
    if (strstr(title, "Health Check") != NULL) {
        gtk_window_set_default_size(GTK_WINDOW(result_window), 900, 700);
    } else {
        gtk_window_set_default_size(GTK_WINDOW(result_window), 800, 400);
    }
    
    gtk_window_set_keep_above(GTK_WINDOW(result_window), TRUE);
    
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(content_box), 15);
    gtk_container_add(GTK_CONTAINER(result_window), content_box);
    
    GtkWidget *title_label = gtk_label_new(title);
    gtk_widget_set_halign(title_label, GTK_ALIGN_CENTER);
    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(1.2));
    gtk_label_set_attributes(GTK_LABEL(title_label), attrs);
    pango_attr_list_unref(attrs);
    
    gtk_box_pack_start(GTK_BOX(content_box), title_label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(content_box), table_widget, TRUE, TRUE, 0);

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

// Function to draw the graph
gboolean on_draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    HealthData *data;
    int data_count = get_all_health_data(&data);
    
    if (data_count == 0) {
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 20);
        cairo_move_to(cr, 200, 200);
        cairo_show_text(cr, "No data available");
        return FALSE;
    }

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;

    int margin_left = 80, margin_right = 80, margin_top = 50, margin_bottom = 80;
    int graph_width = width - margin_left - margin_right;
    int graph_height = height - margin_top - margin_bottom;

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, margin_left, margin_top, graph_width, graph_height);
    cairo_stroke(cr);

    double min_bp = 1000, max_bp = 0, min_sugar = 1000, max_sugar = 0;
    for (int i = 0; i < data_count; i++) {
        if (data[i].bp_systolic < min_bp) min_bp = data[i].bp_systolic;
        if (data[i].bp_systolic > max_bp) max_bp = data[i].bp_systolic;
        if (data[i].bp_diastolic < min_bp) min_bp = data[i].bp_diastolic;
        if (data[i].bp_diastolic > max_bp) max_bp = data[i].bp_diastolic;
        if (data[i].blood_sugar < min_sugar) min_sugar = data[i].blood_sugar;
        if (data[i].blood_sugar > max_sugar) max_sugar = data[i].blood_sugar;
    }

    double bp_range = max_bp - min_bp;
    double sugar_range = max_sugar - min_sugar;
    if (bp_range > 0) {
        min_bp -= bp_range * 0.1;
        max_bp += bp_range * 0.1;
    }
    if (sugar_range > 0) {
        min_sugar -= sugar_range * 0.1;
        max_sugar += sugar_range * 0.1;
    }

    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_move_to(cr, margin_left + 20, margin_top - 30);
    cairo_show_text(cr, "Systolic BP");
    
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_move_to(cr, margin_left + 120, margin_top - 30);
    cairo_show_text(cr, "Diastolic BP");
    
    cairo_set_source_rgb(cr, 0, 0.7, 0);
    cairo_move_to(cr, margin_left + 220, margin_top - 30);
    cairo_show_text(cr, "Blood Sugar");

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 10);
    for (int i = 0; i <= 5; i++) {
        double bp_value = min_bp + (max_bp - min_bp) * i / 5.0;
        int y = margin_top + graph_height - (graph_height * i / 5);
        cairo_move_to(cr, margin_left - 50, y + 5);
        char label[20];
        snprintf(label, sizeof(label), "%.0f", bp_value);
        cairo_show_text(cr, label);
        
        cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 0.5);
        cairo_move_to(cr, margin_left, y);
        cairo_line_to(cr, margin_left + graph_width, y);
        cairo_stroke(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
    }

    for (int i = 0; i <= 5; i++) {
        double sugar_value = min_sugar + (max_sugar - min_sugar) * i / 5.0;
        int y = margin_top + graph_height - (graph_height * i / 5);
        cairo_move_to(cr, margin_left + graph_width + 10, y + 5);
        char label[20];
        snprintf(label, sizeof(label), "%.0f", sugar_value);
        cairo_show_text(cr, label);
    }

    if (data_count > 1) {
        for (int i = 0; i < data_count; i++) {
            int x = margin_left + (graph_width * i / (data_count - 1));
            cairo_move_to(cr, x - 20, margin_top + graph_height + 20);
            cairo_show_text(cr, data[i].date + 5);
        }
    }

    if (data_count > 1) {
        cairo_set_line_width(cr, 3);

        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_move_to(cr, margin_left, 
                      margin_top + graph_height - 
                      ((data[0].bp_systolic - min_bp) / (max_bp - min_bp)) * graph_height);
        
        for (int i = 1; i < data_count; i++) {
            int x = margin_left + (graph_width * i / (data_count - 1));
            int y = margin_top + graph_height - 
                    ((data[i].bp_systolic - min_bp) / (max_bp - min_bp)) * graph_height;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 0, 0, 1);
        cairo_move_to(cr, margin_left, 
                      margin_top + graph_height - 
                      ((data[0].bp_diastolic - min_bp) / (max_bp - min_bp)) * graph_height);
        
        for (int i = 1; i < data_count; i++) {
            int x = margin_left + (graph_width * i / (data_count - 1));
            int y = margin_top + graph_height - 
                    ((data[i].bp_diastolic - min_bp) / (max_bp - min_bp)) * graph_height;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 0, 0.7, 0);
        cairo_move_to(cr, margin_left, 
                      margin_top + graph_height - 
                      ((data[0].blood_sugar - min_sugar) / (max_sugar - min_sugar)) * graph_height);
        
        for (int i = 1; i < data_count; i++) {
            int x = margin_left + (graph_width * i / (data_count - 1));
            int y = margin_top + graph_height - 
                    ((data[i].blood_sugar - min_sugar) / (max_sugar - min_sugar)) * graph_height;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);
    }

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_font_size(cr, 14);
    
    cairo_save(cr);
    cairo_translate(cr, 20, margin_top + graph_height/2);
    cairo_rotate(cr, -M_PI/2);
    cairo_move_to(cr, 0, 0);
    cairo_show_text(cr, "Blood Pressure (mmHg)");
    cairo_restore(cr);
    
    cairo_save(cr);
    cairo_translate(cr, width - 20, margin_top + graph_height/2);
    cairo_rotate(cr, M_PI/2);
    cairo_move_to(cr, 0, 0);
    cairo_show_text(cr, "Blood Sugar (mg/dL)");
    cairo_restore(cr);
    
    cairo_move_to(cr, margin_left + graph_width/2 - 30, height - 20);
    cairo_show_text(cr, "Date");

    cairo_set_font_size(cr, 16);
    cairo_move_to(cr, margin_left + graph_width/2 - 100, 30);
    cairo_show_text(cr, "Health Parameter Trends");

    free(data);
    return FALSE;
}

// Callback for "Graphical View" button
void on_graphical_view(GtkWidget *widget, gpointer data) {
    GtkWidget *graph_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(graph_window), "Graphical View - Health Trends");
    gtk_window_set_default_size(GTK_WINDOW(graph_window), 1200, 800);
    
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(graph_window), drawing_area);
    
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw_graph), NULL);
    g_signal_connect(graph_window, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    
    gtk_widget_show_all(graph_window);
}

// Callback for "Input Health Data" button
void on_input_health_data(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Input Health Data",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "Save Data", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

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
        char *date = get_date_from_calendar(GTK_CALENDAR(calendar));

        const char *height = gtk_entry_get_text(GTK_ENTRY(entry_height));
        const char *weight = gtk_entry_get_text(GTK_ENTRY(entry_weight));
        const char *bp_sys = gtk_entry_get_text(GTK_ENTRY(entry_bp_sys));
        const char *bp_dia = gtk_entry_get_text(GTK_ENTRY(entry_bp_dia));
        const char *blood_sugar = gtk_entry_get_text(GTK_ENTRY(entry_blood_sugar));
        const char *temp = gtk_entry_get_text(GTK_ENTRY(entry_temp));

        if (validate_patient_entries(height, weight, bp_sys, bp_dia, blood_sugar, temp)) {
            write_data_to_file(date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
            
            char success_message[100];
            snprintf(success_message, sizeof(success_message), "Health data saved successfully for %s", date);
            show_message(success_message, GTK_MESSAGE_INFO);
        } else {
            show_message("Please fill in all fields before saving.", GTK_MESSAGE_ERROR);
        }
        
        g_free(date);
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Daily Report" button
void on_daily_report(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Daily Report",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "View Report", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    add_label_to_grid(grid, "Select Date:", 0, 0);
    calendar = gtk_calendar_new();
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 0, 2, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char *date = get_date_from_calendar(GTK_CALENDAR(calendar));
        
        GtkWidget *table = create_comparison_table(date);
        show_table_in_new_window("Daily Report", table);
        
        g_free(date);
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Report Summary" button
void on_report_summary(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Report Summary",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "Generate Summary", GTK_RESPONSE_OK,
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

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char *start_date = get_date_from_calendar(GTK_CALENDAR(calendar_start));
        char *end_date = get_date_from_calendar(GTK_CALENDAR(calendar_end));

        if (strcmp(start_date, end_date) <= 0) {
            GtkWidget *table = create_stats_table(start_date, end_date);
            show_table_in_new_window("Report Summary", table);
        } else {
            show_message("Error: Start date must be before or equal to end date.", GTK_MESSAGE_ERROR);
        }
        
        g_free(start_date);
        g_free(end_date);
    }

    gtk_widget_destroy(dialog);
}

// Callback for "Health Check & Advice" button
void on_health_check_advice(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Health Check & Advice",
                                                    GTK_WINDOW(window),
                                                    GTK_DIALOG_MODAL,
                                                    "Generate Report", GTK_RESPONSE_OK,
                                                    "Cancel", GTK_RESPONSE_CANCEL,
                                                    NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content), grid);

    add_label_to_grid(grid, "Start Date:", 0, 0);
    GtkWidget *calendar_start = gtk_calendar_new();
    
    add_label_to_grid(grid, "End Date:", 1, 0);
    GtkWidget *calendar_end = gtk_calendar_new();
    
    gtk_grid_attach(GTK_GRID(grid), calendar_start, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar_end, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char *start_date = get_date_from_calendar(GTK_CALENDAR(calendar_start));
        char *end_date = get_date_from_calendar(GTK_CALENDAR(calendar_end));
        
        GtkWidget *table = create_health_check_table(start_date, end_date);
        show_table_in_new_window("Health Check & Advice", table);
        
        g_free(start_date);
        g_free(end_date);
    }

    gtk_widget_destroy(dialog);
}

// Main function - entry point of the program
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    apply_clean_css();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Health Monitoring System");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *btn_input_health_data = gtk_button_new_with_label("Input Health Data");
    GtkWidget *btn_daily_report = gtk_button_new_with_label("Daily Report");
    GtkWidget *btn_report_summary = gtk_button_new_with_label("Report Summary");
    GtkWidget *btn_health_check_advice = gtk_button_new_with_label("Health Check & Advice");
    GtkWidget *btn_graphical_view = gtk_button_new_with_label("Graphical View");
    
    gtk_widget_set_size_request(btn_input_health_data, -1, 50);
    gtk_widget_set_size_request(btn_daily_report, -1, 50);
    gtk_widget_set_size_request(btn_report_summary, -1, 50);
    gtk_widget_set_size_request(btn_health_check_advice, -1, 50);
    gtk_widget_set_size_request(btn_graphical_view, -1, 50);

    g_signal_connect(btn_input_health_data, "clicked", G_CALLBACK(on_input_health_data), NULL);
    g_signal_connect(btn_daily_report, "clicked", G_CALLBACK(on_daily_report), NULL);
    g_signal_connect(btn_report_summary, "clicked", G_CALLBACK(on_report_summary), NULL);
    g_signal_connect(btn_health_check_advice, "clicked", G_CALLBACK(on_health_check_advice), NULL);
    g_signal_connect(btn_graphical_view, "clicked", G_CALLBACK(on_graphical_view), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), btn_input_health_data, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_daily_report, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_report_summary, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_health_check_advice, FALSE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), btn_graphical_view, FALSE, TRUE, 10);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

// gcc health_logic.c health_ui.c -o health_analyzer $(pkg-config --cflags --libs gtk+-3.0)
// ./health_analyzer

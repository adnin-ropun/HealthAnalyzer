#include "health_logic.h"

void write_data_to_file(const char *date, const char *height, const char *weight, 
                       const char *bp_sys, const char *bp_dia, const char *blood_sugar, 
                       const char *temp) {
    // Open the file in append mode
    FILE *file = fopen("input.txt", "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%s,%s,%s,%s\n", date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
        fclose(file);
    }
}


char* get_comparison_with_previous_day(const char *current_date) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        return strdup("Error: Could not open input.txt\n");
    }

    char line[256];
    char date[20], prev_date[20] = "", height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    char prev_height[10] = "", prev_weight[10] = "", prev_bp_sys[10] = "", prev_bp_dia[10] = "", prev_sugar[10] = "", prev_temp[10] = "";
    int found = 0, prev_found = 0;
    char* output = malloc(1024);
    
    if (!output) return NULL;
    output[0] = '\0';

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

    if (!found) {
        snprintf(output, 1024, "No data found for the selected date: %s", current_date);
    } else {
        snprintf(output, 1024, "Data for %s:\nHeight: %s cm\nWeight: %s kg\nBlood Pressure: %s/%s mmHg\nBlood Sugar: %s mg/dL\nBody Temperature: %s °C\n", 
                 current_date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (prev_found) {
            snprintf(output + strlen(output), 1024 - strlen(output),
                     "\nComparison with Previous Day (%s):\nWeight Change: %.2f kg\nBlood Pressure Change (Sys/Dia): %.2f/%.2f mmHg\nBlood Sugar Change: %.2f mg/dL\n", 
                     prev_date, atof(weight) - atof(prev_weight), atof(bp_sys) - atof(prev_bp_sys), 
                     atof(bp_dia) - atof(prev_bp_dia), atof(sugar) - atof(prev_sugar));
        } else {
            snprintf(output + strlen(output), 1024 - strlen(output), "\nNo previous day's data available for comparison.\n");
        }
    }

    return output;
}

char* calculate_stats_for_range(const char *start_date, const char *end_date) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        return strdup("Error: Could not open input.txt\n");
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

    char* result = malloc(1024);
    if (!result) return NULL;

    if (count == 0) {
        snprintf(result, 1024, "No data found for the specified range.\n");
        return result;
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
    snprintf(result, 1024, "Stats from %s to %s:\n"
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

    return result;
}

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

char* get_advice_based_on_abnormalities(int abnormal_weight, int abnormal_bp, 
                                      int abnormal_sugar, int abnormal_temp) {
    char* advice = malloc(2048);
    if (!advice) return NULL;
    
    strcpy(advice, "Abnormalities Count and Advice:\n\n");
    
    // Add the type-wise abnormalities count
    char count_buffer[50];
    
    sprintf(count_buffer, "Weight Abnormalities: %d\n", abnormal_weight);
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Blood Pressure Abnormalities: %d\n", abnormal_bp);
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Blood Sugar Abnormalities: %d\n", abnormal_sugar);
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Temperature Abnormalities: %d\n\n", abnormal_temp);
    strcat(advice, count_buffer);
    
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
    
    return advice;
}


// cd "D:\VS STUDIO\HealthAnalyzer" ; gcc health_logic.c health_ui.c -o health_analyzer $(pkg-config --cflags --libs gtk+-3.0)
// ./health_analyzer
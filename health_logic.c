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

// Helper function to get status indicators for health metrics
const char* get_status_indicator(double value, double low_threshold, double normal_threshold, double high_threshold) {
    if (value < low_threshold)
        return "LOW ↓";
    else if (value > high_threshold)
        return "HIGH ↑";
    else if (value >= low_threshold && value <= normal_threshold)
        return "NORMAL ✓";
    else
        return "CAUTION ⚠";
}

// Helper function to get BP status indicator
const char* get_bp_status(int systolic, int diastolic) {
    if (systolic < 90 || diastolic < 60)
        return "LOW ↓";
    else if (systolic >= 140 || diastolic >= 90)
        return "HIGH ↑";
    else if (systolic >= 120 || diastolic >= 80)
        return "ELEVATED ⚠";
    else
        return "NORMAL ✓";
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
        // Get status indicators
        const char *weight_status = get_status_indicator(atof(weight), 18.5, 24.9, 30.0);
        const char *bp_status = get_bp_status(atoi(bp_sys), atoi(bp_dia));
        const char *sugar_status = get_status_indicator(atof(sugar), 70.0, 99.0, 126.0);
        const char *temp_status = get_status_indicator(atof(temp), 36.1, 37.0, 38.0);
        
        snprintf(output, 1024, 
                "HEALTH REPORT DATA\n"
                "----------------------------\n"
                "Date: %s\n\n"
                "Height: %s cm\n"
                "Weight: %s kg     %s\n"
                "Blood Pressure: %s/%s mmHg   %s\n"
                "Blood Sugar: %s mg/dL   %s\n"
                "Body Temperature: %s °C   %s\n", 
                current_date, height, weight, weight_status, bp_sys, bp_dia, bp_status, sugar, sugar_status, temp, temp_status);

        if (prev_found) {
            // Calculate changes
            double weight_change = atof(weight) - atof(prev_weight);
            double bp_sys_change = atof(bp_sys) - atof(prev_bp_sys);
            double bp_dia_change = atof(bp_dia) - atof(prev_bp_dia);
            double sugar_change = atof(sugar) - atof(prev_sugar);
            double temp_change = atof(temp) - atof(prev_temp);
            
            // Create change indicators
            const char *weight_change_symbol = (weight_change > 0) ? "↑" : (weight_change < 0) ? "↓" : "→";
            const char *bp_sys_change_symbol = (bp_sys_change > 0) ? "↑" : (bp_sys_change < 0) ? "↓" : "→";
            const char *bp_dia_change_symbol = (bp_dia_change > 0) ? "↑" : (bp_dia_change < 0) ? "↓" : "→";
            const char *sugar_change_symbol = (sugar_change > 0) ? "↑" : (sugar_change < 0) ? "↓" : "→";
            const char *temp_change_symbol = (temp_change > 0) ? "↑" : (temp_change < 0) ? "↓" : "→";
            
            char comparison[1024];
            snprintf(comparison, sizeof(comparison),
                    "\n\nCOMPARISON WITH PREVIOUS DAY\n"
                    "-----------------------------------------\n"
                    "Previous Date: %s\n\n"
                    "Weight Change: %s %.2f kg\n" 
                    "Blood Pressure Change (Systolic): %s %.2f mmHg\n"
                    "Blood Pressure Change (Diastolic): %s %.2f mmHg\n"
                    "Blood Sugar Change: %s %.2f mg/dL\n"
                    "Temperature Change: %s %.2f °C\n",
                    prev_date,
                    weight_change_symbol, fabs(weight_change),
                    bp_sys_change_symbol, fabs(bp_sys_change),
                    bp_dia_change_symbol, fabs(bp_dia_change),
                    sugar_change_symbol, fabs(sugar_change),
                    temp_change_symbol, fabs(temp_change));
            
            // Append comparison data to output
            strcat(output, comparison);
        } else {
            strcat(output, "\nNo previous day's data available for comparison.\n");
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

    char* result = malloc(1536);  // Increased size for formatted output
    if (!result) return NULL;

    if (count == 0) {
        snprintf(result, 1536, "No data found for the specified range.\n");
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

    // Get status indicators for average values
    const char *weight_status = get_status_indicator(weight_avg, 18.5, 24.9, 30.0);
    const char *bp_status = get_bp_status((int)bp_sys_avg, (int)bp_dia_avg);
    const char *sugar_status = get_status_indicator(sugar_avg, 70.0, 99.0, 126.0);
    const char *temp_status = get_status_indicator(temp_avg, 36.1, 37.0, 38.0);

    // Prepare the result text with precise column alignment
    snprintf(result, 1536,
        "HEALTH STATISTICS REPORT\n"
        "--------------------------------\n"
        "Period: %s to %s\n"
        "Total Readings: %d\n\n"
        "METRIC                          AVERAGE              SD          STATUS\n"
        "--------------------------------------------------------------------\n"
        "Height (cm)             %17.2f    %13.2f\n"
        "Weight (kg)             %16.2f    %15.2f   %15s\n"
        "Blood Pressure (mmHg)   %-0.0f/%-0.0f   %10.2f   %15s\n"
        "Blood Sugar (mg/dL)     %9.2f    %13.2f   %14s\n"
        "Temperature (°C)        %12.2f    %14.2f   %15s\n",
        start_date, end_date, count,
        height_avg, height_sd,
        weight_avg, weight_sd, weight_status,
        bp_sys_avg, bp_dia_avg, bp_sys_sd, bp_status,
        sugar_avg, sugar_sd, sugar_status,
        temp_avg, temp_sd, temp_status);
    
    

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
    
    strcpy(advice, "HEALTH ABNORMALITIES AND ADVICE\n"
                  "---------------------------------------------\n\n");
    
    // Add the type-wise abnormalities count with icons
    char count_buffer[256];
    
    sprintf(count_buffer, "Weight Abnormalities: %d %s\n", 
            abnormal_weight, (abnormal_weight > 0) ? "⚠️" : "✓");
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Blood Pressure Abnormalities: %d %s\n", 
            abnormal_bp, (abnormal_bp > 0) ? "⚠️" : "✓");
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Blood Sugar Abnormalities: %d %s\n", 
            abnormal_sugar, (abnormal_sugar > 0) ? "⚠️" : "✓");
    strcat(advice, count_buffer);
    
    sprintf(count_buffer, "Temperature Abnormalities: %d %s\n\n", 
            abnormal_temp, (abnormal_temp > 0) ? "⚠️" : "✓");
    strcat(advice, count_buffer);
    
    strcat(advice, "RECOMMENDATIONS\n"
                  "--------------------------\n\n");
    
    // Provide advice based on abnormalities
    if (abnormal_weight > 0) {
        strcat(advice, 
               "⚖️ WEIGHT:\n"
               "• Consider tracking your diet and exercise\n"
               "• Maintain a balanced diet with proper nutrition\n"
               "• Consult with a nutritionist if needed\n\n");
    }
    
    if (abnormal_bp > 0) {
        strcat(advice, 
               "💓 BLOOD PRESSURE:\n"
               "• Reduce sodium intake and stress levels\n"
               "• Regular physical activity is recommended\n"
               "• Consult with healthcare professional for advice\n\n");
    }
    
    if (abnormal_sugar > 0) {
        strcat(advice, 
               "🍬 BLOOD SUGAR:\n"
               "• Monitor carbohydrate intake carefully\n"
               "• Regular meals and exercise help maintain levels\n"
               "• Follow-up with healthcare provider for testing\n\n");
    }
    
    if (abnormal_temp > 0) {
        strcat(advice, 
               "🌡️ BODY TEMPERATURE:\n"
               "• Rest and stay hydrated\n"
               "• Monitor for additional symptoms\n"
               "• Seek medical attention if temperature persists\n\n");
    }
    
    if (abnormal_weight == 0 && abnormal_bp == 0 && 
        abnormal_sugar == 0 && abnormal_temp == 0) {
        strcat(advice,
               "✅ ALL METRICS NORMAL:\n"
               "• Continue maintaining your healthy lifestyle\n"
               "• Regular check-ups are still recommended\n"
               "• Keep monitoring your health parameters\n\n");
    }

    return advice;
}
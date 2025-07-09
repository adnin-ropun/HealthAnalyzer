#include "health_logic.h"

void write_data_to_file(const char *date, const char *height, const char *weight, 
                       const char *bp_sys, const char *bp_dia, const char *blood_sugar, 
                       const char *temp) {
    FILE *file = fopen("input.txt", "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%s,%s,%s,%s\n", date, height, weight, bp_sys, bp_dia, blood_sugar, temp);
        fclose(file);
    }
}

// Helper function to get status indicators for health metrics
const char* get_status_indicator(double value, double low_threshold, double normal_threshold, double high_threshold) {
    if (value < low_threshold)
        return "Below Normal";
    else if (value > high_threshold)
        return "Above Normal";
    else if (value >= low_threshold && value <= normal_threshold)
        return "Normal";
    else
        return "Above Normal";
}

const char* get_bp_status(int systolic, int diastolic) {
    if (systolic >= 140 || diastolic >= 90)
        return "Above Normal";
    else if (systolic < 90 || diastolic < 60)
        return "Below Normal";
    else if (systolic >= 120 || diastolic >= 80)
        return "Above Normal";
    else
        return "Normal";
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

    *abnormal_weight = 0;
    *abnormal_bp = 0;
    *abnormal_sugar = 0;
    *abnormal_temp = 0;

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (strcmp(date, start_date) >= 0 && strcmp(date, end_date) <= 0) {
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

int get_all_health_data(HealthData **data) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        return 0;
    }

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        count++;
    }
    
    if (count == 0) {
        fclose(file);
        return 0;
    }

    *data = malloc(count * sizeof(HealthData));
    if (!*data) {
        fclose(file);
        return 0;
    }

    rewind(file);
    int index = 0;
    char date[20], height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    
    while (fgets(line, sizeof(line), file) && index < count) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", 
               date, height, weight, bp_sys, bp_dia, sugar, temp);
        
        strcpy((*data)[index].date, date);
        (*data)[index].bp_systolic = atof(bp_sys);
        (*data)[index].bp_diastolic = atof(bp_dia);
        (*data)[index].blood_sugar = atof(sugar);
        index++;
    }

    fclose(file);
    return count;
}

int get_comparison_table_data(const char *current_date, ComparisonTableData **data) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        return 0;
    }

    char line[256];
    char date[20], prev_date[20] = "", height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    char prev_height[10] = "", prev_weight[10] = "", prev_bp_sys[10] = "", prev_bp_dia[10] = "", prev_sugar[10] = "", prev_temp[10] = "";
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

    if (!found) {
        return 0;
    }

    *data = malloc(6 * sizeof(ComparisonTableData));
    if (!*data) return 0;

    int row = 0;

    // Height
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "Height (cm)");
    strcpy((*data)[row].current_value, height);
    strcpy((*data)[row].previous_value, prev_found ? prev_height : "N/A");
    strcpy((*data)[row].change, prev_found ? "No change" : "N/A");
    strcpy((*data)[row].status, "N/A");
    row++;

    // Weight
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "Weight (kg)");
    strcpy((*data)[row].current_value, weight);
    strcpy((*data)[row].previous_value, prev_found ? prev_weight : "N/A");
    if (prev_found) {
        double change = atof(weight) - atof(prev_weight);
        char change_str[20];
        if (change > 0.1) {
            snprintf(change_str, sizeof(change_str), "+%.1f kg", change);
        } else if (change < -0.1) {
            snprintf(change_str, sizeof(change_str), "%.1f kg", change);
        } else {
            strcpy(change_str, "No change");
        }
        strcpy((*data)[row].change, change_str);
    } else {
        strcpy((*data)[row].change, "N/A");
    }
    strcpy((*data)[row].status, get_status_indicator(atof(weight), 30, 55, 65));
    row++;

    // Blood Pressure Systolic
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "BP Systolic (mmHg)");
    strcpy((*data)[row].current_value, bp_sys);
    strcpy((*data)[row].previous_value, prev_found ? prev_bp_sys : "N/A");
    if (prev_found) {
        double change = atof(bp_sys) - atof(prev_bp_sys);
        char change_str[20];
        if (change > 0) {
            snprintf(change_str, sizeof(change_str), "+%.0f mmHg", change);
        } else if (change < 0) {
            snprintf(change_str, sizeof(change_str), "%.0f mmHg", change);
        } else {
            strcpy(change_str, "No change");
        }
        strcpy((*data)[row].change, change_str);
    } else {
        strcpy((*data)[row].change, "N/A");
    }
    strcpy((*data)[row].status, get_bp_status(atoi(bp_sys), atoi(bp_dia)));
    row++;

    // Blood Pressure Diastolic
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "BP Diastolic (mmHg)");
    strcpy((*data)[row].current_value, bp_dia);
    strcpy((*data)[row].previous_value, prev_found ? prev_bp_dia : "N/A");
    if (prev_found) {
        double change = atof(bp_dia) - atof(prev_bp_dia);
        char change_str[20];
        if (change > 0) {
            snprintf(change_str, sizeof(change_str), "+%.0f mmHg", change);
        } else if (change < 0) {
            snprintf(change_str, sizeof(change_str), "%.0f mmHg", change);
        } else {
            strcpy(change_str, "No change");
        }
        strcpy((*data)[row].change, change_str);
    } else {
        strcpy((*data)[row].change, "N/A");
    }
    strcpy((*data)[row].status, get_bp_status(atoi(bp_sys), atoi(bp_dia)));
    row++;

    // Blood Sugar
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "Blood Sugar (mg/dL)");
    strcpy((*data)[row].current_value, sugar);
    strcpy((*data)[row].previous_value, prev_found ? prev_sugar : "N/A");
    if (prev_found) {
        double change = atof(sugar) - atof(prev_sugar);
        char change_str[20];
        if (change > 0) {
            snprintf(change_str, sizeof(change_str), "+%.0f mg/dL", change);
        } else if (change < 0) {
            snprintf(change_str, sizeof(change_str), "%.0f mg/dL", change);
        } else {
            strcpy(change_str, "No change");
        }
        strcpy((*data)[row].change, change_str);
    } else {
        strcpy((*data)[row].change, "N/A");
    }
    strcpy((*data)[row].status, get_status_indicator(atof(sugar), 70.0, 99.0, 126.0));
    row++;

    // Temperature
    strcpy((*data)[row].date, current_date);
    strcpy((*data)[row].metric, "Temperature (°C)");
    strcpy((*data)[row].current_value, temp);
    strcpy((*data)[row].previous_value, prev_found ? prev_temp : "N/A");
    if (prev_found) {
        double change = atof(temp) - atof(prev_temp);
        char change_str[20];
        if (change > 0.1) {
            snprintf(change_str, sizeof(change_str), "+%.1f °C", change);
        } else if (change < -0.1) {
            snprintf(change_str, sizeof(change_str), "%.1f °C", change);
        } else {
            strcpy(change_str, "No change");
        }
        strcpy((*data)[row].change, change_str);
    } else {
        strcpy((*data)[row].change, "N/A");
    }
    strcpy((*data)[row].status, get_status_indicator(atof(temp), 36.1, 37.0, 38.0));

    return 6;
}

int get_stats_table_data(const char *start_date, const char *end_date, StatsTableData **data) {
    FILE *file = fopen("input.txt", "r");
    if (!file) {
        return 0;
    }

    double height_sum = 0, weight_sum = 0, bp_sys_sum = 0, bp_dia_sum = 0, sugar_sum = 0, temp_sum = 0;
    int count = 0;
    
    char line[256], date[20], height[10], weight[10], bp_sys[10], bp_dia[10], sugar[10], temp[10];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (strcmp(date, start_date) >= 0 && strcmp(date, end_date) <= 0) {
            height_sum += atof(height);
            weight_sum += atof(weight);
            bp_sys_sum += atof(bp_sys);
            bp_dia_sum += atof(bp_dia);
            sugar_sum += atof(sugar);
            temp_sum += atof(temp);
            count++;
        }
    }

    if (count == 0) {
        fclose(file);
        return 0;
    }

    double height_avg = height_sum / count;
    double weight_avg = weight_sum / count;
    double bp_sys_avg = bp_sys_sum / count;
    double bp_dia_avg = bp_dia_sum / count;
    double sugar_avg = sugar_sum / count;
    double temp_avg = temp_sum / count;

    rewind(file);
    double height_var = 0, weight_var = 0, bp_sys_var = 0, bp_dia_var = 0, sugar_var = 0, temp_var = 0;
    
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s", date, height, weight, bp_sys, bp_dia, sugar, temp);

        if (strcmp(date, start_date) >= 0 && strcmp(date, end_date) <= 0) {
            height_var += pow(atof(height) - height_avg, 2);
            weight_var += pow(atof(weight) - weight_avg, 2);
            bp_sys_var += pow(atof(bp_sys) - bp_sys_avg, 2);
            bp_dia_var += pow(atof(bp_dia) - bp_dia_avg, 2);
            sugar_var += pow(atof(sugar) - sugar_avg, 2);
            temp_var += pow(atof(temp) - temp_avg, 2);
        }
    }

    fclose(file);

    *data = malloc(6 * sizeof(StatsTableData));
    if (!*data) return 0;

    int row = 0;

    // Height
    strcpy((*data)[row].metric, "Height (cm)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", height_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(height_var / count));
    strcpy((*data)[row].status, "N/A");
    row++;

    // Weight
    strcpy((*data)[row].metric, "Weight (kg)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", weight_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(weight_var / count));
    strcpy((*data)[row].status, get_status_indicator(weight_avg, 30, 55, 65));
    row++;

    // BP Systolic
    strcpy((*data)[row].metric, "BP Systolic (mmHg)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", bp_sys_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(bp_sys_var / count));
    strcpy((*data)[row].status, get_bp_status((int)bp_sys_avg, (int)bp_dia_avg));
    row++;

    // BP Diastolic
    strcpy((*data)[row].metric, "BP Diastolic (mmHg)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", bp_dia_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(bp_dia_var / count));
    strcpy((*data)[row].status, get_bp_status((int)bp_sys_avg, (int)bp_dia_avg));
    row++;

    // Blood Sugar
    strcpy((*data)[row].metric, "Blood Sugar (mg/dL)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", sugar_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(sugar_var / count));
    strcpy((*data)[row].status, get_status_indicator(sugar_avg, 70.0, 99.0, 126.0));
    row++;

    // Temperature
    strcpy((*data)[row].metric, "Temperature (°C)");
    snprintf((*data)[row].average, sizeof((*data)[row].average), "%.1f", temp_avg);
    snprintf((*data)[row].std_deviation, sizeof((*data)[row].std_deviation), "%.1f", sqrt(temp_var / count));
    strcpy((*data)[row].status, get_status_indicator(temp_avg, 36.1, 37.0, 38.0));

    return 6;
}

int get_abnormality_table_data(const char *start_date, const char *end_date, AbnormalityTableData **data) {
    int abnormal_weight = 0, abnormal_bp = 0, abnormal_sugar = 0, abnormal_temp = 0;
    
    check_for_abnormalities_typewise_in_range(start_date, end_date, 
                                            &abnormal_weight, &abnormal_bp, 
                                            &abnormal_sugar, &abnormal_temp);

    *data = malloc(4 * sizeof(AbnormalityTableData));
    if (!*data) return 0;

    int row = 0;

    strcpy((*data)[row].category, "Weight Management");
    snprintf((*data)[row].count, sizeof((*data)[row].count), "%d", abnormal_weight);
    strcpy((*data)[row].advice, abnormal_weight > 0 ? "Needs attention" : "Good condition");
    row++;

    strcpy((*data)[row].category, "Blood Pressure");
    snprintf((*data)[row].count, sizeof((*data)[row].count), "%d", abnormal_bp);
    strcpy((*data)[row].advice, abnormal_bp > 0 ? "Needs attention" : "Good condition");
    row++;

    strcpy((*data)[row].category, "Blood Sugar");
    snprintf((*data)[row].count, sizeof((*data)[row].count), "%d", abnormal_sugar);
    strcpy((*data)[row].advice, abnormal_sugar > 0 ? "Needs attention" : "Good condition");
    row++;

    strcpy((*data)[row].category, "Body Temperature");
    snprintf((*data)[row].count, sizeof((*data)[row].count), "%d", abnormal_temp);
    strcpy((*data)[row].advice, abnormal_temp > 0 ? "Needs attention" : "Good condition");

    return 4;
}

char* get_health_recommendations(const char *start_date, const char *end_date) {
    int abnormal_weight = 0, abnormal_bp = 0, abnormal_sugar = 0, abnormal_temp = 0;
    
    check_for_abnormalities_typewise_in_range(start_date, end_date, 
                                            &abnormal_weight, &abnormal_bp, 
                                            &abnormal_sugar, &abnormal_temp);

    char* recommendations = malloc(2000);
    if (!recommendations) return NULL;
    
    if (abnormal_weight == 0 && abnormal_bp == 0 && abnormal_sugar == 0 && abnormal_temp == 0) {
        strcpy(recommendations, 
               "Health Overview\n"
               "===============\n\n"
               "Everything looked normal in your health data for this period.\n\n"
               "Health Tips:\n"
               "• Continue your current healthy habits\n"
               "• Keep doing regular physical activity\n"
               "• Drink plenty of water daily (8-10 glasses)\n"
               "• Get good sleep every night (7-8 hours)\n"
               "• Visit your doctor for regular check-ups\n"
               "• Keep tracking your health as you're doing\n"
               "• Find healthy ways to manage stress\n\n");
        return recommendations;
    }
    
    strcpy(recommendations, "Health Overview\n");
    strcat(recommendations, "===============\n\n");
    
    if (abnormal_weight > 0) {
        strcat(recommendations, "Weight Management:\n");
        char weight_info[200];
        snprintf(weight_info, sizeof(weight_info), 
                "Unusual weight readings detected on %d day(s).\n\n", abnormal_weight);
        strcat(recommendations, weight_info);
        strcat(recommendations, "• Focus on eating balanced, nutritious meals\n");
        strcat(recommendations, "• Try to be more active in your daily routine\n");
        strcat(recommendations, "• Keep track of what you eat and drink\n");
        strcat(recommendations, "• Consider talking to a nutrition expert\n");
        strcat(recommendations, "• Set realistic weight goals\n\n");
    }

    if (abnormal_bp > 0) {
        strcat(recommendations, "Blood Pressure Care:\n");
        char bp_info[200];
        snprintf(bp_info, sizeof(bp_info), 
                "Unusual blood pressure levels detected on %d day(s).\n\n", abnormal_bp);
        strcat(recommendations, bp_info);
        strcat(recommendations, "• Reduce salt in your food\n");
        strcat(recommendations, "• Limit coffee and alcohol intake\n");
        strcat(recommendations, "• Try relaxation techniques like deep breathing\n");
        strcat(recommendations, "• Stay active with regular exercise\n");
        strcat(recommendations, "• Schedule a doctor visit soon\n");
        strcat(recommendations, "• Monitor your blood pressure regularly\n\n");
    }

    if (abnormal_sugar > 0) {
        strcat(recommendations, "Blood Sugar Management:\n");
        char sugar_info[200];
        snprintf(sugar_info, sizeof(sugar_info), 
                "Unusual blood sugar levels detected on %d day(s).\n\n", abnormal_sugar);
        strcat(recommendations, sugar_info);
        strcat(recommendations, "• Watch your intake of sweets and carbs\n");
        strcat(recommendations, "• Check your blood sugar as recommended\n");
        strcat(recommendations, "• Take your medications on time\n");
        strcat(recommendations, "• See a diabetes specialist\n");
        strcat(recommendations, "• Stay active after meals\n");
        strcat(recommendations, "• Eat meals at regular times\n\n");
    }

    if (abnormal_temp > 0) {
        strcat(recommendations, "Temperature Monitoring:\n");
        char temp_info[200];
        snprintf(temp_info, sizeof(temp_info), 
                "Unusual temperature readings detected on %d day(s).\n\n", abnormal_temp);
        strcat(recommendations, temp_info);
        strcat(recommendations, "• Get plenty of rest and good sleep\n");
        strcat(recommendations, "• Drink lots of fluids\n");
        strcat(recommendations, "• Watch for other symptoms\n");
        strcat(recommendations, "• See a doctor if fever continues\n");
        strcat(recommendations, "• Take it easy with physical activities\n\n");
    }

    strcat(recommendations, "General Health Tips:\n");
    strcat(recommendations, "• Keep up with regular doctor visits\n");
    strcat(recommendations, "• Continue monitoring your health daily\n");
    strcat(recommendations, "• Maintain good sleep habits\n");
    strcat(recommendations, "• Practice stress management\n");

    return recommendations;
}
#ifndef HEALTH_LOGIC_H
#define HEALTH_LOGIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Function declarations for core logic
void write_data_to_file(const char *date, const char *height, const char *weight, 
                       const char *bp_sys, const char *bp_dia, const char *blood_sugar, 
                       const char *temp);

char* get_comparison_with_previous_day(const char *current_date);
char* calculate_stats_for_range(const char *start_date, const char *end_date);
void check_for_abnormalities_typewise_in_range(const char *start_date, const char *end_date, 
                                              int *abnormal_weight, int *abnormal_bp, 
                                              int *abnormal_sugar, int *abnormal_temp);
char* get_advice_based_on_abnormalities(int abnormal_weight, int abnormal_bp, 
                                      int abnormal_sugar, int abnormal_temp);

#endif // HEALTH_LOGIC_H
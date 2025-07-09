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

void check_for_abnormalities_typewise_in_range(const char *start_date, const char *end_date, 
                                              int *abnormal_weight, int *abnormal_bp, 
                                              int *abnormal_sugar, int *abnormal_temp);

// Structure for graph data
typedef struct {
    char date[20];
    double bp_systolic;
    double bp_diastolic;
    double blood_sugar;
} HealthData;

// Structures for table data
typedef struct {
    char date[20];
    char metric[30];
    char current_value[20];
    char previous_value[20];
    char change[20];
    char status[20];
} ComparisonTableData;

typedef struct {
    char metric[30];
    char average[20];
    char std_deviation[20];
    char status[20];
} StatsTableData;

typedef struct {
    char category[30];
    char count[10];
    char advice[200];
} AbnormalityTableData;

// Function declarations
int get_all_health_data(HealthData **data);
int get_comparison_table_data(const char *current_date, ComparisonTableData **data);
int get_stats_table_data(const char *start_date, const char *end_date, StatsTableData **data);
int get_abnormality_table_data(const char *start_date, const char *end_date, AbnormalityTableData **data);
char* get_health_recommendations(const char *start_date, const char *end_date);

#endif // HEALTH_LOGIC_H
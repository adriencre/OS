#ifndef HEALTH_H
#define HEALTH_H

#include <stdint.h>

// Structure pour représenter un carnet de santé
typedef struct {
    int id;
    const char* name;
    const char* birth_date;
    const char* blood_type;
    const char* allergies;
    const char* last_visit;
} HealthRecord;

// Fonctions pour interagir avec les données
void list_all_patients();
void show_patient_by_id(int id);
int get_num_patients();

// Fonction principale de l'application Carnet de Santé
void health_app();

#endif // HEALTH_H

// Huseyin Alper Karadeniz (28029) - CS 307 PA1

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

// Define the maximum number of people in database
#define NUM_PEOPLE 4096
#define SIZE_GENDER 4
#define SIZE_NAME 1024
#define SIZE_SURNAME 1024

// Define the Person structure
struct Person {
    char gender[SIZE_GENDER];
    char name[SIZE_NAME];
    char surname[SIZE_SURNAME];
};

// Create the array, which will store the data of each people
struct Person people[NUM_PEOPLE];

// Store the total number of people in the database
int peopleCount = 0;

int findIndex(char* name) {
    
    for (int i = 0; i < peopleCount; i++) {
        
        // Find the index of the desired person name
        if (strcmp(name, people[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void modifyFile(char* path) {
    
    // Open the desired input file
    FILE* inputFile = fopen(path, "r+");
    
    // Declare a variable to store the words
    char word[1024];
    char gender[SIZE_GENDER];
    char name[SIZE_NAME];
    char surname[SIZE_SURNAME];
    
    // Read the words from the input file, one at a time
    while (fscanf(inputFile, "%s", word) != EOF) {
        // printf("The word: %s\n", word);
        
        // Check if there is a person after the current word
        if ((strcmp(word, "Mr.") == 0) || (strcmp(word, "Ms.") == 0)) {
            
            // Copy the word into the gender
            strcpy(gender, word);
            
            // Get the name and surname of the person
            fscanf(inputFile, "%s %s", name, surname);
            
            // Find original data of the person from the database
            struct Person originalPerson = people[findIndex(name)];
            
            fseek(inputFile, - (strlen(gender) + strlen(name) + strlen(surname) + 2), SEEK_CUR);
            char replaced[strlen(gender) + strlen(name) + strlen(surname) + 2];
            
            strcpy(replaced, originalPerson.gender);
            strcat(replaced, " ");
            strcat(replaced, originalPerson.name);
            strcat(replaced, " ");
            strcat(replaced, originalPerson.surname);
            
            fputs(replaced, inputFile);
            
            // long curr = ftell(inputFile);
            // printf("Current Pos: %ld, Heyyo: %ld %ld %ld\n", curr, strlen(gender), strlen(name), strlen(surname));
            
            // printf("Current: %s %s %s\n", gender, name, surname);
            // printf("Original: %s %s %s\n", originalPerson.gender, originalPerson.name, originalPerson.surname);
            
        }
    }
    
    // Close the input file
    fclose(inputFile);
}

void modifyDirectory(char* dirname) {
    
    // Open the desired directory
    DIR* dir = opendir(dirname);
    if (!dir) {return;}
    
    // Read the contents by traversing directories
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        
        // Skip the '.' and '..' entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            
            // Declare a path variable to store the new path
            char path[1024];
            strcpy(path, dirname);
            strcat(path, "/");
            strcat(path, entry->d_name);
            
            // Check if the entry is a sub-directory
            if (entry->d_type == DT_DIR) {
                // Modify the sub-directory
                modifyDirectory(path);
                // printf("Directory: %s\n", path);
            }
            
            // Check if the entry is a regular file
            else if (entry->d_type == DT_REG) {
                
                // Get the file format of the desired file
                char* fileFormat = strrchr(entry->d_name, '.');
                
                // Check if the file format is a .txt file
                if (fileFormat != NULL && strcmp(fileFormat, ".txt") == 0) {
                    
                    // Check if the file in the root directory
                    if (strcmp(dirname, ".") == 0) {
                        
                        // Check if it is the database file in the root directory
                        if (strcmp(entry->d_name, "database.txt") != 0) {
                            // Modify the file in the path
                            modifyFile(path);
                            // printf("File: %s\n", path);
                        }
                        
                    } else {
                        // Modify the file in the path
                        modifyFile(path);
                        // printf("File: %s\n", path);
                    }
                }
            }
        }
    }
    
    // Close the directory
    closedir(dir);
}

int main() {
    
    // Open database file for reading
    FILE* dbFile = fopen("database.txt", "r");

    // Declare variables to store the data temporarily
    char gender[SIZE_GENDER];
    char storedGender[SIZE_GENDER];
    char name[SIZE_NAME];
    char surname[SIZE_SURNAME];

    // Read the data in the database file, one line at a time
    while (fscanf(dbFile, "%s %s %s", gender, name, surname) == 3) {
        
        // Create a new Person structure
        struct Person newPerson;
        if (gender[0] == 'f') {strcpy(storedGender, "Ms.");} else {strcpy(storedGender, "Mr.");}
        strcpy(newPerson.gender, storedGender);
        strcpy(newPerson.name, name);
        strcpy(newPerson.surname, surname);
        
        // Add the new Person to the people array
        people[peopleCount] = newPerson;
        
        // Increment the count for the total number of people in the database
        peopleCount++;
    }
    
    // Close the database file
    fclose(dbFile);
    
    // Modify all the files inside of the desired directory
    modifyDirectory(".");
    
    /*
    // Print the data for each person in the database
    for (int i = 0; i < peopleCount; i++) {
        printf("Gender: %s\n", people[i].gender);
        printf("Name: %s\n", people[i].name);
        printf("Surname: %s\n\n", people[i].surname);
    }
    */

    return 0;
}

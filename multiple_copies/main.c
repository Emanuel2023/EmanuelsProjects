//Emanuel Contreras lct377
//EE-3323-0A1
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ARG_COUNT 4 //arg

//Arument count is defined as 4, because it represents the number of command lines. In our case ./multiple_copies source_file.txt destination_file1.txt destination_file2.txt

bool fileExists(const char *fileName) {//This boolean function checks if a file exists.
    FILE *file = fopen(fileName, "r");
    if (file) { //If the file pointer is no NULL then (file exists).
        fclose(file);//Used to close the file
        return true;//Return a value true to indicate that the file does indeed exists.
    }
    return false;// If the file pointer is indeed NULL, then a value false will be returned.
}

int main(int argc, char *argv[]) {
    if (argc != ARG_COUNT) { //This if statement check if the number of command-line arguments is not equal to ARG_COUNT
        printf("Usage: multiple_copies source_file.txt destination_file1.txt destination_file2.txt\n");
        return 1;//Prints the error statement above and returns a 1 to indicate that there was some sort of error.
    } else {
        // THis else statement checks if the source file exists.
        if (!fileExists(argv[1])) { //Calling the fileExists function to check if the source file exists.
            printf("Opening Error!: %s\n", argv[1]); //This line is used to print an error message with the filename.
            return 1;//Returns a 1 to indicate that the was some sort of error occuring.
        }

        const char *sourceFileName = argv[1];//Sets the source filename from command-line arguments
        const char *destinationFileName1 = argv[2]; //Sets the first source filename from command-line arguments.
        const char *destinationFileName2 = argv[3];

        // Opens the destination files for writing (overwrite if they already exist).
        FILE *sourceFile = fopen(sourceFileName, "r");
        FILE *destinationFile1 = fopen(destinationFileName1, "w");
        FILE *destinationFile2 = fopen(destinationFileName2, "w");

        int ch;
        while ((ch = fgetc(sourceFile)) != EOF) {// This while loop is used to copy characters from source file to both destination files.
            fputc(ch, destinationFile1);
            fputc(ch, destinationFile2);
        }
        // This portion of the code is used to close the sourcefile and destination files.
        fclose(sourceFile);
        fclose(destinationFile1);
        fclose(destinationFile2);
        printf("The contents of %s copied to %s and %s.\n", sourceFileName, destinationFileName1, destinationFileName2);
        return 0; //Returns that there was a success.
    }
}


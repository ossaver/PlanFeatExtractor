/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* State Information Extractor                          */
/********************************************************/
/* Main method: parses the command-line arguments and   */
/* launches the program.                                */
/********************************************************/

#include <iostream>
#include <string.h>
#include "parser/parser.h"
#include "parser/parsedTask.h"
#include "preprocess/preprocess.h"
#include "stages/task.h"
#include "stages/stages.h"

using namespace std;

// Prints the command-line arguments of the planner
void printUsage() {
    cout << "Usage to extract stages:" << endl;
    cout << "\tplanFeatExtractor <domain_file>" << endl;
    cout << "Usage to classify problem objects:" << endl;
    cout << "\tplanFeatExtractor <domain_file> <problem_file>" << endl;
}

// Parses the domain and problem files
ParsedTask* parseStage(char* domainFileName, char* problemFileName) {
    Parser parser;
    ParsedTask* parsedTask = parser.parseDomain(domainFileName);
    if (problemFileName != NULL) parser.parseProblem(problemFileName);
    return parsedTask;
}

// Preprocesses the parsed task
PreprocessedTask* preprocessStage(ParsedTask* parsedTask) {
    Preprocess preprocess;
    PreprocessedTask* prepTask = preprocess.preprocessTask(parsedTask);
    return prepTask;
}

// Computes the stages and classifies the objects (if the problem is given)
void computeStages(PreprocessedTask* prepTask, bool classify) {
    Task task(prepTask);
    Stages stages(&task);
    if (classify) {
        stages.classify();
    }
    else {
        stages.toJSON();
    }
}

// Main method
int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
       printUsage();
    } else {
        char* domainFileName = argv[1];
        char* problemFileName = argc == 3 ? argv[2] : NULL;
        
        ParsedTask* parsedTask = parseStage(domainFileName, problemFileName);
        if (parsedTask != nullptr) {
            PreprocessedTask* prepTask = preprocessStage(parsedTask);
            if (prepTask != nullptr) {
                computeStages(prepTask, problemFileName != NULL);
                delete prepTask;
            }
            delete parsedTask;
        }
    }
    return 0;
}

#ifndef MUTEX_H
#define MUTEX_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Mutex calculation from a grounded problem.           */
/********************************************************/

#include "../grounder/groundedTask.h"

class Mutex {
private:
    GroundedTask* gTask;
    bool** mutex;
    unsigned int numVars;
    unsigned int numActions;
    bool* literalInF;
    bool* isLiteral;
    unsigned int numNewLiterals;
    bool* actions;
    bool* literalInFNA;
    std::unordered_map<unsigned long long, bool> mutexChanges;

    void getInitialStateLiterals();
    void checkAction(GroundedAction* a);
    bool holdsCondition(const GroundedCondition* c, std::vector<unsigned int>* preconditions);
    void computeMutex(GroundedAction* a, const std::vector<unsigned int> preconditions, unsigned int startEndPrec);
    inline static int findInVector(unsigned int value, const std::vector<unsigned int>* v) {
        for (unsigned int i = 0; i < v->size(); i++)
            if ((*v)[i] == value) return (int)i;
        return -1;
    }
    inline unsigned long long mutexIndex(unsigned int v1, unsigned int v2) {
        if (v1 > v2) {
            unsigned int aux = v1;
            v1 = v2;
            v2 = aux;
        }
        return (((unsigned long long) v1) << 32) + v2;
    }
    inline void deleteMutex(unsigned int v1, unsigned int v2) {
        if (mutex[v1][v2]) {
            mutex[v1][v2] = false;
            mutex[v2][v1] = false;
            unsigned long long code = mutexIndex(v1, v2);
            std::unordered_map<unsigned long long, bool>::const_iterator got = mutexChanges.find(code);
            if (got == mutexChanges.end() || got->second) {
                mutexChanges[code] = false;
            }
        }
    }
    inline void addMutex(unsigned int v1, unsigned int v2) {
        if (!mutex[v1][v2]) {
            mutex[v1][v2] = true;
            mutex[v2][v1] = true;
            unsigned long long code = mutexIndex(v1, v2);
            std::unordered_map<unsigned long long, bool>::const_iterator got = mutexChanges.find(code);
            if (got == mutexChanges.end() || !got->second) {
                mutexChanges[code] = true;
            }
        }
    }
    inline static int literalInAtStartAdd(unsigned int value, std::vector<unsigned int>* add, unsigned int statAddEndEff) {
        for (unsigned int i = 0; i < statAddEndEff; i++)
            if ((*add)[i] == value) return (int)i;
        return -1;
    }

public:
    void computeMutex(GroundedTask* gTask);
};

#endif

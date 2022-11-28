#include <iostream>
#include "mutex.h"

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Mutex calculation from a grounded problem.           */
/********************************************************/

using namespace std;

// F* <- I
void Mutex::getInitialStateLiterals() {
    literalInF = new bool[numVars] {false};
    isLiteral = new bool[numVars] {false};
    numNewLiterals = 0;
    for (unsigned int i = 0; i < numVars; i++) {
        GroundedVar& v = gTask->variables[i];
        if (gTask->task->isBooleanFunction(v.fncIndex)) {
            isLiteral[i] = true;
            bool holds = false;
            for (unsigned int j = 0; j < v.initialValues.size(); j++) {
                if (v.initialValues[j].value == gTask->task->CONSTANT_TRUE) {
                    holds = true;
                    break;
                }
            }
            if (holds) {
                numNewLiterals++;
                literalInF[i] = true;
            }
        }
    }
}

// Checks if the conditions is a literal that holds in F*
bool Mutex::holdsCondition(const GroundedCondition* c, vector<unsigned int>* preconditions) {
    if (!isLiteral[c->varIndex]) return true;           // It's not a literal
    if (c->valueIndex == gTask->task->CONSTANT_FALSE) return true; // Negated literal
    preconditions->push_back(c->varIndex);
    return literalInFNA[c->varIndex];
}

// Computes the new mutex that this action generates
void Mutex::computeMutex(GroundedAction* a, const vector<unsigned int> preconditions, unsigned int startEndPrec) {
    vector<unsigned int> newA, add, del;
    unsigned int varIndex, statAddEndEff, startDelEndEff, startNewEndEff;
    for (unsigned int i = 0; i < a->startEff.size(); i++) { // New(a) <- Add(a) - F*
        varIndex = a->startEff[i].varIndex;
        if (isLiteral[varIndex]) {
            if (a->startEff[i].valueIndex == gTask->task->CONSTANT_TRUE) {
                add.push_back(varIndex);
                if (!literalInF[varIndex])
                    newA.push_back(varIndex);
            }
            else del.push_back(varIndex);
        }
    }
    statAddEndEff = add.size();
    startDelEndEff = del.size();
    startNewEndEff = newA.size();
    for (unsigned int i = 0; i < a->endEff.size(); i++) {
        varIndex = a->endEff[i].varIndex;
        if (isLiteral[varIndex]) {
            if (a->endEff[i].valueIndex == gTask->task->CONSTANT_TRUE) {
                add.push_back(varIndex);
                if (!literalInF[varIndex])
                    newA.push_back(varIndex);
            }
            else del.push_back(varIndex);
        }
    }
    for (unsigned int f = 0; f < newA.size(); f++) {  // forall f in New(a)
        for (unsigned int h = 0; h < del.size(); h++) {  // forall h in Del(a)
            if (f >= startNewEndEff || h < startDelEndEff) {  // f is at-end or h is at-start
                if (findInVector(del[h], &preconditions) != -1 || literalInAtStartAdd(del[h], &add, statAddEndEff) != -1) {	// h in Pre(a) or h in AtStartAdd(a) -> condition added
                    addMutex(newA[f], del[h]);
                }
            }
        }
        for (unsigned int p = 0; p < preconditions.size(); p++) {  // p in Pre(a)
            for (unsigned int q = 0; q < numVars; q++) {           // (p,q) in M* / q not in Del(a)
                if (isLiteral[q] && q != newA[f] && mutex[preconditions[p]][q] &&
                    (p < startEndPrec || f >= startNewEndEff) &&   // p is at-start or over-all, or f is at-end
                    findInVector(q, &del) == -1) {
                    addMutex(newA[f], q);
                }
            }
        }
    }
    if (!actions[a->index]) {  // a in A
        unsigned int addSize = statAddEndEff > 0 ? statAddEndEff - 1 : statAddEndEff;
        for (unsigned int p = 0; p <= addSize; p++) {  // p,q in Add(a) / (p,q) in M*
            for (unsigned int q = p + 1; q < statAddEndEff; q++) {
                if (mutex[add[p]][add[q]]) {
                    deleteMutex(add[p], add[q]);
                }
            }
            for (unsigned int q = statAddEndEff; q < add.size(); q++) {
                if (mutex[add[p]][add[q]]) {

                    if (mutex[add[p]][add[q]] && findInVector(add[p], &del) == -1) {
                        deleteMutex(add[p], add[q]);
                    }
                }
            }
        }
        addSize = add.size() > 0 ? add.size() - 1 : add.size();
        for (unsigned int p = statAddEndEff; p < addSize; p++)  // p,q in Add(a) / (p,q) in M*
            for (unsigned int q = p + 1; q < add.size(); q++) {
                if (mutex[add[p]][add[q]]) {
                    deleteMutex(add[p], add[q]);
                }
            }
    }
    for (unsigned int i = 0; i < add.size(); i++) { // L <- Add(a) - New(a)
        if (findInVector(add[i], &newA) == -1) {     // i in L
            for (unsigned int q = 0; q < numVars; q++) {
                if (isLiteral[q] && mutex[add[i]][q] &&  // (i,q) in M*
                    findInVector(q, &del) == -1) {        // q not in Del(a)
                    bool existsP = false;                // not exits p in Pre(a) / (p,q) in M*
                    for (unsigned p = 0; p < preconditions.size(); p++) {
                        if (mutex[preconditions[p]][q]) {
                            existsP = true;
                            break;
                        }
                    }
                    if (!existsP) {
                        deleteMutex(add[i], q);
                    }
                }
            }
        }
    }
    for (unsigned int i = 0; i < newA.size(); i++) {  // F* <- F* U New(a)
        literalInF[newA[i]] = true;
        numNewLiterals++;
    }
    actions[a->index] = true;  // A <- A U {a}
}

// Checks id the given action generates new mutex
void Mutex::checkAction(GroundedAction* a) {
    vector<unsigned int> preconditions;
    unsigned int startEndPrec;
    for (unsigned int i = 0; i < a->startCond.size(); i++) {
        if (!holdsCondition(&(a->startCond[i]), &preconditions)) return;
    }
    for (unsigned int i = 0; i < a->overCond.size(); i++) {
        if (!holdsCondition(&(a->overCond[i]), &preconditions)) return;
    }
    startEndPrec = preconditions.size();
    for (unsigned int i = 0; i < a->endCond.size(); i++) {
        if (!holdsCondition(&(a->endCond[i]), &preconditions)) return;
    }
    unsigned int psize = preconditions.size() > 0 ? preconditions.size() - 1 : 0;
    for (unsigned int p = 0; p < psize; p++) {
        for (unsigned int q = p + 1; q < preconditions.size(); q++) {
            if (mutex[p][q]) return;
        }
    }
    computeMutex(a, preconditions, startEndPrec);
}

void Mutex::computeMutex(GroundedTask* gTask)
{
    this->gTask = gTask;
    numVars = gTask->variables.size();
    numActions = gTask->actions.size();
    getInitialStateLiterals();
    mutex = new bool* [numVars];
    for (unsigned int i = 0; i < numVars; i++)
        mutex[i] = new bool[numVars] {false};
    actions = new bool[numActions] {false};
    literalInFNA = new bool[numVars];
    for (unsigned int i = 0; i < numVars; i++) literalInFNA[i] = literalInF[i];
    while (numNewLiterals > 0 || mutexChanges.size() > 0) {
        mutexChanges.clear();
        numNewLiterals = 0;
        for (unsigned int i = 0; i < numActions; i++) {
            checkAction(&(gTask->actions[i]));
        }
        for (unsigned int i = 0; i < numVars; i++) literalInFNA[i] = literalInF[i];
    }
    delete[] literalInFNA;
    /*
    ParsedTask* task = gTask->task;
    for (unsigned int v1 = 0; v1 < numVars; v1++) {
        for (unsigned int v2 = v1 + 1; v2 < numVars; v2++) {
            if (mutex[v1][v2]) {
                cout << gTask->variables[v1].toString(task) << " " << gTask->variables[v2].toString(task) << endl;
            }
        }
    }
    */
}

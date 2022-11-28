#ifndef TASK_H
#define TASK_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Extracts relevant information from the parsed	    */
/* planning task.                               	    */
/********************************************************/

#include "../preprocess/preprocessedTask.h"

class TaskType {
public:
	int index;
	std::string name;
	std::vector<TaskType*> compatibleTypes;

	TaskType(int index, Type& t);
	void addCompatibleType(TaskType* pt);
	bool compatible(TaskType* t);
};

class TaskPredicate {
public:
	int index;
	std::string name;
	std::vector<TaskType*> arguments;

	TaskPredicate(int index, std::string name, std::vector<TaskType*>* args);
	std::string toString();
};

class TaskEffect {
public:
	TaskPredicate* predicate;
	std::vector<int> paramIndex;	// Number of the argument in the operator parameters (-1 if it is a constant)

	TaskEffect(TaskPredicate* pred, std::vector<int>& paramIndex);
	std::string toString();
};

class TaskOperator {
public:
	int index;
	std::string name;
	std::vector<TaskType*> parameters;
	std::vector<TaskEffect> prec; 
	std::vector<TaskEffect> add;
	std::vector<TaskEffect> del;

	TaskOperator(int index, std::string name, std::vector<TaskType*>* params);
	std::string toString();
	void addAddEffect(TaskPredicate* pred, std::vector<int>& parameterIndex);
	void addDelEffect(TaskPredicate* pred, std::vector<int>& parameterIndex);
	void addPrecondition(TaskPredicate* pred, std::vector<int>& parameterIndex);
};

class TaskObject {
public:
	std::string name;
	TaskType* type;

	TaskObject(std::string& name, TaskType* t) { this->name = name; this->type = t; }
};

class TaskLiteral {
public:
	TaskPredicate* predicate;
	std::vector<TaskObject*> arguments;

	TaskLiteral(TaskPredicate* predicate, std::vector<TaskObject*>& args);
	bool contains(TaskObject* obj);
	int find(TaskObject* obj);
	std::string toString();
	bool equals(TaskLiteral* l);
};

class Task {
private:
	PreprocessedTask* task;
	std::vector<int> newTypeIndex;
	std::vector<int> oldTypeIndex;
	std::vector<int> oldPredicateIndex;
	std::vector<int> newObjectIndex;

	void processTypes();
	void addCompatibleTypes(TaskType* t, std::vector<unsigned int>& parentTypes);
	void processPredicates();
	void generatePredicate(Function* f, std::vector<TaskType*>* args);
	void processOperators();
	void generateOperator(Operator* o, std::vector<TaskType*>* parameters);
	void computeEffects(TaskOperator& to, Operator* o);
	void generateEffect(TaskOperator* to, OpFluent* eff, Operator* o, bool isPrecondition);
	TaskPredicate* matchEffect(Literal *l, std::vector<TaskType*>* opParams, Operator* o, std::vector<int>& parameterIndex);
	void processObjects();
	void processInitialState();
	void processGoal();
	TaskPredicate* findPredicate(Fact* f);
	TaskPredicate* findPredicate(Literal* l);

public:
	std::vector<TaskType> types;
	std::vector<TaskPredicate> predicates;
	std::vector<TaskOperator> operators;
	std::vector<TaskObject> objects;
	std::vector<TaskLiteral> state;
	std::vector<TaskLiteral> goal;

	Task(PreprocessedTask* pTask);
};

#endif
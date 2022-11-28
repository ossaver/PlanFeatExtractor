#include "task.h"
#include <iostream>

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Extracts relevant information from the parsed	    */
/* planning task.                               	    */
/********************************************************/

using namespace std;

/********************************************************/
/* CLASS: Task (processed planning task)                */
/********************************************************/

// Gets the object types from the domain
void Task::processTypes()
{
	std::vector<Type>& parsedTypes = task->task->types;
	int numTypes = (int)parsedTypes.size();
	newTypeIndex.resize(numTypes, -1);
	oldTypeIndex.resize(numTypes, -1);
	int start = task->task->INTEGER_TYPE + 1;
	while (start < numTypes && parsedTypes[start].name.at(0) == '#') start++;
	for (int i = start; i < numTypes; i++) {
		Type& t = parsedTypes[i];
		int index = (int)this->types.size();
		this->types.emplace_back(index, t);
		this->newTypeIndex[t.index] = index;
		this->oldTypeIndex[index] = t.index;
	}
	for (TaskType& t : this->types) {
		Type& pType = parsedTypes[oldTypeIndex[t.index]];
		addCompatibleTypes(&t, pType.parentTypes);
	}
}

// Adds compatible types according to the type hierarchy
void Task::addCompatibleTypes(TaskType* t, std::vector<unsigned int>& parentTypes)
{
	for (unsigned int parentOldIndex : parentTypes) {
		int parentNewIndex = this->newTypeIndex[parentOldIndex];
		if (parentNewIndex != -1) {
			t->addCompatibleType(&this->types[parentNewIndex]);
			Type* parentType = &task->task->types[parentOldIndex];
			addCompatibleTypes(t, parentType->parentTypes);
		}
	}
}

// Gets the predicates defined in the domain
void Task::processPredicates()
{
	for (Function& f : task->task->functions) {
		if (f.valueTypes.size() == 1 && f.valueTypes[0] == task->task->BOOLEAN_TYPE && !f.parameters.empty()) {
			//cout << f.name << endl;
			vector<TaskType*> args;
			generatePredicate(&f, &args);
		}
	}
	/*
	for (TaskPredicate& p : this->predicates) {
		cout << p.toString() << endl;
	}*/
}

// Generates a new predicate
void Task::generatePredicate(Function* f, vector<TaskType*>* args)
{
	if (f->parameters.size() == args->size()) {
		int index = (int)this->predicates.size();
		this->predicates.emplace_back(index, f->name, args);
		oldPredicateIndex.push_back(f->index);
	}
	else {
		int argNumber = (int)args->size();
		Variable& arg = f->parameters[argNumber];
		for (unsigned int oldTypeIndex : arg.types) {
			int newIndex = this->newTypeIndex[oldTypeIndex];
			if (newIndex != -1) {
				TaskType& t = this->types[newIndex];
				vector<TaskType*> argsExt = *args;
				argsExt.push_back(&t);
				generatePredicate(f, &argsExt);
				for (TaskType& subtype : this->types) {
					if (subtype.index != t.index && subtype.compatible(&t)) {
						vector<TaskType*> argsExt = *args;
						argsExt.push_back(&subtype);
						generatePredicate(f, &argsExt);
					}
				}
			}
		}
	}
}

// Gets the operators defined in the domain 
void Task::processOperators()
{
	for (Operator& o : task->operators) {
		if (!o.isGoal && o.name.at(0) != '#') {
			vector<TaskType*> parameters;
			generateOperator(&o, &parameters);
		}
	}
}

// Generates a new operator
void Task::generateOperator(Operator* o, std::vector<TaskType*>* parameters)
{
	if (o->parameters.size() == parameters->size()) {
		int index = (int)this->operators.size();
		this->operators.emplace_back(index, o->name, parameters);
		computeEffects(operators.back(), o);
	}
	else {
		int paramNumber = (int)parameters->size();
		Variable& arg = o->parameters[paramNumber];
		for (unsigned int oldTypeIndex : arg.types) {
			int newIndex = this->newTypeIndex[oldTypeIndex];
			if (newIndex != -1) {
				TaskType& t = this->types[newIndex];
				vector<TaskType*> paramsExt = *parameters;
				paramsExt.push_back(&t);
				generateOperator(o, &paramsExt);
				for (TaskType& subtype : this->types) {
					if (subtype.index != t.index && subtype.compatible(&t)) {
						vector<TaskType*> paramsExt = *parameters;
						paramsExt.push_back(&subtype);
						generateOperator(o, &paramsExt);
					}
				}
			}
		}
	}
}

// Gets the effects of an operator
void Task::computeEffects(TaskOperator& to, Operator* o)
{
	if (!o->isGoal && o->name.at(0) != '#') {
		//cout << to.toString() << endl;
		for (OpFluent& prec : o->atStart.prec) {
			generateEffect(&to, &prec, o, true);
		}
		for (OpFluent& prec : o->overAllPrec) {
			generateEffect(&to, &prec, o, true);
		}
		for (OpFluent& prec : o->atEnd.prec) {
			generateEffect(&to, &prec, o, true);
		}
		for (OpFluent& eff : o->atStart.eff) {
			generateEffect(&to, &eff, o, false);
		}
		for (OpFluent& eff : o->atEnd.eff) {
			generateEffect(&to, &eff, o, false);
		}
	}
}

// Creates a new operator effect
void Task::generateEffect(TaskOperator* to, OpFluent* eff, Operator* o, bool isPrecondition)
{
	if (eff->value.type == task->task->BOOLEAN_TYPE) {
		bool addEffect = eff->value.index == task->task->CONSTANT_TRUE;
		// cout << "* " << eff->toString(task->task->functions, task->task->objects) << endl;
		vector<int> parameterIndex;
		TaskPredicate* pred = matchEffect(&eff->variable, &to->parameters, o, parameterIndex);
		if (pred != NULL) {
			// cout << ", matches with: " << pred->toString() << endl;
			if (isPrecondition) to->addPrecondition(pred, parameterIndex);
			else if (addEffect) to->addAddEffect(pred, parameterIndex);
			else to->addDelEffect(pred, parameterIndex);
		}
	}
}

// Checks if an operator effect matches the given literal
TaskPredicate* Task::matchEffect(Literal* l, std::vector<TaskType*>* opParams, Operator* o, std::vector<int>& parameterIndex)
{
	for (TaskPredicate& pred : this->predicates) {
		if (this->oldPredicateIndex[pred.index] == l->fncIndex) {
			bool matchParams = true;
			vector<unsigned int>* argTypes;
			int paramNumber = 0;
			parameterIndex.clear();
			for (Term& term : l->params) {
				switch (term.type) {
				case TERM_CONSTANT:
					argTypes = &task->task->objects[term.index].types;
					parameterIndex.push_back(-1);
					break;
				case TERM_PARAMETER:
					argTypes = &o->parameters[term.index].types;
					parameterIndex.push_back(term.index);
					break;
				default:
					argTypes = NULL;
				}
				if (argTypes == NULL) {
					matchParams = false;
					break;
				}
				else {
					bool matchArg = false;
					for (unsigned int argType : *argTypes) {
						int taskTypeIndex = this->newTypeIndex[argType];
						if (taskTypeIndex != -1 && pred.arguments[paramNumber]->index == taskTypeIndex) {
							matchArg = true;
							break;
						}
					}
					if (!matchArg) {
						matchParams = false;
						break;
					}
					paramNumber++;
				}
			}
			if (matchParams) return &pred;
		}
	}
	return NULL;
}

// Gets the objects from the problem
void Task::processObjects()
{
	for (Object& o : task->task->objects) {
		if (o.index != task->task->CONSTANT_TRUE && o.index != task->task->CONSTANT_FALSE && o.name.at(0) != '#') {
			int typeIndex = newTypeIndex[o.types[0]];
			if (typeIndex != -1) {
				newObjectIndex.push_back((int)objects.size());
				TaskType* t = &types[typeIndex];
				objects.emplace_back(o.name, t);
			}
			else newObjectIndex.push_back(-1);
		}
		else newObjectIndex.push_back(-1);
	}
}

// Gets the initial state from the problem
void Task::processInitialState()
{
	for (Fact& f : task->task->init) {
		if (!f.valueIsNumeric) {
			TaskPredicate* predicate = findPredicate(&f);
			if (predicate != NULL) {
				vector<TaskObject*> params;
				for (unsigned int obj : f.parameters) {
					int newObj = newObjectIndex[obj];
					if (newObj != -1) {
						params.push_back(&objects[newObj]);
					}
					else {
						predicate = NULL;
						break;
					}
				}
				if (predicate != NULL) state.emplace_back(predicate, params);
			}
		}
	}
}

// Gets the goals from the problem
void Task::processGoal()
{
	for (Operator& o : task->operators) {
		if (o.isGoal) {
			for (OpFluent& prec : o.atStart.prec) {
				if (prec.value.type == task->task->BOOLEAN_TYPE) {
					TaskPredicate* predicate = findPredicate(&prec.variable);
					if (predicate != NULL) {
						vector<TaskObject*> params;
						for (Term& term : prec.variable.params) {
							int newObj = newObjectIndex[term.index];
							if (newObj != -1) {
								params.push_back(&objects[newObj]);
							}
							else {
								predicate = NULL;
								break;
							}
						}
						if (predicate != NULL) goal.emplace_back(predicate, params);
					}
				}
			}
			return;
		}
	}
}

// Searches the corresponding predicate of a given initial-state fact
TaskPredicate* Task::findPredicate(Fact* f)
{
	//cout << f->toString(task->task->functions, task->task->objects);
	for (TaskPredicate& pred : predicates) {
		if (pred.name == task->task->functions[f->function].name && pred.arguments.size() == f->parameters.size()) {
			bool match = true;
			for (int arg = 0; arg < (int)pred.arguments.size(); arg++) {
				TaskObject& obj = objects[newObjectIndex[f->parameters[arg]]];
				if (pred.arguments[arg] != obj.type) {
					match = false;
					break;
				}
			}
			if (match) {
				//cout << " matches " << pred.toString() << endl;
				return &pred;
			}
		}
	}
	//cout << endl;
	return NULL;
}

// Searches for the predicate of a given literal
TaskPredicate* Task::findPredicate(Literal* l)
{
	for (TaskPredicate& pred : predicates) {
		if (pred.name == task->task->functions[l->fncIndex].name && pred.arguments.size() == l->params.size()) {
			bool match = true;
			for (int arg = 0; arg < (int)pred.arguments.size(); arg++) {
				TaskObject& obj = objects[newObjectIndex[l->params[arg].index]];
				if (pred.arguments[arg] != obj.type) {
					match = false;
					break;
				}
			}
			if (match) {
				//cout << " matches " << pred.toString() << endl;
				return &pred;
			}
		}
	}
	//cout << endl;
	return NULL;
}

// Creates the task
Task::Task(PreprocessedTask* pTask)
{
	this->task = pTask;
	processTypes();
	processPredicates();
	processOperators();
	processObjects();
	processInitialState();
	processGoal();
}

/********************************************************/
/* CLASS: TaskType (Object type)                        */
/********************************************************/

// Creates a new type
TaskType::TaskType(int index, Type& t)
{
	this->index = index;
	this->name = t.name;
}

// Adds a compatible type to a given type
void TaskType::addCompatibleType(TaskType* pt)
{
	for (TaskType* ct : compatibleTypes) {
		if (ct->index == pt->index) return;
	}
	compatibleTypes.push_back(pt);
}

// Checks if the current type is compatible with the given one
bool TaskType::compatible(TaskType* t)
{
	for (TaskType* ct : compatibleTypes)
		if (ct->index == t->index)
			return true;
	return false;
}

/********************************************************/
/* CLASS: TaskPredicate (domain predicate)              */
/********************************************************/

// New predicate
TaskPredicate::TaskPredicate(int index, std::string name, std::vector<TaskType*>* args)
{
	this->index = index;
	this->name = name;
	for (TaskType* t : *args) this->arguments.push_back(t);
}

// String representation of a predicate
std::string TaskPredicate::toString()
{
	string s = name + "(";
	int i = 0;
	for (TaskType* arg : arguments) {
		if (i++ > 0) s += ", ";
		s += arg->name;
	}
	return s + ")";
}

/********************************************************/
/* CLASS: TaskOperator (Planning operator)              */
/********************************************************/

// New operator
TaskOperator::TaskOperator(int index, std::string name, std::vector<TaskType*>* params)
{
	this->index = index;
	this->name = name;
	for (TaskType* t : *params) this->parameters.push_back(t);
}

// String representation of an operator
std::string TaskOperator::toString()
{
	string s = name + "(";
	int i = 0;
	for (TaskType* param : parameters) {
		if (i++ > 0) s += ", ";
		s += param->name;
	}
	return s + ")";

}

// Add a precondition to an operator
void TaskOperator::addPrecondition(TaskPredicate* pred, std::vector<int>& parameterIndex)
{
	for (TaskEffect& e : prec)
		if (e.predicate->index == pred->index) return;
	prec.emplace_back(pred, parameterIndex);
}

// Adds a positive effect to an operator
void TaskOperator::addAddEffect(TaskPredicate* pred, std::vector<int>& parameterIndex)
{
	for (TaskEffect& e : add)
		if (e.predicate->index == pred->index) return;
	add.emplace_back(pred, parameterIndex);
}

// Adds a negative effect to an operator
void TaskOperator::addDelEffect(TaskPredicate* pred, std::vector<int>& parameterIndex)
{
	for (TaskEffect& e : del)
		if (e.predicate->index == pred->index) return;
	del.emplace_back(pred, parameterIndex);
}

/********************************************************/
/* CLASS: TaskEffect (Operator effect or precondition)  */
/********************************************************/

// New operator effect
TaskEffect::TaskEffect(TaskPredicate* pred, std::vector<int>& paramIndex)
{
	this->predicate = pred;
	for (int index : paramIndex)
		this->paramIndex.push_back(index);
}

// String representation of an operator effect
std::string TaskEffect::toString()
{
	string s = predicate->name + "(";
	for (int i = 0; i < (int)paramIndex.size(); i++) {
		if (i > 0) s += ", ";
		s += predicate->arguments[i]->name + "[" + std::to_string(paramIndex[i]) + "]";
	}
	return s + ")";
}

/********************************************************/
/* CLASS: TaskLiteral (Initial-state or goal literal)   */
/********************************************************/

// New literal
TaskLiteral::TaskLiteral(TaskPredicate* predicate, std::vector<TaskObject*>& args)
{
	this->predicate = predicate;
	for (TaskObject* o : args) this->arguments.push_back(o);
}

// Checks if a literal contains an object as argument
bool TaskLiteral::contains(TaskObject* obj)
{
	for (TaskObject* a : arguments)
		if (a == obj) return true;
	return false;
}

// Finds the index of a given object in the list of arguments of the literal
int TaskLiteral::find(TaskObject* obj)
{
	int arg = 0;
	for (TaskObject* a : arguments) {
		if (a == obj) return arg;
		arg++;
	}
	return -1;
}

// String representation of a literal
std::string TaskLiteral::toString()
{
	string s = this->predicate->name + "(";
	for (int i = 0; i < (int)this->arguments.size(); i++) {
		s += this->arguments[i]->name;
		if (i < (int)this->arguments.size() - 1) s += ", ";
	}
	return s + ")";
}

// Checks if two literals are equal
bool TaskLiteral::equals(TaskLiteral* l)
{
	if (predicate != l->predicate) return false;
	for (int i = 0; i < (int)arguments.size(); i++) {
		if (arguments[i] != l->arguments[i])
			return false;
	}
	return true;
}

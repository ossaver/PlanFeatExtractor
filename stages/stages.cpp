#include "stages.h"
#include <iostream>

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Functions for calculating features and stages.	    */
/********************************************************/

using namespace std;

// Computes the set of features for each type
void Stages::calculateFeatures()
{
	for (TaskType& t : task->types) {
		// cout << t.name << endl;
		this->featuresOfType.emplace_back(&t);
		FeaturesOfType& tf = this->featuresOfType.back();
		for (TaskPredicate& pred : task->predicates) {
			int argNumber = 0;
			for (TaskType* arg : pred.arguments) {
				if (arg->index == t.index) {
					tf.addFeature(&pred, argNumber);
				}
				argNumber++;
			}
		}
	}
	//for (FeaturesOfType& tf : this->featuresOfType) cout << tf.toString() << endl;
}

// Computes the set of transition rules for each type
void Stages::calculateTransitionRules()
{
	for (TaskOperator& o : task->operators) {
		//cout << endl << o.toString() << endl;
		for (int paramNumber = 0; paramNumber < o.parameters.size(); paramNumber++) {
			calculateOperatorTransitionRule(&o, paramNumber);
		}
	}
	// for (FeaturesOfType& tf : this->featuresOfType) cout << tf.toStringTransitionRules() << endl;
}

// Computes the set of transition rules from a fiven operator
void Stages::calculateOperatorTransitionRule(TaskOperator* o, int paramNumber)
{
	TaskType* v = o->parameters[paramNumber];
	//cout << "* Operator param: " << v->name << endl;
	for (FeaturesOfType& ft : this->featuresOfType) {
		if (ft.getType()->index == v->index) {
			std::vector<Feature*> enabler;
			std::vector<Feature*> left;
			std::vector<Feature*> right;
			for (TaskEffect& prec : o->prec)
				addFeatureToTransitionRule(&prec, ft, paramNumber, enabler);
			for (TaskEffect& eff : o->del)
				addFeatureToTransitionRule(&eff, ft, paramNumber, left);
			for (TaskEffect& eff : o->add)
				addFeatureToTransitionRule(&eff, ft, paramNumber, right);
			if (!left.empty() || !right.empty()) {
				ft.addTransitionRule(enabler, left, right);
				/*
				cout << "* " << ft.getType()->name << ":";
				for (Feature* f : left) { cout << " " << f->toString(); }
				cout << " =>";
				for (Feature* f : right) { cout << " " << f->toString(); }
				cout << endl;*/
			}
		}
	}
}

// Adds a feature to a transition rule
void Stages::addFeatureToTransitionRule(TaskEffect* eff, FeaturesOfType& ft, int paramNumber, std::vector<Feature*>& rule)
{
	//cout << "Eff: " << eff->predicate->toString() << endl;
	for (int i = 0; i < ft.numFeatures(); i++) {
		Feature* f = ft.getFeature(i);
		if (f->getPredicate()->index == eff->predicate->index) {
			int argNumber = -1;
			for (int i = 0; i < eff->paramIndex.size(); i++) {
				if (eff->paramIndex[i] == paramNumber) {
					argNumber = i;
					break;
				}
			}
			if (argNumber >= 0 && argNumber < f->numArguments() && f->getArgument(argNumber) == ft.getType()) {
				//cout << eff->toString() << " -> " << f->toString() << " (arg: " << argNumber << ")" << endl;
				rule.push_back(f);
			}
		}
	}
}

// Classifies each feature (static, attribute, permanent, multiple, reversible, transient)
void Stages::classifyFeatures()
{
	for (FeaturesOfType& ft : this->featuresOfType) {
		//cout << "\nTYPE: " << ft.getType()->name << endl;
		int numFeatures = ft.numFeatures();
		for (int i = 0; i < numFeatures; i++) {
			Feature* f = ft.getFeature(i);
			if (!ft.findInTransitionRules(f)) {
				f->setType(FT_UNUSED);
				//cout << "UNUSED: " << f->toString() << endl;
			}
			else {
				vector<Feature*> adj;
				ft.getOutAdjacents(f, adj);
				ft.getInAdjacents(f, adj);
				if (adj.empty()) {
					f->setType(FT_STATIC);
					//cout << "STATIC: " << f->toString() << endl;
				}
				else if (adj.size() == 1 && adj[0] == NULL) {
					f->setType(FT_ATTRIBUTE);
					//cout << "ATTRIBUTE: " << f->toString() << endl;
				}
				else if (adj.size() == 1 && adj[0] == f) {
					f->setType(FT_PERMANENT);
					//cout << "PERMANENT: " << f->toString() << endl;
				}
				else if (ft.existsPath(NULL, f)) {
					f->setType(FT_MULTIPLE);
					//cout << "MULTIPLE: " << f->toString() << endl;
				}
				else if (ft.existsPath(f, f)) {
					f->setType(FT_REVERSIBLE);
					//cout << "REVERSIBLE: " << f->toString() << endl;
				}
				else {
					f->setType(FT_TRANSIENT);
					//cout << "TRANSIENT: " << f->toString() << endl;
				}
			}
		}
	}
}

// Computes mutex features
void Stages::computeMutex()
{
	for (FeaturesOfType& ft : this->featuresOfType) {
		ft.startCheckMutex();
		//cout << "\nTYPE: " << ft.getType()->name << endl;
		int numFeatures = ft.numFeatures();
		for (int i = 0; i < numFeatures; i++) {
			Feature* f1 = ft.getFeature(i);
			if (f1->getType() == FT_REVERSIBLE || f1->getType() == FT_TRANSIENT) {
				for (int j = i + 1; j < numFeatures; j++) {
					Feature* f2 = ft.getFeature(j);
					if (f2->getType() == FT_REVERSIBLE || f2->getType() == FT_TRANSIENT) {
						ft.checkMutex(i, j);
					}
				}
			}
		}
	}
	/*
	for (FeaturesOfType& ft : this->featuresOfType) {
		cout << ft.toStringMutex() << endl;
	}*/
}

// Computes the basic stages
void Stages::computeBasicStages()
{
	for (FeaturesOfType& ft : this->featuresOfType) {
		//cout << "TYPE: " << ft.getType()->name << endl;
		int numFeatures = ft.numFeatures();
		std::vector<Feature*> stage;
		for (int i = 0; i < numFeatures; i++) {
			Feature* f = ft.getFeature(i);
			if (f->getType() == FT_PERMANENT) stage.push_back(f);
		}
		addReversibleFeaturesToBasicStage(&ft, &stage);
	}
}

// Add reversible features to basic stages
void Stages::addReversibleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage)
{
	vector<Feature*> reversible;
	for (int i = 0; i < ft->numFeatures(); i++)
		if (ft->getFeature(i)->getType() == FT_REVERSIBLE)
			reversible.push_back(ft->getFeature(i));
	addReversibleFeaturesToBasicStage(ft, stage, &reversible);
}

// Add a reversible feature to a basic stage if it is not mutex
void Stages::addReversibleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage,
	std::vector<Feature*>* reversible)
{
	if (reversible->empty()) {  // Continue with transient features
		vector<Feature*> transient;
		for (int i = 0; i < ft->numFeatures(); i++)
			if (ft->getFeature(i)->getType() == FT_TRANSIENT)
				transient.push_back(ft->getFeature(i));
		addTransientFeaturesToBasicStage(ft, stage, &transient);
	}
	else {
		for (int i = 0; i < reversible->size(); i++) {
			Feature* current = reversible->at(i);
			bool repeatedOrMutex = false;
			for (Feature* f : *stage) {
				if (f == current || ft->areMutex(f, current)) {
					repeatedOrMutex = true;
					break;
				}
			}
			vector<Feature*> copy = *reversible;
			copy.erase(copy.begin() + i); 
			if (!repeatedOrMutex) stage->push_back(current);
			addReversibleFeaturesToBasicStage(ft, stage, &copy);
			if (!repeatedOrMutex) stage->pop_back();
		}
	}
}

// Add a transient feature to a basic stage if it is not mutex
void Stages::addTransientFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage,
	std::vector<Feature*>* transient)
{
	if (transient->empty()) {  // Continue with multiple features
		addMultipleFeaturesToBasicStage(ft, stage);
	}
	else {
		for (int i = 0; i < transient->size(); i++) {
			Feature* current = transient->at(i);
			bool repeatedOrMutex = false;
			for (Feature* f : *stage) {
				if (f == current || ft->areMutex(f, current)) {
					repeatedOrMutex = true;
					break;
				}
			}
			vector<Feature*> copy = *transient;
			copy.erase(copy.begin() + i);
			addTransientFeaturesToBasicStage(ft, stage, &copy); // Not adding this transient feature
			if (!repeatedOrMutex) {
				stage->push_back(current);
				addTransientFeaturesToBasicStage(ft, stage, &copy); // Not adding this transient feature
				stage->pop_back();
			}
		}
	}
}

// Add multiple features to basic stages
void Stages::addMultipleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage)
{
	if (!ft->repeatedBasicStage(stage)) {
		addMultipleFeaturesToBasicStage(ft, stage, 0);
	}
}

// Add a multiple feature to a basic stage
void Stages::addMultipleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage, int index)
{
	while (index < ft->numFeatures() && ft->getFeature(index)->getType() != FT_MULTIPLE) index++;
	if (index < ft->numFeatures()) {
		addMultipleFeaturesToBasicStage(ft, stage, index + 1);
		std::vector<Feature*> copy = *stage;
		copy.push_back(ft->getFeature(index));
		addMultipleFeaturesToBasicStage(ft, &copy, index + 1);
	}
	else if (!ft->repeatedBasicStage(stage)) {
		ft->addBasicStage(stage);
		/*
		cout << "STAGE: ";
		for (int i = 0; i < (int)stage->size(); i++) {
			if (i > 0) cout << ", ";
			cout << stage->at(i)->toString();
		}
		cout << endl;*/
	}
}

// Computes additional stages
void Stages::computeAdditionalStages()
{
	for (FeaturesOfType& ft : this->featuresOfType) {
		std::vector<Feature*> stage;
		addAdditionalStage(&ft, &stage, 0);
	}
}

// Adds an additional stage
void Stages::addAdditionalStage(FeaturesOfType* ft, std::vector<Feature*>* stage, int index)
{
	while (index < ft->numFeatures() && ft->getFeature(index)->getType() != FT_STATIC
		&& ft->getFeature(index)->getType() != FT_ATTRIBUTE) index++;
	if (index < ft->numFeatures()) {
		addAdditionalStage(ft, stage, index + 1);
		std::vector<Feature*> copy = *stage;
		copy.push_back(ft->getFeature(index));
		addAdditionalStage(ft, &copy, index + 1);
	}
	else if (!ft->repeatedAdditionalStage(stage)) {
		ft->addAdditionalStage(stage);
		/*
		cout << "ADD STAGE (" << ft->getType()->name << "): ";
		for (int i = 0; i < (int)stage->size(); i++) {
			if (i > 0) cout << ", ";
			cout << stage->at(i)->toString();
		}
		cout << endl;*/
	}
}

// Computes combined stages
void Stages::computeCombinedStages()
{
	for (FeaturesOfType& ft : this->featuresOfType) {
		int numStages = ft.getNumBasicStages();
		for (int i = 0; i < numStages; i++) {
			vector< vector<Feature*> > setOfCombinedStages;
			vector<Feature*>* stage = ft.getBasicStage(i);
			addCombinedStages(&ft, stage, 'x', &setOfCombinedStages);
			int insertedStages = 0;
			for (vector<Feature*>& cs : setOfCombinedStages) {
				if (!cs.empty() && cs.at(cs.size() - 1)->isCombined()) {
					insertedStages++;
					ft.addCombinedStage(&cs);
					/*
					cout << "COMBINED STAGE (" << ft.getType()->name << "): ";
					for (int i = 0; i < (int)cs.size(); i++) {
						if (i > 0) cout << ", ";
						cout << cs.at(i)->toString();
					}
					cout << endl;
					*/
				}
			}
			if (insertedStages == 0) {
				ft.addCombinedStage(stage);
				/*
				cout << "COMBINED STAGE (" << ft.getType()->name << "): ";
				for (int i = 0; i < (int)stage->size(); i++) {
					if (i > 0) cout << ", ";
					cout << stage->at(i)->toString();
				}
				cout << endl;*/
			}
		}
	}
}

// Adds a combined stage
void Stages::addCombinedStages(FeaturesOfType* ft, std::vector<Feature*>* stage, char letter,
	std::vector< std::vector<Feature*> >* setOfCombinedStages)
{
	for (int i = 0; i < (int)stage->size(); i++) {
		Feature* f = stage->at(i);
		if (f->isCombined()) break;
		int numArgs = f->numArguments();
		for (int j = 0; j < numArgs; j++) {
			if (f->getArgument(j) == NULL) {
				char newLetter = letter + 1;
				if (newLetter > 'z') newLetter = 'a';
				FeaturesOfType* ot = &featuresOfType[typeIndex(f->getPredicate()->arguments[j])];
				Feature* instFeat = f->instance(j, ot->getType(), newLetter, featurePool);
				Feature* eqFeature = ot->findEquivalentFeature(instFeat, j);
				int oNumStages = ot->getNumBasicStages();
				for (int k = 0; k < oNumStages; k++) {
					vector<Feature*>* oStage = ot->getBasicStage(k);
					if (ot->findFeatureInVector(eqFeature, oStage)) {
						vector<Feature*> combinedStage = *stage;
						combinedStage[i] = instFeat;
						for (Feature* cf : *oStage) {
							if (cf != eqFeature) {
								combinedStage.push_back(cf->replaceLetter(newLetter, featurePool));
							}
						}
						addCombinedStages(ft, &combinedStage, newLetter, setOfCombinedStages);
					}
				}
				return;
			}
		}
	}
	// Add combined stage
	setOfCombinedStages->push_back(*stage);
}

// Gets the index of a given type
int Stages::typeIndex(TaskType* t)
{
	for (int i = 0; i < (int)featuresOfType.size(); i++)
		if (featuresOfType[i].getType()->index == t->index) return i;
	return -1;
}

// Gets the combined stage of an object
int Stages::getCombinedStage(TaskObject* obj, FeaturesOfType* ft)
{
	int numStages = ft->getNumCombinedStages();
	for (int i = 0; i < numStages; i++) {
		vector<Feature*>* stage = ft->getCombinedStage(i);
		if (checkCombinedStage(obj, stage))
			return i + 1;
	}
	return 0;
}

// Gets the additional stage of an object
int Stages::getAdditionalStage(TaskObject* obj, FeaturesOfType* ft)
{
	int numStages = ft->getNumAdditionalStages();
	for (int i = numStages - 1; i >= 0; i--) {
		vector<Feature*>* stage = ft->getAdditionalStage(i);
		if (checkAdditionalStage(obj, stage))
			return i + 1;
	}
	return 0;
}

// Checks if an object is in a given combined stage
bool Stages::checkCombinedStage(TaskObject* obj, std::vector<Feature*>* stage)
{
	unordered_map<char, string> mapping;
	mapping['x'] = obj->name;
	int featureNumber = 0;
	return checkCombinedStage(featureNumber, stage, &mapping);
}

// Checks if an object is in a given additional stage
bool Stages::checkAdditionalStage(TaskObject* obj, std::vector<Feature*>* stage)
{
	for (Feature* f : *stage) {
		bool match = false;
		for (TaskLiteral& l : task->state) {
			if (l.predicate->index == f->getPredicate()->index && 
				l.arguments[f->getFirstArgument()] == obj) {
				match = true;
				break;
			}
		}
		if (!match) return false;
	}
	return true;
}

// Checks if a given feature in a combined stage holds
bool Stages::checkCombinedStage(int featureNumber, std::vector<Feature*>* stage, unordered_map<char, string>* mapping)
{
	if (featureNumber >= (int)stage->size()) return true;
	Feature* f = stage->at(featureNumber);
	for (TaskLiteral& l : task->state) {
		if (l.predicate->index == f->getPredicate()->index) {
			bool matching = true;
			vector<char> newLetters;
			for (int argNumber = 0; argNumber < (int)l.arguments.size(); argNumber++) {
				TaskObject* arg = l.arguments[argNumber];
				if (f->getArgument(argNumber) != NULL) {
					char letter = f->getLetter(argNumber);
					unordered_map<char, string>::const_iterator got = mapping->find(letter);
					if (got == mapping->end()) { // New letter: store the matching
						(*mapping)[letter] = arg->name;
						newLetters.push_back(letter);
					}
					else { // Check that arguments match
						if (got->second != arg->name) {
							matching = false;
							for (char newLetter : newLetters) mapping->erase(newLetter);
							break;
						}
					}
				}
			}
			if (matching && checkCombinedStage(featureNumber + 1, stage, mapping))
				return true;
		}
	}
	return false;
}

// Gets the goal stages of a given object
void Stages::getGoalStages(TaskObject* obj, FeaturesOfType* ft, vector<int>& goalStages)
{
	int numStages = ft->getNumCombinedStages();
	for (int i = 0; i < numStages; i++) {
		vector<Feature*>* stage = ft->getCombinedStage(i);
		if (checkGoalStage(obj, stage)) {
			goalStages.push_back(i + 1);
		}
	}
}

// Gets the additional goal stages of a given object
void Stages::getAdditionalGoalStages(TaskObject* obj, FeaturesOfType* ft, std::vector<int>& goalStages)
{
	int numStages = ft->getNumAdditionalStages();
	for (int i = 0; i < numStages; i++) {
		vector<Feature*>* stage = ft->getAdditionalStage(i);
		bool match = true;
		// All static features must be in the intial state
		for (TaskLiteral& l : task->state) {
			int objIndex = l.find(obj);
			if (objIndex != -1) {
				Feature* f = ft->getFeature(l.predicate, objIndex);
				if (f != NULL && f->getType() == FT_STATIC) {
					if (!ft->findFeatureInVector(f, stage)) {
						match = false;
						break;
					}
				}
			}
		}
		if (match) { // All attributes must be in the goal
			for (TaskLiteral& l : task->goal) {
				int objIndex = l.find(obj);
				if (objIndex != -1) {
					Feature* f = ft->getFeature(l.predicate, objIndex);
					if (f != NULL && f->getType() == FT_ATTRIBUTE) {
						if (!ft->findFeatureInVector(f, stage)) {
							match = false;
							break;
						}
					}
				}
			}
			if (match) 
				goalStages.push_back(i + 1);
		}
	}
}

// Checks if an object is in a given goal stage
bool Stages::checkGoalStage(TaskObject* obj, std::vector<Feature*>* stage)
{
	// All literals in the goal containing obj must match with features in the stage
	unordered_map<char, string> mapping;
	mapping['x'] = obj->name;
	return instanceGoalStage(0, obj, stage, &mapping);
}

// Tries to instantiate the arguments of a feature in a goal stage
bool Stages::instanceGoalStage(int featureNumber, TaskObject* obj, std::vector<Feature*>* stage,
	std::unordered_map<char, std::string>* mapping)
{
	//cout << obj->name << endl;
	if (featureNumber >= (int)stage->size()) { // Instantiation done
		return validateGoalStage(stage, mapping);
	}
	int numMatchings = 0;
	Feature* f = stage->at(featureNumber);
	//cout << f->toString() << endl;
	if (hasInstancedParameters(f, mapping)) {
		for (TaskLiteral& l : task->goal) {
			if (l.predicate->index == f->getPredicate()->index) {
				bool matching = true;
				vector<char> newLetters;
				for (int argNumber = 0; argNumber < (int)l.arguments.size(); argNumber++) {
					TaskObject* arg = l.arguments[argNumber];
					if (f->getArgument(argNumber) != NULL) {
						char letter = f->getLetter(argNumber);
						unordered_map<char, string>::const_iterator got = mapping->find(letter);
						if (got == mapping->end()) { // New letter: store the matching
							(*mapping)[letter] = arg->name;
							newLetters.push_back(letter);
						}
						else { // Check that arguments match
							if (got->second != arg->name) {
								matching = false;
								break;
							}
						}
					}
				}
				if (matching) {
					//cout << " * Match: " << l.toString() << endl;
					numMatchings++;
					if (instanceGoalStage(featureNumber + 1, obj, stage, mapping))
						return true;
				}
				for (char newLetter : newLetters) mapping->erase(newLetter);
			}
		}
	}
	if (numMatchings == 0) {
		return instanceGoalStage(featureNumber + 1, obj, stage, mapping);
	}
	return false;
}

// Check if a feature has grounded parameters
bool Stages::hasInstancedParameters(Feature* f, std::unordered_map<char, std::string>* mapping)
{
	for (int i = 0; i < f->numArguments(); i++)
		if (f->getArgument(i) != NULL && mapping->find(f->getLetter(i)) != mapping->end())
			return true;
	return false;
}

// Check if a grounded goal stage holds
bool Stages::validateGoalStage(std::vector<Feature*>* stage, std::unordered_map<char, std::string>* mapping)
{
	for (Feature* f : *stage) {
		if (hasInstancedParameters(f, mapping)) {
			TaskLiteral* match = findInGoal(f, mapping);
			if (match == NULL) {
				// Check if feature is not mutex with goals
				if (mutexWithGoals(f, mapping))
					return false;
			}
		}
	}
	return true;
}

// Searches for a feature in the goal
TaskLiteral* Stages::findInGoal(Feature* f, std::unordered_map<char, std::string>* mapping)
{
	for (TaskLiteral& l : task->goal) {
		if (l.predicate->index == f->getPredicate()->index) {
			bool match = true;
			for (int argNumber = 0; argNumber < (int)l.arguments.size(); argNumber++) {
				if (f->getArgument(argNumber) != NULL) {
					char letter = f->getLetter(argNumber);
					unordered_map<char, string>::const_iterator got = mapping->find(letter);
					if (got != mapping->end()) { // Letter found -> must match literal argument
						if (got->second != l.arguments[argNumber]->name) {
							match = false;
							break;
						}
					}
				}
			}
			if (match) {
				return &l;
			}
		}
	}
	return NULL;
}

// Check if a feature is mutex with the goals
bool Stages::mutexWithGoals(Feature* f, std::unordered_map<char, std::string>* mapping)
{
	for (TaskLiteral& l : task->goal) {
		if (mutexWithGoal(f, &l, mapping))
			return true;
	}
	return false;
}

// Checks if a feature is mutex with a goal literal
bool Stages::mutexWithGoal(Feature* f, TaskLiteral* l, std::unordered_map<char, std::string>* mapping)
{
	for (int argNumber = 0; argNumber < f->numArguments(); argNumber++) {
		if (f->getArgument(argNumber) != NULL) {
			char letter = f->getLetter(argNumber);
			unordered_map<char, string>::const_iterator got = mapping->find(letter);
			if (got != mapping->end()) {
				string objName = got->second;
				for (int literalParam = 0; literalParam < (int)l->arguments.size(); literalParam++) {
					if (l->arguments[literalParam]->name == objName) { // Check mutex
						TaskType* litParamType = l->predicate->arguments[literalParam];
						FeaturesOfType* ft = getFeatureOfType(litParamType);
						vector<Feature*>* mutex = ft->getMutex(l->predicate, literalParam);
						if (mutex != NULL) {
							for (Feature* mutexFeat : *mutex) {
								if (f->getPredicate() == mutexFeat->getPredicate() &&
									mutexFeat->getFirstArgument() == argNumber)
									return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

// Gets the features of a given type
FeaturesOfType* Stages::getFeatureOfType(TaskType* t)
{
	for (FeaturesOfType& ft : featuresOfType)
		if (ft.getType() == t) return &ft;
	return NULL;
}

// Checks if the goal is achieved for a given object
int Stages::goalAchieved(TaskObject* obj)
{
	for (TaskLiteral& goal : task->goal) {
		if (goal.contains(obj)) {
			bool found = false;
			for (TaskLiteral& l : task->state) {
				if (l.equals(&goal)) {
					found = true;
					break;
				}
			}
			if (!found) return 0;
		}
	}
	return 1;
}

// Analyses the task data
Stages::Stages(Task* task)
{
	this->task = task;
	calculateFeatures();
	calculateTransitionRules();
	classifyFeatures();
	computeMutex();
	computeBasicStages();
	computeAdditionalStages();
	computeCombinedStages();
}

// Classifies the objects in the problem
void Stages::classify()
{
	cout << "{" << endl;
	for (int i = 0; i < (int)task->objects.size(); i++) {
		TaskObject* obj = &task->objects[i];
		for (FeaturesOfType& ft : featuresOfType) {
			if (ft.getType() == obj->type) {
				cout << "  \"" << obj->name << "\": {" << endl;
				cout << "    \"type\": \"" << ft.getType()->name << "\"," << endl;
				int stage = getCombinedStage(obj, &ft);
				cout << "    \"stage\": \"CS" << stage << "\"," << endl;
				int addStage = getAdditionalStage(obj, &ft);
				cout << "    \"addStage\": \"AS" << addStage << "\"," << endl;
				vector<int> goalStages;
				getGoalStages(obj, &ft, goalStages);
				cout << "    \"goalStages\": [";
				for (int i = 0; i < (int)goalStages.size(); i++) {
					cout << "\"CS" << goalStages[i] << "\"";
					if (i < (int)goalStages.size() - 1) cout << ", ";
				}
				cout << "]," << endl;
				vector<int> addGoalStages;
				getAdditionalGoalStages(obj, &ft, addGoalStages);
				cout << "    \"addGoalStages\": [";
				for (int i = 0; i < (int)addGoalStages.size(); i++) {
					cout << "\"AS" << addGoalStages[i] << "\"";
					if (i < (int)addGoalStages.size() - 1) cout << ", ";
				}
				cout << "]," << endl;
				cout << "    \"goalAchieved\": " << goalAchieved(obj) << endl;
				cout << "  }";
				break;
			}
		}
		if (i < (int)task->objects.size() - 1) cout << ",";
		cout << endl;
	}
	cout << "}";
}

// Prints the type features
void Stages::toJSON()
{
	cout << "{" << endl;
	int index = 0;
	for (FeaturesOfType& ft : featuresOfType) {
		ft.toJSON();
		if (++index < (int)featuresOfType.size()) cout << ",";
		cout << endl;
	}
	cout << "}";
}

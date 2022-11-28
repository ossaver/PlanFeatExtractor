#include "features.h"
#include <iostream>

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Classes to store type features.              	    */
/********************************************************/

using namespace std;

/********************************************************/
/* CLASS: FeaturesOfType                                */
/********************************************************/

// Adds a new feature (if not repeated)
void FeaturesOfType::addFeatureNoRepeat(Feature* f, std::vector<Feature*>* v)
{
	bool found = false;
	for (Feature* aux : *v) {
		if (aux == f) {
			found = true;
			break;
		}
	}
	if (!found) v->push_back(f);
}

// Checks if there is a path in the transition grapg from the origin to the destination feature
bool FeaturesOfType::existsPath(Feature* current, Feature* orig, Feature* dst, unordered_set<Feature*>* visited, int distance)
{
	vector<Feature*> adj;
	getOutAdjacents(current, adj);
	for (Feature *f: adj) {
		if (f == dst) {
			if (distance > 0) return true;
			else return f != orig;  // Distance 0 means a self-loop
		}
		else if (visited->find(f) == visited->end()) {
			visited->insert(f);
			if (existsPath(f, orig, dst, visited, distance + 1))
				return true;
		}
	}
	return false;
}

// Checks if from a node w with exit AND edges, i.e., {w,...}->{w1,w2,...}, there is a path w->w1->...->u
// and a path w->w2->...->v, and hese paths only have node w in common
bool FeaturesOfType::foundDivergentOutPaths(std::vector<Feature*>* pathToU, Feature* u, Feature* v, std::vector<Feature*>* wNext)
{
	if (pathToU->back() == u) {
		// cout << "   * PATH:";
		// for (Feature* p : *pathToU) cout << " (" << p->toString() << ")";
		// cout << endl;
		vector<Feature*> pathToV;
		pathToV.push_back(pathToU->at(0));
		for (Feature* next : *wNext) {
			if (!findFeatureInVector(next, pathToU)) {
				pathToV.push_back(next);
				if (foundDivergentOutPath(&pathToV, pathToU, v))
					return true;
				pathToV.pop_back();
			}
		}
		return false;
	}
	else {
		vector<Feature*> adj;
		getOutAdjacents(pathToU->back(), adj);
		for (Feature* a : adj) {
			if (!findFeatureInVector(a, pathToU)) {
				pathToU->push_back(a);
				if (foundDivergentOutPaths(pathToU, u, v, wNext))
					return true;
				pathToU->pop_back();
			}
		}
		return false;
	}
}

// Checks if from a node w with exit AND edges, i.e., {w,...}->{w1,w2,...}, there is a path w->w1->...->u
// and a path w->w2->...->v, and hese paths only have node w in common
bool FeaturesOfType::foundDivergentOutPath(std::vector<Feature*>* pathToV, std::vector<Feature*>* pathToU, Feature* v)
{
	if (pathToV->back() == v) {
		/*
		cout << "   * PATH 2:";
		for (Feature* p : *pathToV) cout << " (" << p->toString() << ")";
		cout << endl;*/
		return true;
	}
	else {
		vector<Feature*> adj;
		getOutAdjacents(pathToV->back(), adj);
		for (Feature* a : adj) {
			if (!findFeatureInVector(a, pathToU) && !findFeatureInVector(a, pathToV)) {
				pathToV->push_back(a);
				if (foundDivergentOutPath(pathToV, pathToU, v))
					return true;
				pathToV->pop_back();
			}
		}
		return false;
	}
}

// Checks if to a node w with input AND edges, i.e., {w1,w2,...}->{w,...}, there is a path u->...->w1->w and 
// a path v->...->w2->w, and these paths only have node w in common
bool FeaturesOfType::divergentInPaths(Feature* w, Feature* u, Feature* v, std::vector<Feature*>* wPrev)
{
	//cout << "Checking divergent path to " << w->toString() << ":" << endl;
	vector<Feature*> pathFromU;
	pathFromU.push_back(w);
	for (Feature* w1 : *wPrev) {
		pathFromU.push_back(w1);
		if (foundDivergentInPaths(&pathFromU, u, v, wPrev))
			return true;
		pathFromU.pop_back();
	}
	return false;

}

// Checks if to a node w with input AND edges, i.e., {w1,w2,...}->{w,...}, there is a path u->...->w1->w and 
// a path v->...->w2->w, and these paths only have node w in common
bool FeaturesOfType::foundDivergentInPaths(std::vector<Feature*>* pathFromU, Feature* u, Feature* v, std::vector<Feature*>* wPrev)
{
	if (pathFromU->back() == u) {
		// cout << "   * REVERSE PATH:";
		// for (Feature* p : *pathFromU) cout << " (" << p->toString() << ")";
		// cout << endl;
		vector<Feature*> pathFromV;
		pathFromV.push_back(pathFromU->at(0));
		for (Feature* prev : *wPrev) {
			if (!findFeatureInVector(prev, pathFromU)) {
				pathFromV.push_back(prev);
				if (foundDivergentInPath(&pathFromV, pathFromU, v))
					return true;
				pathFromV.pop_back();
			}
		}
		return false;
	}
	else {
		vector<Feature*> adj;
		getInAdjacents(pathFromU->back(), adj);
		for (Feature* a : adj) {
			if (!findFeatureInVector(a, pathFromU)) {
				pathFromU->push_back(a);
				if (foundDivergentInPaths(pathFromU, u, v, wPrev))
					return true;
				pathFromU->pop_back();
			}
		}
		return false;
	}
}

// Checks if to a node w with input AND edges, i.e., {w1,w2,...}->{w,...}, there is a path u->...->w1->w and 
// a path v->...->w2->w, and these paths only have node w in common
bool FeaturesOfType::foundDivergentInPath(std::vector<Feature*>* pathFromV, std::vector<Feature*>* pathFromU, Feature* v)
{
	if (pathFromV->back() == v) {
		/*
		cout << "   * REVERSE PATH 2:";
		for (Feature* p : *pathFromV) cout << " (" << p->toString() << ")";
		cout << endl;*/
		return true;
	}
	else {
		vector<Feature*> adj;
		getInAdjacents(pathFromV->back(), adj);
		for (Feature* a : adj) {
			if (!findFeatureInVector(a, pathFromU) && !findFeatureInVector(a, pathFromV)) {
				pathFromV->push_back(a);
				if (foundDivergentInPath(pathFromV, pathFromU, v))
					return true;
				pathFromV->pop_back();
			}
		}
		return false;
	}
}

// Searches for a feature in a vector
bool FeaturesOfType::findFeatureInVector(Feature* f, std::vector<Feature*>* v)
{
	for (Feature* a : *v)
		if (a == f) return true;
	return false;
}

// Returns the mutex features for a given predicate with an instanced argument
std::vector<Feature*>* FeaturesOfType::getMutex(TaskPredicate* pred, int argNumber)
{
	for (int i = 0; i < (int)features.size(); i++) {
		Feature& f = features[i];
		if (f.getPredicate() == pred && f.getFirstArgument() == argNumber)
			return &mutex[i];
	}
	return NULL;
}

// Adds mutex features
void FeaturesOfType::addMutex(int numFeature1, int numFeature2)
{
	if (!findFeatureInVector(&features[numFeature2], &mutex[numFeature1]))
		mutex[numFeature1].push_back(&features[numFeature2]);
	if (!findFeatureInVector(&features[numFeature1], &mutex[numFeature2]))
		mutex[numFeature2].push_back(&features[numFeature1]);
}

// Prints information about the given stages
void FeaturesOfType::toJSONStages(std::string prefix, std::vector<std::vector<Feature*>>& stages)
{
	for (int i = 0; i < (int)stages.size(); i++) {
		cout << "      \"" << prefix << (i + 1) << "\": [";
		for (int j = 0; j < (int)stages[i].size(); j++) {
			cout << "\"" << stages[i].at(j)->toString() << "\"";
			if (j < (int)stages[i].size() - 1) cout << ", ";
		}
		cout << "]";
		if (i < (int)stages.size() - 1) cout << ",";
		cout << endl;
	}
}

// New object to store the features of a type
FeaturesOfType::FeaturesOfType(TaskType* t)
{
	this->type = t;
}

// Adds a new feature
void FeaturesOfType::addFeature(TaskPredicate* pred, int argNumber)
{
	features.emplace_back(pred, argNumber, this->type);
}

// Adds a new transition rule
void FeaturesOfType::addTransitionRule(std::vector<Feature*>& enabler, std::vector<Feature*>& left, std::vector<Feature*>& right)
{
	for (TransitionRule& tr : transitionRules)
		if (tr.compare(&enabler, &left, &right)) return;
	transitionRules.emplace_back(&enabler, &left, &right);
}

// Gets the features with an edge to the given one in the transition graph 
void FeaturesOfType::getInAdjacents(Feature* f, std::vector<Feature*>& adj)
{
	for (TransitionRule& tr : transitionRules) {
		if (tr.findInRight(f) != -1) {
			if (tr.left.empty()) addFeatureNoRepeat(NULL, &adj);
			else {
				for (Feature* af : tr.left)
					addFeatureNoRepeat(af, &adj);
			}
		}
	}
}

// Searches a feature in the transition rules 
bool FeaturesOfType::findInTransitionRules(Feature* f)
{
	for (TransitionRule& tr : transitionRules) {
		if (tr.findInEnabler(f) != -1 || tr.findInLeft(f) != -1 || tr.findInRight(f) != -1)
			return true;
	}
	return false;
}

// Checks if there is a path from the origin to the destination feature in the transition graph 
bool FeaturesOfType::existsPath(Feature* orig, Feature* dst)
{
	unordered_set<Feature*> visited;
	visited.insert(orig);
	return existsPath(orig, orig, dst, &visited, 0);
}

// The process for checking mutex is started -> reserve memory
void FeaturesOfType::startCheckMutex()
{
	mutex.resize(features.size());
}

// Checks if two features are mutex
void FeaturesOfType::checkMutex(int numFeature1, int numFeature2)
{
	Feature* u = &features[numFeature1];
	Feature* v = &features[numFeature2];
	if (existsPath(u, v) || existsPath(v, u)) {
		bool mutex = true;
		// cout << u->toString() << " and " << v->toString() << " can be mutex" << endl;
		for (Feature& w : features) {
			if (&w != u && &w != v) {
				for (TransitionRule& tr : transitionRules) {
					if (tr.right.size() > 1 && tr.findInLeft(&w) != -1) {
						if (divergentOutPaths(&w, u, v, &tr.right)) {
							mutex = false;
							break;
						}
					}
					if (tr.left.size() > 1 && tr.findInRight(&w) != -1) {
						if (divergentInPaths(&w, u, v, &tr.left)) {
							mutex = false;
							break;
						}
					}
				}
				if (!mutex) break;
			}
		}
		if (mutex) {
			addMutex(numFeature1, numFeature2);
		}
	}
}

// Checks if two features are mutex
bool FeaturesOfType::areMutex(Feature* f1, Feature* f2)
{
	int index = 0;
	for (Feature& f : features) {
		if (&f == f1) {
			return findFeatureInVector(f2, &mutex[index]);
		}
		else if (&f == f2) {
			return findFeatureInVector(f1, &mutex[index]);
		}
		index++;
	}
	return false;
}

// Checks if a basic stage is repeated
bool FeaturesOfType::repeatedBasicStage(std::vector<Feature*>* stage)
{
	bool repeated = false;
	for (std::vector<Feature*>& s : basicStages) {
		if (s.size() == stage->size()) {
			repeated = true;
			for (Feature* f : s) {
				if (!findFeatureInVector(f, stage)) {
					repeated = false;
					break;
				}
			}
			if (repeated) break;
		}
	}
	return repeated;
}

// Adds a new basic stage
void FeaturesOfType::addBasicStage(std::vector<Feature*>* stage)
{
	basicStages.emplace_back();
	std::vector<Feature*>& newStage = basicStages.back();
	for (Feature* f : *stage) newStage.push_back(f);
}

// Checks if an additional stage is repeated
bool FeaturesOfType::repeatedAdditionalStage(std::vector<Feature*>* stage)
{
	if (stage->empty()) {
		for (std::vector<Feature*>& s : additionalStages)
			if (s.empty()) return true;
	}
	return false;
}

// Adds a new additional stage
void FeaturesOfType::addAdditionalStage(std::vector<Feature*>* stage)
{
	additionalStages.emplace_back();
	std::vector<Feature*>& newStage = additionalStages.back();
	for (Feature* f : *stage) newStage.push_back(f);
}

// Adds a new combined stage
void FeaturesOfType::addCombinedStage(std::vector<Feature*>* stage)
{
	combinedStages.emplace_back();
	std::vector<Feature*>& newStage = combinedStages.back();
	for (Feature* f : *stage) newStage.push_back(f);
}

// Gets the feature that matches a partially instanced literal
Feature* FeaturesOfType::getFeature(TaskPredicate* predicate, int argNumber)
{
	for (Feature& f : features)
		if (f.getPredicate() == predicate && f.getFirstArgument() == argNumber)
			return &f;
	return NULL;
}

// Gets the features with an edge from the given one in the transition graph 
void FeaturesOfType::getOutAdjacents(Feature* f, std::vector<Feature*>& adj)
{
	for (TransitionRule& tr : transitionRules) {
		if (tr.findInLeft(f) != -1) {
			if (tr.right.empty()) addFeatureNoRepeat(NULL, &adj);
			else {
				for (Feature* af : tr.right)
					addFeatureNoRepeat(af, &adj);
			}
		}
	}
}

// Finds the equivalent original feature 
Feature* FeaturesOfType::findEquivalentFeature(Feature* f, int arg)
{
	for (Feature& aux : features) {
		if (aux.getPredicate()->index == f->getPredicate()->index &&
			aux.getFirstArgument() == arg)
			return &aux;
	}
	return NULL;
}

// String representation of the type features
string FeaturesOfType::toStringFeatures()
{
	string res = "Features of type " + type->name + "\n";
	for (Feature& f : features) {
		res += f.toString() + "\n";
	}
	return res;
}

// String representation of the transition rules 
std::string FeaturesOfType::toStringTransitionRules()
{
	string res = "Transition rules of type " + type->name + "\n";
	for (TransitionRule& tr : transitionRules) {
		res += tr.toString() + "\n";
	}
	return res;
}

// String representation of the feature mutex
std::string FeaturesOfType::toStringMutex()
{
	string s = "";
	for (int i = 0; i < numFeatures(); i++) {
		for (Feature* f : mutex[i]) {
			s += "MUTEX: " + features[i].toString() + " <-> " + f->toString() + "\n";
		}
	}
	return s;
}

// Prints information about features, transition rules and stages
void FeaturesOfType::toJSON()
{
	cout << "  \"" << type->name << "\": {" << endl;
	cout << "    \"features\": {" << endl;
	int i = 0;
	for (Feature& f : features) {
		if (f.getType() != FT_UNUSED) {
			cout << "      \"" << f.toString() << "\": \"" << f.getTypeName() << "\"";
			for (int j = ++i; j < (int)features.size(); j++)
				if (features[j].getType() != FT_UNUSED) { cout << ","; break; }
			cout << endl;
		}
		else i++;
	}
	cout << "    }," << endl;
	cout << "    \"transitionRules\": [" << endl;
	i = 0;
	for (TransitionRule& tr : transitionRules) {
		cout << "      \"" << tr.toString() << "\"";
		if (++i < (int)transitionRules.size()) cout << ",";
		cout << endl;
	}
	cout << "    ]," << endl;
	cout << "    \"mutex\": {" << endl;
	i = 0;
	for (Feature& f : features) {
		if (f.getType() != FT_UNUSED && !mutex[i].empty()) {
			cout << "      \"" << f.toString() << "\": [";
			for (int j = 0; j < (int)mutex[i].size(); j++) {
				cout << "\"" << mutex[i].at(j)->toString() << "\"";
				if (j < (int)mutex[i].size() - 1) cout << ",";
			}
			cout << "]";
			for (int j = ++i; j < (int)features.size(); j++)
				if (features[j].getType() != FT_UNUSED) { cout << ","; break; }
			cout << endl;
		}
		else i++;
	}
	cout << "    }," << endl;
	cout << "    \"basicStages\": {" << endl;
	toJSONStages("BS", basicStages);
	cout << "    }," << endl;
	cout << "    \"additionalStages\": {" << endl;
	toJSONStages("AS", additionalStages);
	cout << "    }," << endl;
	cout << "    \"combinedStages\": {" << endl;
	toJSONStages("CS", combinedStages);
	cout << "    }" << endl;
	cout << "  }";
}

// Checks if from a node w with exit AND edges, i.e., {w,...}->{w1,w2,...}, there is a path w->w1->...->u
// and a path w->w2->...->v, and hese paths only have node w in common
bool FeaturesOfType::divergentOutPaths(Feature* w, Feature* u, Feature* v, std::vector<Feature*>* wNext)
{
	//cout << "Checking divergent path from " << w->toString() << ":" << endl;
	vector<Feature*> pathToU;
	pathToU.push_back(w);
	for (Feature* w1 : *wNext) {
		pathToU.push_back(w1);
		if (foundDivergentOutPaths(&pathToU, u, v, wNext))
			return true;
		pathToU.pop_back();
	}
	return false;
}

/********************************************************/
/* CLASS: Feature (feature of type)                     */
/********************************************************/

// New feature
Feature::Feature(TaskPredicate* pred, int argNumber, TaskType* t)
{
	this->predicate = pred;
	this->firstArgument = argNumber;
	arguments.resize(pred->arguments.size(), NULL);
	letter.resize(pred->arguments.size(), NULL);
	arguments[argNumber] = t;
	letter[argNumber] = 'x';
	type = FT_UNUSED;
	combined = false;
}

// Grounds an argument of the feature
Feature* Feature::instance(int arg, TaskType* t, char newLetter, std::vector<Feature*>& pool)
{
	Feature* f = new Feature(predicate, firstArgument, arguments[firstArgument]);
	pool.push_back(f);
	f->type = type;
	for (int i = 0; i < (int)arguments.size(); i++) {
		f->arguments[i] = arguments[i];
		f->letter[i] = letter[i];
	}
	f->arguments[arg] = t;
	f->letter[arg] = newLetter;
	return f;
}

// Changes the letter assigned to a grounded argument
Feature* Feature::replaceLetter(char newLetter, std::vector<Feature*>& pool)
{
	Feature* f = new Feature(predicate, firstArgument, arguments[firstArgument]);
	pool.push_back(f);
	f->type = type;
	for (int i = 0; i < (int)arguments.size(); i++) {
		f->arguments[i] = arguments[i];
		f->letter[i] = letter[i];
	}
	f->letter[firstArgument] = newLetter;
	f->combined = true; 
	return f;
}

// Returns the feature type
std::string Feature::getTypeName()
{
	switch (type)
	{
	case FT_STATIC:
		return "static";
	case FT_ATTRIBUTE:
		return "attribute";
	case FT_PERMANENT:
		return "permanent";
	case FT_MULTIPLE:
		return "multiple";
	case FT_TRANSIENT:
		return "transient";
	case FT_REVERSIBLE:
		return "reversible";
	default:
		return "unused";
	}
}

// String representation of the feature
string Feature::toString()
{
	string s = predicate->name;
	s += "(";
	for (int i = 0; i < arguments.size(); i++) {
		if (i > 0) s += ", ";
		if (arguments[i] == NULL) s += "* - " + predicate->arguments[i]->name;
		else {
			s += letter[i];
			s += " - " + arguments[i]->name;
		}
	}
	return s + ")";
}

/********************************************************/
/* CLASS: TransitionRule (Transition rule)              */
/********************************************************/

// Compares two vectors of features
bool TransitionRule::compareFeatureVector(std::vector<Feature*>* v1, std::vector<Feature*>* v2)
{
	if (v1->size() != v2->size()) return false;
	for (Feature* f1 : *v1) {
		bool found = false;
		for (Feature* f2 : *v2) {
			if (f1 == f2) {
				found = true;
				break;
			}
		}
		if (!found) return false;
	}
	return true;
}

// New transition rule
TransitionRule::TransitionRule(std::vector<Feature*>* enabler, std::vector<Feature*>* left, std::vector<Feature*>* right)
{
	for (Feature* f : *enabler) this->enabler.push_back(f);
	for (Feature* f : *left) this->left.push_back(f);
	for (Feature* f : *right) this->right.push_back(f);
}

// Compares two transition rules
bool TransitionRule::compare(std::vector<Feature*>* enabler, std::vector<Feature*>* left, std::vector<Feature*>* right)
{
	return compareFeatureVector(&this->left, left) && compareFeatureVector(&this->right, right)
		&& compareFeatureVector(&this->enabler, enabler);
}

// Finds a feature in the left part of the rule
int TransitionRule::findInLeft(Feature* f)
{
	for (int i = 0; i < (int)left.size(); i++)
		if (left[i] == f) return i;
	return -1;
}

// Finds a feature in the right part of the rule
int TransitionRule::findInRight(Feature* f)
{
	for (int i = 0; i < (int)right.size(); i++)
		if (right[i] == f) return i;
	return -1;
}

// Finds a feature in the enablers
int TransitionRule::findInEnabler(Feature* f)
{
	for (int i = 0; i < (int)enabler.size(); i++)
		if (enabler[i] == f) return i;
	return -1;
}

// String representation of a transition rule
string TransitionRule::toString()
{
	string res = "";
	if (enabler.empty()) res += "null";
	else {
		for (int i = 0; i < (int)enabler.size(); i++) {
			if (i > 0) res += ", ";
			res += enabler[i]->toString();
		}
	}
	res += ": ";
	if (left.empty()) res += "null";
	else {
		for (int i = 0; i < (int)left.size(); i++) {
			if (i > 0) res += ", ";
			res += left[i]->toString();
		}
	}
	res += " -> ";
	if (right.empty()) res += "null";
	else {
		for (int i = 0; i < (int)right.size(); i++) {
			if (i > 0) res += ", ";
			res += right[i]->toString();
		}
	}
	return res;
}

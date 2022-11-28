#ifndef FEATURES_H
#define FEATURES_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Classes to store type features.              	    */
/********************************************************/

#include "task.h"
#include <unordered_set>

enum FeatureType {
	FT_UNUSED = 0, FT_STATIC = 1, FT_ATTRIBUTE = 2, 
	FT_PERMANENT = 3, FT_MULTIPLE = 4, FT_TRANSIENT = 5, FT_REVERSIBLE = 6
};

class Feature {
private:
	TaskPredicate* predicate;
	std::vector<TaskType*> arguments;
	std::vector<char> letter;
	FeatureType type;
	int firstArgument;
	bool combined;

public:
	Feature(TaskPredicate* pred, int argNumber, TaskType* t);
	inline TaskPredicate* getPredicate() { return predicate; }
	inline int numArguments() { return (int)arguments.size(); }
	inline TaskType* getArgument(int index) { return arguments[index]; }
	std::string toString();
	void setType(FeatureType t) { type = t; }
	inline FeatureType getType() { return type; }
	Feature* instance(int arg, TaskType* t, char newLetter, std::vector<Feature*>& pool);
	inline int getFirstArgument() { return firstArgument; }
	Feature* replaceLetter(char newLetter, std::vector<Feature*>& pool);
	inline bool isCombined() { return combined; }
	inline char getLetter(int index) { return letter[index]; }
	std::string getTypeName();
};

class TransitionRule {
private:
	bool compareFeatureVector(std::vector<Feature*>* v1, std::vector<Feature*>* v2);

public:
	std::vector<Feature*> enabler;
	std::vector<Feature*> left;
	std::vector<Feature*> right;

	TransitionRule(std::vector<Feature*>* enabler, std::vector<Feature*>* left, std::vector<Feature*>* right);
	bool compare(std::vector<Feature*>* enabler, std::vector<Feature*>* left, std::vector<Feature*>* right);
	int findInLeft(Feature* f);
	int findInRight(Feature* f);
	int findInEnabler(Feature* f);
	std::string toString();
};

class FeaturesOfType {
private:
	TaskType* type;
	std::vector<Feature> features;
	std::vector<TransitionRule> transitionRules;
	std::vector< std::vector<Feature*> > mutex;
	std::vector< std::vector<Feature*> > basicStages;
	std::vector< std::vector<Feature*> > additionalStages;
	std::vector< std::vector<Feature*> > combinedStages;

	void addFeatureNoRepeat(Feature* f, std::vector<Feature*>* v);
	bool existsPath(Feature* current, Feature* orig, Feature* dst, std::unordered_set<Feature*>* visited, int distance);
	bool divergentOutPaths(Feature* w, Feature* u, Feature* v, std::vector<Feature*>* wNext);
	bool foundDivergentOutPaths(std::vector<Feature*>* pathToU, Feature* u, Feature* v, std::vector<Feature*>* wNext);
	bool foundDivergentOutPath(std::vector<Feature*>* pathToV, std::vector<Feature*>* pathToU, Feature* v);
	bool divergentInPaths(Feature* w, Feature* u, Feature* v, std::vector<Feature*>* wPrev);
	bool foundDivergentInPaths(std::vector<Feature*>* pathFromU, Feature* u, Feature* v, std::vector<Feature*>* wPrev);
	bool foundDivergentInPath(std::vector<Feature*>* pathFromV, std::vector<Feature*>* pathFromU, Feature* v);
	void addMutex(int numFeature1, int numFeature2);
	void toJSONStages(std::string prefix, std::vector< std::vector<Feature*> >& stages);

public:
	FeaturesOfType(TaskType* t);
	void addFeature(TaskPredicate* pred, int argNumber);
	void addTransitionRule(std::vector<Feature*>& enabler, std::vector<Feature*>& left, std::vector<Feature*>& right);
	inline TaskType* getType() { return type; }
	inline int numFeatures() { return (int)features.size(); }
	inline Feature* getFeature(int index) { return &features[index]; }
	Feature* getFeature(TaskPredicate* predicate, int argNumber);
	void getOutAdjacents(Feature* f, std::vector<Feature*>& adj);
	void getInAdjacents(Feature* f, std::vector<Feature*>& adj);
	bool findInTransitionRules(Feature* f);
	bool existsPath(Feature* orig, Feature* dst);
	void startCheckMutex();
	void checkMutex(int numFeature1, int numFeature2);
	bool areMutex(Feature* f1, Feature* f2);
	bool repeatedBasicStage(std::vector<Feature*>* stage);
	void addBasicStage(std::vector<Feature*>* stage);
	bool repeatedAdditionalStage(std::vector<Feature*>* stage);
	void addAdditionalStage(std::vector<Feature*>* stage);
	void addCombinedStage(std::vector<Feature*>* stage);
	inline int getNumBasicStages() { return (int)basicStages.size(); }
	inline int getNumAdditionalStages() { return (int)additionalStages.size(); }
	inline int getNumCombinedStages() { return (int)combinedStages.size(); }
	inline std::vector<Feature*>* getBasicStage(int index) { return &basicStages[index]; }
	inline std::vector<Feature*>* getAdditionalStage(int index) { return &additionalStages[index]; }
	inline std::vector<Feature*>* getCombinedStage(int index) { return &combinedStages[index]; }
	Feature* findEquivalentFeature(Feature* f, int arg);
	bool findFeatureInVector(Feature* f, std::vector<Feature*>* v);
	std::vector<Feature*>* getMutex(TaskPredicate* pred, int argNumber);
	std::string toStringFeatures();
	std::string toStringTransitionRules();
	std::string toStringMutex();
	void toJSON();
};

#endif
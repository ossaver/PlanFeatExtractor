#ifndef STAGES_H
#define STAGES_H

/********************************************************/
/* Oscar Sapena Vercher - DSIC - UPV                    */
/* November 2022                                        */
/********************************************************/
/* Functions for calculating features and stages.	    */
/********************************************************/

#include "task.h"
#include "features.h"

class Stages {
private:
	Task* task;
	std::vector<FeaturesOfType> featuresOfType;
	std::vector<Feature*> featurePool;

	void calculateFeatures();
	void calculateTransitionRules();
	void calculateOperatorTransitionRule(TaskOperator* o, int paramNumber);
	void addFeatureToTransitionRule(TaskEffect* eff, FeaturesOfType& ft, int paramNumber, std::vector<Feature*>& rule);
	void classifyFeatures();
	void computeMutex();
	void computeBasicStages();
	void addReversibleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage);
	void addReversibleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage,
		std::vector<Feature*>* reversible);
	void addTransientFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage,
		std::vector<Feature*>* transient);
	void addMultipleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage);
	void addMultipleFeaturesToBasicStage(FeaturesOfType* ft, std::vector<Feature*>* stage, int index);
	void computeAdditionalStages();
	void addAdditionalStage(FeaturesOfType* ft, std::vector<Feature*>* stage, int index);
	void computeCombinedStages();
	void addCombinedStages(FeaturesOfType* ft, std::vector<Feature*>* stage, char letter,
		std::vector< std::vector<Feature*> >* setOfCombinedStages);
	int typeIndex(TaskType* t);
	int getCombinedStage(TaskObject* obj, FeaturesOfType* ft);
	int getAdditionalStage(TaskObject* obj, FeaturesOfType* ft);
	bool checkCombinedStage(TaskObject* obj, std::vector<Feature*>* stage);
	bool checkAdditionalStage(TaskObject* obj, std::vector<Feature*>* stage);
	bool checkCombinedStage(int featureNumber, std::vector<Feature*>* stage, std::unordered_map<char, std::string>* mapping);
	void getGoalStages(TaskObject* obj, FeaturesOfType* ft, std::vector<int>& goalStages);
	void getAdditionalGoalStages(TaskObject* obj, FeaturesOfType* ft, std::vector<int>& goalStages);
	bool checkGoalStage(TaskObject* obj, std::vector<Feature*>* stage);
	bool instanceGoalStage(int featureNumber, TaskObject* obj, std::vector<Feature*>* stage, 
		std::unordered_map<char, std::string>* mapping);
	bool hasInstancedParameters(Feature* f, std::unordered_map<char, std::string>* mapping);
	bool validateGoalStage(std::vector<Feature*>* stage, std::unordered_map<char, std::string>* mapping);
	TaskLiteral* findInGoal(Feature* f, std::unordered_map<char, std::string>* mapping);
	bool mutexWithGoals(Feature* f, std::unordered_map<char, std::string>* mapping);
	bool mutexWithGoal(Feature* f, TaskLiteral* l, std::unordered_map<char, std::string>* mapping);
	FeaturesOfType* getFeatureOfType(TaskType* t);
	int goalAchieved(TaskObject* obj);

public:
	Stages(Task* task);
	void classify();
	void toJSON();
};

#endif // !#ifndef STAGES_H

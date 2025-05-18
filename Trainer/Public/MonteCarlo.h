#pragma once
#include "Trainer.h"
#include <mutex>

struct treeNode 
{
	int Visits;
	double TotalScore;
	std::vector<treeNode*> Children;
	treeNode* Parent;
	int PreviousMove = -1;
	std::mutex ExpansionMutex;
	std::mutex ValueChangeMute;
	std::mutex ChildrenMutex;
	treeNode() : Visits(0), TotalScore(0.0), Parent(nullptr), Children() {}
	~treeNode() 
	{
		for (auto& child : Children)
		{
			delete child;
		}
	}
};

class MonteCarlo
{
public:
	struct EvaluationAndMove
	{
		int Move;
		float stateEvaluation;
	};

	static int MonteCarloTreeSearch(IGame& initialState, float seconds, NeuralNetwork* ai);
	static EvaluationAndMove MonteCarloTreeSearch(IGame& initialState, int iterations, NeuralNetwork* ai);
private:
	static int SelectBestAction(treeNode& root, IGame& initialState);

	static void RunMCTSLoop(IGame* initialState, int iterations, treeNode* root, NeuralNetwork* ai, int rootPlayer);
	static void RunMCTSLoop(IGame* initialState, std::chrono::high_resolution_clock::time_point startTime, 
		std::chrono::duration<double> timeRestriction, treeNode* root, NeuralNetwork* ai, int rootPlayer);
	static void PerformMCTSTurn(IGame& initialState, treeNode* rootNode, NeuralNetwork* ai, int rootPlayer);
	static treeNode* SelectNodeUCB(treeNode* parent, bool isFirst);
	static bool ExpandNode(IGame& board, treeNode* parent);
};
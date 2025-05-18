#include "MonteCarlo.h"
#include <iostream>
#include <thread>


void MonteCarlo::RunMCTSLoop(IGame* initialState, std::chrono::high_resolution_clock::time_point startTime, std::chrono::duration<double> timeRestriction, treeNode* root, NeuralNetwork* ai, int rootPlayer)
{
	while (std::chrono::high_resolution_clock::now() - startTime < timeRestriction) 
	{
		PerformMCTSTurn(*initialState, root, ai, rootPlayer);
	}
	return;
}

void MonteCarlo::PerformMCTSTurn(IGame& initialState, treeNode* rootNode, NeuralNetwork* ai, int rootPlayer)
{
	treeNode* node = rootNode;
	while (!node->Children.empty()) 
	{
		node = SelectNodeUCB(node, initialState.GetCurrentPlayer() == 1);
		if (!initialState.MakeMove(node->PreviousMove))
		{
			std::cout << "Impossible move attempted!\n";
			initialState.PrintBoard();
			std::cout << "Attempted move: " << node->PreviousMove << "\n";
		}
	}

	if (!ExpandNode(initialState, node))
	{
		while (node->Parent != nullptr) 
		{
			node = node->Parent;
			initialState.UnMakeMove();
		}
		return;
	}

	float score = 0;
	if (initialState.GetWinner() == IGame::Winner::FirstPlayer)
	{
		score = 1;
	}
	else if (initialState.GetWinner() == IGame::Winner::SecondPlayer)
	{
		score = -1;
	}
	else if (initialState.GetWinner() == IGame::Winner::Draw)
	{
		score = 0;
	}
	else
	{
		score = ai->GetClampedEvaluation(initialState.GetBoardState());
	}

	//initialState.PrintBoard();
	//std::cout << "Evaluation: " << score << "\n";

	/*if (initialState.GetCurrentPlayer() != rootPlayer) 
	{
		score = -score;
	}*/

	while (node->Parent != nullptr) 
	{
		node->ValueChangeMute.lock();
		node->Visits++;
		node->TotalScore += score;
		node->ValueChangeMute.unlock();
		node = node->Parent;
		initialState.UnMakeMove();
		//score *= 0.75f;
	}
	node->ValueChangeMute.lock();
	node->Visits++;
	node->TotalScore += score;
	node->ValueChangeMute.unlock();
}

void MonteCarlo::RunMCTSLoop(IGame* initialState, int iterations, treeNode* root, NeuralNetwork* ai, int rootPlayer)
{
	while (root->Visits < iterations)
	{
		PerformMCTSTurn(*initialState, root, ai, rootPlayer);
	}
	return;
}

int MonteCarlo::MonteCarloTreeSearch(IGame& initialState, float seconds, NeuralNetwork* ai)
{
	treeNode* rootNode = new treeNode();
	rootNode->Visits = 1;
	ExpandNode(initialState, rootNode);

	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> timeRestrictionInSeconds = std::chrono::duration<double>(seconds);

	std::vector<std::thread> threads;
	std::vector<std::unique_ptr<IGame>> boards;

	int rootPlayer = initialState.GetCurrentPlayer();

	for (int i = 0; i < 1; ++i) 
	{
		auto boardCopy = initialState.Clone();
		threads.emplace_back([boardCopy = std::move(boardCopy), startTime, timeRestrictionInSeconds, rootNode, ai, rootPlayer]() mutable 
		{
			RunMCTSLoop(boardCopy.get(), startTime, timeRestrictionInSeconds, rootNode, ai, rootPlayer);
		});
	}
	for (auto& thread : threads) 
	{
		thread.join();
	}
	int bestAction = SelectBestAction(*rootNode, initialState);

	delete rootNode;
	
	return bestAction;
}

MonteCarlo::EvaluationAndMove MonteCarlo::MonteCarloTreeSearch(IGame& initialState, int iterations, NeuralNetwork* ai)
{
	treeNode* rootNode = new treeNode();
	rootNode->Visits = 1;
	ExpandNode(initialState, rootNode);

	int rootPlayer = initialState.GetCurrentPlayer();

	if (iterations < 500) {
		auto boardCopy = initialState.Clone();
		RunMCTSLoop(boardCopy.get(), iterations, rootNode, ai, rootPlayer);
	}
	else {
		unsigned int threadCount = std::thread::hardware_concurrency();
		if (threadCount == 0) threadCount = 4;

		std::vector<std::thread> threads;
		for (unsigned int i = 0; i < threadCount; ++i)
		{
			auto boardCopy = initialState.Clone();
			threads.emplace_back([boardCopy = std::move(boardCopy), iterations, rootNode, ai, rootPlayer]() mutable
			{
				RunMCTSLoop(boardCopy.get(), iterations, rootNode, ai, rootPlayer);
			});
		}

		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	int bestAction = SelectBestAction(*rootNode, initialState);

	float evaluation = static_cast<float>(rootNode->TotalScore == 0.0f ? 0.0f : rootNode->TotalScore/ static_cast<double>(rootNode->Visits));
	delete rootNode;

	return { bestAction, evaluation };
}


int MonteCarlo::SelectBestAction(treeNode& root, IGame& initialState)
{
	treeNode* bestChild = nullptr;
	double bestScore;
	if (initialState.GetCurrentPlayer() == 1)
	{
		bestScore = INT_MIN;
	}
	else
	{
		bestScore = INT_MAX;
	}
	for (treeNode* child : root.Children)
	{
		double score = child->TotalScore / child->Visits;
		if (initialState.GetCurrentPlayer() == 1)
		{
			if (score > bestScore)
			{
				bestScore = score;
				bestChild = child;
			}
		}
		else
		{
			if (score < bestScore)
			{
				bestScore = score;
				bestChild = child;
			}
		}
	}
	int bestMove = bestChild->PreviousMove;
	return bestMove;
}

treeNode* MonteCarlo::SelectNodeUCB(treeNode* parent, bool isFirst)
{
	double explorationParameter = 1.41f;
	treeNode* bestChild = nullptr;
	bool allScoresZero = true;
	double bestUCT = INT_MIN;

	parent->ChildrenMutex.lock();
	for (treeNode* child : parent->Children) 
	{
		if (child->Visits == 0) 
		{
			allScoresZero = false;
			bestChild = child;
			break;
		}

		double score = (isFirst ? child->TotalScore : -child->TotalScore);
		double uct = score / child->Visits + explorationParameter * sqrt(log(parent->Visits) / child->Visits);

		if (uct > bestUCT) 
		{
			bestUCT = uct;
			bestChild = child;
		}
	}
	if (allScoresZero) 
	{
		int minVisits = INT_MAX;
		for (treeNode* child : parent->Children) 
		{
			if (child->Visits < minVisits) 
			{
				minVisits = child->Visits;
				bestChild = child;
			}
		}
	}
	parent->ChildrenMutex.unlock();
	return bestChild;
}

bool MonteCarlo::ExpandNode(IGame& board, treeNode* parent)
{
	parent->ExpansionMutex.lock();
	parent->ChildrenMutex.lock();
	if (!parent->Children.empty()) 
	{
		parent->ExpansionMutex.unlock();
		parent->ChildrenMutex.unlock();
		return false;
	}
	if (board.GetWinner() != IGame::Winner::OnGoing) 
	{
		parent->ExpansionMutex.unlock();
		parent->ChildrenMutex.unlock();
		return true;
	}

	std::vector<int> allMoves = board.GetValidMoves();
	for (auto& move : allMoves) 
	{
		treeNode* newNode = new treeNode();
		newNode->PreviousMove = move;
		newNode->Parent = parent;
		parent->Children.push_back(newNode);
	}
	parent->ExpansionMutex.unlock();
	parent->ChildrenMutex.unlock();
	return true;
}


#pragma once
#include <sstream>
#include <vector>

class NeuralNetwork {
public:
    static int NextId;
    int Id;

    NeuralNetwork(int inputSize, const std::vector<int>& hiddenLayers);

    void SetKnownEvaluationBounds(float minEval, float maxEval);
    float GetClampedEvaluation(const std::vector<float>& input) const;
    float Evaluate(const std::vector<float>& input) const;
    NeuralNetwork Mutate(int weightRate, int biasRate) const;
    void GradientDescent(const std::vector<float>& input, float target, float learningRate);
    void TrainSingle(const std::vector<float>& input, float target, float learningRate);


    NeuralNetwork CloneWithNewId() const;
    void Save(std::string gameName) const;
    static NeuralNetwork Load(const std::string& filename);

private:
    float m_minEvalKnown = -1.0f;
    float m_maxEvalKnown = 1.0f;

    std::vector<std::vector<float>> m_weights;
    std::vector<std::vector<float>> m_biases;

    inline float Sigmoid(float x) const;
    std::vector<float> FeedForward(const std::vector<float>& input) const;
};
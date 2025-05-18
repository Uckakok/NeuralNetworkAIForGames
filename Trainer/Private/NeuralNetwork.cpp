#include "NeuralNetwork.h"
#include <random>
#include <cmath>
#include <fstream>

NeuralNetwork::NeuralNetwork(int inputSize, const std::vector<int>& hiddenLayers) : Id(NextId++) 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    int prevSize = inputSize;
    for (int layerSize : hiddenLayers) 
    {
        m_weights.emplace_back(prevSize * layerSize);
        m_biases.emplace_back(layerSize);
        for (auto& w : m_weights.back()) w = dist(gen);
        for (auto& b : m_biases.back()) b = dist(gen);
        prevSize = layerSize;
    }

    m_weights.emplace_back(prevSize);
    m_biases.emplace_back(1);
    for (auto& w : m_weights.back())
    {
        w = dist(gen);
    }
    m_biases.back()[0] = dist(gen);
}

float NeuralNetwork::Sigmoid(float x) const 
{
    x = (x > 60.0f ? 60.0f : (x < -60.0f ? 60.0f : x));
    return  1.0f / (1.0f + std::exp(-x));
}


std::vector<float> NeuralNetwork::FeedForward(const std::vector<float>& input) const 
{
    std::vector<float> layer = input;
    int wIdx = 0;

    for (size_t l = 0; l < m_weights.size() - 1; ++l) 
    {
        int inputSize = static_cast<int>(layer.size());
        int outputSize = static_cast<int>(m_biases[l].size());
        std::vector<float> next(outputSize);

        for (int j = 0; j < outputSize; ++j) 
        {
            float sum = m_biases[l][j];
            int rowOffset = j * inputSize;
            for (int i = 0; i < inputSize; ++i) 
            {
                sum += m_weights[l][rowOffset + i] * layer[i];
            }
            next[j] = Sigmoid(sum);
        }

        layer = next;
    }

    float final = m_biases.back()[0];
    for (int i = 0; i < layer.size(); ++i) 
    {
        final += m_weights.back()[i] * layer[i];
    }

    return { Sigmoid(final) };
}

void NeuralNetwork::SetKnownEvaluationBounds(float minEval, float maxEval)
{
    m_minEvalKnown = minEval;
    m_maxEvalKnown = maxEval;
    m_clampedEvaluationPossible = true;
}

float NeuralNetwork::GetClampedEvaluation(const std::vector<float>& input) const
{
    float rawEval = Evaluate(input);
    if (m_maxEvalKnown == m_minEvalKnown)
    {
        return 0.0f;
    }
    float normalized = 2.0f * (rawEval - m_minEvalKnown) / (m_maxEvalKnown - m_minEvalKnown) - 1.0f;
    return std::max(-1.0f, std::min(1.0f, normalized));
}

float NeuralNetwork::Evaluate(const std::vector<float>& input) const 
{
    return FeedForward(input)[0];
}

NeuralNetwork NeuralNetwork::Mutate(int weightRate, int biasRate) const 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noiseDist(0.0f, 1.0f);

    NeuralNetwork copy = *this;

    std::vector<float*> allWeights;
    for (auto& layer : copy.m_weights) 
    {
        for (auto& w : layer)
        {
            allWeights.push_back(&w);
        }
    }

    std::shuffle(allWeights.begin(), allWeights.end(), gen);
    int weightMutations = std::min(weightRate, static_cast<int>(allWeights.size()));
    for (int i = 0; i < weightMutations; ++i) 
    {
        *allWeights[i] += noiseDist(gen);
    }

    std::vector<float*> allBiases;
    for (auto& layer : copy.m_biases) 
    {
        for (auto& b : layer) 
        {
            allBiases.push_back(&b);
        }
    }

    std::shuffle(allBiases.begin(), allBiases.end(), gen);
    int biasMutations = std::min(biasRate, static_cast<int>(allBiases.size()));
    for (int i = 0; i < biasMutations; ++i) 
    {
        *allBiases[i] += noiseDist(gen);
    }

    copy.m_clampedEvaluationPossible = false;

    return copy;
}



NeuralNetwork NeuralNetwork::CloneWithNewId() const 
{
    NeuralNetwork clone = *this;
    clone.Id = NextId++;
    return clone;
}

void NeuralNetwork::Save(std::string gameName) const
{
    std::ofstream out(gameName + std::to_string(Id) + ".nn");
    if (!out)
    {
        throw std::runtime_error("Failed to open file for saving.");
    }

    out << Id << '\n';
    out << m_weights.size() << '\n';

    for (size_t i = 0; i < m_weights.size(); ++i)
    {
        out << m_weights[i].size() << '\n';
        for (float w : m_weights[i])
        {
            out << w << ' ';
        }
        out << '\n';

        out << m_biases[i].size() << '\n';
        for (float b : m_biases[i])
        {
            out << b << ' ';
        }
        out << '\n';
    }

    out << m_minEvalKnown << ' ' << m_maxEvalKnown << '\n';
    out << m_clampedEvaluationPossible << '\n';
}

NeuralNetwork NeuralNetwork::Load(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in)
    {
        throw std::runtime_error("Failed to open file for loading: " + filename);
    }

    NeuralNetwork nn(0, {});
    in >> nn.Id;

    size_t numLayers;
    in >> numLayers;
    nn.m_weights.resize(numLayers);
    nn.m_biases.resize(numLayers);

    for (size_t i = 0; i < numLayers; ++i)
    {
        size_t wSize;
        in >> wSize;
        nn.m_weights[i].resize(wSize);
        for (size_t j = 0; j < wSize; ++j)
        {
            in >> nn.m_weights[i][j];
        }

        size_t bSize;
        in >> bSize;
        nn.m_biases[i].resize(bSize);
        for (size_t j = 0; j < bSize; ++j)
        {
            in >> nn.m_biases[i][j];
        }
    }

    in >> nn.m_minEvalKnown >> nn.m_maxEvalKnown;
    in >> nn.m_clampedEvaluationPossible;

    return nn;
}

void NeuralNetwork::GradientDescent(const std::vector<float>& input, float target, float learningRate) 
{
    std::vector<std::vector<float>> activations = { input };
    std::vector<std::vector<float>> zVectors;

    std::vector<float> layer = input;
    for (size_t l = 0; l < m_weights.size() - 1; ++l) 
    {
        int inputSize = static_cast<int>(layer.size());
        int outputSize = static_cast<int>(m_biases[l].size());
        std::vector<float> z(outputSize);
        std::vector<float> next(outputSize);

        for (int j = 0; j < outputSize; ++j) 
        {
            float sum = m_biases[l][j];
            for (int i = 0; i < inputSize; ++i) 
            {
                sum += m_weights[l][j * inputSize + i] * layer[i];
            }
            z[j] = sum;
            next[j] = Sigmoid(sum);
        }

        zVectors.push_back(z);
        activations.push_back(next);
        layer = next;
    }

    float finalZ = m_biases.back()[0];
    for (size_t i = 0; i < layer.size(); ++i)
    {
        finalZ += m_weights.back()[i] * layer[i];
    }

    float output = finalZ;
    float error = output - target;

    for (size_t i = 0; i < m_weights.back().size(); ++i) 
    {
        m_weights.back()[i] -= learningRate * error * layer[i];
    }
    m_biases.back()[0] -= learningRate * error;

    std::vector<float> deltaNext = { error };
    for (int l = (int)m_weights.size() - 2; l >= 0; --l) 
    {
        std::vector<float> delta(m_biases[l].size(), 0.0f);
        int inputSize = static_cast<int>(activations[l].size());

        for (int j = 0; j < m_biases[l].size(); ++j) 
        {
            float sigmoidDeriv = activations[l + 1][j] * (1 - activations[l + 1][j]);
            float errorSum = 0.0f;
            for (int k = 0; k < deltaNext.size(); ++k) 
            {
                errorSum += m_weights[l + 1][k * m_biases[l].size() + j] * deltaNext[k];
            }
            delta[j] = errorSum * sigmoidDeriv;

            for (int i = 0; i < inputSize; ++i) 
            {
                m_weights[l][j * inputSize + i] -= learningRate * delta[j] * activations[l][i];
            }
            m_biases[l][j] -= learningRate * delta[j];
        }
        deltaNext = delta;
    }
}

void NeuralNetwork::TrainSingle(const std::vector<float>& input, float target, float learningRate)
{
    std::vector<std::vector<float>> activations; 
    std::vector<std::vector<float>> zs;          

    std::vector<float> layer = input;
    activations.push_back(layer);

    for (size_t l = 0; l < m_weights.size() - 1; ++l) 
    {
        int inputSize = static_cast<int>(layer.size());
        int outputSize = static_cast<int>(m_biases[l].size());
        std::vector<float> z(outputSize);
        std::vector<float> activation(outputSize);

        for (int j = 0; j < outputSize; ++j) 
        {
            float sum = m_biases[l][j];
            int rowOffset = j * inputSize;
            for (int i = 0; i < inputSize; ++i) 
            {
                sum += m_weights[l][rowOffset + i] * layer[i];
            }
            z[j] = sum;
            activation[j] = Sigmoid(sum);
        }

        zs.push_back(z);
        activations.push_back(activation);
        layer = activation;
    }

    float finalSum = m_biases.back()[0];
    for (int i = 0; i < layer.size(); ++i) 
    {
        finalSum += m_weights.back()[i] * layer[i];
    }
    float output = finalSum;
    float error = output - target;
    error = std::max(-1000.0f, std::min(1000.0f, error));

    std::vector<float> dEdw(m_weights.back().size());
    for (int i = 0; i < m_weights.back().size(); ++i) 
    {
        dEdw[i] = error * activations.back()[i]; 
        m_weights.back()[i] -= learningRate * dEdw[i];
    }

    m_biases.back()[0] -= learningRate * error;

    std::vector<float> deltaNext(m_weights.back().size());
    for (int i = 0; i < m_weights.back().size(); ++i) 
    {
        deltaNext[i] = error * m_weights.back()[i];
    }

    for (int l = static_cast<int>(m_weights.size()) - 2; l != (size_t)-1; --l) 
    {
        const auto& z = zs[l];
        const auto& aPrev = activations[l];
        auto& w = m_weights[l];
        auto& b = m_biases[l];

        int outputSize = static_cast<int>(b.size());
        int inputSize = static_cast<int>(aPrev.size());

        std::vector<float> delta(outputSize);
        for (int j = 0; j < outputSize; ++j) 
        {
            float sigmoidDeriv = Sigmoid(z[j]) * (1 - Sigmoid(z[j]));
            delta[j] = deltaNext[j] * sigmoidDeriv;

            for (int i = 0; i < inputSize; ++i) 
            {
                w[j * inputSize + i] -= learningRate * delta[j] * aPrev[i];
            }

            b[j] -= learningRate * delta[j];
        }

        deltaNext.assign(inputSize, 0.0f);
        for (int i = 0; i < inputSize; ++i) 
        {
            for (int j = 0; j < outputSize; ++j) 
            {
                deltaNext[i] += delta[j] * w[j * inputSize + i];
            }
        }
    }
}

float NeuralNetwork::UnclampEvaluation(float clamped) const
{
    if (m_maxEvalKnown == m_minEvalKnown)
    {
        return 0.0f;
    }
    clamped = std::max(-1.0f, std::min(1.0f, clamped));
    return ((clamped + 1.0f) / 2.0f) * (m_maxEvalKnown - m_minEvalKnown) + m_minEvalKnown;
}

bool NeuralNetwork::ClampedEvaluationPossible()
{
    return m_clampedEvaluationPossible;
}


int NeuralNetwork::NextId = 0;
#include "RandomPrioritySolver.h"
#include <algorithm>
#include <random>
#include <thread>
#include <barrier>

RandomPrioritySolver::RandomPrioritySolver(int num_threads) : num_threads(num_threads), gen(RANDOM_SEED) {}

void RandomPrioritySolver::solve(Graph &original_graph)
{
    Graph uncolored_graph(original_graph);
    color_t color = 0;
    while (!uncolored_graph.empty())
    {
        compute_MIS(uncolored_graph);
        for (uint32_t vertex : MIS)
        {
            original_graph.colors[vertex] = color;
            uncolored_graph.remove_vertex(vertex);
        }
        color++;
    }
}

// Best explained here:
// https://en.wikipedia.org/wiki/Maximal_independent_set#Random-priority_parallel_algorithm
void RandomPrioritySolver::compute_MIS(const Graph &src)
{
    // Reset solver state
    MIS.clear();
    Remaining_Vertices.clear();

    uint32_t num_verticies_graph = src.vertices.size();

    for (uint32_t i = 0; i < num_verticies_graph; i++)
    {
        // It suffices to check for is_deleted here, since we don't delete vertices inside the function
        if (!src.is_deleted(i))
        {
            Remaining_Vertices.emplace_back(i);
        }
    }

    uint32_t num_vertices_Remaining = Remaining_Vertices.size();

    std::barrier sync_point1(num_vertices_at_begining);
    std::barrier sync_point2(num_vertices_at_begining);

    Random_Priority_Vec.reserve(num_verticies_graph);
    Destroy_Vec.assign(num_verticies_graph, false);

    while (!Remaining_Vertices.empty())
    {
        std::vector<std::thread> threadPool;
        for (uint32_t i = 0; i < num_vertices_Remaining; i++)
        {
            threadPool.emplace_back(vertex_job, i, src);
        }
        for (auto &th : threadPool)
        {
            th.join();
        }
    }
}

void RandomPrioritySolver::vertex_job(uint32_t thID, Graph &src, std::barrier &sync_point1, std::barrier &sync_point2)
{

    uint32_t vertexID = Remaining_Vertices[thID];
    // Select random number
    uint32_t RandNum = rand() % 100;
    // Send it to the neighbours "Announce it"
    Random_Priority_Vec[vertexID] = RandNum;

    edges_t neighbors = src.neighbors_of(vertexID);
    edges_t remaining_neighbors;
    bool IamTheSmallest = true;

    // a barrier should be here !!!!
    sync_point1.arrive_and_wait();

    for (uint32_t neighbor : neighbors)
    {
        if (std::find(Remaining_Vertices.begin(), Remaining_Vertices.end(), neighbor) != Remaining_Vertices.end())
        {
            remaining_neighbors.emplace_back(neighbor);
            if (Random_Priority_Vec[neighbor] < RandNum)
            {
                IamTheSmallest = false;
                break;
            }
        }
    }

    if (IamTheSmallest)
    {
        // Insert itself into MIS
        wrt_mutex.lock();
        MIS.emplace_back(vertexID);
        wrt_mutex.unlock();
        // Remove itself from Remaining_Vertices
        std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);
        // Tells the neighbors
        for (uint32_t neighbor : remaining_neighbors)
        {
            wrt_mutex.lock();
            Destroy_Vec[neighbor] = true;
            wrt_mutex.unlock();
        }
    }

    // a barrier should be here !!!
    sync_point2.arrive_and_wait();

    if (Destroy_Vec[vertexID] == true)
    {
        // Remove itself from Remaining_Vertices
        std::remove(Remaining_Vertices.begin(), Remaining_Vertices.end(), vertexID);
    }
}

std::string RandomPrioritySolver::name() const
{
    return "RandomPrioritySolver (" + std::to_string(num_threads) + " threads)";
}

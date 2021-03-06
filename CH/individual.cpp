#include <fstream>
#include <cstdlib>
#include <algorithm>
#include "individual.hpp"

using std::ifstream;
using std::rand;
using std::swap;
#ifdef DEBUG_MODE
#include <iostream>
using std::cout;
using std::endl;
#endif

// Initialize static member variables in Individual //
int    Individual::FACILITY_NUM      = 0;     
int    Individual::CUSTOMER_NUM      = 0;     
int**  Individual::ASSIGN_COST       = nullptr;  
int*   Individual::CUSTOMER_DEMAND   = nullptr;      
int*   Individual::OPEN_COST         = nullptr; 
int*   Individual::FACILITY_CAPACITY = nullptr;
int*   Individual::MIN_ASSIGN_COST   = nullptr;
int*   Individual::MIN_ASSIGN        = nullptr;

// Clear all data and recycle memory space allocated //
void Individual::clearData() {
  if (ASSIGN_COST != nullptr){
    for (int i = 0; i < FACILITY_NUM; ++i) {
      delete[] ASSIGN_COST[i];
    }
    delete[] ASSIGN_COST;
    ASSIGN_COST = nullptr;
  }
  if (MIN_ASSIGN_COST != nullptr) {
    delete[] MIN_ASSIGN_COST;
    MIN_ASSIGN_COST = nullptr;
  }
  if (MIN_ASSIGN != nullptr) {
    delete[] MIN_ASSIGN;
    MIN_ASSIGN = nullptr;
  }
  if (CUSTOMER_DEMAND != nullptr) {
    delete[] CUSTOMER_DEMAND;
    CUSTOMER_DEMAND = nullptr;
  }
  if (OPEN_COST != nullptr) {
    delete[] OPEN_COST;
    OPEN_COST = nullptr;
  }
  if (FACILITY_CAPACITY != nullptr) {
    delete[] FACILITY_CAPACITY;
    FACILITY_CAPACITY = nullptr;
  }
}

// Initialize case info with the given file //
void Individual::init(string filePath) {
  // Create a input file stream //
  ifstream is;
  is.open(filePath); 
  double temp = 0;
  
  // If the file is not open properly //
  if (!is.is_open()) { 
    throw string("E: Unable To Open File");
    return;
  }

  // Clear existed data and free memory space
  clearData();
  
  // Number of facilities and customers //
  is >> FACILITY_NUM >> CUSTOMER_NUM;
  
  // Capacity and Open Cost //
  FACILITY_CAPACITY = new int[FACILITY_NUM];
  OPEN_COST = new int [FACILITY_NUM];
  for (int i = 0; i < FACILITY_NUM; ++i) {
    is >> FACILITY_CAPACITY[i] >> OPEN_COST[i];
  }

  // Demand of customers //
  CUSTOMER_DEMAND = new int[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    is >> temp;
    CUSTOMER_DEMAND[i] = temp;
  }

  // Assign Cost //
  MIN_ASSIGN_COST = new int[CUSTOMER_NUM];
  MIN_ASSIGN = new int[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i) MIN_ASSIGN_COST[i] = INT32_MAX;
  ASSIGN_COST = new int*[FACILITY_NUM];
  for (int i = 0; i < FACILITY_NUM; ++i) {
    ASSIGN_COST[i] = new int[CUSTOMER_NUM];
    for (int t = 0; t < CUSTOMER_NUM; ++t) {
      is >> temp;
      ASSIGN_COST[i][t] = temp;
      if (temp < MIN_ASSIGN_COST[t]){
        MIN_ASSIGN_COST[t] = temp;
        MIN_ASSIGN[t] = i;
      }
    }
  }

  // Debug info //
  #ifdef DEBUG_MODE
    cout << "Status after reading data:" << endl
      << "Facility Number: " << FACILITY_NUM << endl
      << "Customer Number: " << CUSTOMER_NUM << endl
      << "Assign Cost: " << endl;
    for (int i = 0; i < FACILITY_NUM; ++i) {
      for (int t = 0; t < CUSTOMER_NUM; ++t) {
        cout << ASSIGN_COST[i][t] << " ";
      }
      cout << endl;
    }
    cout << "Facility Capacities: " << endl;
    for (int i = 0; i < FACILITY_NUM; ++i) {
      cout << FACILITY_CAPACITY[i] << " ";
    }
    cout << endl
      << "Facility Open Cost: " << endl;
    for (int i = 0; i < FACILITY_NUM; ++i) {
      cout << OPEN_COST[i] << " ";
    } 
    cout << endl
      << "Customers' demand: " << endl;
    for (int i = 0; i < CUSTOMER_NUM; ++i) {
      cout << CUSTOMER_DEMAND[i] << " ";
    }
    cout << endl;
  #endif
}

// Estimate the cost of current solution //
int Individual::estimateCost(Individual& individual) {
  // If this solution is not a legal one, return the max cost //
  if (!isValid(individual.gene))
    return INT32_MAX;
  
  // Store the status of each facility //
  bool* facilities = new bool[FACILITY_NUM];
  int cost = 0;
  for (int i = 0; i < FACILITY_NUM; ++i) {
    facilities[i] = false;
  }

  // Count the cost //
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    if (facilities[individual.gene[i]] == false) {
      // Open the facility and count the cost //
      cost += OPEN_COST[individual.gene[i]];
      facilities[individual.gene[i]] = true;
    }
    // Count assignment cost //
    cost += ASSIGN_COST[individual.gene[i]][i];
  }

  // Free allocated memory space //
  delete []facilities;
  // Update and return the cost //
  return individual.cost = cost;
}

Individual::Individual() {
  // Brand new gene //
  this->gene = new int[CUSTOMER_NUM];
  
  // Sequence used to check the facility capacity //
  // int* facilities = new int[FACILITY_NUM];
  // for (int i = 0; i < FACILITY_NUM; ++i) {
  //   facilities[i] = 0;
  // }

  
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    // Random until valid //
    // do {
      // Assign a random facility to each customer //
      gene[i] = rand() % FACILITY_NUM;
    // } while (CUSTOMER_DEMAND[i] + 
    //   facilities[gene[i]] > FACILITY_CAPACITY[gene[i]]);
  }
  
  // Estimate the cost //
  cost = estimateCost(*this);
  // Free allocated memory space //
  // delete[] facilities;
}

Individual::~Individual() {
  // Free allocated memory space //
  if (gene != nullptr) delete[] gene;
}

int* Individual::getSolution() {
  return gene;
}

// Check whether this solution match requirement //
bool Individual::isValid(int* gene) {
  // Count demand //
  int* facilities = new int[FACILITY_NUM];
  bool flag = true;
  for (int i = 0; i < FACILITY_NUM; ++i) {
    facilities[i] = 0;
  }

  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    facilities[gene[i]] += CUSTOMER_DEMAND[i];
    if (facilities[gene[i]] > FACILITY_CAPACITY[gene[i]]) {
      // Overflow! Not a valid solution //
      flag = false;
    }
  }

  // Free allocated memory //
  delete[] facilities;
  return flag;
}

Individual::Individual(const Individual& individual) {
  gene = new int[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    // Copy gene //
    gene[i] = individual.gene[i];
  }
  // Copy cost //
  cost = individual.cost;
}

// Similar to copy constructor //
Individual& Individual::operator=(const Individual& individual) {
  if (gene == nullptr) gene = new int[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    gene[i] = individual.gene[i];
  }
  cost = individual.cost;
  return *this;
}

// Get cost //
int Individual::getCost() {
  return cost;
}

// Get case info //
int Individual::getFacilityNum() {
  return FACILITY_NUM;
}
int Individual::getCustomerNum() {
  return CUSTOMER_NUM;
}

Individual** Individual::getNeighborsWithTryMin() {
  Individual** result = new Individual*[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i) {
    auto temp = getGeneCopy();
    temp[i] = MIN_ASSIGN[i];
    result[i] = new Individual(temp);
  }
  return result;
}

// Get a copy of gene //
int* Individual::getGeneCopy() {
  int *copy = new int[CUSTOMER_NUM];
  for (int i = 0; i < CUSTOMER_NUM; ++i)
    copy[i] = gene[i];
  return copy;
}

Individual::Individual(int* gene) {
  // Copy gene //
  this->gene = gene;
  // Get Cost //
  cost = estimateCost(*this);
}
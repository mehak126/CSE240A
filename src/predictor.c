//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <string.h>

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

int numPerceptrons; // Number of perceptrons
int weightBits;     // Number of bits used for weights in perceptron
int threshold;      // Threshold used for training perceptron


#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t gHistory;
// For tournament
uint8_t *tournamentGBHT;  // global tournament BHT
uint8_t *lBHT;  // local BHT
uint32_t *lIndexTable;  // for indexing lBHT
uint8_t *selector;  // selector
int gBHTIndex; //global BHT index

uint8_t lPrediction; //local prediction
uint8_t gPrediction; //global prediction

uint8_t *gshareBHT; //global gshare BHT

//For perceptron
int prediction; // perceptron prediction
int numWeights; // number of weights in a perceptron, same as number of bits used for gHistory
int absMaxWeight; // absolute maximum value of a weight in a perceptron
int32_t **perceptronWeightsTable; // Table to store weights of perceptrons
int32_t *perceptronBiasTable;     // Table to store biases of perceptrons

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch(bpType){
    gHistory = 0;
    case STATIC:
      return;
    case GSHARE:
      gshareBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
      memset(gshareBHT, WN, sizeof(uint8_t) * (1 << ghistoryBits));
      break;
    case TOURNAMENT:
      tournamentGBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
      lBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
      lIndexTable = malloc((1 << pcIndexBits) * sizeof(uint32_t));
      selector = malloc((1 << ghistoryBits) * sizeof(uint8_t));

      memset(tournamentGBHT, WN, sizeof(uint8_t) * (1 << ghistoryBits));
      memset(lBHT, WN, sizeof(uint8_t) * (1 << lhistoryBits));
      memset(lIndexTable, 0, sizeof(uint32_t) * (1 << pcIndexBits));
      memset(selector, WN, sizeof(uint8_t) * (1 << ghistoryBits)); //xxx
      break;
    case CUSTOM:
      // weightBits = 5; // can set it as argument
      numWeights = ghistoryBits;
      
      perceptronBiasTable = (int32_t*)calloc(numPerceptrons, sizeof(int32_t));
      perceptronWeightsTable = (int32_t**)malloc(numPerceptrons*sizeof(int32_t*));
      for (int i = 0; i < numPerceptrons; i++)
        perceptronWeightsTable[i] = (int32_t*)calloc(numWeights,sizeof(int32_t));

      // threshold = (1 << lhistoryBits); //set it as 8 bits; threshold = 2^8 = 256
      absMaxWeight = (1 << weightBits) - 1;
      printf("Number of perceptrons: %d\n", numPerceptrons);
      printf("Number of weights: %d\n", numWeights);
      printf("Number of weightBits: %d\n", weightBits);
      printf("total memory of perceptron predictor: %d Kbits \n", (numPerceptrons*(numWeights + 1)*(weightBits + 1))/1024);
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

uint8_t gshare_global_prediction(uint32_t pc, int gBHTIndex) {
  gBHTIndex = ((pc ^ gHistory) & ((1 << ghistoryBits) - 1));
  uint8_t prediction = gshareBHT[gBHTIndex];
  if (prediction == WN || prediction == SN)
    return NOTTAKEN;
  else
    return TAKEN;
}

uint8_t tournament_global_prediction(uint32_t pc, int gBHTIndex) {
  uint8_t prediction = tournamentGBHT[gBHTIndex];
  if (prediction == WN || prediction == SN)
    return NOTTAKEN;
  else
    return TAKEN;
}

uint8_t tournament_local_prediction(uint32_t pc) {
  int lIndexTableIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lBHTindex = lIndexTable[lIndexTableIndex];
  uint8_t prediction  = lBHT[lBHTindex];
  if (prediction == WN || prediction == SN)
    return NOTTAKEN;
  else
    return TAKEN;
}

int perceptron_prediction_raw(uint32_t pc){
  //Hash function to index perceptron table
  int perceptronIndex = pc & (numPerceptrons - 1);
  int output = perceptronBiasTable[perceptronIndex];
  for (int i = 0; i < numWeights; i++)
    output = (gHistory >> i) & 1? output + perceptronWeightsTable[perceptronIndex][i] : output - perceptronWeightsTable[perceptronIndex][i];
  return output;
}

uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {

    case STATIC:
      return TAKEN;

    case GSHARE:
      gPrediction = gshare_global_prediction(pc, gBHTIndex);
      return gPrediction;

    case TOURNAMENT:
      // select the predictor (local or global)
      gBHTIndex = gHistory & ((1 << ghistoryBits) - 1);

      gPrediction = tournament_global_prediction(pc, gBHTIndex);
      lPrediction = tournament_local_prediction(pc);

      uint8_t predictor = selector[gBHTIndex];

      if (predictor == WN || predictor == SN)
        return gPrediction;
      else
        return lPrediction;

    case CUSTOM:
      prediction = perceptron_prediction_raw(pc);
      gPrediction = prediction >= 0? TAKEN: NOTTAKEN;
      return gPrediction;
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void update_ghistory_shift_register(uint8_t outcome)
{
  gHistory <<= 1;
  gHistory  &= ((1 << ghistoryBits) - 1);
  gHistory |= outcome;
}

void train_tournament(uint32_t pc, uint8_t outcome) {
  int gBHTIndex = gHistory & ((1 << ghistoryBits) - 1);
  int lIndexTableIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lBHTindex = lIndexTable[lIndexTableIndex];

  //Update gBHT and lBHT
  if (outcome == TAKEN) {
    if (tournamentGBHT[gBHTIndex] != ST)
      tournamentGBHT[gBHTIndex] += 1;
    if (lBHT[lBHTindex] != ST)
      lBHT[lBHTindex] += 1;
  }
  else {
    if (tournamentGBHT[gBHTIndex] != SN)
      tournamentGBHT[gBHTIndex] -= 1;
    if (lBHT[lBHTindex] != SN)
      lBHT[lBHTindex] -= 1;
  }

  // Update selector
  if (gPrediction != lPrediction) {
    if (lPrediction == outcome) {
      if(selector[gBHTIndex] != ST)
        selector[gBHTIndex] += 1;
    }
    else {
      if (selector[gBHTIndex] != SN)
        selector[gBHTIndex] -= 1;
    }
  }

  //Update global history and local index table
  lIndexTable[lIndexTableIndex] <<= 1;
  lIndexTable[lIndexTableIndex] &= ((1 << lhistoryBits) - 1);
  lIndexTable[lIndexTableIndex] |= outcome;
  gHistory <<= 1;
  gHistory  &= ((1 << ghistoryBits) - 1);
  gHistory |= outcome;
  //update_ghistory_shift_register(outcome);
}


void train_gshare(uint32_t pc, uint8_t outcome) {
  int gBHTIndex =  (pc ^ gHistory) & ((1 << ghistoryBits) - 1);

  //Update gshareBHT
  if (outcome == TAKEN) {
    if (gshareBHT[gBHTIndex] != ST)
      gshareBHT[gBHTIndex] += 1;
  }
  else {
    if (gshareBHT[gBHTIndex] != SN)
      gshareBHT[gBHTIndex] -= 1;
  }

  //Update global history shift register
  // gHistory <<= 1;
  // gHistory &= ((1 << ghistoryBits) - 1);
  // gHistory |= outcome;
  update_ghistory_shift_register(outcome);

}

void train_perceptron(uint32_t pc, uint8_t outcome)
{
  prediction = perceptron_prediction_raw(pc);
  gPrediction = prediction >= 0? TAKEN: NOTTAKEN;

  int perceptronIndex = pc & (numPerceptrons - 1);
  
  if ((gPrediction != outcome) || (abs(prediction) < threshold))
  {
    
    for (int j = 0; j < numWeights; j++)
    {
      int8_t signMultiplier = ((gHistory >> j)&1) == 1 ? 1: -1; 

      perceptronWeightsTable[perceptronIndex][j] += signMultiplier*(outcome==TAKEN?1:-1); //(((gHistory>>j) & 1)==outcome) ? 1 : -1;
      perceptronWeightsTable[perceptronIndex][j] = max(min(perceptronWeightsTable[perceptronIndex][j], absMaxWeight), -absMaxWeight - 1);
    }
    perceptronBiasTable[perceptronIndex] += (outcome==TAKEN ?1:-1);
  }
  // for (int j = 0; j< numWeights; j++)
  //   printf("\n##%d",perceptronWeightsTable[perceptronIndex][j]);
  //Update global history shift 
  gHistory = (gHistory<<1 | outcome) & ((1<<numWeights) - 1);
  // gHistory <<= 1;
  // gHistory  &= ((1 << ghistoryBits) - 1);
  // gHistory |= outcome;
  //update_ghistory_shift_register(outcome);
}

void train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_gshare(pc, outcome);
      break;
    case TOURNAMENT:
      train_tournament(pc, outcome);
      break;
    case CUSTOM:
      train_perceptron(pc, outcome);
      break;
    default:
      break;
    }
  
}

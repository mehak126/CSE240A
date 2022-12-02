//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

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

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t gHistory;
// For tournament
uint8_t *tournamentGBHT;  // global BHT
uint8_t *lBHT;  // local BHT
uint32_t *lIndexTable;  // for indexing lBHT
uint8_t *selector;  // selector

uint8_t lPrediction; //local prediction
uint8_t gPrediction; //global prediction

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  switch(bpType):
    case STATIC:
      return;
    case GSHARE:
      return;
    case TOURNAMENT:
      tournamentGBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
      lBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
      lIndexTable = malloc((1 << pcIndexBits) * sizeof(uint32_t));
      selector = malloc((1 << ghistoryBits) * sizeof(uint8_t));

      memset(tournamentGBHT, WN, sizeof(uint8_t) * (1 << ghistoryBits));
      memset(lBHT, WN, sizeof(uint8_t) * (1 << lhistoryBits));
      memset(lIndexTable, 0, sizeof(uint32_t) * (1 << pcIndexBits));
      memset(selector, WN, sizeof(uint8_t) * (1 << ghistoryBits)); //xxx
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//

uint8_t tournament_global_prediction(uint32_t pc, int gBHTIndex) {
  uint8_t prediction = tournamentGBHT[gBHTIndex];
  if (prediction == WN || predictor == SN)
    return NOTTAKEN
  else
    return TAKEN
}

uint8_t tournament_local_prediction(uint32_t pc) {
  int lIndexTableIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lBHTindex = lIndexTable[lIndexTableIndex];
  uint8_t prediction  = lBHT[lBHTindex];
  rif (prediction == WN || predictor == SN)
    return NOTTAKEN
  else
    return TAKEN
}

uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
    case TOURNAMENT:
      // select the predictor (local or global)
      int gBHTIndex = gHistory & ((1 << ghistoryBits) - 1);

      gPrediction = tournament_global_prediction(pc, gBHTIndex)
      lPrediction = tournament_local_prediction(pc)

      uint8_t predictor = selector[gBHTIndex];

      if (predictor == WN || predictor == SN)
        return gPrediction;
      else
        return lPrediction;
    case CUSTOM:
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
void train_tournament(uint32_t pc, uint8_t outcome) {
  int gBHTIndex = gHistory & ((1 << ghistoryBits) - 1);
  int lIndexTableIndex = pc & ((1 << pcIndexBits) - 1);
  uint32_t lBHTindex = lIndexTable[lIndexTableIndex];

  //Update gBHT and lBHT
  if outcome == TAKEN {
    if (gBHT[gBHTIndex] != ST)
      gBHT[gBHTIndex] += 1
    if (lBHT[lBHTindex] != ST)
      lBHT[lBHTindex] += 1
  }
  else {
    if (gBHT[gBHTIndex] != SN)
      gBHT[gBHTIndex] -= 1
    if (lBHT[lBHTindex] != SN)
      lBHT[lBHTindex] -= 1
  }

  // Update selector
  if (gPrediction != lPrediction) {
    if (lPrediction == outcome) {
      if(selector[gBHTIndex] != ST)
        selector[gBHTIndex] += 1;
    }
    else {
      if (selector[gBHTIndex] != NT)
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
  }
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      break;
    case TOURNAMENT:
      train_tournament(pc, outcome);
      break;
    case CUSTOM:
      break;
    default:
      break;
    }
  
}

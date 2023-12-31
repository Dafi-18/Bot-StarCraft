#pragma once

#include "MapTools.h"

#include <BWAPI.h>

class StarterBot
{
    MapTools m_mapTools;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
    void drawDebugInformation();
	void trainInfantery();
	void buildBasicArmyBuilding();
	void buildUpdateBasic();
	void AttackZealots();
	void AttackDragoons();
	void buildAssimilator();
	void sendIdleWorkersToRefineries();
	void trainDragoons();
	void buildForge();
	void buildPhotonCannon();
	void buildSecondUpdate();
	void ResetAttackLogicIfNeeded();
	void AttackTemplars();
	void buildTemplarArchives();
	void trainTemplars();



    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);
};
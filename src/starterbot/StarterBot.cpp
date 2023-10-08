#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"

StarterBot::StarterBot()
{
    
}

// ESTE ES EL PRIMER INTENTO :D 
// Como puedohacer un push??
// 
// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(8);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();
    auto& units = BWAPI::Broodwar->getAllUnits();
    for (auto u : units)
    {
        BWAPI::Position pos_u = u->getPosition();
        std::string posStr = "(" + std::to_string(pos_u.x) + ", " + std::to_string(pos_u.y) + ")";
        const char* posCStr = posStr.c_str();

        BWAPI::Broodwar->drawTextMap(u->getPosition(), "%s", posCStr);
    }

    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();

    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

    // Build a barrack or a gateway
    buildBasicArmyBuilding();

    // Build para extraer gas vespeno
    buildAssimilator();

    sendIdleWorkersToGas();

    trainInfantery();

    buildUpdateBasic();

    //Attack Zealots
    AttackZealots();

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

void StarterBot::sendIdleWorkersToGas()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestGas = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getGeysers());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestGas) { unit->rightClick(closestGas); }
        }
    }
}
// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    const int currentSupply = Tools::GetTotalSupply(true);

    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= (2 * (1 + currentSupply / 15))) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
}

void StarterBot::trainInfantery()
{
    const BWAPI::UnitType infanteryType = BWAPI::Broodwar->self()->getRace().getInfanteryBasic();

    const BWAPI::UnitType buildingType = BWAPI::Broodwar->self()->getRace().getBasicArmyBuilding();

    const int minerals = BWAPI::Broodwar->self()->minerals();

    const int requiredMinerals = infanteryType.mineralPrice();

    if (minerals < requiredMinerals) { return; }

    // get the unit pointer to my basic Army Building (hatchery, barracks or gateway)
    const BWAPI::Unit myArmyBuilding = Tools::GetAvailableTrainingBuilding(buildingType);

    // if we have a valid depot unit and it's currently not training something, train a worker
    // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
    if (myArmyBuilding) { myArmyBuilding->train(infanteryType); }
}

void StarterBot::buildBasicArmyBuilding()
{
    const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType buildingType = BWAPI::Broodwar->self()->getRace().getBasicArmyBuilding();
    // Para construir este edificio tengo que tener minerales suficientes
    // Obtener la cantidad de minerales
    const int minerals = BWAPI::Broodwar->self()->minerals();

    //const int requiredMinerales = (raceName = "Zerg" ? 200 : 150);
    const int requiredMinerales = buildingType.mineralPrice();
    // Obtener la cantidad de edificios hasta el momento.
    const int basicArmyBuildingsOwned = Tools::CountUnitsOfType(buildingType, BWAPI::Broodwar->self()->getUnits());

    // Si no tengo suficientes minerales y ya tengo 3 edificios construidos, no hago nada.
    if (minerals < requiredMinerales or basicArmyBuildingsOwned == 3) { return; }

    const bool startedBuilding = Tools::BuildBuilding(buildingType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", buildingType.getName());
    }
}

void StarterBot::buildUpdateBasic()
{
    const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType buildingType = BWAPI::Broodwar->self()->getRace().getUpdateBasic();
    // Para construir este edificio tengo que tener minerales suficientes
    // Obtener la cantidad de minerales
    const int minerals = BWAPI::Broodwar->self()->minerals();

    //const int requiredMinerales = (raceName = "Zerg" ? 200 : 150);
    const int requiredMinerales = buildingType.mineralPrice();
    // Obtener la cantidad de edificios hasta el momento.
    const int updateBasicOwned = Tools::CountUnitsOfType(buildingType, BWAPI::Broodwar->self()->getUnits());

    // Si no tengo suficientes minerales y ya tengo 3 edificios construidos, no hago nada.
    if (minerals < requiredMinerales or updateBasicOwned == 1) { return; }

    const bool startedBuilding = Tools::BuildBuilding(buildingType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", buildingType.getName());
    }
}

void StarterBot::buildAssimilator()
{
    const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();
    // Para construir este edificio tengo que tener minerales suficientes
    // Obtener la cantidad de minerales
    const int minerals = BWAPI::Broodwar->self()->minerals();

    //const int requiredMinerales = (raceName = "Zerg" ? 200 : 150);
    const int requiredMinerales = refineryType.mineralPrice();
    // Obtener la cantidad de edificios hasta el momento.
    const int refineryOwned = Tools::CountUnitsOfType(refineryType, BWAPI::Broodwar->self()->getUnits());

    // Si no tengo suficientes minerales y ya tengo 3 edificios construidos, no hago nada.
    if (minerals < requiredMinerales or refineryOwned == 1) { return; }

    const bool startedBuilding = Tools::BuildBuilding(refineryType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", refineryType.getName());
     
    }
}

void StarterBot::AttackZealots()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

    static int zealotCount = 0; // Contador total de Zealots que han atacado
    static int aliveZealotCount = 0; // Contador de Zealots vivos en el grupo actual
    static BWAPI::TilePosition tilestartLocation = BWAPI::Broodwar->self()->getStartLocation(); // Punto de inicio
    static BWAPI::Position startLocation(tilestartLocation);
    static bool attackingEnemyBase = false; // Indica si estamos atacando la base enemiga

    // Encuentra la posición objetivo, que es la base enemiga más lejana de las dos coordenadas
    BWAPI::Position enemyBase;
    double maxDistance = 0;
    double distance1 = startLocation.getDistance(BWAPI::Position(320, 3264));
    double distance2 = startLocation.getDistance(BWAPI::Position(3264, 320));

    if (distance1 > distance2) {
        enemyBase = BWAPI::Position(320, 3264);
    }
    else {
        enemyBase = BWAPI::Position(3264, 320);
    }

    // Realiza el movimiento y el ataque en grupos de 20
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
        {
            // Verifica si hay enemigos cerca de la posición de inicio y ataca si los encuentra
            if (attackingEnemyBase)
            {
                BWAPI::Unit closestEnemyNearBase = Tools::GetClosestUnitTo(unit, enemyUnits);
                if (closestEnemyNearBase) {
                    unit->attack(closestEnemyNearBase);
                }
            }
            else
            {
                // Verifica si hay enemigos en el camino hacia la posición de inicio
                BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);

                // Si quedan menos de 20 Zealots vivos y no están atacando, regresan al punto de inicio
                if (aliveZealotCount < 10 && !unit->isAttacking() && !attackingEnemyBase)
                {
                    unit->rightClick(startLocation);
                    aliveZealotCount = 0; // Reinicia el contador
                }
                else if (closestEnemy) {
                    unit->attack(closestEnemy);
                }
                else {
                    if (aliveZealotCount > 10) {
                        // Si tienes más de 10 Zealots agrupados, cambia a atacar la base enemiga
                        unit->attack(enemyBase);
                        attackingEnemyBase = true;
                    }
                    else {
                        unit->rightClick(startLocation);
                    }
                }
            }

            // Incrementa el contador de Zealots vivos
            aliveZealotCount++;

            // Si hay 20 Zealots en el grupo, reinicia el contador y continúa con el siguiente grupo
            if (aliveZealotCount >= 10)
            {
                aliveZealotCount = 0;
            }
        }
    }
}



// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner)
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
	
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
	
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}
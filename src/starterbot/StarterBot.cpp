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

    buildForge();

    // Build para extraer gas vespeno
    buildAssimilator();

    sendIdleWorkersToRefineries();

    buildUpdateBasic();

    buildSecondUpdate();

    buildTemplarArchives();

    trainTemplars();

    trainDragoons();

    trainInfantery();

    buildPhotonCannon();

    //Attack Zealots
    AttackZealots();

    //Attack Dragoons
    AttackDragoons();

    AttackTemplars();

    // Reset attack logic if needed
    ResetAttackLogicIfNeeded();

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    const int maxGasWorkers = 3; // Máximo de trabajadores en las refinerías

    // Contar la cantidad de trabajadores en las refinerías
    int gasWorkers = 0;
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType().isWorker() && unit->isGatheringGas())
        {
            gasWorkers++;
        }
    }

    // Verificar si ya hemos construido al menos una refinería
    const int refineryCount = Tools::CountUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, BWAPI::Broodwar->self()->getUnits());

    // Iterar a través de todas nuestras unidades
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        // Si la unidad es un obrero y está inactiva, enviarla a recolectar minerales
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Si aún no hemos construido ninguna refinería, enviar a los obreros a recolectar minerales
            if (refineryCount == 0)
            {
                BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());
                if (closestMineral) { unit->rightClick(closestMineral); }
            }
            // Si ya tenemos suficientes obreros extrayendo gas, enviarlos a recolectar minerales
            else if (gasWorkers >= maxGasWorkers)
            {
                BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());
                if (closestMineral) { unit->rightClick(closestMineral); }
            }
        }
    }
}

void StarterBot::sendIdleWorkersToRefineries()
{
    int gasWorkers = 0;
    const int maxGasWorkers = 3; // Máximo de trabajadores en las refinerías

    // Contar la cantidad de trabajadores en las refinerías
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType().isWorker() && unit->isGatheringGas())
        {
            gasWorkers++;
        }
    }

    // Iterar a través de todas nuestras unidades
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        // Si la unidad es un obrero y está inactiva, enviarla a extraer gas
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Verificar si ya tenemos suficientes obreros extrayendo gas
            if (gasWorkers >= maxGasWorkers)
            {
                // Si ya tenemos suficientes obreros en las refinerías, enviarlos a extraer minerales
                sendIdleWorkersToMinerals();
                break; // Salir del bucle
            }

            // Buscar la refinería más cercana
            BWAPI::Unit closestRefinery = nullptr;
            for (auto& u : BWAPI::Broodwar->self()->getUnits())
            {
                if (u->getType().isRefinery())
                {
                    if (closestRefinery == nullptr || unit->getDistance(u) < unit->getDistance(closestRefinery))
                    {
                        closestRefinery = u;
                    }
                }
            }

            // Si encontramos una refinería, enviar al obrero a extraer gas
            if (closestRefinery != nullptr)
            {
                unit->gather(closestRefinery);
                gasWorkers++;
            }
        }
    }
}


// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 21;
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

void StarterBot::trainDragoons()
{
    const BWAPI::UnitType dragoonType = BWAPI::Broodwar->self()->getRace().getSecondInfantery();
    const BWAPI::UnitType gatewayType = BWAPI::Broodwar->self()->getRace().getBasicArmyBuilding();

    const BWAPI::UnitType zealogsType = BWAPI::Broodwar->self()->getRace().getInfanteryBasic();
    const int zealogsWanted = 10;
    const int zealogsOwned = Tools::CountUnitsOfType(zealogsType, BWAPI::Broodwar->self()->getUnits());


    if (zealogsWanted < zealogsOwned) {
        // Verificar recursos disponibles
        const int minerals = BWAPI::Broodwar->self()->minerals();
        const int gas = BWAPI::Broodwar->self()->gas();

        const int requiredMinerals = dragoonType.mineralPrice();
        const int requiredGas = dragoonType.gasPrice();


        // Check if you have enough resources to train a dragoon
        if (minerals >= requiredMinerals && gas >= requiredGas) {
            // Find a gateway that can train dragoons
            BWAPI::Unit myGateway = Tools::GetAvailableTrainingBuilding(gatewayType);

            // If you have a valid gateway, train a dragoon
            if (myGateway && !myGateway->isTraining()) {
                myGateway->train(dragoonType);
            }
        }
    }
}

void StarterBot::trainTemplars()
{
    const BWAPI::UnitType templarType = BWAPI::Broodwar->self()->getRace().getThreeInfantery();
    const BWAPI::UnitType gatewayType = BWAPI::Broodwar->self()->getRace().getBasicArmyBuilding();

    const BWAPI::UnitType dragonsType = BWAPI::Broodwar->self()->getRace().getInfanteryBasic();
    const int dragonWanted = 5;
    const int dragonOwned = Tools::CountUnitsOfType(dragonsType, BWAPI::Broodwar->self()->getUnits());


    if (dragonWanted < dragonOwned) {
        // Verificar recursos disponibles
        const int minerals = BWAPI::Broodwar->self()->minerals();
        const int gas = BWAPI::Broodwar->self()->gas();

        const int requiredMinerals = templarType.mineralPrice();
        const int requiredGas = templarType.gasPrice();


        // Check if you have enough resources to train a dragoon
        if (minerals >= requiredMinerals && gas >= requiredGas) {
            // Find a gateway that can train dragoons
            BWAPI::Unit myGateway = Tools::GetAvailableTrainingBuilding(gatewayType);

            // If you have a valid gateway, train a dragoon
            if (myGateway && !myGateway->isTraining()) {
                myGateway->train(templarType);
            }
        }
    }
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
    const BWAPI::UnitType coreType = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
    const int minerals = BWAPI::Broodwar->self()->minerals();

    // Verificar si ya tienes una Cibernética Core construida
    const int cyberneticsCoreCount = Tools::CountUnitsOfType(coreType, BWAPI::Broodwar->self()->getUnits());

    // Si no tienes suficientes minerales o ya tienes una Cibernética Core, no hagas nada.
    if (minerals < coreType.mineralPrice() || cyberneticsCoreCount == 1) {
        return;
    }

    const bool startedBuilding = Tools::BuildBuilding(coreType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", coreType.getName());
    }
}



void StarterBot::buildAssimilator()
{
    const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType refineryType = BWAPI::Broodwar->self()->getRace().getRefinery();


    const BWAPI::UnitType buildingType = BWAPI::Broodwar->self()->getRace().getBasicArmyBuilding();
    const int gatewayWanted = 3;
    const int gatewayOwned = Tools::CountUnitsOfType(buildingType, BWAPI::Broodwar->self()->getUnits());

    if (gatewayOwned >= gatewayWanted) {
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


}

void StarterBot::buildSecondUpdate()
{
    const BWAPI::UnitType secondType = BWAPI::UnitTypes::Protoss_Citadel_of_Adun;
    const int minerals = BWAPI::Broodwar->self()->minerals();

    // Verificar si ya tienes una Cibernética Core construida
    const int CitadelOfAdunCount = Tools::CountUnitsOfType(secondType, BWAPI::Broodwar->self()->getUnits());

    // Si no tienes suficientes minerales o ya tienes una Cibernética Core, no hagas nada.
    if (minerals < secondType.mineralPrice() || CitadelOfAdunCount == 1) {
        return;
    }

    const bool startedBuilding = Tools::BuildBuilding(secondType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", secondType.getName());
    }
}

void StarterBot::buildTemplarArchives()
{
    const BWAPI::UnitType TemplarArchivesType = BWAPI::UnitTypes::Protoss_Templar_Archives;
    const int minerals = BWAPI::Broodwar->self()->minerals();

    // Verificar si ya tienes una Cibernética Core construida
    const int CitadelOfAdunCount = Tools::CountUnitsOfType(TemplarArchivesType, BWAPI::Broodwar->self()->getUnits());

    // Si no tienes suficientes minerales o ya tienes una Cibernética Core, no hagas nada.
    if (minerals < TemplarArchivesType.mineralPrice() || CitadelOfAdunCount == 1) {
        return;
    }

    const bool startedBuilding = Tools::BuildBuilding(TemplarArchivesType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", TemplarArchivesType.getName());
    }
}

void StarterBot::buildPhotonCannon()
{
    /* const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType defenseType = BWAPI::Broodwar->self()->getRace().getDefenseBasic();
    // Para construir este edificio tengo que tener minerales suficientes
    // Obtener la cantidad de minerales
    const int minerals = BWAPI::Broodwar->self()->minerals();

    //const int requiredMinerales = (raceName = "Zerg" ? 200 : 150);
    const int requiredMinerales = defenseType.mineralPrice();
    // Obtener la cantidad de edificios hasta el momento.
    const int defenseOwned = Tools::CountUnitsOfType(defenseType, BWAPI::Broodwar->self()->getUnits());

    // Si no tengo suficientes minerales y ya tengo 3 edificios construidos, no hago nada.
    if (minerals < requiredMinerales or defenseOwned == 2) { return; }

    const bool startedBuilding = Tools::BuildBuilding(defenseType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", defenseType.getName());

    }*/
}

void StarterBot::buildForge()
{
    /*const std::string raceName = BWAPI::Broodwar->self()->getRace().getName();
    // Obtener el tipo de edificio (UnitType)
    const BWAPI::UnitType forgeType = BWAPI::Broodwar->self()->getRace().getFirstUpdate();
    // Para construir este edificio tengo que tener minerales suficientes
    // Obtener la cantidad de minerales
    const int minerals = BWAPI::Broodwar->self()->minerals();

    //const int requiredMinerales = (raceName = "Zerg" ? 200 : 150);
    const int requiredMinerales = forgeType.mineralPrice();
    // Obtener la cantidad de edificios hasta el momento.
    const int ForgeOwned = Tools::CountUnitsOfType(forgeType, BWAPI::Broodwar->self()->getUnits());

    // Si no tengo suficientes minerales y ya tengo 3 edificios construidos, no hago nada.
    if (minerals < requiredMinerales or ForgeOwned == 1) { return; }

    const bool startedBuilding = Tools::BuildBuilding(forgeType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", forgeType.getName());

    }*/
}

void StarterBot::AttackZealots()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

    static int zealotCount = 0; // Contador total de Zealots que han atacado
    static int aliveZealotCount = 0; // Contador de Zealots vivos en el grupo actual
    static int zealotsSentToAttack = 0; // Contador de Zealots enviados a atacar

    // Encuentra las coordenadas de inicio del enemigo (simétricas)
    static BWAPI::TilePosition tilestartLocation = BWAPI::Broodwar->self()->getStartLocation();
    static BWAPI::Position startLocation(tilestartLocation); // Posición de inicio
    static BWAPI::Position enemyBase;
    static bool enemyBaseSet = false;

    if (!enemyBaseSet) {
        double maxDistance = 0;
        double distance1 = startLocation.getDistance(BWAPI::Position(320, 3264));
        double distance2 = startLocation.getDistance(BWAPI::Position(3264, 320));

        if (distance1 > distance2) {
            enemyBase = BWAPI::Position(320, 3264);
        }
        else {
            enemyBase = BWAPI::Position(3264, 320);
        }
        enemyBaseSet = true;
    }

    // Contador de Zealots vivos en el grupo actual
    const BWAPI::UnitType zealotsType = BWAPI::Broodwar->self()->getRace().getInfanteryBasic();
    const int aliveZealotsInGroup = Tools::CountUnitsOfType(zealotsType, BWAPI::Broodwar->self()->getUnits());

    // Realiza el movimiento y el ataque
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot)
        {
            // Verifica si hay enemigos en el camino hacia la posición
            const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
            BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);

            if (closestEnemy) {
                unit->attack(closestEnemy, true);
            }
            else {
                unit->attack(enemyBase, true);
            }

        }
    }

    // Si no hay suficientes Zealots vivos en el grupo, agrúpalos en tu base
    if (aliveZealotsInGroup < 10) {
        for (auto& unit : myUnits)
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot) {

                if (!unit->isAttacking()) {
                    BWAPI::TilePosition myBaseTile = BWAPI::Broodwar->self()->getStartLocation();
                    unit->rightClick(BWAPI::Position(myBaseTile));;
                }
                else 
                {
                    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
					BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);
                    if (closestEnemy) {
						unit->attack(closestEnemy, true);
					}

                }
            } 
        }
    }
}

void StarterBot::AttackDragoons()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

    static int dragoonCount = 0; // Contador total de Zealots que han atacado
    static int aliveDragoonCount = 0; // Contador de Zealots vivos en el grupo actual
    static int DragoonsSentToAttack = 0; // Contador de Zealots enviados a atacar

    // Encuentra las coordenadas de inicio del enemigo (simétricas)
    static BWAPI::TilePosition tilestartLocation = BWAPI::Broodwar->self()->getStartLocation();
    static BWAPI::Position startLocation(tilestartLocation); // Posición de inicio
    static BWAPI::Position enemyBase;
    static bool enemyBaseSet = false;

    if (!enemyBaseSet) {
        double maxDistance = 0;
        double distance1 = startLocation.getDistance(BWAPI::Position(320, 3264));
        double distance2 = startLocation.getDistance(BWAPI::Position(3264, 320));

        if (distance1 > distance2) {
            enemyBase = BWAPI::Position(320, 3264);
        }
        else {
            enemyBase = BWAPI::Position(3264, 320);
        }
        enemyBaseSet = true;
    }

    // Contador de Zealots vivos en el grupo actual
    const BWAPI::UnitType dragoonsType = BWAPI::Broodwar->self()->getRace().getSecondInfantery();
    const int aliveDragoonsInGroup = Tools::CountUnitsOfType(dragoonsType, BWAPI::Broodwar->self()->getUnits());

    // Realiza el movimiento y el ataque
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
        {
            // Verifica si hay enemigos en el camino hacia la posición
            const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
            BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);

            if (closestEnemy) {
                unit->attack(closestEnemy, true);
            }
            else {
                unit->attack(enemyBase, true);
            }
 
        }
    }

    // Si no hay suficientes Zealots vivos en el grupo, agrúpalos en tu base
    if (aliveDragoonsInGroup < 10) {
        for (auto& unit : myUnits)
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon) {
                
                if (!unit->isAttacking()) {
                    BWAPI::TilePosition myBaseTile = BWAPI::Broodwar->self()->getStartLocation();
                    unit->rightClick(BWAPI::Position(myBaseTile));;
                }
                else
                {
                    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
                    BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);
                    if (closestEnemy) {
                        unit->attack(closestEnemy, true);
                    }

                }
            }
            
        }
    }
}

void StarterBot::AttackTemplars()
{
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();

    static int templarCount = 0; // Contador total de Zealots que han atacado
    static int aliveTemplarCount = 0; // Contador de Zealots vivos en el grupo actual
    static int TemplarsSentToAttack = 0; // Contador de Zealots enviados a atacar

    // Encuentra las coordenadas de inicio del enemigo (simétricas)
    static BWAPI::TilePosition tilestartLocation = BWAPI::Broodwar->self()->getStartLocation();
    static BWAPI::Position startLocation(tilestartLocation); // Posición de inicio
    static BWAPI::Position enemyBase;
    static bool enemyBaseSet = false;

    if (!enemyBaseSet) {
        double maxDistance = 0;
        double distance1 = startLocation.getDistance(BWAPI::Position(320, 3264));
        double distance2 = startLocation.getDistance(BWAPI::Position(3264, 320));

        if (distance1 > distance2) {
            enemyBase = BWAPI::Position(320, 3264);
        }
        else {
            enemyBase = BWAPI::Position(3264, 320);
        }
        enemyBaseSet = true;
    }

    // Contador de Zealots vivos en el grupo actual
    const BWAPI::UnitType templarsType = BWAPI::Broodwar->self()->getRace().getThreeInfantery();
    const int aliveTemplarsInGroup = Tools::CountUnitsOfType(templarsType, BWAPI::Broodwar->self()->getUnits());

    // Realiza el movimiento y el ataque
    for (auto& unit : myUnits)
    {
        if (unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar)
        {
            // Verifica si hay enemigos en el camino hacia la posición
            const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
            BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);

            if (closestEnemy) {
                unit->attack(closestEnemy, true);
            }
            else {
                unit->attack(enemyBase, true);
            }

        }
    }

    // Si no hay suficientes Zealots vivos en el grupo, agrúpalos en tu base
    if (aliveTemplarsInGroup < 2) {
        for (auto& unit : myUnits)
        {
            if (unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar) {

                if (!unit->isAttacking()) {
                    BWAPI::TilePosition myBaseTile = BWAPI::Broodwar->self()->getStartLocation();
                    unit->rightClick(BWAPI::Position(myBaseTile));;
                }
                else
                {
                    const BWAPI::Unitset& enemyUnits = BWAPI::Broodwar->enemy()->getUnits();
                    BWAPI::Unit closestEnemy = Tools::GetClosestUnitTo(unit, enemyUnits);
                    if (closestEnemy) {
                        unit->attack(closestEnemy, true);
                    }

                }
            }

        }
    }
}

void StarterBot::ResetAttackLogicIfNeeded()
{
    // Definir una ubicación específica de la base enemiga a la que deseas enviar tus unidades
    // Encuentra las coordenadas de inicio del enemigo (simétricas)
    static BWAPI::TilePosition tilestartLocation = BWAPI::Broodwar->self()->getStartLocation();
    static BWAPI::Position startLocation(tilestartLocation); // Posición de inicio
    static BWAPI::Position enemyBase;
    static bool enemyBaseSet = false;

    if (!enemyBaseSet) {
        double maxDistance = 0;
        double distance1 = startLocation.getDistance(BWAPI::Position(320, 3264));
        double distance2 = startLocation.getDistance(BWAPI::Position(3264, 320));

        if (distance1 > distance2) {
            enemyBase = BWAPI::Position(320, 3264);
        }
        else {
            enemyBase = BWAPI::Position(3264, 320);
        }
        enemyBaseSet = true;
    }

    const BWAPI::UnitType zealotsType = BWAPI::Broodwar->self()->getRace().getInfanteryBasic();
    const int aliveZealots = Tools::CountUnitsOfType(zealotsType, BWAPI::Broodwar->self()->getUnits());

    const BWAPI::UnitType dragoonsType = BWAPI::Broodwar->self()->getRace().getSecondInfantery();
    const int aliveDragoons = Tools::CountUnitsOfType(dragoonsType, BWAPI::Broodwar->self()->getUnits());

    const BWAPI::UnitType templarsType = BWAPI::Broodwar->self()->getRace().getThreeInfantery();
    const int aliveTemplars = Tools::CountUnitsOfType(templarsType, BWAPI::Broodwar->self()->getUnits());
    // Si tienes 20 o más unidades entre Zealots y Dragoons y no se ha dado la orden,
    // envía las unidades a la ubicación de la base enemiga y marca la bandera como verdadera
    if (aliveZealots + aliveDragoons + aliveTemplars >= 20)
    {
        // Mueve las unidades a la ubicación de la base enemiga
        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (unit->getType() == zealotsType || unit->getType() == dragoonsType || unit->getType() == templarsType)
            {
                unit->attack(enemyBase);
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
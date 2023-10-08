#include <string>
#include <BWAPI/Race.h>
#include <BWAPI/UnitType.h>

#include <Debug.h>

namespace BWAPI
{
  // NAMES
  template <>
  const std::string Type<Race, Races::Enum::Unknown>::typeNames[Races::Enum::MAX] =
  {
    "Zerg", "Terran", "Protoss",
    "Other", "Unused", "Select",
    "Random", "None", "Unknown"
  };
  
  // (local scope)
  namespace RaceInternal
  {
    using namespace UnitTypes::Enum;
  
    // LOCALIZATION
    std::string raceLocalNames[Races::Enum::MAX];

    // WORKER TYPES
    static const int workerTypes[Races::Enum::MAX] =
    {
      Zerg_Drone, Terran_SCV, Protoss_Probe, 
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // BASE TYPES
    static const int baseTypes[Races::Enum::MAX] =
    {
      Zerg_Hatchery, Terran_Command_Center, Protoss_Nexus,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // REFINERY TYPES
    static const int refineryTypes[Races::Enum::MAX] =
    {
      Zerg_Extractor, Terran_Refinery, Protoss_Assimilator,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // INFANTERY basic TYPES
    static const int infanteryTypes[Races::Enum::MAX] =
    {
      Zerg_Zergling, Terran_Marine, Protoss_Zealot,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // DEFENSE basic TYPES
    static const int defenseTypes[Races::Enum::MAX] =
    {
     Zerg_Sunken_Colony, Terran_Bunker, Protoss_Photon_Cannon,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk0
    };

    // Second Infantery TYPES
    static const int dragonTypes[Races::Enum::MAX] = 
    {
		Zerg_Hydralisk, Terran_Firebat, Protoss_Dragoon,
		None, None, None, // unused
		Unknown, None, Unknown // random, none, unk
	};


    // BASIC ARMY BUILDING TYPES
    static const int basicArmyBuildingTypes[Races::Enum::MAX] =
    {
      Zerg_Spawning_Pool, Terran_Barracks, Protoss_Gateway,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // TRANSPORT TYPES
    static const int transportTypes[Races::Enum::MAX] =
    {
      Zerg_Overlord, Terran_Dropship, Protoss_Shuttle,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // SUPPLY TYPES
    static const int supplyTypes[Races::Enum::MAX] =
    {
      Zerg_Overlord, Terran_Supply_Depot, Protoss_Pylon,
      None, None, None, // unused
      Unknown, None, Unknown // random, none, unk
    };

    // UPDATE BASIC TYPES
    static const int updateBasicTypes[Races::Enum::MAX] = 
    {
	  Zerg_Lair, Terran_Academy, Protoss_Cybernetics_Core,
	  None, None, None, // unused
	  Unknown, None, Unknown // random, none, unk
	};
  };// end local scope

  namespace RaceSet
  {
    using namespace Races::Enum;
    const Race::set raceSet = { Zerg, Terran, Protoss, None, Unknown };
  }
  UnitType Race::getWorker() const
  {
    return RaceInternal::workerTypes[this->getID()];
  }
  UnitType Race::getResourceDepot() const
  {
    return RaceInternal::baseTypes[this->getID()];
  }
  UnitType Race::getInfanteryBasic() const
  {
      return RaceInternal::infanteryTypes[this->getID()];
  }
  UnitType Race::getDefenseBasic() const
  {
      return RaceInternal::defenseTypes[this->getID()];
  }
  UnitType Race::getSecondInfantery() const
  {
      return RaceInternal::dragonTypes[this->getID()];
  }
  UnitType Race::getBasicArmyBuilding() const
  {
      return RaceInternal::basicArmyBuildingTypes[this->getID()];
  }
  UnitType Race::getUpdateBasic() const
  {
	  return RaceInternal::updateBasicTypes[this->getID()];
  }
  UnitType Race::getCenter() const
  {
    return getResourceDepot();
  }
  UnitType Race::getRefinery() const
  {
    return RaceInternal::refineryTypes[this->getID()];
  }
  UnitType Race::getTransport() const
  {
    return RaceInternal::transportTypes[this->getID()];
  }
  UnitType Race::getSupplyProvider() const
  {
    return RaceInternal::supplyTypes[this->getID()];
  }
  const Race::set& Races::allRaces()
  {
    return RaceSet::raceSet;
  }
}

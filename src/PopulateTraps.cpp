#include "PopulateTraps.h"

#include "Engine.h"
#include "Map.h"
#include "MapGen.h"
#include "FeatureTrap.h"
#include "FeatureFactory.h"
#include "MapParsing.h"
#include "Utils.h"

void PopulateTraps::populateRoomAndCorridorLevel(
  RoomThemeId themeMap[MAP_W][MAP_H],
  const vector<Room*>& rooms) const {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false, eng), blockers);

  //Put traps in non-plain rooms
  for(unsigned int i = 0; i < rooms.size(); i++) {
    Room* const room = rooms.at(i);
    const RoomThemeId theme = room->roomTheme;

    if(theme != roomTheme_plain) {

      int chanceForTrappedRoom = 0;

      switch(theme) {
        case roomTheme_human:    chanceForTrappedRoom = 25;  break;
        case roomTheme_ritual:   chanceForTrappedRoom = 25;  break;
        case roomTheme_spider:   chanceForTrappedRoom = 75;  break;
        case roomTheme_crypt:    chanceForTrappedRoom = 75;  break;
        case roomTheme_monster:  chanceForTrappedRoom = 25;  break;
        case roomTheme_plain:                                break;
//        case roomTheme_dungeon:                              break;
        case roomTheme_flooded:                              break;
        case roomTheme_muddy:                                break;
//        case roomTheme_chasm:                                break;
        case endOfRoomThemes:                                break;
      }

      if(Rnd::range(1, 100) < chanceForTrappedRoom) {

        vector<Pos> trapPositionCandidates;

        const Pos& x0y0 = room->getX0Y0();
        const Pos& x1y1 = room->getX1Y1();
        for(int y = x0y0.y; y <= x1y1.y; y++) {
          for(int x = x0y0.x; x <= x1y1.x; x++) {
            if(blockers[x][y] == false) {
              if(eng.map->cells[x][y].featureStatic->canHaveStaticFeature()) {
                trapPositionCandidates.push_back(Pos(x, y));
              }
            }
          }
        }

        const int NR_POS_CAND =
          int(trapPositionCandidates.size());
        if(NR_POS_CAND > 0) {
          const int MIN_NR_TRAPS =
            min(NR_POS_CAND / 4, theme == roomTheme_spider ? 4 : 1);
          const int MAX_NR_TRAPS =
            min(NR_POS_CAND / 2, theme == roomTheme_spider ? 8 : 2);
          const int NR_TRAPS = Rnd::range(MIN_NR_TRAPS, MAX_NR_TRAPS);
          for(int i_trap = 0; i_trap < NR_TRAPS; i_trap++) {
            const unsigned int CANDIDATE_ELEMENT =
              Rnd::range(0, trapPositionCandidates.size() - 1);
            const Pos& pos = trapPositionCandidates.at(CANDIDATE_ELEMENT);
            const TrapId trapType =
              theme == roomTheme_spider ? trap_spiderWeb : trap_any;
            FeatureStatic* const f =
              eng.map->cells[pos.x][pos.y].featureStatic;
            const FeatureData* const dataAtTrap =
              eng.featureDataHandler->getData(f->getId());
            eng.featureFactory->spawnFeatureAt(
              feature_trap, pos, new TrapSpawnData(dataAtTrap, trapType));
            trapPositionCandidates.erase(
              trapPositionCandidates.begin() + CANDIDATE_ELEMENT);
            blockers[pos.x][pos.y] = true;
          }
        }
      }
    }
  }

  const int CHANCE_FOR_ALLOW_TRAPPED_PLAIN_AREAS =
    min(85, 30 + (eng.map->getDlvl() * 5));
  if(Rnd::percentile() < CHANCE_FOR_ALLOW_TRAPPED_PLAIN_AREAS) {
    vector<Pos> trapPositionCandidates;
    for(int y = 1; y < MAP_H - 1; y++) {
      for(int x = 1; x < MAP_W - 1; x++) {
        if(blockers[x][y] == false && themeMap[x][y] == roomTheme_plain) {
          trapPositionCandidates.push_back(Pos(x, y));
        }
      }
    }
    const int NR_POS_CAND = int(trapPositionCandidates.size());
    if(NR_POS_CAND > 0) {
      const int NR_TRAPS = min(NR_POS_CAND, Rnd::range(5, 9));
      for(int i_trap = 0; i_trap < NR_TRAPS; i_trap++) {
        const unsigned int CANDIDATE_ELEMENT =
          Rnd::range(0, trapPositionCandidates.size() - 1);
        const Pos& pos = trapPositionCandidates.at(CANDIDATE_ELEMENT);
        FeatureStatic* const f =
          eng.map->cells[pos.x][pos.y].featureStatic;
        const FeatureData* const dataAtTrap =
          eng.featureDataHandler->getData(f->getId());
        eng.featureFactory->spawnFeatureAt(
          feature_trap, pos, new TrapSpawnData(dataAtTrap, trap_any));
        trapPositionCandidates.erase(
          trapPositionCandidates.begin() + CANDIDATE_ELEMENT);
      }
    }
  }
}

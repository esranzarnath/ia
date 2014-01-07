#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "Actor.h"
#include "CommonData.h"

const int MIN_SHOCK_WHEN_OBSESSION = 35;

enum InsanityPhobia_t {
  insanityPhobia_rat,
  insanityPhobia_spider,
  insanityPhobia_dog,
  insanityPhobia_undead,
  insanityPhobia_openPlace,
  insanityPhobia_closedPlace,
  insanityPhobia_deepPlaces,
  endOfInsanityPhobias
};

enum InsanityObsession_t {
  insanityObsession_sadism,
  insanityObsession_masochism,
  endOfInsanityObsessions
};

class Monster;
class MedicalBag;

class Player: public Actor {
public:
  Player(Engine& eng);
  virtual ~Player() {}

  void updateFov();

  void moveDir(Dir_t dir);

  void specific_init() {}

  void specificSpawnStartItems() override;

  void onActorTurn();

  void specificOnStandardTurn();

  void hearSound(const Sound& snd, const bool IS_ORIGIN_SEEN_BY_PLAYER,
                 const Dir_t dirToOrigin,
                 const int PERCENT_AUDIBLE_DISTANCE);

  void explosiveThrown();

  MedicalBag* activeMedicalBag;
  int waitTurnsLeft;

  void incrShock(const ShockValues_t shockValue);
  void incrShock(const int VAL);
  void restoreShock(const int amountRestored,
                    const bool IS_TEMP_SHOCK_RESTORED);
  int getShockTotal() {return int(floor(shock_ + shockTemp_));}
  int getShockTmp() {return shockTemp_;}
  int getInsanity() const {return min(100, insanity_);}
  void incrMth(const int VAL, const bool IS_MSG_ALLOWED);
  int getMth() const {return mth;}
  void setTempShockFromFeatures();

  int getShockResistance() const;

  int getCarryWeightLimit() const;

  int dynamiteFuseTurns;
  int molotovFuseTurns;
  int flareFuseTurns;

  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

  bool insanityPhobias[endOfInsanityPhobias];
  bool insanityObsessions[endOfInsanityObsessions];

  Actor* target;

  void autoMelee();

  void kick(Actor& actorToKick);
  void punch(Actor& actorToPunch);

  void updateColor();

  void specificAddLight(bool light[MAP_W][MAP_H]) const;

  void grantMthPower() const;

private:
  friend class DungeonMaster;
  friend class GameTime;
  friend class Log;

  void incrInsanity();
//  void tryIdentifyItems();
  void testPhobias();
  void specificHit(const int DMG, const bool ALLOW_WOUNDS);
  void FOVhack();
  void interruptActions();
  bool isStandingInOpenSpace() const;
  bool isStandingInCrampedSpace() const;

  int insanity_;
  double shock_, shockTemp_;

  int mth;

  int nrMovesUntilFreeAction;

  const int carryWeightBase;
};


#endif

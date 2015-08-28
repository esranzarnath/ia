#include "ai.hpp"

#include "actor_player.hpp"
#include "msg_log.hpp"
#include "map.hpp"
#include "utils.hpp"
#include "feature_mob.hpp"
#include "feature_door.hpp"
#include "actor_mon.hpp"
#include "line_calc.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "fov.hpp"

namespace ai
{

namespace action
{

bool try_cast_random_spell(Mon& mon)
{
    if (
        !mon.is_alive()                 ||
        mon.spell_cool_down_cur_ > 0    ||
        mon.spells_known_.empty()       ||
        !mon.prop_handler().allow_cast_spell(Verbosity::silent))
    {
        return false;
    }

    std::vector<Spell*> spell_bucket = mon.spells_known_;

    std::random_shuffle(begin(spell_bucket), end(spell_bucket));

    while (!spell_bucket.empty())
    {
        Spell* const spell = spell_bucket.back();

        if (spell->allow_mon_cast_now(mon))
        {
            const int CUR_SPI = mon.spi();
            const int SPELL_MAX_SPI = spell->spi_cost(false, &mon).max;

            //Cast spell if max spirit cost is lower than current spirit,
            if (SPELL_MAX_SPI < CUR_SPI)
            {
                spell->cast(&mon, true);
                return true;
            }

            //This point reached means SPI was lower than the spells potential cost
            const int CUR_HP  = mon.hp();
            const int MAX_HP  = mon.hp_max(true);

            //If monster is not allied to player, with a small chance, cast the spell
            //anyway if HP is low.
            if (
                !map::player->is_leader_of(&mon)    &&
                CUR_HP < (MAX_HP / 3)               &&
                rnd::one_in(20))
            {
                if (map::player->can_see_actor(mon))
                {
                    msg_log::add(mon.name_the() + " looks desperate.");
                }

                spell->cast(&mon, true);
                return true;
            }

            return false;
        }
        else //Spell does not allow monster to cast now
        {
            spell_bucket.pop_back();
        }
    }

    return false;
}

bool handle_closed_blocking_door(Mon& mon, std::vector<Pos> path)
{
    if (!mon.is_alive() || path.empty())
    {
        return false;
    }

    const Pos& p = path.back();
    Feature* const f = map::cells[p.x][p.y].rigid;

    if (f->id() == Feature_id::door)
    {
        Door* const door = static_cast<Door*>(f);

        if (!door->can_move(mon))
        {
            if (!door->is_stuck())
            {
                if (mon.data().can_open_doors)
                {
                    door->try_open(&mon);
                    return true;
                }
                else if (mon.data().can_bash_doors)
                {
                    door->hit(Dmg_type::physical, Dmg_method::kick, &mon);
                    return true;
                }
            }
            else if (mon.data().can_bash_doors)
            {
                door->hit(Dmg_type::physical, Dmg_method::kick, &mon);
                return true;
            }
        }
    }

    return false;
}

bool handle_inventory(Mon& mon)
{
    (void)mon;
    return false;
}

//Helper functions for make_room_for_friend()
namespace
{

//Check if acting monster is on a line between player and other monster
bool check_if_blocking_mon(const Pos& pos, Mon& other)
{
    std::vector<Pos> line;
    line_calc::calc_new_line(other.pos, map::player->pos, true, 9999, false, line);

    for (const Pos& pos_in_line : line) {if (pos_in_line == pos) {return true;}}

    return false;
}

//Returns all free positions around the acting monster that is further
//from the player than the monster's current position
void move_bucket(Mon& self, std::vector<Pos>& dirs_to_mk)
{

    dirs_to_mk.clear();

    const int PLAYER_X = map::player->pos.x;
    const int PLAYER_Y = map::player->pos.y;
    const int OLD_X = self.pos.x;
    const int OLD_Y = self.pos.y;

    bool blocked[MAP_W][MAP_H];
    map_parse::run(cell_check::Blocks_actor(self, true), blocked);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            if (x != OLD_X || y != OLD_Y)
            {
                const int NEW_X = OLD_X + x;
                const int NEW_Y = OLD_Y + y;
                const int DIST_CUR = utils::king_dist(OLD_X, OLD_Y, PLAYER_X, PLAYER_Y);
                const int DIST_NEW = utils::king_dist(NEW_X, NEW_Y, PLAYER_X, PLAYER_Y);

                if (DIST_NEW <= DIST_CUR)
                {
                    if (!blocked[NEW_X][NEW_Y]) {dirs_to_mk.push_back(Pos(NEW_X, NEW_Y));}
                }
            }
        }
    }
}

/*
 The purpose of the following test is to handle this situation:
 #####
 #.A.#
 #@#B#
 #####
 */
bool is_adj_and_no_vision(const Mon& self, Mon& other,
                          bool blocked_los[MAP_W][MAP_H])
{
    //If the pal is next to me
    if (utils::is_pos_adj(self.pos, other.pos, false))
    {
        //If pal does not see player
        if (!other.can_see_actor(*map::player, blocked_los)) {return true;}
    }

    return false;
}

} //namespace

bool make_room_for_friend(Mon& mon)
{
    if (mon.is_alive())
    {
        bool blocked_los[MAP_W][MAP_H];
        map_parse::run(cell_check::Blocks_los(), blocked_los);

        if (mon.can_see_actor(*map::player, blocked_los))
        {
            //Loop through all actors
            for (Actor* actor : game_time::actors_)
            {
                if (actor != map::player && actor != &mon && actor->is_alive())
                {
                    Mon* other = static_cast<Mon*>(actor);

                    bool is_other_adj_with_no_los = is_adj_and_no_vision(mon, *other, blocked_los);

                    //Other monster can see player, or it's an adjacent monster that
                    //does not see the player?
                    if (
                        other->can_see_actor(*map::player, blocked_los) ||
                        is_other_adj_with_no_los)
                    {
                        //If we are blocking a pal, check every neighbouring position
                        //that is at equal or closer distance to the player, to check
                        //whether they are fine.

                        //TODO: Vision must be checked from the cell candidates!

                        if (check_if_blocking_mon(mon.pos, *other) || is_other_adj_with_no_los)
                        {
                            // Get a list of neighbouring free cells
                            std::vector<Pos> pos_bucket;
                            move_bucket(mon, pos_bucket);

                            //Sort the list by closeness to player
                            Is_closer_to_pos cmp(map::player->pos);
                            sort(pos_bucket.begin(), pos_bucket.end(), cmp);

                            //Test positions until one is found that is not blocking
                            //another monster
                            for (const auto& tgt_pos : pos_bucket)
                            {
                                bool is_good_candidate_found = true;

                                for (Actor* actor2 : game_time::actors_)
                                {
                                    if (!actor2->is_player() && actor2 != &mon)
                                    {
                                        other = static_cast<Mon*>(actor2);

                                        const bool OTHER_IS_SEEING_PLAYER =
                                            other->can_see_actor(*map::player,
                                                                 blocked_los);

                                        if (
                                            OTHER_IS_SEEING_PLAYER &&
                                            check_if_blocking_mon(tgt_pos, *other))
                                        {
                                            is_good_candidate_found = false;
                                            break;
                                        }
                                    }
                                }

                                if (is_good_candidate_found)
                                {
                                    const Pos offset = tgt_pos - mon.pos;
                                    mon.move(dir_utils::dir(offset));
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

//Helper function for move_to_random_adj_cell()
namespace
{

Dir dir_to_rnd_adj_free_cell(Mon& mon)
{
    bool blocked[MAP_W][MAP_H];
    cell_check::Blocks_actor cellcheck(mon, true);

    const Pos& mon_pos = mon.pos;

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            const Pos p(mon_pos.x + dx, mon_pos.y + dy);
            blocked[p.x][p.y] = cellcheck.check(map::cells[p.x][p.y]);
        }
    }

    for (Actor* actor : game_time::actors_)
    {
        const Pos& p = actor->pos;
        blocked[p.x][p.y] = true;
    }

    for (Mob* mob : game_time::mobs_)
    {
        const Pos& p = mob->pos();
        blocked[p.x][p.y] = cellcheck.check(*mob);
    }

    const Rect area_allowed(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2));

    //First, try the same direction as last travelled
    const Dir last_dir_travelled = mon.last_dir_moved_;

    if (last_dir_travelled != Dir::center && last_dir_travelled != Dir::END)
    {
        const Pos tgt_cell(mon_pos + dir_utils::offset(last_dir_travelled));

        if (
            !blocked[tgt_cell.x][tgt_cell.y] &&
            utils::is_pos_inside(tgt_cell, area_allowed))
        {
            return last_dir_travelled;
        }
    }

    //Attempt to find a random non-blocked adjacent cell
    std::vector<Dir> dir_bucket;
    dir_bucket.clear();

    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx != 0 || dy != 0)
            {
                const Pos offset(dx, dy);
                const Pos tgt_cell(mon_pos + offset);

                if (
                    !blocked[tgt_cell.x][tgt_cell.y] &&
                    utils::is_pos_inside(tgt_cell, area_allowed))
                {
                    dir_bucket.push_back(dir_utils::dir(offset));
                }
            }
        }
    }

    const int NR_ELEMENTS = dir_bucket.size();

    if (NR_ELEMENTS == 0)
    {
        return Dir::center;
    }
    else
    {
        return dir_bucket[rnd::range(0, NR_ELEMENTS - 1)];
    }
}

} //namespace

bool move_to_random_adj_cell(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    if (mon.is_roaming_allowed_ || mon.aware_counter_ > 0)
    {
        const Dir dir = dir_to_rnd_adj_free_cell(mon);

        if (dir != Dir::center)
        {
            mon.move(dir);
            return true;
        }
    }

    return false;
}

bool move_to_tgt_simple(Mon& mon)
{
    if (!mon.is_alive() || !mon.tgt_)
    {
        return false;
    }

    if (mon.aware_counter_ > 0 || map::player->is_leader_of(&mon))
    {
        const Pos offset  = mon.tgt_->pos - mon.pos;
        const Pos signs   = offset.signs();

        const Pos new_pos(mon.pos + signs);

        const bool IS_BLOCKED = map_parse::cell(cell_check::Blocks_actor(mon, true), new_pos);

        if (!IS_BLOCKED)
        {
            mon.move(dir_utils::dir(signs));
            return true;
        }
    }

    return false;
}

bool step_path(Mon& mon, std::vector<Pos>& path)
{
    if (mon.is_alive())
    {
        if (!path.empty())
        {
            const Pos delta = path.back() - mon.pos;
            mon.move(dir_utils::dir(delta));
            return true;
        }
    }

    return false;
}

bool step_to_lair_if_los(Mon& mon, const Pos& lair_p)
{
    if (mon.is_alive())
    {
        bool blocked[MAP_W][MAP_H];

        map_parse::run(cell_check::Blocks_los(), blocked);

        const Los_result los = fov::check_cell(mon.pos, lair_p, blocked);

        if (!los.is_blocked_hard)
        {
            Pos delta = lair_p - mon.pos;

            delta.x = delta.x == 0 ? 0 : (delta.x > 0 ? 1 : -1);
            delta.y = delta.y == 0 ? 0 : (delta.y > 0 ? 1 : -1);

            const Pos new_pos = mon.pos + delta;

            map_parse::run(cell_check::Blocks_actor(mon, true), blocked);

            if (blocked[new_pos.x][new_pos.y])
            {
                return false;
            }
            else //Step is not blocked
            {
                mon.move(dir_utils::dir(delta));
                return true;
            }
        }
    }

    return false;
}

} //Action

namespace info
{

bool look_become_player_aware(Mon& mon)
{
    if (!mon.is_alive())
    {
        return false;
    }

    const bool WAS_AWARE_BEFORE = mon.aware_counter_ > 0;

    std::vector<Actor*> seen_foes;
    mon.seen_foes(seen_foes);

    if (!seen_foes.empty() && WAS_AWARE_BEFORE)
    {
        mon.become_aware(false);
        return false;
    }

    for (Actor* actor : seen_foes)
    {
        if (actor->is_player())
        {
            if (mon.is_spotting_sneaking_actor(*actor))
            {
                mon.become_aware(true);

                if (WAS_AWARE_BEFORE)
                {
                    return false;
                }
                else //Was not aware before
                {
                    game_time::tick();
                    return true;
                }
            }
        }
        else //Other actor is monster
        {
            mon.become_aware(false);

            if (WAS_AWARE_BEFORE)
            {
                return false;
            }
            else //Was not aware before
            {
                game_time::tick();
                return true;
            }
        }
    }

    return false;
}

void try_set_path_to_lair_if_no_los(Mon& mon, std::vector<Pos>& path,
                                    const Pos& lair_p)
{
    if (mon.is_alive())
    {
        bool blocked[MAP_W][MAP_H];

        const Rect fov_lmt = fov::get_fov_rect(mon.pos);

        map_parse::run(cell_check::Blocks_los(), blocked, Map_parse_mode::overwrite, fov_lmt);

        const Los_result los = fov::check_cell(mon.pos, lair_p, blocked);

        if (!los.is_blocked_hard)
        {
            path.clear();
            return;
        }

        map_parse::run(cell_check::Blocks_actor(mon, false), blocked);

        map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos),
                       blocked, Map_parse_mode::append);

        path_find::run(mon.pos, lair_p, blocked, path);
        return;
    }

    path.clear();
}

void try_set_path_to_leader(Mon& mon, std::vector<Pos>& path)
{
    if (mon.is_alive())
    {
        Actor* leader = mon.leader_;

        if (leader)
        {
            if (leader->is_alive())
            {
                bool blocked[MAP_W][MAP_H];

                const Rect fov_lmt = fov::get_fov_rect(mon.pos);

                map_parse::run(cell_check::Blocks_los(), blocked, Map_parse_mode::overwrite,
                               fov_lmt);

                const Los_result los = fov::check_cell(mon.pos, leader->pos, blocked);

                if (!los.is_blocked_hard)
                {
                    path.clear();
                    return;
                }

                map_parse::run(cell_check::Blocks_actor(mon, false), blocked);

                map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos),
                               blocked, Map_parse_mode::append);

                path_find::run(mon.pos, leader->pos, blocked, path);
                return;
            }
        }
    }

    path.clear();
}

void try_set_path_to_player(Mon& mon, std::vector<Pos>& path)
{
    if (!mon.is_alive() || mon.aware_counter_ <= 0)
    {
        path.clear();
        return;
    }

    bool blocked[MAP_W][MAP_H];
    utils::reset_array(blocked, false);

    const int X0 = 1;
    const int Y0 = 1;
    const int X1 = MAP_W - 1;
    const int Y1 = MAP_H - 1;

    //Mark blocking features in the blocking array
    for (int x = X0; x < X1; ++x)
    {
        for (int y = Y0; y < Y1; ++y)
        {
            const auto* const f = map::cells[x][y].rigid;

            if (!f->can_move(mon))
            {
                if (f->id() == Feature_id::door)
                {
                    //Mark doors as blocked depending on if the monster can bash or open doors,

                    const Actor_data_t& d = mon.data();

                    //TODO: What if there is a monster that can open doors but not bash them,
                    //and the door is stuck?

                    if (!d.can_open_doors && !d.can_bash_doors)
                    {
                        blocked[x][y] = true;
                    }
                }
                else //Not a door (e.g. a wall)
                {
                    blocked[x][y] = true;
                }
            }
        }
    }

    //If there is an unblocked LOS between the monster and the player we cancel the pathfinding.
    //The monster should not use the pathfinder to move towards the player in this case. If the
    //player is invisible for example, we DO want pathfinding as long as the monster is aware,
    //and is around corner (they are guided by sound or something else) - but when they come into
    //LOS of an invisible player, they should not approach further.
    //This creates a pretty cool effect, where monsters appear a bit confused that they cannot see
    //anyone when they should have come into sight.
    const Pos& player_pos = map::player->pos;

    Los_result los_result = fov::check_cell(mon.pos, player_pos, blocked);

    if (los_result.is_blocked_hard || los_result.is_blocked_by_drk)
    {
        //Append living adjacent actors to the blocking array
        map_parse::run(cell_check::Living_actors_adj_to_pos(mon.pos), blocked,
                       Map_parse_mode::append);

        //Find a path
        path_find::run(mon.pos, player_pos, blocked, path);
    }
}

void set_special_blocked_cells(Mon& mon, bool a[MAP_W][MAP_H])
{
    (void)mon;
    (void)a;
}

} //Info

} //Ai


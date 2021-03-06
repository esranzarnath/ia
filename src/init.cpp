#include "init.hpp"

#include "player_bon.hpp"
#include "sdl_wrapper.hpp"
#include "config.hpp"
#include "input.hpp"
#include "render.hpp"
#include "audio.hpp"
#include "line_calc.hpp"
#include "gods.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "map.hpp"
#include "msg_log.hpp"
#include "dungeon_master.hpp"
#include "bot.hpp"
#include "manual.hpp"
#include "player_spells_handling.hpp"
#include "credits.hpp"
#include "map_templates.hpp"
#include "map_travel.hpp"
#include "query.hpp"
#include "item_jewelry.hpp"

using namespace std;

namespace init
{

bool is_cheat_vision_enabled = false;
bool quit_to_main_menu       = false;

//NOTE: Initialization order matters in some cases
void init_iO()
{
    TRACE_FUNC_BEGIN;
    sdl_wrapper::init();
    config::init();
    input::init();
    query::init();
    render::init();
    audio::init();
    TRACE_FUNC_END;
}

void cleanup_iO()
{
    TRACE_FUNC_BEGIN;
    audio::cleanup();
    render::cleanup();
    input::cleanup();
    sdl_wrapper::cleanup();
    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void init_game()
{
    TRACE_FUNC_BEGIN;
    line_calc::init();
    gods::init();
    manual::init();
    credits::init();
    map_templ_handling::init();
    TRACE_FUNC_END;
}

void cleanup_game()
{
    TRACE_FUNC_BEGIN;

    TRACE_FUNC_END;
}

//NOTE: Initialization order matters in some cases
void init_session()
{
    TRACE_FUNC_BEGIN;
    actor_data::init();
    feature_data::init();
    prop_data::init();
    item_data::init();
    scroll_handling::init();
    potion_handling::init();
    inv_handling::init();
    game_time::init();
    map_travel::init();
    map::init();
    player_bon::init();
    msg_log::init();
    dungeon_master::init();
    bot::init();
    player_spells_handling::init();
    jewelry_handling::init();
    TRACE_FUNC_END;
}

void cleanup_session()
{
    TRACE_FUNC_BEGIN;
    player_spells_handling::cleanup();
    map::cleanup();
    game_time::cleanup();
    item_data::cleanup();
    TRACE_FUNC_END;
}

} //Init

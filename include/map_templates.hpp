#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>

#include "feature_data.hpp"
#include "item_data.hpp"
#include "actor_data.hpp"

enum class Map_templ_id
{
    church,
    egypt,
    leng,
    rats_in_the_walls,
    boss_level,
    trapezohedron_level,
    END
};

struct map_templ_cell
{
    map_templ_cell(Feature_id feature_id_ = Feature_id::END,
                   Actor_id actor_id_     = Actor_id::END,
                   Item_id item_id_       = Item_id::END,
                   int val_             = 0) :
        feature_id(feature_id_),
        actor_id(actor_id_),
        item_id(item_id_),
        val(val_) {}

    Feature_id feature_id;
    Actor_id   actor_id;
    Item_id    item_id;
    int       val; //Can be used for different things depending on context
};

struct map_templ
{
public:
    map_templ() {cells_.clear();}

    const map_templ_cell& cell(const int X, const int Y) const {return cells_[Y][X];}

    void add_row(std::vector<map_templ_cell>& row) {cells_.push_back(row);}

    Pos dims() const {return Pos(cells_.back().size(), cells_.size());}

private:
    std::vector< std::vector<map_templ_cell> > cells_;
};

namespace map_templ_handling
{

void init();

const map_templ& templ(const Map_templ_id id);

} //map_templ_handling

#endif

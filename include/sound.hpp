#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "cmn_data.hpp"
#include "item_data.hpp"
#include "cmn_types.hpp"

class Actor;

enum class Snd_vol                      {low, high};
enum class Alerts_mon                   {no, yes};
enum class Ignore_msg_if_origin_seen    {no, yes};

class Snd
{
public:
    Snd(const std::string&              msg,
        const Sfx_id                    sfx,
        const Ignore_msg_if_origin_seen ignore_msg_if_origin_seen,
        const Pos&                      origin,
        Actor* const                    actor_who_made_sound,
        const Snd_vol                   vol,
        const Alerts_mon                alerting_mon,
        const More_prompt_on_msg        add_more_prompt_on_msg = More_prompt_on_msg::no);

    Snd() {}
    ~Snd() {}

    const std::string& msg() const
    {
        return msg_;
    }

    Sfx_id sfx() const
    {
        return sfx_;
    }

    void clear_msg()
    {
        msg_ = "";
    }

    bool is_msg_ignored_if_origin_seen() const
    {
        return is_msg_ignored_if_origin_seen_ == Ignore_msg_if_origin_seen::yes;
    }

    bool is_alerting_mon() const
    {
        return is_alerting_mon_ == Alerts_mon::yes;
    }

    More_prompt_on_msg should_add_more_prompt_on_msg() const
    {
        return add_more_prompt_on_msg_;
    }

    Pos origin() const
    {
        return origin_;
    }

    Actor* actor_who_made_sound() const
    {
        return actor_who_made_sound_;
    }

    int is_loud() const
    {
        return vol_ == Snd_vol::high;
    }

    void add_string(const std::string& str)
    {
        msg_ += str;
    }

private:
    std::string msg_;
    Sfx_id sfx_;
    Ignore_msg_if_origin_seen is_msg_ignored_if_origin_seen_;
    Pos origin_;
    Actor* actor_who_made_sound_;
    Snd_vol vol_;
    Alerts_mon is_alerting_mon_;
    More_prompt_on_msg add_more_prompt_on_msg_;
};

namespace snd_emit
{

void emit_snd(Snd snd);

void reset_nr_snd_msg_printed_cur_turn();

} //snd_emit

#endif


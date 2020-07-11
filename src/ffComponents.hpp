// #include "plugin.hpp"
// #include <rack.hpp>

extern Plugin *pluginInstance;

// Switches
struct HCKSS : app::SvgSwitch
{
    HCKSS()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_1.svg")));
    }
};

struct FFDPW : app::SvgSwitch
{
    FFDPW()
    {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FFDPW_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FFDPW_1.svg")));
    }
};

// Knobs
struct FF06BKnob : RoundKnob
{
    FF06BKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF06B.svg")));
    }
};
struct FF06GKnob : RoundKnob
{
    FF06GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF06G.svg")));
    }
};
struct FF08GKnob : RoundKnob
{
    FF08GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF08G.svg")));
    }
};
struct FF08GSnapKnob : FF08GKnob
{
    FF08GSnapKnob()
    {
        snap = true;
    }
};
struct FF10BKnob : RoundKnob
{
    FF10BKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF10B.svg")));
    }
};
struct FF10GKnob : RoundKnob
{
    FF10GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF10G.svg")));
    }
};
struct FF10GSnapKnob : FF10GKnob
{
    FF10GSnapKnob()
    {
        snap = true;
    }
};
struct FF15GKnob : RoundKnob
{
    FF15GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF15G.svg")));
    }
};
struct FF15GSnapKnob : FF15GKnob
{
    FF15GSnapKnob()
    {
        snap = true;
    }
};
struct FF20GKnob : RoundKnob
{
    FF20GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF20G.svg")));
    }
};

// Port
struct FF01JKPort : app::SvgPort
{
    FF01JKPort()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF01JK.svg")));
    }
};

// Misc
struct FFHexScrew : app::SvgScrew
{
    FFHexScrew()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FFHexScrew.svg")));
    }
};
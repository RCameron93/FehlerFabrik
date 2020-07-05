// #include "plugin.hpp"
// #include <rack.hpp>

extern Plugin *pluginInstance;

// Switch
struct HCKSS : app::SvgSwitch
{
    HCKSS()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_1.svg")));
    }
};

// Knobs
struct FF08GKnob : RoundKnob
{
    FF08GKnob()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/FF08GK.svg")));
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
#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;

extern Model *modelPSIOP;
extern Model *modelPlanck;
extern Model *modelLuigi;
extern Model *modelAspect;
extern Model *modelMonte;
extern Model *modelArpanet;
extern Model *modelSigma;

struct HCKSS : app::SvgSwitch
{
    HCKSS()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/HCKSS_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ComponentLibrary/HCKSS_1.svg")));
    }
};
#pragma once
#include <rack.hpp>

using namespace rack;

#include "ffComponents.hpp"

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
extern Model *modelFax;
extern Model *modelSlip;

// Components
// SHOULD BE UPDATED TO OWN AT SOME POINT
// struct HCKSS : app::SvgSwitch
// {
//     HCKSS()
//     {
//         addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_0.svg")));
//         addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/HCKSS_1.svg")));
//     }
// };

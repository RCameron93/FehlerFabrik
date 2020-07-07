// Title font - Fruktur Reqular
// https://fonts.google.com/specimen/Fruktur
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "common.hpp"

struct Planck : Module
{
    enum ParamIds
    {
        DEPTH_PARAM,
        RATE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        QUANT_INPUT,
        DEPTH_INPUT,
        CRUSH_INPUT,
        RATE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        QUANT_OUTPUT,
        CRUSH_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    BitDepthReducer reducer;
    SampleRateCrusher crusher;

    Planck()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(DEPTH_PARAM, 1.f, 16.f, 16.f, "");
        configParam(RATE_PARAM, 0.f, 100.f, 0.f, "");
    }

    void process(const ProcessArgs &args) override
    {
        // Bit depth reduction
        // Get parameters and input
        float depthIn = inputs[QUANT_INPUT].getVoltage();

        // TODO
        // ADD OVERLOAD LIGHT IF |INPUT| > 5V

        int depthAmount = (int)params[DEPTH_PARAM].getValue();
        // 1V corresponds to 2^n - 1 bit depth
        depthAmount -= (int)2 * inputs[DEPTH_INPUT].getVoltage();
        depthAmount = clamp(depthAmount, 1, 16);
        // Perform bit depth reduction
        // Clamps input to 10v pp / +-5v
        float depthOut = reducer.process(depthIn, depthAmount, 10.f);
        outputs[QUANT_OUTPUT].setVoltage(depthOut);

        // Sample rate crushing
        // Get parameters and input
        // Normalised to depth reduction output
        float crushIn = inputs[CRUSH_INPUT].getNormalVoltage(depthOut);
        int crushAmount = (int)params[RATE_PARAM].getValue();
        crushAmount += (int)10 * inputs[RATE_INPUT].getVoltage();
        crushAmount = clamp(crushAmount, 0, 100);
        // Perfrom sample rate crushing
        crusher.process(crushAmount, crushIn);
        float crushOut = crusher.out;
        outputs[CRUSH_OUTPUT].setVoltage(crushOut);
    }
};

struct PlanckWidget : ModuleWidget
{
    PlanckWidget(Planck *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Planck.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<FF15GSnapKnob>(mm2px(Vec(10.971, 58.109)), module, Planck::DEPTH_PARAM));
        addParam(createParamCentered<FF15GKnob>(mm2px(Vec(29.737, 36.251)), module, Planck::RATE_PARAM));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(30.757, 100.434)), module, Planck::CRUSH_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(30.757, 87.594)), module, Planck::RATE_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(9.843, 100.434)), module, Planck::QUANT_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(9.843, 87.594)), module, Planck::DEPTH_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(30.757, 113.225)), module, Planck::CRUSH_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(9.843, 113.225)), module, Planck::QUANT_OUTPUT));
    }
};

Model *modelPlanck = createModel<Planck, PlanckWidget>("Planck");
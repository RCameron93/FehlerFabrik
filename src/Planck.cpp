// Title font - Fruktur Reqular
// https://fonts.google.com/specimen/Fruktur
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
// #include "common.hpp"

struct BitDepthReducer
{
    // Powers of 2, minus 1
    float powers[16] = {1.f, 3.f, 7.f, 15.f, 31.f, 63.f, 127.f, 255.f, 511.f, 1023.f, 2047.f, 4095.f, 8191.f, 16383.f, 32767.f, 65535.f};

    float process(float in, int depth, float range)
    {
        // Quantises a voltage signal of "range" volts peak to peak (eg 10 volts) to a given bit depth
        float maxVolts = range / 2.f;
        // Clamp incoming signal
        in = clamp(in, -(maxVolts), (maxVolts));
        // Offset input by eg 5v so we're dealing with a number between 0v and 10v
        in += maxVolts;
        // How many possible values we have
        float steps = powers[depth - 1];
        // The step size of each of those values
        float stepSize = range / steps;
        // Quantise
        float out = round(in / stepSize) * stepSize;
        // Remove offset
        out -= maxVolts;
        return out;
    }

    // Process between -1V/+1V at 12 bit
    // Same idea I just wanted to have a streamlined version for a plugin that only uses 12bit reduction
    // Probably not any apreciable decrease in time but hey ho
    float process12bit(float in)
    {
        // in = clamp(in, -1.f, 1.f);
        in += 1.f;
        float stepSize = 0.0004884004884004884f;
        float out = int(in / stepSize) * stepSize;
        out -= 1.0f;
        return out;
    }
};

struct SampleRateCrusher
{
    float out = 0.f;
    int counter = 0;

    void process(int n, float in)
    {
        // Hold every Nth sample
        if (counter < n)
        {
            counter++;
        }
        else
        {
            counter = 0;
            out = in;
        }
    }
};

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
        DEPTH_INPUT,
        DEPTH_AMT_INPUT,
        CRUSH_INPUT,
        RATE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        DEPTH_OUTPUT,
        CRUSH_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    BitDepthReducer reducers[16];
    SampleRateCrusher crushers[16];

    Planck()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(DEPTH_PARAM, 1.f, 16.f, 16.f, "");
        configParam(RATE_PARAM, 0.f, 100.f, 0.f, "");
    }

    void process(const ProcessArgs &args) override
    {
        // Bit depth reduction
        // Seperate amount of channels for the depth reducer and the rate crusher
        int depthChannels = std::max(inputs[DEPTH_INPUT].getChannels(), 1);
        // Get knob value
        int globalDepthAmount = (int)params[DEPTH_PARAM].getValue();
        // Array to hold output values, will be normalled to the rate crusher input
        float depthOuts[16] = {0.f};

        for (int c = 0; c < depthChannels; ++c)
        {
            float depthIn = inputs[DEPTH_INPUT].getPolyVoltage(c);

            // 1V corresponds to 2^n - 1 bit depth
            int depthAmount = globalDepthAmount;
            depthAmount -= (int)2 * inputs[DEPTH_AMT_INPUT].getPolyVoltage(c);
            depthAmount = clamp(depthAmount, 1, 16);
            // Perform bit depth reduction
            // Clamps input to 10v pp / +-5v
            depthOuts[c] = reducers[c].process(depthIn, depthAmount, 10.f);
            outputs[DEPTH_OUTPUT].setVoltage(depthOuts[c], c);
        }
        outputs[DEPTH_OUTPUT].setChannels(depthChannels);

        // Sample rate crushing
        // Get number of channels
        int crushChannels = inputs[CRUSH_INPUT].getChannels();
        // Get knob value
        int globalCrushAmount = (int)params[RATE_PARAM].getValue();
        // If there's nothing connected we set the number of channels to be the same as the depth reducer
        if (crushChannels == 0)
            crushChannels = depthChannels;

        for (int c = 0; c < crushChannels; ++c)
        {
            // Get parameters and input
            // Normalised to depth reduction output
            float crushIn = inputs[CRUSH_INPUT].getNormalPolyVoltage(depthOuts[c], c);

            int crushAmount = globalCrushAmount;
            crushAmount += (int)10 * inputs[RATE_INPUT].getPolyVoltage(c);
            crushAmount = clamp(crushAmount, 0, 100);
            // Perfrom sample rate crushing
            crushers[c].process(crushAmount, crushIn);
            float crushOut = crushers[c].out;
            outputs[CRUSH_OUTPUT].setVoltage(crushOut, c);
        }
        outputs[CRUSH_OUTPUT].setChannels(crushChannels);
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
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(9.843, 100.434)), module, Planck::DEPTH_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(9.843, 87.594)), module, Planck::DEPTH_AMT_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(30.757, 113.225)), module, Planck::CRUSH_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(9.843, 113.225)), module, Planck::DEPTH_OUTPUT));
    }
};

Model *modelPlanck = createModel<Planck, PlanckWidget>("Planck");
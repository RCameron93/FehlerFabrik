// Basic preset adder/subtractor
// Ross Cameron 2020/06/05
// Title font - AppleMyunjo
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct Sigma : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        IN_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        MINUS4_OUTPUT,
        MINUS3_OUTPUT,
        MINUS2_OUTPUT,
        MINUS1_OUTPUT,
        ADD1_OUTPUT,
        ADD2_OUTPUT,
        ADD3_OUTPUT,
        ADD4_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Sigma()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(IN_INPUT, "Main");

        std::string offsets[8] = {"-4", "-3", "-2", "-1", "+1", "+2", "+3", "+4"};
        for (int i = 0; i < 8; ++i)
        {
            configOutput(MINUS4_OUTPUT + i, offsets[i] + "v");
            configBypass(IN_INPUT, MINUS4_OUTPUT + i);
        }
    }

    void process(const ProcessArgs &args) override
    {
        int channels = inputs[IN_INPUT].getChannels();

        for (int c = 0; c < channels; ++c)
        {
            float input = inputs[IN_INPUT].getPolyVoltage(c);

            for (int i = 0; i < 4; ++i)
            {
                float outAdd = input + 4 - i;
                float outMinus = input - 4 + i;
                // out = clamp(out, -10.f, 10.f);
                outputs[MINUS4_OUTPUT + i].setVoltage(outMinus, c);
                outputs[ADD4_OUTPUT - i].setVoltage(outAdd, c);
            }
        }

        for (int i = 0; i < NUM_OUTPUTS; ++i)
        {
            outputs[MINUS4_OUTPUT + i].setChannels(channels);
        }
    }
};

struct SigmaWidget : ModuleWidget
{
    SigmaWidget(Sigma *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Sigma.svg")));

        addChild(createWidget<FFHexScrew>(Vec(0, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(10.16, 65.896)), module, Sigma::IN_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 113.225)), module, Sigma::MINUS4_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 101.393)), module, Sigma::MINUS3_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 89.561)), module, Sigma::MINUS2_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 77.728)), module, Sigma::MINUS1_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 54.064)), module, Sigma::ADD1_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 42.231)), module, Sigma::ADD2_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 30.399)), module, Sigma::ADD3_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(10.16, 18.567)), module, Sigma::ADD4_OUTPUT));
    }
};

Model *modelSigma = createModel<Sigma, SigmaWidget>("Sigma");
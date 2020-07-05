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
    }

    void process(const ProcessArgs &args) override
    {
        float input = inputs[IN_INPUT].getVoltage();
        for (int i = 0; i < 4; ++i)
        {
            float out = input + i - 4;
            out = clamp(out, -10.f, 10.f);
            outputs[MINUS4_OUTPUT + i].setVoltage(out);
        }
        for (int i = 0; i < 4; ++i)
        {
            float out = input + i + 1;
            out = clamp(out, -10.f, 10.f);
            outputs[ADD1_OUTPUT + i].setVoltage(out);
        }
    }
};

struct SigmaWidget : ModuleWidget
{
    SigmaWidget(Sigma *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Sigma.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.16, 64.25)), module, Sigma::IN_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 10.875)), module, Sigma::MINUS4_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 24.219)), module, Sigma::MINUS3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 37.562)), module, Sigma::MINUS2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 50.906)), module, Sigma::MINUS1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 77.594)), module, Sigma::ADD1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 90.937)), module, Sigma::ADD2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 104.281)), module, Sigma::ADD3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.16, 117.625)), module, Sigma::ADD4_OUTPUT));
    }
};

Model *modelSigma = createModel<Sigma, SigmaWidget>("Sigma");
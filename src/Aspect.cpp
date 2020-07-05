#include "plugin.hpp"

struct Clocks : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        TRIG_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        D2_OUTPUT,
        D4_OUTPUT,
        D8_OUTPUT,
        D16_OUTPUT,
        D32_OUTPUT,
        D64_OUTPUT,
        S1_OUTPUT,
        S2_OUTPUT,
        S3_OUTPUT,
        S4_OUTPUT,
        S5_OUTPUT,
        S6_OUTPUT,
        S7_OUTPUT,
        S8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        D2_LIGHT,
        D4_LIGHT,
        D8_LIGHT,
        D16_LIGHT,
        D32_LIGHT,
        D64_LIGHT,
        S1_LIGHT,
        S2_LIGHT,
        S3_LIGHT,
        S4_LIGHT,
        S5_LIGHT,
        S6_LIGHT,
        S7_LIGHT,
        S8_LIGHT,

        NUM_LIGHTS
    };

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;

    int divisors[6] = {2, 4, 8, 16, 32, 64};
    int index = 0;

    Clocks()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override
    {
        // Determine index
        if (clockTrigger.process(inputs[TRIG_INPUT].getVoltage()))
        {
            ++index;
        }
        // Reset if rising edge on reset input
        if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            index = 0;
        }

        // Reset index after 64 pulses
        if (index > 63)
        {
            index = 0;
        }

        // Process clock divisors
        for (int i = 0; i < 6; ++i)
        {
            int out = 0;
            if (index % divisors[i] == 0)
            {
                out = 10;
            }
            outputs[D2_OUTPUT + i].setVoltage(out);
            lights[D2_LIGHT + i].setBrightness(out);
        }

        // Process gate sequencer
        int seqIndex = index % 8;
        int seqGates[8] = {0};
        seqGates[seqIndex] = 10;
        for (int i = 0; i < 8; ++i)
        {
            outputs[S1_OUTPUT + i].setVoltage(seqGates[i]);
            lights[S1_LIGHT + i].setBrightness(seqGates[i]);
        }
    }
};

struct ClocksWidget : ModuleWidget
{
    ClocksWidget(Clocks *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/clocks.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.324, 9.143)), module, Clocks::TRIG_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11.324, 25.165)), module, Clocks::RESET_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 41.188)), module, Clocks::D2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 57.21)), module, Clocks::D4_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 73.232)), module, Clocks::D8_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 89.255)), module, Clocks::D16_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 105.277)), module, Clocks::D32_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11.324, 121.299)), module, Clocks::D64_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 9.753)), module, Clocks::S1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 25.775)), module, Clocks::S2_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 41.797)), module, Clocks::S3_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 57.82)), module, Clocks::S4_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 73.842)), module, Clocks::S5_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 89.864)), module, Clocks::S6_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 105.886)), module, Clocks::S7_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.209, 121.909)), module, Clocks::S8_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 41.188)), module, Clocks::D2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 57.21)), module, Clocks::D4_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 73.232)), module, Clocks::D8_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 89.255)), module, Clocks::D16_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 105.277)), module, Clocks::D32_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(3.048, 121.299)), module, Clocks::D64_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 9.55)), module, Clocks::S1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 25.601)), module, Clocks::S2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 41.797)), module, Clocks::S3_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 57.82)), module, Clocks::S4_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 73.842)), module, Clocks::S5_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 89.864)), module, Clocks::S6_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 105.886)), module, Clocks::S7_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(23.933, 121.909)), module, Clocks::S8_LIGHT));
    }
};

Model *modelClocks = createModel<Clocks, ClocksWidget>("clocks");
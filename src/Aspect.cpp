// Clock Divider and sequential sequencer
// Ross Cameron
// Title font - Resistance Regular
// https://velvetyne.fr/fonts/resistance/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct Aspect : Module
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
        ENUMS(DIVISOR1_OUTPUT, 6),
        ENUMS(SEQ1_OUTPUT, 8),
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(DIVISOR1_LIGHT, 6),
        ENUMS(SEQ1_LIGHT, 8),
        NUM_LIGHTS
    };

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;

    int divisors[6] = {2, 4, 8, 16, 32, 64};
    int index = 0;

    Aspect()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configInput(TRIG_INPUT, "Trigger");
        configInput(RESET_INPUT, "Reset");

        for (int i = 0; i < 6; ++i)
        {
            configOutput(DIVISOR1_OUTPUT + i, string::f("%dth's", divisors[i]));
            configLight(DIVISOR1_LIGHT + i, string::f("%dth's", divisors[i]));
        }

        for (int i = 0; i < 8; ++i)
        {
            configOutput(SEQ1_OUTPUT + i, string::f("Step %d", i + 1));
            configLight(SEQ1_LIGHT + i, string::f("Step %d", i + 1));
        }
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
            outputs[DIVISOR1_OUTPUT + i].setVoltage(out);
            lights[DIVISOR1_LIGHT + i].setBrightness(out);
        }

        // Process gate sequencer
        int seqIndex = index % 8;
        int seqGates[8] = {0};
        seqGates[seqIndex] = 10;
        for (int i = 0; i < 8; ++i)
        {
            outputs[SEQ1_OUTPUT + i].setVoltage(seqGates[i]);
            lights[SEQ1_LIGHT + i].setBrightness(seqGates[i]);
        }
    }
};

struct AspectWidget : ModuleWidget
{
    AspectWidget(Aspect *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Aspect.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(18.714, 23.417)), module, Aspect::TRIG_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(18.714, 36.251)), module, Aspect::RESET_INPUT));

        for (int i = 0; i < 6; i++)
        {
            addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(18.714, 49.09 + i * 12.83)), module, Aspect::DIVISOR1_OUTPUT + i));
            addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(25.714, 49.09 + i * 12.83)), module, Aspect::DIVISOR1_LIGHT + i));
        }

        for (int i = 0; i < 8; i++)
        {
            addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(38.771, 23.417 + i * 12.83)), module, Aspect::SEQ1_OUTPUT + i));
            addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(45.771, 23.417 + i * 12.83)), module, Aspect::SEQ1_LIGHT + i));
        }
    }
};

Model *modelAspect = createModel<Aspect, AspectWidget>("Aspect");
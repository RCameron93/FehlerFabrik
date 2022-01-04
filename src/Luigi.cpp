// Digital clock and noise generator
// Ross Cameron
// Title font - GlpyhWorld Meadow
// www.glyphworld.online
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct Luigi : Module
{
    enum ParamIds
    {
        RATE_PARAM,
        AMP_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        RATE_INPUT,
        AMP_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        RANDOM_OUTPUT,
        DUST_OUTPUT,
        NOISE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    dsp::SchmittTrigger clockTrigger;

    float phase = 0.f;
    float noise = 0.f;
    float rndClock = 0.f;
    float dust = 0.f;

    void noiseGen(float amplitude)
    {
        noise = 3.f * amplitude * random::normal();
        noise = clamp(noise, -5.f, 5.f);
        dust = noise;
        rndClock = noise > 0 ? 10 : 0;
        rndClock *= amplitude;
    }

    Luigi()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(RATE_PARAM, 0.f, 12.f, 7.f, "Noise Generator Rate");
        configParam(AMP_PARAM, 0.f, 1.f, 1.f, "Noise Amplitude");

        configInput(CLOCK_INPUT, "External Clock Trigger");
        configInput(RATE_INPUT, "Internal Clock Rate CV");
        configInput(AMP_INPUT, "Output Amplitude CV");

        configOutput(RANDOM_OUTPUT, "Random Clock");
        configOutput(DUST_OUTPUT, "Dust");
        configOutput(NOISE_OUTPUT, "Noise");
    }

    void process(const ProcessArgs &args) override
    {
        float amplitude = params[AMP_PARAM].getValue();
        amplitude += 0.1 * inputs[AMP_INPUT].getVoltage();
        amplitude = clamp(amplitude, -1.f, 1.f);

        dust = 0.f;
        // External Clock
        if (inputs[CLOCK_INPUT].isConnected())
        {
            if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
            {
                noiseGen(amplitude);
            }
        }
        // Internal Clock
        else
        {
            float rate = params[RATE_PARAM].getValue();
            rate += inputs[RATE_INPUT].getVoltage();
            rate = clamp(rate, 0.f, 12.f);
            rate = 5.f * rack::dsp::approxExp2_taylor5(rate);

            phase += rate * args.sampleTime;
            if (phase >= 0.5f)
            {
                phase -= 1.f;
                noiseGen(amplitude);
            }
            else if (phase <= -0.5f)
            {
                phase += 1.f;
            }
        }

        outputs[RANDOM_OUTPUT].setVoltage(rndClock);
        outputs[DUST_OUTPUT].setVoltage(dust);
        outputs[NOISE_OUTPUT].setVoltage(noise);
    }
};

struct LuigiWidget : ModuleWidget
{
    LuigiWidget(Luigi *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Luigi.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(30.789, 54.414)), module, Luigi::RATE_PARAM));
        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(9.851, 54.414)), module, Luigi::AMP_PARAM));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(30.789, 74.757)), module, Luigi::RATE_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(20.32, 31.441)), module, Luigi::CLOCK_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(9.851, 74.757)), module, Luigi::AMP_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(20.32, 100.386)), module, Luigi::RANDOM_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(30.789, 113.225)), module, Luigi::DUST_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(9.851, 113.225)), module, Luigi::NOISE_OUTPUT));
    }
};

Model *modelLuigi = createModel<Luigi, LuigiWidget>("Luigi");
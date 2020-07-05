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
        rndClock = 2.5f * amplitude * (sgn(noise) + 1.f);
    }

    Luigi()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RATE_PARAM, 0.f, 12.f, 12.f, "");
        configParam(AMP_PARAM, 0.f, 1.f, 1.f, "");
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

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(29.095, 17.137)), module, Luigi::RATE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(29.095, 27.137)), module, Luigi::AMP_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.346, 14.846)), module, Luigi::RATE_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.346, 34.846)), module, Luigi::CLOCK_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.346, 44.846)), module, Luigi::AMP_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.791, 55.59)), module, Luigi::RANDOM_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.479, 77.188)), module, Luigi::DUST_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8.479, 87.188)), module, Luigi::NOISE_OUTPUT));
    }
};

Model *modelLuigi = createModel<Luigi, LuigiWidget>("Luigi");
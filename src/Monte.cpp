// Probabalistic Gate Sequencer
// Ross Cameron 2020/05/26

// Title font - ITC Santa Fe
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

struct Monte : Module
{
    enum ParamIds
    {
        INTCLOCK_PARAM,
        STEPS_PARAM,
        PROB1_PARAM,
        PROB2_PARAM,
        PROB3_PARAM,
        PROB4_PARAM,
        PROB5_PARAM,
        PROB6_PARAM,
        PROB7_PARAM,
        PROB8_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        INTCLOCK_INPUT,
        EXTCLOCK_INPUT,
        STEPS_INPUT,
        RESET_INPUT,
        PROB1_INPUT,
        PROB2_INPUT,
        PROB3_INPUT,
        PROB4_INPUT,
        PROB5_INPUT,
        PROB6_INPUT,
        PROB7_INPUT,
        PROB8_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        GATE1_OUTPUT,
        GATE2_OUTPUT,
        GATE3_OUTPUT,
        GATE4_OUTPUT,
        GATE5_OUTPUT,
        GATE6_OUTPUT,
        GATE7_OUTPUT,
        GATE8_OUTPUT,
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        GATE1_LIGHT,
        GATE2_LIGHT,
        GATE3_LIGHT,
        GATE4_LIGHT,
        GATE5_LIGHT,
        GATE6_LIGHT,
        GATE7_LIGHT,
        GATE8_LIGHT,
        MAIN_LIGHT,
        NUM_LIGHTS
    };

    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;

    float phase = 0.f; // Phase of internal LFO
    int index = 0;     // Sequencer index
    int seqPos = 0;    // Which of the 8 literal gate outputs is currently active. if index = 12, seqPos = 12 % 8 = 4 (fifth gate)
    int steps = 8;

    bool gates[8] = {};
    bool out = false;

    Monte()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(INTCLOCK_PARAM, -2.f, 6.f, 2.f, "Clock Rate", "BPM", 2.f, 60.f);
        configParam(STEPS_PARAM, 1.f, 16.f, 8.f, "Sequencer Steps");
        for (int i = 0; i < 8; ++i)
        {
            configParam(PROB1_PARAM + i, 0.f, 1.f, 0.5f, "Step Probability", " %", 0.f, 100.f, 0.f);
        }
    }

    int getSteps()
    {
        int steps = (int)params[STEPS_PARAM].getValue();
        steps += (int)inputs[STEPS_INPUT].getVoltage();
        steps = clamp(steps, 1, 32);
        return steps;
    }

    float getProb(int i)
    {
        float prob = params[PROB1_PARAM + i].getValue();
        prob += 0.1f * inputs[PROB1_INPUT + i].getVoltage();
        prob = clamp(prob, 0.f, 1.f);
        return prob;
    }

    void resetSeq()
    {
        index = -1;
    }

    void advanceIndex()
    {
        // Advance sequence
        ++index;

        // Reset if we've reached the max number of steps
        if (!(index < steps))
        {
            index = 0;
        }

        seqPos = index % 8;

        // Get probability that this step will trigger a gate
        float prob = getProb(seqPos);

        // Set all gates to zero
        for (int i = 0; i < 8; ++i)
        {
            gates[i] = false;
        }

        // Determine if we have a gate this step
        gates[seqPos] = prob > random::uniform();
    }

    float getRate()
    {
        float rate = std::pow(2.f, params[INTCLOCK_PARAM].getValue() + inputs[INTCLOCK_INPUT].getVoltage());
        return rate;
    }

    bool lfoPhase(float rate, float delta)
    {
        // Accumulate phase
        phase += rate * delta;

        // After one cycle advance the sequencer index
        if (phase >= 1.f)
        {
            advanceIndex();
            phase = 0.f;
        }

        // Outputs a squarewave with 50% duty cycle
        bool lfo = phase < 0.5f;
        return lfo;
    }

    void process(const ProcessArgs &args) override
    {
        // Get number of steps
        steps = getSteps();

        // Check for a reset
        if (resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            resetSeq();
        }

        if (inputs[EXTCLOCK_INPUT].isConnected())
        {
            // External Clock
            if (clockTrigger.process(inputs[EXTCLOCK_INPUT].getVoltage()))
            {
                advanceIndex();
            }

            out = clockTrigger.isHigh();
        }
        else
        {
            // Internal Clock (LFO)
            float clockRate = getRate();

            // Accumulate LFO Phase & advance index
            out = lfoPhase(clockRate, args.sampleTime);
        }

        // Output main gate
        out = (gates[seqPos] && out);
        outputs[MAIN_OUTPUT].setVoltage(int(out * 10));
        lights[MAIN_LIGHT].setBrightness(int(out));

        // Output individual gates
        for (int i = 0; i < 8; ++i)
        {
            int gateOut = (int)(gates[i] && out);
            outputs[GATE1_OUTPUT + i].setVoltage(gateOut * 10);
            lights[GATE1_LIGHT + i].setBrightness(gateOut);
        }
    }
};

struct MonteWidget : ModuleWidget
{
    MonteWidget(Monte *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Monte.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(20.215, 49.089)), module, Monte::INTCLOCK_PARAM));
        addParam(createParamCentered<FF08GSnapKnob>(mm2px(Vec(20.215, 87.568)), module, Monte::STEPS_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 23.428)), module, Monte::PROB1_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 36.252)), module, Monte::PROB2_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 49.089)), module, Monte::PROB3_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 61.923)), module, Monte::PROB4_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 74.758)), module, Monte::PROB5_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 87.592)), module, Monte::PROB6_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 100.429)), module, Monte::PROB7_PARAM));
        addParam(createParamCentered<FF08GKnob>(mm2px(Vec(46.624, 113.263)), module, Monte::PROB8_PARAM));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(20.215, 61.928)), module, Monte::INTCLOCK_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(20.215, 23.417)), module, Monte::EXTCLOCK_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(20.215, 100.092)), module, Monte::STEPS_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(20.215, 36.251)), module, Monte::RESET_INPUT));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 23.428)), module, Monte::PROB1_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 36.252)), module, Monte::PROB2_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 49.089)), module, Monte::PROB3_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 61.923)), module, Monte::PROB4_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 74.758)), module, Monte::PROB5_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 87.592)), module, Monte::PROB6_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 100.429)), module, Monte::PROB7_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(34.043, 113.263)), module, Monte::PROB8_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 23.428)), module, Monte::GATE1_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 36.252)), module, Monte::GATE2_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 49.089)), module, Monte::GATE3_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 61.923)), module, Monte::GATE4_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 74.758)), module, Monte::GATE5_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 87.592)), module, Monte::GATE6_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 100.429)), module, Monte::GATE7_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(60.924, 113.263)), module, Monte::GATE8_OUTPUT));
        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(20.214, 113.263)), module, Monte::MAIN_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 23.418)), module, Monte::GATE1_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 36.252)), module, Monte::GATE2_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 49.089)), module, Monte::GATE3_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 61.923)), module, Monte::GATE4_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 74.758)), module, Monte::GATE5_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 87.592)), module, Monte::GATE6_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 100.429)), module, Monte::GATE7_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(67.705, 113.263)), module, Monte::GATE8_LIGHT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.214, 120.263)), module, Monte::MAIN_LIGHT));
    }
};

Model *modelMonte = createModel<Monte, MonteWidget>("Monte");
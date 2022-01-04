// Title Font - Jaapokki
// https://mikkonuuttila.com/jaapokki/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"
#include "PSIOP.hpp"
#include "ffCommon.hpp"
#include "ffFilters.hpp"

struct PSIOP : Module
{
    enum ParamIds
    {
        START_PARAM,
        FINE_PARAM,
        END_PARAM,
        RATIO_PARAM,
        WAVE_PARAM,
        ALGO_PARAM,
        FB_PARAM,
        RATE1_PARAM,
        RATE2_PARAM,
        SPEED_PARAM,
        RATE2ATTEN_PARAM,
        WAVEATTEN_PARAM,
        RATIOATTEN_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        START_INPUT,
        END_INPUT,
        RATIO_INPUT,
        WAVE_INPUT,
        ALGO_INPUT,
        FB_INPUT,
        RATE1_INPUT,
        RATE2_INPUT,
        SPEED_INPUT,
        TRIGGER_INPUT,
        ACCENT_INPUT,
        CHOKE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        OUT_LIGHT,
        NUM_LIGHTS
    };

    Operator operators[4];
    Ramp ramps[3];

    DCBlock dcBlock;
    bool blocking = true;  // DC filter enabled
    bool looping = false;  // Pitch envelope looping disabled
    bool indexMod = false; // Tigger level moduates FM mod index disabled
    bool sync = false;     // Operators re-sync on trigger

    dsp::SchmittTrigger trigger;
    dsp::SchmittTrigger choke;
    dsp::SchmittTrigger accent;

    float startPitch = 0;
    float endPitch = 0;
    float finePitch = 0;
    float rates[3] = {};
    int algo = 0;
    int ratioIndex = 0;
    float feedback = 0;
    int table = 0;
    float index = 0.6f; // Global modulation index
    float level = 1.0f;

    PSIOP()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
     
        configParam(START_PARAM, -4.f, 4.f, 0.f, "Start Freq", "Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
        configParam(FINE_PARAM, -0.2f, 0.2f, 0.f, "Start Fine Freq");
        configParam(END_PARAM, -4.f, 4.f, 0.f, "End Freq", "Hz", dsp::FREQ_SEMITONE, dsp::FREQ_C4);
        configParam(RATIO_PARAM, 0.f, 31.f, 0.f, "FM Ratios", "", 0.f, 1.f, 1.f);
        configParam(WAVE_PARAM, 0.f, 63.f, 0.f, "Wave Combination", "", 0.f, 1.f, 1.f);
        configParam(ALGO_PARAM, 0.f, 5.f, 0.f, "FM Algorithm", "", 0.f, 1.f, 1.f);
        configParam(FB_PARAM, 0.f, 1.f, 0.f, "OP 1 Feedback");
        configParam(RATE1_PARAM, 0.f, 1.f, 0.5f, "Operator 1 & 3 Release Envelope");
        configParam(RATE2_PARAM, 0.f, 1.f, 0.5f, "Operator 2 & 4 Release Envelope");
        configParam(SPEED_PARAM, 0.f, 1.f, 0.f, "Pitch Envelope Speed");
        configParam(RATE2ATTEN_PARAM, -1.f, 1.f, 0.f, "Rate 2 Attenuverter", "%", 0.f, 100.f);
        configParam(WAVEATTEN_PARAM, -1.f, 1.f, 0.f, "Wave Attenuverter", "%", 0.f, 100.f);
        configParam(RATIOATTEN_PARAM, -1.f, 1.f, 0.f, "Ratio Attenuverter", "%", 0.f, 100.f);

        configInput(START_INPUT, "Start Freq CV");
        configInput(END_INPUT, "End Freq CV");
        configInput(RATIO_INPUT, "FM Ratio CV");
        configInput(WAVE_INPUT, "Wave Combination CV");
        configInput(ALGO_INPUT, "FM Algorithm CV");
        configInput(FB_INPUT, "OP 1 Feedback CV");
        configInput(RATE1_INPUT, "Operator 1 & 3 Release Envelope CV");
        configInput(RATE2_INPUT, "Operator 2 & 4 Release Envelope CV");
        configInput(SPEED_INPUT, "Pitch Envelope Speed CV");
        configInput(TRIGGER_INPUT, "Trigger");
        configInput(ACCENT_INPUT, "Accent Trigger");
        configInput(CHOKE_INPUT, "Choke Trigger");

        configOutput(OUT_OUTPUT, "Main");
        configLight(OUT_LIGHT, "Output");
    }

    void process(const ProcessArgs &args) override
    {
        // Look for input on the trigger
        // All parameters are held on trigger input
        if (trigger.process(inputs[TRIGGER_INPUT].getVoltage() / 2.0f))
        {
            if (sync)
            {
                // Reset operators
                for (int i = 0; i < 4; ++i)
                {
                    operators[i].phase = 0.f;
                }
            }

            // Look for accent trigger
            if (accent.process(inputs[ACCENT_INPUT].getVoltage() / 2.0f))
            {
                index = 1.f;
                level = 1.8f;
            }
            else
            {
                index = 0.6f;
                level = 1.f;
            }

            // Modulation index is determined by trigger level
            if (indexMod)
            {
                index *= fabs(inputs[TRIGGER_INPUT].getVoltage() / 10.f);
            }

            // Compute the start and end pitches
            startPitch = params[START_PARAM].getValue();
            startPitch += inputs[START_INPUT].getVoltage();
            finePitch = params[FINE_PARAM].getValue();
            startPitch += finePitch;
            startPitch = clamp(startPitch, -4.f, 4.f);

            endPitch = params[END_PARAM].getValue();
            endPitch += inputs[END_INPUT].getVoltage();
            endPitch = clamp(endPitch, -4.f, 4.f);

            // Get the index for the ratio matrix
            ratioIndex = (int)params[RATIO_PARAM].getValue();
            ratioIndex += (int)round(inputs[RATIO_INPUT].getVoltage() * params[RATIOATTEN_PARAM].getValue());
            ratioIndex = clamp(ratioIndex, 0, 31);

            // Get the wavetable index
            table = (int)params[WAVE_PARAM].getValue();
            table += (int)round(inputs[WAVE_INPUT].getVoltage() * params[WAVEATTEN_PARAM].getValue());
            table = clamp(table, 0, 63);

            // Get the algorithim
            algo = (int)params[ALGO_PARAM].getValue();
            algo += (int)round(inputs[ALGO_INPUT].getVoltage());
            algo = clamp(algo, 0, 5);

            // Get the OP1 feedback amount
            feedback = params[FB_PARAM].getValue();
            feedback += 0.2f * inputs[FB_INPUT].getVoltage();
            feedback = clamp(feedback, 0.f, 1.f);

            // Get the rates for the volume and pitch envelopes
            for (int i = 0; i < 3; i++)
            {
                rates[i] = params[RATE1_PARAM + i].getValue();
                // Special case to factor in rate 2 attenuator
                rates[i] += i == 1 ? 0.2 * params[RATE2ATTEN_PARAM].getValue() * inputs[RATE2_INPUT].getVoltage() : 0.2 * inputs[RATE1_INPUT + i].getVoltage();
                rates[i] = clamp(rates[i], 0.f, 1.f);
            }

            // Trigger
            for (int i = 0; i < 3; i++)
            {
                // Set the gate for the ramps to active
                ramps[i].gate = true;
            }
        }

        // Look for Choke trigger
        if (choke.process(inputs[CHOKE_INPUT].getVoltage() / 2.0f))
        {
            for (int i = 0; i < 3; i++)
            {
                // Set the gate for the ramps to off
                ramps[i].gate = false;
                ramps[i].out = 0.f;
            }
        }

        // Process amplitude ramps
        for (int i = 0; i < 2; i++)
        {
            ramps[i].process(0, 0, rates[i], args.sampleTime, false);
        }

        // Compute current pitch as a function of pitchStart, pitchEnd and the pitch speed envelope
        float pitch = startPitch;
        if (rates[2] > 0.2)
        {
            ramps[2].process(0.3, 0, 1 - rates[2], args.sampleTime, looping);

            // Crossfade from start pitch to end pitch
            float xf = ramps[2].out;
            pitch = crossfade(endPitch, startPitch, xf);
        }

        // Process operators
        float output = 0.f;

        for (int i = 0; i < 4; i++)
        {
            // Set initial pitch for each operator
            operators[i].setPitch(pitch);

            // Actual per operator ratio to be used is taken from the LUT of magic ratios
            float ratio = fm_frequency_ratios[ratioMatrix[ratioIndex][i]];
            operators[i].applyRatio(ratio);

            float fmMod = 0;

            // Determine how much operator i is modulated by other modulators j++
            for (int j = 0; j < 4; j++)
            {
                fmMod += operators[j].out * index * modMatrix[algo][j][i];
            }

            // Accumulate phase, apply FM modulation, apply appropriate amp modulation
            // Feedback is applied for OP1 only
            // Ramp 1 affects OP1 & OP3 VCA, ramp 2 affects OP2 & OP4
            if (i == 0)
            {
                operators[i].process(args.sampleTime, ramps[0].out, fmMod, feedback, tableMatrix[table][i]);
            }
            else if (i == 2)
            {
                operators[i].process(args.sampleTime, ramps[0].out, fmMod, 0, tableMatrix[table][i]);
            }
            else
            {
                operators[i].process(args.sampleTime, ramps[1].out, fmMod, 0, tableMatrix[table][i]);
            }

            // Send to output as dependent on Algorithim
            output += operators[i].out * modMatrix[algo][i][4];
        }

        // Filter DC content from output
        if (blocking)
        {
            output = dcBlock.process(output);
        }

        // Check for NaN
        output = std::isfinite(output) ? output : 0.f;

        // Send output signal to output jack
        outputs[OUT_OUTPUT].setVoltage(output * 3.5 * level);
    }

    void onReset() override
    {
        blocking = true;
        looping = false;
        indexMod = false;
        sync = false;
    }

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "DC Blocking", json_boolean(blocking));
        json_object_set_new(rootJ, "Speed Looping", json_boolean(looping));
        json_object_set_new(rootJ, "FM Index Modulation", json_boolean(indexMod));
        json_object_set_new(rootJ, "Operator Resyncing", json_boolean(sync));

        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *dcJ = json_object_get(rootJ, "DC Blocking");
        if (dcJ)
            blocking = json_boolean_value(dcJ);

        json_t *loopJ = json_object_get(rootJ, "Speed Looping");
        if (loopJ)
            looping = json_boolean_value(loopJ);

        json_t *indexJ = json_object_get(rootJ, "FM Index Modulation");
        if (indexJ)
            indexMod = json_boolean_value(indexJ);

        json_t *syncJ = json_object_get(rootJ, "Operator Resyncing");
        if (syncJ)
            sync = json_boolean_value(syncJ);
    }
};

struct PSIOPWidget : ModuleWidget
{
    PSIOPWidget(PSIOP *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PSIOP.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<FF15GKnob>(mm2px(Vec(12.098, 38.016)), module, PSIOP::START_PARAM));
        addParam(createParamCentered<FF06BKnob>(mm2px(Vec(18.829, 47.995)), module, PSIOP::FINE_PARAM));
        addParam(createParamCentered<FF15GKnob>(mm2px(Vec(79.414, 38.016)), module, PSIOP::END_PARAM));
        addParam(createParamCentered<FF10GSnapKnob>(mm2px(Vec(45.756, 72.726)), module, PSIOP::RATIO_PARAM));
        addParam(createParamCentered<FF10GSnapKnob>(mm2px(Vec(76.049, 72.762)), module, PSIOP::WAVE_PARAM));
        addParam(createParamCentered<FF10GSnapKnob>(mm2px(Vec(55.854, 40.581)), module, PSIOP::ALGO_PARAM));
        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(62.585, 55.501)), module, PSIOP::FB_PARAM));

        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(28.927, 55.505)), module, PSIOP::RATE1_PARAM)); // Release B
        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(15.463, 72.762)), module, PSIOP::RATE2_PARAM)); // Release A
        addParam(createParamCentered<FF10GKnob>(mm2px(Vec(35.636, 40.581)), module, PSIOP::SPEED_PARAM));
        addParam(createParamCentered<FF06GKnob>(mm2px(Vec(18.829, 89.907)), module, PSIOP::RATE2ATTEN_PARAM));
        addParam(createParamCentered<FF06GKnob>(mm2px(Vec(72.683, 89.907)), module, PSIOP::WAVEATTEN_PARAM));
        addParam(createParamCentered<FF06GKnob>(mm2px(Vec(45.756, 89.907)), module, PSIOP::RATIOATTEN_PARAM));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(12.098, 23.418)), module, PSIOP::START_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(79.414, 23.418)), module, PSIOP::END_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(45.756, 100.427)), module, PSIOP::RATIO_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(72.683, 100.427)), module, PSIOP::WAVE_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(55.854, 27.393)), module, PSIOP::ALGO_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(59.219, 100.427)), module, PSIOP::FB_INPUT));

        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(32.031, 100.427)), module, PSIOP::RATE1_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(18.829, 100.427)), module, PSIOP::RATE2_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(35.658, 27.393)), module, PSIOP::SPEED_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(25.527, 113.264)), module, PSIOP::TRIGGER_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(38.99, 113.264)), module, PSIOP::ACCENT_INPUT));
        addInput(createInputCentered<FF01JKPort>(mm2px(Vec(52.454, 113.264)), module, PSIOP::CHOKE_INPUT));

        addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(65.951, 113.264)), module, PSIOP::OUT_OUTPUT));

        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(74, 113.264)), module, PSIOP::OUT_LIGHT));
    }

    void appendContextMenu(Menu *menu) override
    {
        PSIOP *psiop = dynamic_cast<PSIOP *>(module);
        assert(psiop);

        struct PSIOPBlockDCItem : MenuItem
        {
            PSIOP *psiop;

            void onAction(const event::Action &e) override
            {
                psiop->blocking = !psiop->blocking;
            }
            void step() override
            {
                rightText = CHECKMARK(psiop->blocking);
            }
        };

        struct PSIOPSpeedLoopItem : MenuItem
        {
            PSIOP *psiop;

            void onAction(const event::Action &e) override
            {
                psiop->looping = !psiop->looping;
            }
            void step() override
            {
                rightText = CHECKMARK(psiop->looping);
            }
        };

        struct PSIOPIndexModItem : MenuItem
        {
            PSIOP *psiop;

            void onAction(const event::Action &e) override
            {
                psiop->indexMod = !psiop->indexMod;
            }
            void step() override
            {
                rightText = CHECKMARK(psiop->indexMod);
            }
        };

        struct PSIOPSyncItem : MenuItem
        {
            PSIOP *psiop;

            void onAction(const event::Action &e) override
            {
                psiop->sync = !psiop->sync;
            }
            void step() override
            {
                rightText = CHECKMARK(psiop->sync);
            }
        };

        menu->addChild(new MenuEntry);
        PSIOPBlockDCItem *blockDC = createMenuItem<PSIOPBlockDCItem>("DC Filter");
        blockDC->psiop = psiop;
        menu->addChild(blockDC);
        PSIOPSpeedLoopItem *speedLoop = createMenuItem<PSIOPSpeedLoopItem>("Speed Ramp Looping");
        speedLoop->psiop = psiop;
        menu->addChild(speedLoop);
        PSIOPIndexModItem *FMIndexMod = createMenuItem<PSIOPIndexModItem>("Trigger mods index");
        FMIndexMod->psiop = psiop;
        menu->addChild(FMIndexMod);
        PSIOPSyncItem *syncOp = createMenuItem<PSIOPSyncItem>("Operators sync on trigger");
        syncOp->psiop = psiop;
        menu->addChild(syncOp);
    }
};

Model *modelPSIOP = createModel<PSIOP, PSIOPWidget>("PSIOP");
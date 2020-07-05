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

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(24.524, 22.756)), module, Planck::DEPTH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(24.322, 76.816)), module, Planck::RATE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 10.362)), module, Planck::QUANT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.4, 22.756)), module, Planck::DEPTH_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 64.25)), module, Planck::CRUSH_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.706, 76.644)), module, Planck::RATE_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 36.898)), module, Planck::QUANT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.881, 90.785)), module, Planck::CRUSH_OUTPUT));
    }
};

Model *modelPlanck = createModel<Planck, PlanckWidget>("Planck");
#include "plugin.hpp"

struct Slip : Module
{
	enum ParamIds
	{
		THRESH_PARAM,
		WET_PARAM,
		HIGHSHIFTTRIM_PARAM,
		HIGHPINCHTRIM_PARAM,
		HIGHFOLDTRIM_PARAM,
		HIGHSLEWTRIM_PARAM,
		HIGHSHIFT_PARAM,
		HIGHPINCH_PARAM,
		HIGHFOLD_PARAM,
		HIGHSLEW_PARAM,
		LOWSHIFT_PARAM,
		LOWPINCH_PARAM,
		LOWFOLD_PARAM,
		LOWSLEW_PARAM,
		LOWSHIFTTRIM_PARAM,
		LOWPINCHTRIM_PARAM,
		LOWFOLDTRIM_PARAM,
		LOWSLEWTRIM_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		THRESH_INPUT,
		IN_INPUT,
		WET_INPUT,
		HIGHSHIFT_INPUT,
		HIGHPINCH_INPUT,
		HIGHFOLD_INPUT,
		HIGHSLEW_INPUT,
		LOWSHIFT_INPUT,
		LOWPINCH_INPUT,
		LOWFOLD_INPUT,
		LOWSLEW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LOW_OUTPUT,
		OUT_OUTPUT,
		HIGH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	Slip()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WET_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSHIFTTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHPINCHTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHFOLDTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSLEWTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHPINCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHFOLD_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGHSLEW_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWSHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWPINCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWFOLD_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWSLEW_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWSHIFTTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWPINCHTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWFOLDTRIM_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOWSLEWTRIM_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs &args) override
	{
	}
};

struct SlipWidget : ModuleWidget
{
	SlipWidget(Slip *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Slip.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(23.097, 23.404)), module, Slip::THRESH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(78.503, 23.404)), module, Slip::WET_PARAM));

		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 48.282)), module, Slip::HIGHSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 48.282)), module, Slip::HIGHPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 48.282)), module, Slip::HIGHFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 48.282)), module, Slip::HIGHSLEWTRIM_PARAM));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 59.027)), module, Slip::HIGHSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 59.027)), module, Slip::HIGHPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 59.027)), module, Slip::HIGHFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 59.027)), module, Slip::HIGHSLEW_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(20.82, 77.584)), module, Slip::LOWSHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(40.813, 77.584)), module, Slip::LOWPINCH_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(60.807, 77.584)), module, Slip::LOWFOLD_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(80.8, 77.584)), module, Slip::LOWSLEW_PARAM));

		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(17.82, 90.023)), module, Slip::LOWSHIFTTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(38.813, 90.023)), module, Slip::LOWPINCHTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(62.807, 90.023)), module, Slip::LOWFOLDTRIM_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(83.8, 90.023)), module, Slip::LOWSLEWTRIM_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(8.0, 23.417)), module, Slip::THRESH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(50.758, 23.417)), module, Slip::IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(93.6, 23.417)), module, Slip::WET_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 36.251)), module, Slip::HIGHSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 36.251)), module, Slip::HIGHPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 36.251)), module, Slip::HIGHFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 36.251)), module, Slip::HIGHSLEW_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.82, 100.386)), module, Slip::LOWSHIFT_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(36.813, 100.386)), module, Slip::LOWPINCH_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(64.807, 100.386)), module, Slip::LOWFOLD_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(86.8, 100.386)), module, Slip::LOWSLEW_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(26.0, 113.225)), module, Slip::LOW_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(50.8, 113.225)), module, Slip::OUT_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(75.6, 113.225)), module, Slip::HIGH_OUTPUT));
	}
};

Model *modelSlip = createModel<Slip, SlipWidget>("Slip");
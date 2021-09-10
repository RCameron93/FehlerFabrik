#include "plugin.hpp"


struct Lilt : Module {
	enum ParamIds {
		ALPHA_RATE_PARAM,
		BETA_SHIFT_PARAM,
		WIDTH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RATE_IN_INPUT,
		SHIFT_IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ALPHA_OUTPUT,
		BETA_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Lilt() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ALPHA_RATE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BETA_SHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WIDTH_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct LiltWidget : ModuleWidget {
	LiltWidget(Lilt* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lilt.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.528, 112.033)), module, Lilt::ALPHA_RATE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(135.661, 169.091)), module, Lilt::BETA_SHIFT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(96.094, 334.231)), module, Lilt::WIDTH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(56.528, 188.92)), module, Lilt::RATE_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(135.661, 247.424)), module, Lilt::SHIFT_IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(56.528, 427.937)), module, Lilt::ALPHA_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(135.661, 427.937)), module, Lilt::BETA_OUTPUT));
	}
};


Model* modelLilt = createModel<Lilt, LiltWidget>("Lilt");
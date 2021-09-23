#include "plugin.hpp"


struct Botzinger : Module {
	enum ParamIds {
		TIME_1_PARAM,
		CIRCLE1961_PARAM,
		CIRCLE1959_PARAM,
		CIRCLE1957_PARAM,
		CIRCLE1955_PARAM,
		CIRCLE1953_PARAM,
		CIRCLE1951_PARAM,
		CIRCLE1965_PARAM,
		START_PARAM,
		REPEAT_1_PARAM,
		CIRCLE1945_PARAM,
		CIRCLE1943_PARAM,
		CIRCLE1941_PARAM,
		CIRCLE1939_PARAM,
		CIRCLE1937_PARAM,
		CIRCLE1935_PARAM,
		CIRCLE1949_PARAM,
		WIDTH_1_PARAM,
		CIRCLE1929_PARAM,
		CIRCLE1927_PARAM,
		CIRCLE1925_PARAM,
		CIRCLE1923_PARAM,
		CIRCLE1921_PARAM,
		CIRCLE1919_PARAM,
		CIRCLE1933_PARAM,
		DIRECTION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TIME_1_INPUT,
		TIME_2_INPUT,
		CIRCLE1991_INPUT,
		CIRCLE1989_INPUT,
		CIRCLE1987_INPUT,
		CIRCLE1985_INPUT,
		CIRCLE1983_INPUT,
		CIRCLE1997_INPUT,
		CLOCK_INPUT,
		REPEAT_1_INPUT,
		CIRCLE1977_INPUT,
		CIRCLE1975_INPUT,
		CIRCLE1973_INPUT,
		CIRCLE1971_INPUT,
		CIRCLE1969_INPUT,
		CIRCLE1967_INPUT,
		CIRCLE1981_INPUT,
		START_INPUT,
		DIRECTION_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_1_OUTPUT,
		CIRCLE1911_OUTPUT,
		CIRCLE1909_OUTPUT,
		CIRCLE1907_OUTPUT,
		CIRCLE1905_OUTPUT,
		CIRCLE1903_OUTPUT,
		CIRCLE1901_OUTPUT,
		CIRCLE1915_OUTPUT,
		MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Botzinger() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TIME_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1961_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1959_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1957_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1955_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1953_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1951_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1965_PARAM, 0.f, 1.f, 0.f, "");
		configParam(START_PARAM, 0.f, 1.f, 0.f, "");
		configParam(REPEAT_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1945_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1943_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1941_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1939_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1937_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1935_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1949_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WIDTH_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1929_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1927_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1925_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1923_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1921_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1919_PARAM, 0.f, 1.f, 0.f, "");
		configParam(CIRCLE1933_PARAM, 0.f, 1.f, 0.f, "");
		configParam(DIRECTION_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct BotzingerWidget : ModuleWidget {
	BotzingerWidget(Botzinger* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Botzinger.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.462, 50.814)), module, Botzinger::TIME_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(46.456, 50.814)), module, Botzinger::CIRCLE1961_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.449, 50.814)), module, Botzinger::CIRCLE1959_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.443, 50.814)), module, Botzinger::CIRCLE1957_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.437, 50.814)), module, Botzinger::CIRCLE1955_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.431, 50.814)), module, Botzinger::CIRCLE1953_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.424, 50.814)), module, Botzinger::CIRCLE1951_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.418, 50.814)), module, Botzinger::CIRCLE1965_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(167.958, 56.407)), module, Botzinger::START_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.462, 89.104)), module, Botzinger::REPEAT_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(46.456, 89.104)), module, Botzinger::CIRCLE1945_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.449, 89.104)), module, Botzinger::CIRCLE1943_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.443, 89.104)), module, Botzinger::CIRCLE1941_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.437, 89.104)), module, Botzinger::CIRCLE1939_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.431, 89.104)), module, Botzinger::CIRCLE1937_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.424, 89.104)), module, Botzinger::CIRCLE1935_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.418, 89.104)), module, Botzinger::CIRCLE1949_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(31.462, 100.669)), module, Botzinger::WIDTH_1_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(46.456, 100.669)), module, Botzinger::CIRCLE1929_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(61.449, 100.669)), module, Botzinger::CIRCLE1927_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(76.443, 100.669)), module, Botzinger::CIRCLE1925_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(91.437, 100.669)), module, Botzinger::CIRCLE1923_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(106.431, 100.669)), module, Botzinger::CIRCLE1921_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(121.424, 100.669)), module, Botzinger::CIRCLE1919_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(136.418, 100.669)), module, Botzinger::CIRCLE1933_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(476.102, 216.827)), module, Botzinger::DIRECTION_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.462, 24.189)), module, Botzinger::TIME_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.456, 24.189)), module, Botzinger::TIME_2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(61.449, 24.189)), module, Botzinger::CIRCLE1991_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(76.443, 24.189)), module, Botzinger::CIRCLE1989_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.437, 24.189)), module, Botzinger::CIRCLE1987_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(106.431, 24.189)), module, Botzinger::CIRCLE1985_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(121.424, 24.189)), module, Botzinger::CIRCLE1983_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(136.418, 24.189)), module, Botzinger::CIRCLE1997_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(161.638, 24.189)), module, Botzinger::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(31.462, 76.492)), module, Botzinger::REPEAT_1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(46.456, 76.492)), module, Botzinger::CIRCLE1977_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(61.449, 76.492)), module, Botzinger::CIRCLE1975_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(76.443, 76.492)), module, Botzinger::CIRCLE1973_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(91.437, 76.492)), module, Botzinger::CIRCLE1971_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(106.431, 76.492)), module, Botzinger::CIRCLE1969_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(121.424, 76.492)), module, Botzinger::CIRCLE1967_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(136.418, 76.492)), module, Botzinger::CIRCLE1981_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(440.269, 159.894)), module, Botzinger::START_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(440.269, 216.827)), module, Botzinger::DIRECTION_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.462, 113.225)), module, Botzinger::OUT_1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(46.456, 113.225)), module, Botzinger::CIRCLE1911_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.449, 113.225)), module, Botzinger::CIRCLE1909_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(76.443, 113.225)), module, Botzinger::CIRCLE1907_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(91.437, 113.225)), module, Botzinger::CIRCLE1905_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(106.431, 113.225)), module, Botzinger::CIRCLE1903_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(121.424, 113.225)), module, Botzinger::CIRCLE1901_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(136.418, 113.225)), module, Botzinger::CIRCLE1915_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(155.317, 113.225)), module, Botzinger::MAIN_OUTPUT));
	}
};


Model* modelBotzinger = createModel<Botzinger, BotzingerWidget>("Botzinger");
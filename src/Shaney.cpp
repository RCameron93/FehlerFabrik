#include "plugin.hpp"

#include "ffCommon.hpp"

struct Shaney : Module {
	enum ParamId {
		ENUMS(PROB_PARAM, 16),
		ENUMS(STEP_PARAM, 16),
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		RESET_INPUT,
		RUN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(GATE_OUTPUT, 16),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(ELEMENT_LIGHT, 16 * 16),
		LIGHTS_LEN
	};

	Shaney() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct ShaneyWidget : ModuleWidget {
	ShaneyWidget(Shaney* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Shaney.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelShaney = createModel<Shaney, ShaneyWidget>("Shaney");

// Voltage spread generator
// Ross Cameron 2025/12/30

// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"


struct Spread : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(OUT_OUTPUTS, 8),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(OUT_LIGHT, 8),
		LIGHTS_LEN
	};

	Spread() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 8; i++) {
			configOutput(OUT_OUTPUTS + i, string::f("Row %d", i + 1));
		}
		// Add switch for bi/uni polar voltage ie (-5/+5 or 0/10)
	}

	void process(const ProcessArgs& args) override {
		float in = 10.f;
		int channels = 1;

		for (int i = 0; i < 8; i++) {

		}
	}
};


struct SpreadWidget : ModuleWidget {
	SpreadWidget(Spread* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Spread.svg")));

        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        for (int i = 0; i < 8; ++i)
        {
			// Widget coordinates here are all in mm
            float y_delta = 12.83;
			float port_x = 60.924;
			float port_y_base = 23.428;
			float light_x = 67.705;
			float light_y_base = 23.418;

			float port_y = port_y_base + i * y_delta;
			Vec port_coords = Vec(port_x, port_y);
			int port_index = Spread::OUT_OUTPUTS + i;
            addOutput(createOutputCentered<FF01JKPort>(mm2px(port_coords), module, port_index));

			float light_y = light_y_base + i * y_delta;
			Vec light_coords = Vec(light_x, light_y);
			int light_index = Spread::OUT_LIGHT + i;
            addChild(createLightCentered<MediumLight<RedLight>>(mm2px(light_coords), module, light_index));
        }

	}
};


Model* modelSpread = createModel<Spread, SpreadWidget>("Spread");

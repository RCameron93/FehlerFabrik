// Phase-Shifted Clock Pair
// Ross Cameron
// Title font - Velvetyne Basteleur Bold
// https://velvetyne.fr/fonts/basteleur/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

// struct phaseClock
// {
// 	float phase = 0.f;
// 	float rate = 0.f;
// 	float width = 0.5f;

// 	void advance()
// 	{
// 	}
// };

struct Lilt : Module
{
	enum ParamIds
	{
		ALPHA_RATE_PARAM,
		BETA_SHIFT_PARAM,
		WIDTH_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		RATE_IN_INPUT,
		SHIFT_IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		ALPHA_OUTPUT,
		BETA_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	float phase = 0.f;
	float pw = 0.5f;
	float freq = 1.f;

	void setPitch(float pitch)
	{
		pitch = fmin(pitch, 10.f);
		freq = dsp::approxExp2_taylor5(pitch + 30) / 1073741824;
	}
	void setPulseWidth(float pw)
	{
		const float pwMin = 0.01f;
		pw = clamp(pw, pwMin, 1.f - pwMin);
	}
	void osc(float dt)
	{
		float deltaPhase = fmin(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
		{
			phase -= 1.0f;
		}
	}
	float sqr()
	{
		float v = (phase < pw) ? 1.0f : -1.0f;
		return v;
	}

	Lilt()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ALPHA_RATE_PARAM, -8.f, 10.f, 1.f, "");
		configParam(BETA_SHIFT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WIDTH_PARAM, 0.01f, 0.99f, 0.5f, "");
	}

	void process(const ProcessArgs &args) override
	{
		float freqParam = params[ALPHA_RATE_PARAM].getValue();
		float pwParam = params[WIDTH_PARAM].getValue();

		setPitch(freqParam);
		setPulseWidth(pwParam);
		osc(args.sampleTime);

		outputs[ALPHA_OUTPUT].setVoltage(5.f * sqr());
	}
};

struct LiltWidget : ModuleWidget
{
	LiltWidget(Lilt *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lilt.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(56.528, 112.033)), module, Lilt::ALPHA_RATE_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(135.661, 169.091)), module, Lilt::BETA_SHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(96.094, 334.231)), module, Lilt::WIDTH_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(56.528, 188.92)), module, Lilt::RATE_IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(135.661, 247.424)), module, Lilt::SHIFT_IN_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(56.528, 427.937)), module, Lilt::ALPHA_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(135.661, 427.937)), module, Lilt::BETA_OUTPUT));
	}
};

Model *modelLilt = createModel<Lilt, LiltWidget>("Lilt");
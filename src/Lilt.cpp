// Phase-Shifted Clock Pair
// Ross Cameron
// Title font - Velvetyne Basteleur Bold
// https://velvetyne.fr/fonts/basteleur/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

const float amplitude = 5.0f;

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
	float phaseShift = 0.f;

	void setPitch(float pitch)
	{
		pitch = fmin(pitch, 10.f);
		freq = dsp::approxExp2_taylor5(pitch + 20) / 1048576;
	}
	void setPulseWidth(float pw)
	{
		const float pwMin = 0.01f;
		this->pw = clamp(pw, pwMin, 1.f - pwMin);
	}

	void setPhaseShift(float shift)
	{
		phaseShift = 1.f - shift;
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
	float alpha()
	{
		float v = (phase < pw) ? 1.0f : 0.f;
		return v;
	}

	float beta()
	{
		float offset = eucMod(phase + phaseShift, 1.0);
		float v = (offset < pw) ? 1.0f : 0.f;
		return v;
	}

	Lilt()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ALPHA_RATE_PARAM, -8.f, 6.f, 1.f, "Alpha Clock Rate", " BPM", 2.f, 60.f);
		configParam(BETA_SHIFT_PARAM, 0.f, 1.f, 0.5f, "Beta Phase Shift", "˚", 0.f, 360.f);
		configParam(WIDTH_PARAM, 0.01f, 0.99f, 0.25f, "Clock Pulse Width", "%", 0.f, 100.f);
	}

	void process(const ProcessArgs &args) override
	{
		float freqParam = params[ALPHA_RATE_PARAM].getValue();
		float pwParam = params[WIDTH_PARAM].getValue();
		float shiftParam = params[BETA_SHIFT_PARAM].getValue();

		setPitch(freqParam);
		setPulseWidth(pwParam);
		setPhaseShift(shiftParam);
		osc(args.sampleTime);

		outputs[ALPHA_OUTPUT].setVoltage(amplitude * alpha());
		outputs[BETA_OUTPUT].setVoltage(amplitude * beta());
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

		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(14.956, 29.642)), module, Lilt::ALPHA_RATE_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(35.894, 44.739)), module, Lilt::BETA_SHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(25.425, 88.432)), module, Lilt::WIDTH_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.956, 49.985)), module, Lilt::RATE_IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(35.894, 65.464)), module, Lilt::SHIFT_IN_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(14.956, 113.225)), module, Lilt::ALPHA_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(35.894, 113.225)), module, Lilt::BETA_OUTPUT));
	}
};

Model *modelLilt = createModel<Lilt, LiltWidget>("Lilt");
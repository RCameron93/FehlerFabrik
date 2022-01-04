// Phase-Shifted Shuffling Clock Pair
// Ross Cameron
// Title font - Velvetyne Basteleur Bold
// https://velvetyne.fr/fonts/basteleur/
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

const float amplitude = 10.0f;

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
		MAIN_OUTPUT,
		ALPHA_OUTPUT,
		BETA_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	Lilt()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(ALPHA_RATE_PARAM, -2.f, 4.f, 1.f, "Alpha Clock Rate", " BPM", 2.f, 60.f);
		configParam(BETA_SHIFT_PARAM, 0.f, 1.f, 0.5f, "Beta Phase Shift", "˚", 0.f, 360.f);
		configParam(WIDTH_PARAM, 0.01f, 0.99f, 0.25f, "Clock Pulse Width", "%", 0.f, 100.f);

		configInput(RATE_IN_INPUT, "Alpha Rate CV");
		configInput(SHIFT_IN_INPUT, "Beta Shift CV");

		configOutput(MAIN_OUTPUT, "Combined");
		configOutput(ALPHA_OUTPUT, "Alpha");
		configOutput(BETA_OUTPUT, "Beta");
	}

	float phase = 0.f;
	float pw = 0.5f;
	float freq = 1.f;
	float phaseShift = 0.f;

	void setPitch(float pitch)
	{
		freq = dsp::approxExp2_taylor5(pitch + 20) / 1048576;
	}
	void setPulseWidth(float pw)
	{
		this->pw = pw;
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

	void process(const ProcessArgs &args) override
	{
		float freqParam = params[ALPHA_RATE_PARAM].getValue();
		float pwParam = params[WIDTH_PARAM].getValue();
		float shiftParam = params[BETA_SHIFT_PARAM].getValue();

		if (inputs[RATE_IN_INPUT].isConnected())
		{
			float freqCV = inputs[RATE_IN_INPUT].getVoltage();
			freqParam = freqParam + freqCV;
			freqParam = clamp(freqParam, -10.f, 10.f);
		}

		if (inputs[SHIFT_IN_INPUT].isConnected())
		{
			float shiftCV = inputs[SHIFT_IN_INPUT].getVoltage();
			shiftParam = shiftParam + 0.1f * shiftCV;
			shiftParam = clamp(shiftParam, 0.f, 1.f);
		}

		setPitch(freqParam);
		setPulseWidth(pwParam);
		setPhaseShift(shiftParam);
		osc(args.sampleTime);

		float alphaOut = amplitude * alpha();
		float betaOut = amplitude * beta();
		float mainOut = fmax(alphaOut, betaOut);

		outputs[ALPHA_OUTPUT].setVoltage(alphaOut);
		outputs[BETA_OUTPUT].setVoltage(betaOut);
		outputs[MAIN_OUTPUT].setVoltage(mainOut);
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

		addParam(createParamCentered<FF10BKnob>(mm2px(Vec(14.956, 29.642)), module, Lilt::ALPHA_RATE_PARAM));
		addParam(createParamCentered<FF10BKnob>(mm2px(Vec(35.894, 48.903)), module, Lilt::BETA_SHIFT_PARAM));
		addParam(createParamCentered<FF10GKnob>(mm2px(Vec(14.956, 78.918)), module, Lilt::WIDTH_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(14.956, 49.985)), module, Lilt::RATE_IN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(35.894, 69.629)), module, Lilt::SHIFT_IN_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(25.425, 100.386)), module, Lilt::MAIN_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(14.956, 113.225)), module, Lilt::ALPHA_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(35.894, 113.225)), module, Lilt::BETA_OUTPUT));
	}
};

Model *modelLilt = createModel<Lilt, LiltWidget>("Lilt");
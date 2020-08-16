// 3-Band Voltage controlled crossover
// Ross Cameron 2020/08/16
// Title Font - Rumeur Bold
// https://github.com/groupeccc/Rumeur
// Main font - Jost
// https://indestructibletype.com/Jost.html

#include "plugin.hpp"

// Magic numbers for the isolater band frequency ranges
static const float 	FREQ_E2 = 82.40689f;
static const float 	FREQ_E5 = 659.2551f;
static const float 	FREQ_C6 = 1046.502f;
static const float 	FREQ_C9 = 8372.018f;

struct LinkwitzRileyFilter {
	// Second order
	// Pirkle - Designing Audio Effect Plugins in C++
	// Obviously we'll be tidying this one up at some point

	float omega;
	float theta;
	float kappa;
	float delta;

	float xStateLP[3] = {0.f};
	float yStateLP[3] = {0.f};

	float xCoeffLP[3] = {0.f};
	
	// Y coefficients are shared
	float yCoeff[2] = {0.f};

	float xStateHP[3] = {0.f};
	float yStateHP[3] = {0.f};

	float xCoeffHP[3] = {0.f};

	float outLP = 0.f;
	float outHP = 0.f;


	void setCutoff(float fc, float fs)
	{
		// fc = cutoff frequency, fs = sampling frequency
		theta = M_PI * fc / fs;
		omega = M_PI * fc;
		kappa = omega/tan(theta);
		delta = kappa * kappa + omega * omega + 2 * kappa * omega;
	}

	void setLPFCoeffs()
	{
		xCoeffLP[0] = omega * omega / delta;
		xCoeffLP[1] = 2 * omega * omega / delta;
		xCoeffLP[2] = omega * omega / delta;
	}

	void setHPFCoeffs()
	{
		xCoeffHP[0] = kappa * kappa / delta;
		xCoeffHP[1] = -2 * kappa * kappa / delta;
		xCoeffHP[2] = kappa * kappa / delta;
	}

	void setYCoeffs()
	{
		yCoeff[0] = (-2 * kappa * kappa + 2 * omega * omega) / delta;
		yCoeff[1] = (-2 * kappa * omega + kappa * kappa + omega * omega) / delta;
	}


	void process(float input)
	{
		xStateLP[0] = input;
		yStateLP[0] = xCoeffLP[0] * xStateLP[0] + xCoeffLP[1] * xStateLP[1] + xCoeffLP[2] * xStateLP[2] - yCoeff[0] * yStateLP[1] - yCoeff[1] * yStateLP[2];
		
		xStateLP[2] = xStateLP[1];
		xStateLP[1] = xStateLP[0];

		yStateLP[2] = yStateLP[1];
		yStateLP[1] = yStateLP[0];

		outLP = yStateLP[0];

		xStateHP[0] = input;
		yStateHP[0] = xCoeffHP[0] * xStateHP[0] + xCoeffHP[1] * xStateHP[1] + xCoeffHP[2] * xStateHP[2] - yCoeff[0] * yStateHP[1] - yCoeff[1] * yStateHP[2];
		
		xStateHP[2] = xStateHP[1];
		xStateHP[1] = xStateHP[0];

		yStateHP[2] = yStateHP[1];
		yStateHP[1] = yStateHP[0];

		outHP = yStateHP[0];
	}
};

struct Chi : Module
{
	enum ParamIds
	{
		LOW_GAIN_PARAM,
		MID_GAIN_PARAM,
		HIGH_GAIN_PARAM,
		LOW_GAIN_CV_PARAM,
		MID_GAIN_CV_PARAM,
		HIGH_GAIN_CV_PARAM,
		LOW_X_PARAM,
		HIGH_X_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		LOW_GAIN_INPUT,
		MID_GAIN_INPUT,
		HIGH_GAIN_INPUT,
		LOW_X_INPUT,
		HIGH_X_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		LOW_OUTPUT,
		MID_OUTPUT,
		HIGH_OUTPUT,
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		NUM_LIGHTS
	};

	Chi()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LOW_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MID_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGH_GAIN_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOW_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MID_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIGH_GAIN_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LOW_X_PARAM, 0.f, 1.f, 0.5f, "Low/Mid X Freq");
		configParam(HIGH_X_PARAM, 0.f, 1.f, 0.5f, "Mid/High X Freq");
	}
	LinkwitzRileyFilter filter[2];

	float lowFreqScale (float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 80 -> 225 -> 640
		float scaled = 540 * knobValue * knobValue + 20 * knobValue + 80;
		return scaled;
	}

	float highFreqScale (float knobValue)
	{
		// Converts a knob value from 0 -> 0.5 -> 1 to 1k -> 2.8k -> 8k
		float scaled = 6800 * knobValue * knobValue + 200 * knobValue + 1000;
		return scaled;
	}

	void process(const ProcessArgs &args) override
	{
		float in = inputs[IN_INPUT].getVoltage();

		// Get parameters
		// For now we're just taking MP2015 frequency ranges
		// Start with all values [0:1]
		float lowX = params[LOW_X_PARAM].getValue();
		lowX += inputs[LOW_X_INPUT].getVoltage() / 10.f;
		lowX = clamp(lowX, -1.f, 1.f);
		// Convert to Hz
		lowX = lowFreqScale(lowX);

		// Start with all values [0:1]
		float highX = params[HIGH_X_PARAM].getValue();
		highX += inputs[HIGH_X_INPUT].getVoltage() / 10.f;
		highX = clamp(highX, -1.f, 1.f);
		// Convert to Hz
		highX = highFreqScale(highX);

		// Process low/mid Xover
		filter[0].setCutoff(lowX, args.sampleRate);
		filter[0].setHPFCoeffs();
		filter[0].setLPFCoeffs();
		filter[0].setYCoeffs();
		filter[0].process(in);

		float lowOut = filter[0].outLP;
		float midOut = filter[0].outHP;

		// Process mid/high Xover
		filter[1].setCutoff(highX, args.sampleRate);
		filter[1].setHPFCoeffs();
		filter[1].setLPFCoeffs();
		filter[1].setYCoeffs();
		filter[1].process(midOut);
		
		midOut = filter[1].outLP;
		float highOut = filter[1].outHP;

		// Set outputs
		outputs[LOW_OUTPUT].setVoltage(lowOut);
		outputs[MID_OUTPUT].setVoltage(midOut);
		outputs[HIGH_OUTPUT].setVoltage(highOut);
	}
};

struct ChiWidget : ModuleWidget
{
	ChiWidget(Chi *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Chi.svg")));

		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<FFHexScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<FFHexScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(18.473, 47.126)), module, Chi::LOW_GAIN_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(55.88, 47.126)), module, Chi::MID_GAIN_PARAM));
		addParam(createParamCentered<FF20GKnob>(mm2px(Vec(93.289, 47.126)), module, Chi::HIGH_GAIN_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(18.473, 70.063)), module, Chi::LOW_GAIN_CV_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(55.88, 70.063)), module, Chi::MID_GAIN_CV_PARAM));
		addParam(createParamCentered<FF06GKnob>(mm2px(Vec(93.289, 70.063)), module, Chi::HIGH_GAIN_CV_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(37.177, 70.063)), module, Chi::LOW_X_PARAM));
		addParam(createParamCentered<FF15GKnob>(mm2px(Vec(74.584, 70.063)), module, Chi::HIGH_X_PARAM));

		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(18.473, 87.595)), module, Chi::LOW_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(55.88, 87.595)), module, Chi::MID_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(93.289, 87.595)), module, Chi::HIGH_GAIN_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(37.177, 87.595)), module, Chi::LOW_X_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(74.584, 87.595)), module, Chi::HIGH_X_INPUT));
		addInput(createInputCentered<FF01JKPort>(mm2px(Vec(37.177, 113.225)), module, Chi::IN_INPUT));

		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(18.473, 23.417)), module, Chi::LOW_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(55.88, 23.417)), module, Chi::MID_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(93.289, 23.417)), module, Chi::HIGH_OUTPUT));
		addOutput(createOutputCentered<FF01JKPort>(mm2px(Vec(74.584, 113.225)), module, Chi::OUT_OUTPUT));
	}
};

Model *modelChi = createModel<Chi, ChiWidget>("Chi");
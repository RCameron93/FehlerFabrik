#include "wavetables/Wavetables.hpp"
struct FFLogger
{
    int counter = 0;

    // Outputs a message to the log once a second
    void log(const char *message, float samplerate)
    {
        if (counter == (int)samplerate)
        {
            DEBUG("error: %s", message);
            counter = 0;
        }
        counter++;
    }
};

struct BitDepthReducer
{
    // Powers of 2, minus 1
    float powers[16] = {1.f, 3.f, 7.f, 15.f, 31.f, 63.f, 127.f, 255.f, 511.f, 1023.f, 2047.f, 4095.f, 8191.f, 16383.f, 32767.f, 65535.f};

    float process(float in, int depth, float range)
    {
        // Quantises a voltage signal of "range" volts peak to peak (eg 10 volts) to a given bit depth
        float maxVolts = range / 2.f;
        // Clamp incoming signal
        in = clamp(in, -(maxVolts), (maxVolts));
        // Offset input by eg 5v so we're dealing with a number between 0v and 10v
        in += maxVolts;
        // How many possible values we have
        float steps = powers[depth - 1];
        // The step size of each of those values
        float stepSize = range / steps;
        // Quantise
        float out = round(in / stepSize) * stepSize;
        // Remove offset
        out -= maxVolts;
        return out;
    }

    // Process between -1V/+1V at 12 bit
    // Same idea I just wanted to have a streamlined version for a plugin that only uses 12bit reduction
    // Probably not any apreciable decrease in time but hey ho
    float process12bit(float in)
    {
        // in = clamp(in, -1.f, 1.f);
        in += 1.f;
        float stepSize = 0.0004884004884004884f;
        float out = int(in / stepSize) * stepSize;
        out -= 1.0f;
        return out;
    }
};

struct SampleRateCrusher
{
    float out = 0.f;
    int counter = 0;

    void process(int n, float in)
    {
        // Hold every Nth sample
        if (counter < n)
        {
            counter++;
        }
        else
        {
            counter = 0;
            out = in;
        }
    }
};

// Ramp generator based on Befaco Rampage
// https://github.com/VCVRack/Befaco/blob/v1/src/Rampage.cpp
struct Ramp
{
    float minTime = 1e-3;
    float shape = 0.0;
    float out = 0.f;
    bool gate = false;

    float shapeDelta(float delta, float tau, float shape)
    {
        float lin = sgn(delta) * 10.f / tau;
        if (shape < 0.f)
        {
            float log = sgn(delta) * 40.f / tau / (fabs(delta) + 1.f);
            return crossfade(lin, log, -shape * 0.95f);
        }
        else
        {
            float exp = M_E * delta / tau;
            return crossfade(lin, exp, shape * 0.90f);
        }
    }

    dsp::PulseGenerator endOfCyclePulse;

    void process(float shape, float riseRate, float fallRate, float time, bool cycle)
    {
        float in = 0;
        if (gate)
        {
            in = 1.f;
        }

        float delta = in - out;

        bool rising = false;
        bool falling = false;

        if (delta > 0)
        {
            // Rise removed for now, just decay
            // Rise
            float riseCv = riseRate;
            float rise = minTime * pow(2.0, riseCv * 20.0);
            out += shapeDelta(delta, rise, shape) * time;
            rising = (in - out > 1e-3);
            if (!rising)
            {
                gate = false;
            }
        }
        else if (delta < 0)
        {
            // Fall
            // Just control knob for now, will add CV control later
            float fallCv = fallRate;
            fallCv = clamp(fallCv, 0.0f, 1.0f);
            float fall = minTime * pow(2.0f, fallCv * 20.0f);
            out += shapeDelta(delta, fall, shape) * time;
            falling = (in - out < -1e-3);
            if (!falling)
            {
                // End of cycle, check if we should turn the gate back on (cycle mode)
                endOfCyclePulse.trigger(1e-3);
                if (cycle)
                {
                    gate = true;
                }
            }
        }
        else
        {
            gate = false;
        }

        if (!rising && !falling)
        {
            out = in;
        }
    }
};

// Wavetable operator for FM synthesis
struct Operator
{
    float phase = 0.f;
    float freq = 0.f;
    float wave = 0.f;
    float out = 0.f;
    float bufferSample1 = 0.f;
    float bufferSample2 = 0.f;
    float feedbackSample = 0.f;

    void setPitch(float pitch)
    {
        // The default pitch is C4 = 256.6256f
        freq = dsp::FREQ_C4 * pow(2.f, pitch);
    }

    void applyRatio(float ratio)
    {
        freq *= ratio;
    }

    void process(float time, float amplitude, float fmMod, float feedback, int table)
    {
        phase += freq * time + fmMod * 0.5f;
        if (phase >= 0.5f)
        {
            phase -= 1.f;
        }
        else if (phase <= -0.5f)
        {
            phase += 1.f;
        }

        float wtPOS = (phase + feedback * feedbackSample);
        // Wrap wavetable position between 0.f and 1.f
        wtPOS = eucMod(wtPOS, 1.f);

        float *waveTable = wavetable_opal[table];
        float tableLength = wavetable_opal_lengths[table];

        wtPOS *= (tableLength);
        wave = interpolateLinear(waveTable, wtPOS);

        out = wave * amplitude;

        bufferSample1 = wave;
        bufferSample2 = bufferSample1;
        feedbackSample = (bufferSample1 + bufferSample2) / 2.f;
    }
};

struct Sequencer
{
	bool running = false;
	// 0 = fwd, 1 = rev, 2 = bounce, 3 = rnd
	int direction = 0;
	// Just used in bounce mode
	int bounceDir = 0;

    int length = 8; // Amount of steps
	int index = 0; // Sequencer index

	void reset()
	{
		index = 0;
	}

    void setLength(int newLength)
    {
        if (newLength > 0)
        {
            length = newLength;
            
            if (index > (length - 1))
            {
                reset();
            }
        }
    }

	void setIndex(int step)
	{
		if (step < length && step > -1)
		{
			index = step;
		}
	}

	void startStop()
	{
		running = !running;
	}

	void directionChange()
	{
		// Cycle through direction modes
		++direction;
		direction %= 4;
	}

	void advanceIndex()
	{
		switch (direction)
		{
		case 0:
			// Forward
			++index;
			index %= length;
			break;

		case 1:
			// Reverse
			--index;
			index = (index % length + length) % length;
			break;

		case 2:
			// Bouncing (plays each end twice so as to be a factor of four)
			if (bounceDir)
			{
				++index;
				if (index == length)
				{
					bounceDir = !bounceDir;
					index = length - 1;
				}
			}
			else
			{
				--index;
				if (index == -1)
				{
					bounceDir = !bounceDir;
					index = 0;
				}
			}

			break;

		case 3:
			// Random
			{
				float rng = (length - 1) * random::uniform();
				index = (int)round(rng);
			}
			break;

		default:
			break;
		}
	}
};
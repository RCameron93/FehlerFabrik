// #include "plugin.hpp"
// #include "wavetables/Wavetables.hpp"

// struct BitDepthReducer
// {
//     // Powers of 2, minus 1
//     float powers[16] = {1.f, 3.f, 7.f, 15.f, 31.f, 63.f, 127.f, 255.f, 511.f, 1023.f, 2047.f, 4095.f, 8191.f, 16383.f, 32767.f, 65535.f};

//     float process(float in, int depth, float range)
//     {
//         // Quantises a voltage signal of "range" volts peak to peak (eg 10 volts) to a given bit depth
//         float maxVolts = range / 2.f;
//         // Clamp incoming signal
//         in = clamp(in, -(maxVolts), (maxVolts));
//         // Offset input by eg 5v so we're dealing with a number between 0v and 10v
//         in += maxVolts;
//         // How many possible values we have
//         float steps = powers[depth - 1];
//         // The step size of each of those values
//         float stepSize = range / steps;
//         // Quantise
//         float out = round(in / stepSize) * stepSize;
//         // Remove offset
//         out -= maxVolts;
//         return out;
//     }

//     // Process between -1V/+1V at 12 bit
//     // Same idea I just wanted to have a streamlined version for a plugin that only uses 12bit reduction
//     // Probably not any apreciable decrease in time but hey ho
//     float process12bit(float in)
//     {
//         // in = clamp(in, -1.f, 1.f);
//         in += 1.f;
//         float stepSize = 0.0004884004884004884f;
//         float out = int(in / stepSize) * stepSize;
//         out -= 1.0f;
//         return out;
//     }
// };

// struct SampleRateCrusher
// {
//     float out = 0.f;
//     int counter = 0;

//     void process(int n, float in)
//     {
//         // Hold every Nth sample
//         if (counter < n)
//         {
//             counter++;
//         }
//         else
//         {
//             counter = 0;
//             out = in;
//         }
//     }
// };

// Wavetable operator for FM synthesis
// struct Operator
// {
//     float phase = 0.f;
//     float freq = 0.f;
//     float wave = 0.f;
//     float out = 0.f;
//     float bufferSample1 = 0.f;
//     float bufferSample2 = 0.f;
//     float feedbackSample = 0.f;

//     void setPitch(float pitch)
//     {
//         // The default pitch is C4 = 256.6256f
//         freq = dsp::FREQ_C4 * pow(2.f, pitch);
//     }

//     void applyRatio(float ratio)
//     {
//         freq *= ratio;
//     }

//     void process(float time, float amplitude, float fmMod, float feedback, int table)
//     {
//         phase += freq * time + fmMod * 0.5f;
//         if (phase >= 0.5f)
//         {
//             phase -= 1.f;
//         }
//         else if (phase <= -0.5f)
//         {
//             phase += 1.f;
//         }

//         float wtPOS = (phase + feedback * feedbackSample);
//         // Wrap wavetable position between 0.f and 1.f
//         wtPOS = eucMod(wtPOS, 1.f);

//         float *waveTable = wavetable_opal[table];
//         float tableLength = wavetable_opal_lengths[table];

//         wtPOS *= (tableLength);
//         wave = interpolateLinear(waveTable, wtPOS);

//         out = wave * amplitude;

//         bufferSample1 = wave;
//         bufferSample2 = bufferSample1;
//         feedbackSample = (bufferSample1 + bufferSample2) / 2.f;
//     }
// };
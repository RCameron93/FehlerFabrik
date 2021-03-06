# Fehler Fabrik

![Image of Fehler Fabrik Modules](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FehlerFabrikModules.png)

Plugin modules for [VCV Rack](http://vcvrack.com)

These are all still a work in progress!

## Arpanet

![Image of Arpanet](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFArpanet.png)

Arpanet is an attempt to recreate the ARP Instruments 1601 step sequencer. Reading the [manual for the original](https://manuals.fdiskc.com/flat/ARP%20Sequencer%201601%20Owners%20Manual.pdf) is the best way to learn how to use Arpanet.

## Aspect

![Image of Aspect](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFAspect.png)

Aspect is a basic clock divider and sequential gate sequencer. The left set of outputs are divisions of the input clock, and the right set are the sequence output. Gate highs are 10V, lows are 0v.

## Chi

![Image of Chi](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFChi.png)

Chi is a three band polyphonic crossover, like those found in high-end DJ mixers and PA/HiFi system controllers. It uses 4th order Linkwitz-Riley filters to ensure flat and coherent recombination of audio bands. Two frequency cutoff controls determine where the low/mid (80Hz - 640Hz) and mid/high (1kHz - 8kHz) filter bands meet. Each band has it's own output with (voltage controlled) gain control (-inf dB through +6 dB), which can be used as feeds for multi-band processing, and a master output which recombines all bands - like a DJ mixers Isolator section.

While Chi can work polyphonically, it's currently **not** very well optimised - I still need to figure out how to utilise SIMD for this kind of thing!

## Fax

![Image of Fax](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFFax.png)

Fax is a sampling sequencer that will produce a crude facsimile  of a recorded input. If no input is present it will record the movement of it's big central knob. The transport controls are similar to Arpanet's. 

When recording is active, the input is sampled whenever the sequencer moves to the next step (which may be caused by the internal clock or a trigger present on the step input/button). Whether the sampled voltage is stored in the step the sequencer just left, or the one it is moving to, is determined by the Pre/Post switch. If the Auto Stop switch is on (up position), then recording will automatically stop when the maximum number of steps is reached. When recording has finished, the sampled voltages are played back in order by the sequencer on the output jack. 

Fax was created with physical controllers in mind. Try mapping it's big knob to a MIDI controller, or triggering the recording of a keyboards CV output using it's gate signal.

## Luigi

![Image of Luigi](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFLuigi.png)

Luigi is a random digital clock and noise generator. It can use either an external or internal clock.

## Monte

![Image of Monte](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFMonte.png)

Monte is a probabilistic trigger sequencer. It can use either an external or internal clock. For each step of the sequence, the probability of a trigger being generated is determined by a CV/knob combo. When CV is present the knobs act as an offset. All of the gates are OR'd to the main output as well as having their own output. Using CV on the Steps input allows for sequences of up to 32 steps.

## Planck

![Image of Planck](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFPlanck.png)

Planck is a decimator and bit depth reducer. The output of the depth reducer is normalled to the input of the decimator.


## PSI OP

![Image of PSI OP](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFPSIOP.png)

PSI OP is a 4 operator FM percussion voice. It's *heavily* based on a popular Eurorack hardware drum module, so if you can find a manual for such a module, that'll explain the functionality until I can write some proper documentation!

By default, PSI OP has a DC offset filter on it's output. This can be toggled via the context menu. The looping behaviour of the Speed envelope can also be toggled in the menu.

The opal wavetable used in PSI OP is taken from [ValleyRack](https://github.com/ValleyAudio/ValleyRackFree/tree/v1.0/src/Common/Wavetables)

## Rasoir

![Image of Rasoir](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFRasoir.png)

Rasoir is an asymmetrical voltage processor. It's based on the the types of distortion found in modules like [Autodafe's FoldBack](https://github.com/antoniograzioli/Autodafe/blob/master/src/FoldBack.cpp) and [HetrickCV's Waveshape](https://github.com/mhetrick/hetrickcv/blob/master/src/Waveshape.cpp). What makes Rasoir unique is it's ability to slice a waveform into two components that lie above or below a threshold voltage and process those components seperately. Both high and low components can be shifted in time, clipped, pinched, folded and slewed. The top row of controls and jacks affects the high component, and the lower row the low component. Each component has it's own output, and they're combined on the main output. The main output also has dry/wet control and a DC offset filter.

See [here](https://www.youtube.com/watch?v=nh-8XyOFzqo&feature=youtu.be) for a demo

## Sigma

![Image of Sigma](https://github.com/RCameron93/FehlerFabrik/blob/master/docs/images/FFSigma.png)

Sigma is a basic preset voltage adder. Sometimes you just want to add 1V to something!
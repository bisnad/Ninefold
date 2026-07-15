(
s.options.memSize = 2**16;
s.options.sampleRate = 48000;
s.options.blockSize = 512;
Server.killAll;
OSCdef.freeAll;
s.waitForBoot({
	~numAgents = 16; // max number of agents

	// --------------------------------------------------------------

	SynthDef(\network, {
		|
		t_trig
		gains = #[1,1,1]
		active = #[1,1,1,1,1,1,1,1,1] // not freezed
		pos = #[0,0,0,0,0,0,0,0,0]
		vel = #[0,0,0,0,0,0,0,0,0]
		|
		var base = SinOsc.ar(0.001 * active).exprange(0.3,1.5);
		var delTimes = Array.fill(9, { |i|
			var n = [
				0,3,9,
				1,5,12,
				2,7,15
			].at(i);
			pos[i].lag(0.01).exprange(0.0001 + (n * 0.005), n + 1 * 0.005);
		}) * base;
		var volumes = (vel.abs.clip(0.2, 0.8) * 3).lag(0.1);

		var ex = WhiteNoise.ar() * 0.01;

		var input = LocalIn.ar(3) + ex;
		var output;

		var numsamp = 4410;
		var rms = RunningSum.rms(input, numsamp).lag(0.01);
		var factor = exp(-4 * rms);

		var del0 = DelayL.ar(input[0], 0.1, [delTimes[0],delTimes[1],delTimes[2]]) * [volumes[0],volumes[1],volumes[2]];
		var del1 = DelayL.ar(input[1], 0.1, [delTimes[3],delTimes[4],delTimes[5]]) * [volumes[3],volumes[4],volumes[5]];
		var del2 = DelayL.ar(input[2], 0.1, [delTimes[6],delTimes[7],delTimes[8]]) * [volumes[6],volumes[4],volumes[7]];

		var reson = SinOsc.ar(0.0011) * 0.1;
		var sig0 = MidEQ.ar(Mix(del0), 100 ** (2.0 + reson), db: 2);
		var sig1 = MidEQ.ar(Mix(del1), 100 ** (1.8 + reson), db: 2);
		var sig2 = MidEQ.ar(Mix(del2), 100 ** (1.6 + reson), db: 2);

		LocalOut.ar(LeakDC.ar([sig0, sig1, sig2]).tanh * factor);

		output = Splay.ar((del0 * gains[0].lag(1))++(del1 * gains[1].lag(1)) ++ (del2 * gains[2].lag(1))) * 0.05;

		Out.ar(0, output);

	}).add;

	SynthDef(\output, {
		|
		mainVolume = 1
		|
		var sig = In.ar(0, 2) * mainVolume.lag2(5,8);
		sig = FreeVerb2.ar(sig[0], sig[1], 0.2);
		ReplaceOut.ar(0, sig)
	}).add;

	s.sync;

	~synths = Array.fill(~numAgents, { Synth(\network) });
	~outputSynth = Synth(\output, addAction: \addToTail);
	~velocitySums = Array.fill(~numAgents, []);

	OSCdef(\posReceiver, {
		| msg |
		var num = (msg.size - 1 / 9).asInteger;
		num.do{ |i|
			~synths[i].set(\pos, msg.at(((i * 9 + 1)..(i * 9 + 9))).clip(-4.9, 4.9) / 5)
		}
	}, "/swarm/0/15/position", recvPort: 9005);
	OSCdef(\velReceiver, {
		| msg |
		~numAgents.do{ |i|
			var vels = msg.at(((i * 9 + 1)..(i * 9 + 9)));
			var velSums = [vels[[0,1,2]].abs.sum, vels[[3,4,5]].abs.sum, vels[[6,7,8]].abs.sum];
			var gains = [1,1,1]; // max
			var active = [1,1,1,1,1,1,1,1,1]; // not freezed
			3.do{ |j|
				if(velSums[j] == ~velocitySums[i][j])
				{ gains[j] = 0.5; active[j*3] = 0; active[j*3+1] = 0; active[j*3+2] = 0 }
			};
			~velocitySums[i] = velSums;
			~synths[i].set(\vel, vels, \gain, gains, \active, active);
		}
	}, "/swarm/0/15/velocity", recvPort: 9005);
	OSCdef(\stateReceiver, {
		| msg |
		~outputSynth.set(\mainVolume, msg[1])
	}, "/synth1", recvPort: 9005);
})
)
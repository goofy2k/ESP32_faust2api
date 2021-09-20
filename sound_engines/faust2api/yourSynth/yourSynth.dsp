//polyphony example from https://ccrma.stanford.edu/~rmichon/faust2api/#customizing-the-api
import("stdfaust.lib");
freq = nentry("freq",200,40,2000,0.01) : si.polySmooth(gate,0.999,2);
gain = nentry("gain",1,0,1,0.01) : si.polySmooth(gate,0.999,2);
gate = button("gate") : si.smoo;
cutoff = nentry("cutoff",500,40,2000,0.01) : si.polySmooth(gate,0.999,2);
process = vgroup("synth",os.sawtooth(freq)*gain*gate : fi.lowpass(3,cutoff) <: _,_);
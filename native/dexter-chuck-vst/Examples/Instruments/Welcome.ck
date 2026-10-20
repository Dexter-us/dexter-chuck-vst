// A free-running four-note sequence makes the instrument audible without MIDI.
SinOsc osc => ADSR envelope => dac;
0.18 => osc.gain;
envelope.set(10::ms, 60::ms, 0.35, 100::ms);
[220.0, 277.18, 329.63, 440.0] @=> float notes[];

while (true)
{
    for (0 => int i; i < notes.size(); i++)
    {
        notes[i] => osc.freq;
        envelope.keyOn();
        220::ms => now;
        envelope.keyOff();
        80::ms => now;
    }
}

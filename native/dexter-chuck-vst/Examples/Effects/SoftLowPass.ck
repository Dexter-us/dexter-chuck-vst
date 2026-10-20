// A separate filter per channel keeps the stereo image intact.
adc.chan(0) => LPF leftFilter => dac.chan(0);
adc.chan(1) => LPF rightFilter => dac.chan(1);
900 => leftFilter.freq;
900 => rightFilter.freq;
0.8 => leftFilter.Q;
0.8 => rightFilter.Q;

while (true)
{
    1::second => now;
}

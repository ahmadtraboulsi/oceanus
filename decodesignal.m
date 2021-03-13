function [result, cycles, output, gamma, RMS, SixHzSNNR]  = decodesignal(x, mode, max_cycles)
%n=45000; %number of samples
n=length(x); %number of samples in x
fs= 375; %sampling frequency
spsymbol = [256, 512, 1024, 2048, 2560];
interval = [2 4 8 16 20];
I=real(x');
Q=imag(x');
K=32;
[result, cycles, output, gamma, RMS, SixHzSNNR]  = framesearch(...
    I, Q, n,fs, 1, 0, 2, 16.25, spsymbol(mode), interval(mode), max_cycles);
end
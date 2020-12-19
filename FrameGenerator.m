% Generate frames, coded in ".wav" files 
% Author: Michel Barbeau, Carleton University
% Version: August 20, 2019
%
clear;
%
%%% PARAMETERS
%
% 1. Input file name (".c2" file format, complex envelope @ 375 samples/sec)
iFile = '150613_1920';
fprintf('Input file name: %s.c2\n', iFile)
% 2. Carrier frequency
f = 209; % Hz
fprintf('Carrier frequency: %d Hz\n', f)
% 3. Time offset (random quiet time prefix, in seconds, in [0,9])
toffset = 0; % second
fprintf('Time offset: %d sec.\n', toffset)
% 4. Output file sampling frequency
oSps = 4500; % samples/sec (a multiple of 375, e.g. 6K, 12K, 24K, 48K, 96K)
fprintf('Output file sampling frequency: %d sps\n', oSps);
% 5. Output file name (broadband signal in ".wav" format)
oFile = int2str(f)+"_"+int2str(oSps)+".wav"; % use frequency number and sampling frequency
fprintf('Output file name: %s\n', oFile);
% 6. Bits per sample
BitsPerSample = 16;
%
%%% CONVERSION
%
c2toWave(iFile, oFile, f, toffset, oSps, BitsPerSample);



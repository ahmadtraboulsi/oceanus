%reading an audio file and down converting the signal
function [xDown,Fs] = readdowncnvaudio(file_name, center_frequency)
[x,Fs] = audioread(file_name);
%might need to change that in the future for now keep only the first 2 minutes
x = x(1:120*Fs);  
%this down converter assumes the sampling frequency to be 12000
dwnConv = dsp.DigitalDownConverter(...
  'DecimationFactor',32,... % yields 45000 samples (375 sps * 120 s)
  'SampleRate', Fs,...
  'Bandwidth', 187,...
  'StopbandAttenuation', 55,...
  'PassbandRipple',0.2,...
  'CenterFrequency',center_frequency); % frequency translation offset
xDown=dwnConv(x);
end





